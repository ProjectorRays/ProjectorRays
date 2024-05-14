/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/stream.h"
#include "director/guid.h"

namespace Director {

/* MoaID */

void MoaID::read(Common::SeekableReadStream &stream) {
	data1 = stream.readUint32();
	data2 = stream.readUint16();
	data3 = stream.readUint16();
	for (size_t i = 0; i < 8; i++) {
		data4[i] = stream.readByte();
	}
}

Common::String MoaID::toString() const {
	return Common::String::format("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
			data1, data2, data3,
			(unsigned int)data4[0], (unsigned int)data4[1], (unsigned int)data4[2], (unsigned int)data4[3],
			(unsigned int)data4[4], (unsigned int)data4[5], (unsigned int)data4[6], (unsigned int)data4[7]
	);
}

bool MoaID::operator==(const MoaID &other) const {
	return data1 == other.data1
			&& data2 == other.data2
			&& data3 == other.data3
			&& data4[0] == other.data4[0]
			&& data4[1] == other.data4[1]
			&& data4[2] == other.data4[2]
			&& data4[3] == other.data4[3]
			&& data4[4] == other.data4[4]
			&& data4[5] == other.data4[5]
			&& data4[6] == other.data4[6]
			&& data4[7] == other.data4[7];
}

bool MoaID::operator!=(const MoaID &other) const {
	return !operator==(other);
}

} // namespace Director
