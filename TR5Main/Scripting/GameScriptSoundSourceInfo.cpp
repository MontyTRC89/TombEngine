#include "framework.h"
#include <sol.hpp>
#include "ScriptAssert.h"
#include "GameScriptSoundSourceInfo.h"
#include "GameScriptPosition.h"
#include "ScriptUtil.h"
/***
Sound source info

@entityclass SoundSourceInfo
@pragma nostrip
*/

static constexpr auto LUA_CLASS_NAME{ "SoundSourceInfo" };

static auto index_error = index_error_maker(GameScriptSoundSourceInfo, LUA_CLASS_NAME);
static auto newindex_error = newindex_error_maker(GameScriptSoundSourceInfo, LUA_CLASS_NAME);

GameScriptSoundSourceInfo::GameScriptSoundSourceInfo(SOUND_SOURCE_INFO & ref, bool temp) : m_soundSource{ref}, m_temporary{ temp }
{};

GameScriptSoundSourceInfo::~GameScriptSoundSourceInfo() {
	if (m_temporary)
	{
		s_callbackRemoveName(m_soundSource.luaName);
	}
}

void GameScriptSoundSourceInfo::Register(sol::state* state)
{
	state->new_usertype<GameScriptSoundSourceInfo>(LUA_CLASS_NAME,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// (@{Position}) position in level
		// @mem pos
		"pos", sol::property(&GameScriptSoundSourceInfo::GetPos, &GameScriptSoundSourceInfo::SetPos),

		/// (string) unique string identifier.
		// e.g. "machine\_sound\_1" or "discordant\_humming"
		// @mem name
		"name", sol::property(&GameScriptSoundSourceInfo::GetName, &GameScriptSoundSourceInfo::SetName),

		/// (int) sound ID 
		// @mem soundID
		"soundID", sol::property(&GameScriptSoundSourceInfo::GetSoundID, &GameScriptSoundSourceInfo::SetSoundID),

		/// (int) flags 
		// @mem flags
		"flags", sol::property(&GameScriptSoundSourceInfo::GetFlags, &GameScriptSoundSourceInfo::SetFlags)
	);
}

GameScriptPosition GameScriptSoundSourceInfo::GetPos() const
{
	return GameScriptPosition{ m_soundSource.x, m_soundSource.y, m_soundSource.z };
}

void GameScriptSoundSourceInfo::SetPos(GameScriptPosition const& pos)
{
	m_soundSource.x = pos.x;
	m_soundSource.y = pos.y;
	m_soundSource.z = pos.z;
}

std::string GameScriptSoundSourceInfo::GetName() const
{
	return m_soundSource.luaName;
}

void GameScriptSoundSourceInfo::SetName(std::string const & id) 
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

int GameScriptSoundSourceInfo::GetSoundID() const
{
	return m_soundSource.soundId;
}

void GameScriptSoundSourceInfo::SetSoundID(int soundID)
{	
	m_soundSource.soundId = soundID;
}

int GameScriptSoundSourceInfo::GetFlags() const
{
	return m_soundSource.flags;
}

void GameScriptSoundSourceInfo::SetFlags(int flags)
{	
	m_soundSource.flags = flags;
}
