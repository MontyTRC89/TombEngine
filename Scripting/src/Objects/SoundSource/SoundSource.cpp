#include "frameworkandsol.h"
#include "ScriptAssert.h"
#include "SoundSource.h"
#include "GameScriptPosition.h"
#include "ScriptUtil.h"
#include "ReservedScriptNames.h"
/***
Sound source

@tenclass SoundSource
@pragma nostrip
*/

static auto index_error = index_error_maker(SoundSource, ScriptReserved_SoundSource);
static auto newindex_error = newindex_error_maker(SoundSource, ScriptReserved_SoundSource);

SoundSource::SoundSource(SOUND_SOURCE_INFO & ref, bool temp) : m_soundSource{ref}, m_temporary{ temp }
{};

SoundSource::~SoundSource() {
	if (m_temporary)
	{
		s_callbackRemoveName(m_soundSource.luaName);
	}
}

void SoundSource::Register(sol::table & parent)
{
	parent.new_usertype<SoundSource>(ScriptReserved_SoundSource,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// (@{Position}) position in level
		// @mem pos
		"pos", sol::property(&SoundSource::GetPos, &SoundSource::SetPos),

		/// (string) unique string identifier.
		// e.g. "machine\_sound\_1" or "discordant\_humming"
		// @mem name
		"name", sol::property(&SoundSource::GetName, &SoundSource::SetName),

		/// (int) sound ID 
		// @mem soundID
		"soundID", sol::property(&SoundSource::GetSoundID, &SoundSource::SetSoundID),

		/// (int) flags 
		// @mem flags
		"flags", sol::property(&SoundSource::GetFlags, &SoundSource::SetFlags)
	);
}

GameScriptPosition SoundSource::GetPos() const
{
	return GameScriptPosition{ m_soundSource.x, m_soundSource.y, m_soundSource.z };
}

void SoundSource::SetPos(GameScriptPosition const& pos)
{
	m_soundSource.x = pos.x;
	m_soundSource.y = pos.y;
	m_soundSource.z = pos.z;
}

std::string SoundSource::GetName() const
{
	return m_soundSource.luaName;
}

void SoundSource::SetName(std::string const & id) 
{
	ScriptAssert(!id.empty(), "Name cannot be blank", ERROR_MODE::TERMINATE);

	// remove the old name if we have one
	s_callbackRemoveName(m_soundSource.luaName);

	// un-register any other objects using this name.
	// maybe we should throw an error if another object
	// already uses the name...
	s_callbackRemoveName(id);
	m_soundSource.luaName = id;
	// todo add error checking
	s_callbackSetName(id, m_soundSource);
}

int SoundSource::GetSoundID() const
{
	return m_soundSource.soundId;
}

void SoundSource::SetSoundID(int soundID)
{	
	m_soundSource.soundId = soundID;
}

int SoundSource::GetFlags() const
{
	return m_soundSource.flags;
}

void SoundSource::SetFlags(int flags)
{	
	m_soundSource.flags = flags;
}
