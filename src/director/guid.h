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

#ifndef DIRECTOR_GUID_H
#define DIRECTOR_GUID_H

#include <cstdint>
#include <string>

namespace Common {
class ReadStream;
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

	void read(Common::ReadStream &stream);
	std::string toString() const;

	bool operator==(const MoaID &other) const;
	bool operator!=(const MoaID &other) const;
};

#define FONTMAP_COMPRESSION_GUID MoaID(0x8A4679A1, 0x3720, 0x11D0, 0x92, 0x23, 0x00, 0xA0, 0xC9, 0x08, 0x68, 0xB1)
#define NULL_COMPRESSION_GUID MoaID(0xAC99982E, 0x005D, 0x0D50, 0x00, 0x00, 0x08, 0x00, 0x07, 0x37, 0x7A, 0x34)
#define SND_COMPRESSION_GUID MoaID(0x7204A889, 0xAFD0, 0x11CF, 0xA2, 0x22, 0x00, 0xA0, 0x24, 0x53, 0x44, 0x4C)
#define ZLIB_COMPRESSION_GUID MoaID(0xAC99E904, 0x0070, 0x0B36, 0x00, 0x00, 0x08, 0x00, 0x07, 0x37, 0x7A, 0x34)

}

#endif // DIRECTOR_GUID_H
