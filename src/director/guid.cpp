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

#include <boost/format.hpp>

#include "common/stream.h"
#include "director/guid.h"

namespace Director {

/* MoaID */

void MoaID::read(Common::ReadStream &stream) {
	data1 = stream.readUint32();
	data2 = stream.readUint16();
	data3 = stream.readUint16();
	for (size_t i = 0; i < 8; i++) {
		data4[i] = stream.readUint8();
	}
}

std::string MoaID::toString() const {
	return boost::str(
		boost::format("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X")
			% data1 % data2 % data3
			% (unsigned int)data4[0] % (unsigned int)data4[1] % (unsigned int)data4[2] % (unsigned int)data4[3]
			% (unsigned int)data4[4] % (unsigned int)data4[5] % (unsigned int)data4[6] % (unsigned int)data4[7]
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

}
