/**
 * ProjectorRays Shockwave Decompiler
 * Copyright (C) 2017 Anthony Kleine, Debby Servilla
 * Copyright (C) 2020-2023 Debby Servilla
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>

#include <boost/format.hpp>
#include <mpg123.h>

#include "common/log.h"
#include "common/stream.h"
#include "director/sound.h"

namespace Director {

ssize_t ReadStream_read(void *stream, void *buf, size_t count) {
	return ((Common::ReadStream *)stream)->readUpToBytes(count, (uint8_t *)buf);
}

off_t ReadStream_lseek(void *stream, off_t offset, int whence) {
	return ((Common::ReadStream *)stream)->lseek(offset, whence);
}

#define CHECK_ERR(name) \
	if (err != MPG123_OK && err != MPG123_DONE) { \
		Common::warning(boost::format(name": %s") % mpg123_plain_strerror(err)); \
		mpg123_close(mh); \
		mpg123_delete(mh); \
		return false; \
	}

size_t samplesToBytes(size_t samples, int channels, int sampleSize) {
	size_t bytes = samples;
	if (channels == 2) {
		bytes *= 2;
	}
	if (sampleSize == 16) {
		bytes *= 2;
	}
	return bytes;
}

bool decodeMP3(
	Common::ReadStream &in,
	Common::WriteStream &out,
	int hdrSampleRate,
	int hdrChannels,
	int hdrSampleSize,
	size_t hdrSkipSamples,
	int32_t chunkID
) {
	size_t bytesToRead = out.size() - out.pos();
	Common::debug(boost::format("Chunk %d: Decoding %zu bytes of MP3 data (rate: %d channels: %d bitdepth: %d)")
					% chunkID % bytesToRead % hdrSampleRate % hdrChannels % hdrSampleSize);

	int err;
	mpg123_handle *mh;

	// initialize an mpg123 handle
	if ((mh = mpg123_new(NULL, &err)) == NULL) {
		Common::warning(boost::format("mpg123_new: %s") % mpg123_plain_strerror(err));
		return false;
	}

	// clear its supported formats
	err = mpg123_format_none(mh);
	CHECK_ERR("mpg123_format_none");

	// set the format specified by the header
	int expectedEncoding = (hdrSampleSize == 8) ? MPG123_ENC_UNSIGNED_8 : MPG123_ENC_SIGNED_16;
	err = mpg123_format(
		mh, hdrSampleRate,
		(hdrChannels == 1) ? MPG123_MONO : MPG123_STEREO,
		expectedEncoding
	);
	CHECK_ERR("mpg123_format");

	// set other format restrictions
	int flags = MPG123_FORCE_ENDIAN | MPG123_BIG_ENDIAN; // big endian output
	flags |= MPG123_NO_FRANKENSTEIN; // don't allow change of format
	mpg123_param(mh, MPG123_FLAGS, flags, 0.0);
	CHECK_ERR("mpg123_param");

	// set mpg123 to use our ReadStream functions
	err = mpg123_replace_reader_handle(mh, ReadStream_read, ReadStream_lseek, NULL);
	CHECK_ERR("mpg123_replace_reader_handle");

	// now begin reading from the stream
	err = mpg123_open_handle(mh, &in);
	CHECK_ERR("mpg123_open_handle");

	long outputSampleRate;
	int outputChannels, outputEncoding;
	err = mpg123_getformat(mh, &outputSampleRate, &outputChannels, &outputEncoding);
	CHECK_ERR("mpg123_getformat");

	if (outputSampleRate != hdrSampleRate) {
		Common::warning(boost::format("Output sample rate (%ld) doesn't match header sample rate (%d)!")
						% outputSampleRate % hdrSampleRate);
		return false;
	}
	if (outputChannels != hdrChannels) {
		Common::warning(boost::format("Output channels (%d) doesn't match header channels (%d)!")
						% outputChannels % hdrChannels);
		return false;
	}
	if (outputEncoding != expectedEncoding) {
		Common::warning(boost::format("Output encoding (%d) doesn't match header sample size (%d)!")
						% outputEncoding % hdrSampleSize);
		return false;
	}

	size_t done;
	size_t bytesToSkip = samplesToBytes(hdrSkipSamples, hdrChannels, hdrSampleSize);
	std::vector<uint8_t> garbage(bytesToSkip);
	while (bytesToSkip && err != MPG123_DONE) {
		err = mpg123_read(mh, garbage.data(), garbage.size(), &done);
		CHECK_ERR("mpg123_read");
		bytesToSkip -= done;
	}

	while (bytesToRead && err != MPG123_DONE) {
		err = mpg123_read(mh, &out.data()[out.pos()], bytesToRead, &done);
		out.skip(done);
		CHECK_ERR("mpg123_read");
		bytesToRead -= done;
	}

	mpg123_close(mh);
	mpg123_delete(mh);

	return true;
}

ssize_t decompressSnd(Common::ReadStream &in, Common::WriteStream &out, int32_t chunkID) {
	if (in.size() == 0)
		return 0;

	in.endianness = Common::kBigEndian;

	// 'snd ' header
	// https://developer.apple.com/library/archive/documentation/mac/Sound/Sound-60.html

	uint16_t format = in.readUint16();
	if (format == 1) {
		// Format 1
		uint16_t dataFormatCount = in.readUint16();
		in.skip(6 * dataFormatCount);
	} else {
		// Format 2
		/* uint16_t referenceCount = */ in.readUint16();
	}
	uint16_t soundCommandCount = in.readUint16();
	in.skip(8 * soundCommandCount);

	// sound header record
	// https://developer.apple.com/library/archive/documentation/mac/Sound/Sound-74.html
	// https://developer.apple.com/library/archive/documentation/mac/Sound/Sound-75.html

	/* uint32_t samplePtr = */ in.readUint32();
	uint32_t encodeDependent = in.readUint32();
	uint16_t sampleRate = in.readUint16();
	/* uint16_t sampleRateFrac = */ in.readUint16();
	/* uint32_t loopStart = */ in.readUint32();
	/* uint32_t loopEnd = */ in.readUint32();
	uint8_t encode = in.readUint8();
	/* uint8_t baseFrequency = */ in.readUint8();

	uint32_t numChannels;
	uint16_t sampleSize;

	if (encode == 0x00) {
		// Standard header
		/* uint32_t numSamples = encodeDependent; */
		numChannels = 1;
		sampleSize = 8;
	} else if (encode == 0xFF) {
		// Extended header
		numChannels = encodeDependent;
		/* uint32_t numSamples = */ in.readUint32();
		/* Extended80 AIFFSampleRate = */ in.skip(10);
		/* uint32_t markerChunk = */ in.readUint32();
		/* uint32_t instrumentChunks = */ in.readUint32();
		/* uint32_t AESRecording = */ in.readUint32();
		sampleSize = in.readUint16();
		/* uint16_t futureUse1 = */ in.readUint16();
		/* uint32_t futureUse2 = */ in.readUint32();
		/* uint32_t futureUse3 = */ in.readUint32();
		/* uint32_t futureUse4 = */ in.readUint32();
	} else {
		Common::warning(boost::format("Unhandled sound encode option 0x%02X!") % (unsigned int)encode);
		return -1;
	}

	out.writeBytes(in.data(), in.pos());

	// skip samples

	uint32_t skipSamples = in.readUint32();

	// MP3 data

	Common::BufferView mp3View = in.readByteView(in.size() - in.pos());
	Common::ReadStream mp3Stream(mp3View, in.endianness);
	if (!decodeMP3(mp3Stream, out, sampleRate, numChannels, sampleSize, skipSamples, chunkID))
		return -1;

	return out.size();
}

}
