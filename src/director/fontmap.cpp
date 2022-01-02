/**
 * ProjectorRays Shockwave Decompiler
 * Copyright (C) 2017 Anthony Kleine, Deborah Servilla
 * Copyright (C) 2020-2022 Deborah Servilla
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

#include "common/stream.h"
#include "director/fontmap.h"

#include "../fontmaps/fontmap_D6.h"
#include "../fontmaps/fontmap_D7.h"
#include "../fontmaps/fontmap_D8.h"
#include "../fontmaps/fontmap_D8_5.h"
#include "../fontmaps/fontmap_D9.h"
#include "../fontmaps/fontmap_D10.h"
#include "../fontmaps/fontmap_D11.h"
#include "../fontmaps/fontmap_D11_5.h"

namespace Director {

void writeFontMap(Common::WriteStream &stream, int version) {
	// Writes the default font map for the given Director version.
	// D12 is the same as D11.5 and D6.5 is the same as D6.
	// Font map compression was added in D6, so we don't need any earlier versions.
	if (version >= 1150) {
		stream.writeBytes(fontmaps_fontmap_D11_5_txt, fontmaps_fontmap_D11_5_txt_len);
	} else if (version >= 1100) {
		stream.writeBytes(fontmaps_fontmap_D11_txt, fontmaps_fontmap_D11_txt_len);
	} else if (version >= 1000) {
		stream.writeBytes(fontmaps_fontmap_D10_txt, fontmaps_fontmap_D10_txt_len);
	} else if (version >= 900) {
		stream.writeBytes(fontmaps_fontmap_D9_txt, fontmaps_fontmap_D9_txt_len);
	} else if (version >= 850) {
		stream.writeBytes(fontmaps_fontmap_D8_5_txt, fontmaps_fontmap_D8_5_txt_len);
	} else if (version >= 800) {
		stream.writeBytes(fontmaps_fontmap_D8_txt, fontmaps_fontmap_D8_txt_len);
	} else if (version >= 700) {
		stream.writeBytes(fontmaps_fontmap_D7_txt, fontmaps_fontmap_D7_txt_len);
	} else {
		stream.writeBytes(fontmaps_fontmap_D6_txt, fontmaps_fontmap_D6_txt_len);
	}
}

}
