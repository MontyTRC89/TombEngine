#pragma once
#include "frameworkandsol.h"

#if TEN_OPTIONAL_LUA
#include "ScriptAssert.h"
#include "Sink.h"
#include "Position/Position.h"
#include "ScriptUtil.h"
#include "ReservedScriptNames.h"
/***
Sink

@tenclass Objects.Sink
@pragma nostrip
*/

static auto index_error = index_error_maker(Sink, ScriptReserved_Sink);
static auto newindex_error = newindex_error_maker(Sink, ScriptReserved_Sink);

Sink::Sink(SINK_INFO & ref, bool temp) : m_sink{ref}, m_temporary{ temp }
{};

Sink::~Sink() {
	if (m_temporary)
	{
		s_callbackRemoveName(m_sink.luaName);
	}
}

void Sink::Register(sol::table& parent)
{
	parent.new_usertype<Sink>(ScriptReserved_Sink,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// (@{Position}) position in level
		// @mem pos
		"pos", sol::property(&Sink::GetPos, &Sink::SetPos),

		/// (string) unique string identifier.
		// e.g. "strong\_river\_current" or "propeller\_death\_sink"
		// @mem name
		"name", sol::property(&Sink::GetName, &Sink::SetName),

		/// (int) strength.
		// Strength of the sink, with higher numbers providing stronger currents. Will be clamped to [1, 32].
		// @mem strength
		"strength", sol::property(&Sink::GetStrength, &Sink::SetStrength),

		/// (int) box index.
		// I don't know what this does and it's not actually in the engine yet
		// @mem boxIndex
		"boxIndex", sol::property(&Sink::GetBoxIndex, &Sink::SetBoxIndex)
		);
}

Position Sink::GetPos() const
{
	return Position{ m_sink.x, m_sink.y, m_sink.z };
}

void Sink::SetPos(Position const& pos)
{
	m_sink.x = pos.x;
	m_sink.y = pos.y;
	m_sink.z = pos.z;
}

std::string Sink::GetName() const
{
	return m_sink.luaName;
}

void Sink::SetName(std::string const & id) 
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

int Sink::GetStrength() const
{
	return m_sink.strength;
}

void Sink::SetStrength(int str)
{
	m_sink.strength = std::clamp(str, 1, 32);
}

int Sink::GetBoxIndex() const
{

	return m_sink.boxIndex;
}

void Sink::SetBoxIndex(int b)
{
	m_sink.boxIndex = b;
}

#endif
