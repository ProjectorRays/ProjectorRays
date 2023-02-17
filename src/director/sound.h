/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef DIRECTOR_SOUND_H
#define DIRECTOR_SOUND_H

#include <sys/types.h> // for ssize_t. not portable...

namespace Common {
class ReadStream;
class WriteStream;
}

namespace Director {

ssize_t decompressSnd(Common::ReadStream &in, Common::WriteStream &out, int32_t castID);

} // namespace Director

#endif // DIRECTOR_SOUND_H
