/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

Common::BufferView getFontMap(int version) {
	// Writes the default font map for the given Director version.
	// D12 is the same as D11.5 and D6.5 is the same as D6.
	// Font map compression was added in D6, so we don't need any earlier versions.
	if (version >= 1150)
		return Common::BufferView(fontmaps_fontmap_D11_5_txt, fontmaps_fontmap_D11_5_txt_len);
	if (version >= 1100)
		return Common::BufferView(fontmaps_fontmap_D11_txt, fontmaps_fontmap_D11_txt_len);
	if (version >= 1000)
		return Common::BufferView(fontmaps_fontmap_D10_txt, fontmaps_fontmap_D10_txt_len);
	if (version >= 900)
		return Common::BufferView(fontmaps_fontmap_D9_txt, fontmaps_fontmap_D9_txt_len);
	if (version >= 850)
		return Common::BufferView(fontmaps_fontmap_D8_5_txt, fontmaps_fontmap_D8_5_txt_len);
	if (version >= 800)
		return Common::BufferView(fontmaps_fontmap_D8_txt, fontmaps_fontmap_D8_txt_len);
	if (version >= 700)
		return Common::BufferView(fontmaps_fontmap_D7_txt, fontmaps_fontmap_D7_txt_len);
	return Common::BufferView(fontmaps_fontmap_D6_txt, fontmaps_fontmap_D6_txt_len);
}

} // namespace Director
