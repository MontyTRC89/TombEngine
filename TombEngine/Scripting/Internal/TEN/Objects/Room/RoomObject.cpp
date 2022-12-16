#pragma once
#include "framework.h"

#include "ScriptAssert.h"
#include "RoomObject.h"
#include "Color/Color.h"
#include "Vec3/Vec3.h"
#include "ScriptUtil.h"
#include "ReservedScriptNames.h"
#include "Specific/level.h"
#include "Specific/trutils.h"
#include "Sound/sound.h"

/***
Rooms

@tenclass Objects.Room
@pragma nostrip
*/

static auto index_error = index_error_maker(Room, ScriptReserved_Volume);
static auto newindex_error = newindex_error_maker(Room, ScriptReserved_Volume);

Room::Room(ROOM_INFO& room) : m_room{ room }
{};

void Room::Register(sol::table& parent)
{
	parent.new_usertype<Room>(ScriptReserved_Room,
		sol::no_constructor,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// Determine whether the room is active or not 
		// @function Room:GetActive
		// @treturn bool true if the room is active
		ScriptReserved_GetActive, &Room::GetActive,

		/// Get the room's reverb type.
		// @function Room:GetReverbType
		// @treturn Objects.RoomReverbType room's reverb type
		ScriptReserved_GetPosition, &Room::GetReverbType,

		/// Set the room's reverb type.
		// @function Room:SetReverbType
		// @tparam Objects.RoomReverbType new reverb type of the room 
		ScriptReserved_SetPosition, &Room::SetReverbType,

		/// Get the room's unique string identifier.
		// @function Room:GetName
		// @treturn string the room's name
		ScriptReserved_GetName, &Room::GetName,

		/// Set the room's name (its unique string identifier).
		// @function Room:SetName
		// @tparam string name The room's new name
		ScriptReserved_SetName, &Room::SetName,

		/// Get the room's specified flag value (true or false).
		// @function Room:GetFlag
		// @tparam Objects.RoomFlag flagID The room's flag ID
		// @treturn bool the room's specified flag value
		ScriptReserved_GetFlag, &Room::GetFlag,

		/// Set the room's ambient light color.
		// @function Room:SetFlag
		// @tparam Objects.RoomFlag flagID The room's flag ID
		// @tparam bool the room's new flag value
		ScriptReserved_SetFlag, &Room::SetFlag,

		/// Checks if specified tag is set for this room.
		// @function Room:IsTagPresent
		// @treturn bool true if tag is present, false if not
		ScriptReserved_IsTagPresent, &Room::IsTagPresent);
}

bool Room::GetActive() const
{
	return m_room.Active();
}

ReverbType Room::GetReverbType() const
{
	return m_room.reverbType;
}

void Room::SetReverbType(ReverbType const& reverb)
{
	m_room.reverbType = reverb;
}

std::string Room::GetName() const
{
	return m_room.name;
}

void Room::SetName(std::string const& name)
{
	if (!ScriptAssert(!name.empty(), "Name cannot be blank. Not setting name."))
	{
		return;
	}

	if (s_callbackSetName(name, m_room))
	{
		// remove the old name if we have one
		s_callbackRemoveName(m_room.name);
		m_room.name = name;
	}
	else
	{
		ScriptAssertF(false, "Could not add name {} - does an object with this name already exist?", name);
		TENLog("Name will not be set", LogLevel::Warning, LogConfig::All);
	}
}

bool Room::GetFlag(RoomEnvFlags const& flag) const
{
	return (m_room.flags & flag) == flag;
}

void Room::SetFlag(RoomEnvFlags const& flag, bool const& value)
{
	if (value)
		m_room.flags |= 1 << (int)flag;
	else
		m_room.flags &= ~(1 << (int)flag);
}

bool Room::IsTagPresent(std::string const& tag) const
{
	if (m_room.tags.empty())
		return false;

	return std::any_of(m_room.tags.begin(), m_room.tags.end(), 
		[&tag](const std::string& value) { return value == tag; });
}