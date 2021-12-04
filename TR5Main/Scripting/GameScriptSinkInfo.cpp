#pragma once
#include "frameworkandsol.h"
#include "ScriptAssert.h"
#include "GameScriptSinkInfo.h"
#include "GameScriptPosition.h"
#include "ScriptUtil.h"
/***
Sink info

@entityclass SinkInfo
@pragma nostrip
*/

constexpr auto LUA_CLASS_NAME{ "SinkInfo" };

static auto index_error = index_error_maker(GameScriptSinkInfo, LUA_CLASS_NAME);
static auto newindex_error = newindex_error_maker(GameScriptSinkInfo, LUA_CLASS_NAME);

GameScriptSinkInfo::GameScriptSinkInfo(SINK_INFO & ref, bool temp) : m_sink{ref}, m_temporary{ temp }
{};

GameScriptSinkInfo::~GameScriptSinkInfo() {
	if (m_temporary)
	{
		s_callbackRemoveName(m_sink.luaName);
	}
}

void GameScriptSinkInfo::Register(sol::state* state)
{
	state->new_usertype<GameScriptSinkInfo>(LUA_CLASS_NAME,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// (@{Position}) position in level
		// @mem pos
		"pos", sol::property(&GameScriptSinkInfo::GetPos, &GameScriptSinkInfo::SetPos),

		/// (string) unique string identifier.
		// e.g. "strong\_river\_current" or "propeller\_death\_sink"
		// @mem name
		"name", sol::property(&GameScriptSinkInfo::GetName, &GameScriptSinkInfo::SetName),

		/// (int) strength.
		// Strength of the sink, with higher numbers providing stronger currents. Will be clamped to [1, 32].
		// @mem strength
		"strength", sol::property(&GameScriptSinkInfo::GetStrength, &GameScriptSinkInfo::SetStrength),

		/// (int) box index.
		// I don't know what this does and it's not actually in the engine yet
		// @mem boxIndex
		"boxIndex", sol::property(&GameScriptSinkInfo::GetBoxIndex, &GameScriptSinkInfo::SetBoxIndex)
		);
}

GameScriptPosition GameScriptSinkInfo::GetPos() const
{
	return GameScriptPosition{ m_sink.x, m_sink.y, m_sink.z };
}

void GameScriptSinkInfo::SetPos(GameScriptPosition const& pos)
{
	m_sink.x = pos.x;
	m_sink.y = pos.y;
	m_sink.z = pos.z;
}

std::string GameScriptSinkInfo::GetName() const
{
	return m_sink.luaName;
}

void GameScriptSinkInfo::SetName(std::string const & id) 
{
	ScriptAssert(!id.empty(), "Name cannot be blank", ERROR_MODE::TERMINATE);

	// remove the old name if we have one
	s_callbackRemoveName(m_sink.luaName);

	// un-register any other sinks using this name.
	// maybe we should throw an error if another sink
	// already uses the name...
	s_callbackRemoveName(id);
	m_sink.luaName = id;
	// todo add error checking
	s_callbackSetName(id, m_sink);
}

int GameScriptSinkInfo::GetStrength() const
{
	return m_sink.strength;
}

void GameScriptSinkInfo::SetStrength(int str)
{
	m_sink.strength = std::clamp(str, 1, 32);
}

int GameScriptSinkInfo::GetBoxIndex() const
{

	return m_sink.boxIndex;
}

void GameScriptSinkInfo::SetBoxIndex(int b)
{
	m_sink.boxIndex = b;
}

