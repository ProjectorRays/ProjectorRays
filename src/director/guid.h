/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef DIRECTOR_GUID_H
#define DIRECTOR_GUID_H

#include <cstdint>
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Director {

struct MoaID {
	uint32_t data1;
	uint16_t data2;
	uint16_t data3;
	uint8_t data4[8];

	MoaID() : MoaID(0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0) {}

	MoaID(
		uint32_t d1, uint16_t d2, uint16_t d3,
		uint8_t d40, uint8_t d41, uint8_t d42, uint8_t d43,
		uint8_t d44, uint8_t d45, uint8_t d46, uint8_t d47
	) {
		data1 = d1;
		data2 = d2;
		data3 = d3;
		data4[0] = d40;
		data4[1] = d41;
		data4[2] = d42;
		data4[3] = d43;
		data4[4] = d44;
		data4[5] = d45;
		data4[6] = d46;
		data4[7] = d47;
	}

	void read(Common::SeekableReadStream &stream);
	Common::String toString() const;

	bool operator==(const MoaID &other) const;
	bool operator!=(const MoaID &other) const;
};

#define FONTMAP_COMPRESSION_GUID MoaID(0x8A4679A1, 0x3720, 0x11D0, 0x92, 0x23, 0x00, 0xA0, 0xC9, 0x08, 0x68, 0xB1)
#define NULL_COMPRESSION_GUID MoaID(0xAC99982E, 0x005D, 0x0D50, 0x00, 0x00, 0x08, 0x00, 0x07, 0x37, 0x7A, 0x34)
#define SND_COMPRESSION_GUID MoaID(0x7204A889, 0xAFD0, 0x11CF, 0xA2, 0x22, 0x00, 0xA0, 0x24, 0x53, 0x44, 0x4C)
#define ZLIB_COMPRESSION_GUID MoaID(0xAC99E904, 0x0070, 0x0B36, 0x00, 0x00, 0x08, 0x00, 0x07, 0x37, 0x7A, 0x34)

} // namespace Director

#endif // DIRECTOR_GUID_H
