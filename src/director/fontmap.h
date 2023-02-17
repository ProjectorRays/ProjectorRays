/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef DIRECTOR_FONTMAP_H
#define DIRECTOR_FONTMAP_H

#include "common/stream.h"

namespace Director {

Common::BufferView getFontMap(int version);

} // namespace Director

#endif // DIRECTOR_FONTMAP_H
