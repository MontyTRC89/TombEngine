#pragma once
#include "framework.h"

#if TEN_OPTIONAL_LUA
#include "ScriptAssert.h"
#include "SinkObject.h"
#include "Vec3/Vec3.h"
#include "ScriptUtil.h"
#include "ReservedScriptNames.h"
/***
Sink

@tenclass Objects.Sink
@pragma nostrip
*/

static auto index_error = index_error_maker(Sink, ScriptReserved_Sink);
static auto newindex_error = newindex_error_maker(Sink, ScriptReserved_Sink);

Sink::Sink(SINK_INFO & ref) : m_sink{ref}
{};

void Sink::Register(sol::table& parent)
{
	parent.new_usertype<Sink>(ScriptReserved_Sink,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// Get the sink's position
		// @function GetPosition
		// @treturn Vec3 a copy of the sink's position
		ScriptReserved_GetPosition, &Sink::GetPos,

		/// Set the sink's position
		// @function SetPosition
		// @tparam Vec3 position the new position of the sink 
		ScriptReserved_SetPosition, &Sink::SetPos,

		/// Get the sink's unique string identifier
		// e.g. "strong\_river\_current" or "propeller\_death\_sink"
		// @function GetName
		// @treturn string the sink's name
		ScriptReserved_GetName, &Sink::GetName,

		/// Set the sink's name (its unique string identifier)
		// @function SetName
		// @tparam string name The sink's new name
		ScriptReserved_SetName, &Sink::SetName,

		/// Get the sink's strength
		// @function GetStrength
		// @treturn int the sink's current strength
		ScriptReserved_GetStrength, &Sink::GetStrength,

		/// Set the strength of the sink
		// Higher numbers provide stronger currents. Will be clamped to [1, 32].
		// @function SetStrength
		// @tparam int strength The sink's new strength
		ScriptReserved_SetStrength, &Sink::SetStrength
		);
}

Vec3 Sink::GetPos() const
{
	return Vec3{ m_sink.x, m_sink.y, m_sink.z };
}

void Sink::SetPos(Vec3 const& pos)
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
	if (!ScriptAssert(!id.empty(), "Name cannot be blank. Not setting name."))
	{
		return;
	}

	if (s_callbackSetName(id, m_sink))
	{
		// remove the old name if we have one
		s_callbackRemoveName(m_sink.luaName);
		m_sink.luaName = id;
	}
	else
	{
		ScriptAssertF(false, "Could not add name {} - does an object with this name already exist?", id);
		TENLog("Name will not be set", LogLevel::Warning, LogConfig::All);
	}
}

int Sink::GetStrength() const
{
	return m_sink.strength;
}

void Sink::SetStrength(int str)
{
	m_sink.strength = std::clamp(str, 1, 32);
}

#endif
