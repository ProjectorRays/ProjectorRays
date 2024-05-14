/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef DIRECTOR_CASTMEMBER_H
#define DIRECTOR_CASTMEMBER_H

namespace Common {
class JSONWriter;
class SeekableReadStream;
}

namespace Director {

class DirectorFile;

enum MemberType {
	kNullMember			= 0,
	kBitmapMember		= 1,
	kFilmLoopMember		= 2,
	kTextMember			= 3,
	kPaletteMember		= 4,
	kPictureMember		= 5,
	kSoundMember		= 6,
	kButtonMember		= 7,
	kShapeMember		= 8,
	kMovieMember		= 9,
	kDigitalVideoMember	= 10,
	kScriptMember		= 11,
	kRTEMember			= 12
};

struct CastMember {
	DirectorFile *dir;
	MemberType type;

	CastMember(DirectorFile *d, MemberType t) : dir(d), type(t) {}
	virtual ~CastMember() = default;
	virtual void read(Common::SeekableReadStream &stream);
	virtual void writeJSON(Common::JSONWriter &json) const;
};

enum ScriptType {
	kScoreScript = 1,
	kMovieScript = 3,
	kParentScript = 7
};

struct ScriptMember : CastMember {
	ScriptType scriptType;

	ScriptMember(DirectorFile *m) : CastMember(m, kScriptMember) {}
	virtual ~ScriptMember() = default;
	virtual void read(Common::SeekableReadStream &stream);
	virtual void writeJSON(Common::JSONWriter &json) const;
};

} // namespace Director

#endif // DIRECTOR_CASTMEMBER_H
