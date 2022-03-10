#pragma once

#include "Objects/NamedBase.h"
#include "Specific/phd_global.h"

namespace sol {
	class state;
}
class Position;

class SoundSource : public NamedBase<SoundSource, SOUND_SOURCE_INFO &>
{
public:
	using IdentifierType = std::reference_wrapper<SOUND_SOURCE_INFO>;
	SoundSource(SOUND_SOURCE_INFO& ref, bool temp);
	~SoundSource();

	SoundSource& operator=(SoundSource const& other) = delete;
	SoundSource(SoundSource const& other) = delete;

	static void Register(sol::table &);
	Position GetPos() const;
	void SetPos(Position const& pos);

	int GetSoundID() const;
	void SetSoundID(int soundID);

	std::string GetName() const;
	void SetName(std::string const &);

private:
	SOUND_SOURCE_INFO & m_soundSource;
	bool m_temporary;
};

