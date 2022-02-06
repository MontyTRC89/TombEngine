#pragma once

#include "Objects/NamedBase.h"
#include "Specific/phd_global.h"

namespace sol {
	class state;
}
class GameScriptPosition;

class SoundSource : public NamedBase<SoundSource, SOUND_SOURCE_INFO &>
{
public:
	using IdentifierType = std::reference_wrapper<SOUND_SOURCE_INFO>;
	SoundSource(SOUND_SOURCE_INFO& ref, bool temp);
	~SoundSource();

	SoundSource& operator=(SoundSource const& other) = delete;
	SoundSource(SoundSource const& other) = delete;

	static void Register(sol::table &);
	GameScriptPosition GetPos() const;
	void SetPos(GameScriptPosition const& pos);

	int GetSoundID() const;
	void SetSoundID(int soundID);

	int GetFlags() const;
	void SetFlags(int flags);

	std::string GetName() const;
	void SetName(std::string const &);

private:
	SOUND_SOURCE_INFO & m_soundSource;
	bool m_temporary;
};

