#pragma once
#include "framework.h"

#include "Renderer/Renderer.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptAssert.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/Objects/Room/RoomFlags.h"
#include "Scripting/Internal/TEN/Objects/Room/RoomObject.h"
#include "Scripting/Internal/TEN/Vec3/Vec3.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/trutils.h"

/***
Rooms

@tenclass Objects.Room
@pragma nostrip
*/

static auto IndexError = index_error_maker(Room, ScriptReserved_Volume);
static auto NewIndexError = newindex_error_maker(Room, ScriptReserved_Volume);

Room::Room(ROOM_INFO& room) : m_room{ room }
{};

void Room::Register(sol::table& parent)
{
	parent.new_usertype<Room>(ScriptReserved_Room,
		sol::no_constructor,
		sol::meta_function::index, IndexError,
		sol::meta_function::new_index, NewIndexError,

		/// Determine whether the room is active or not 
		// @function Room:GetActive
		// @treturn bool true if the room is active
		ScriptReserved_GetActive, &Room::GetActive,

		/// Get the room ID
		// @function Room:GetRoomNumber
		// @treturn int the room ID
		ScriptReserved_GetRoomNumber, & Room::GetRoomNumber,

		/// Get the room's ambient light color.
		// @function Room:GetColor
		// @treturn Color ambient light color of the room
		ScriptReserved_GetColor, & Room::GetColor,

		/// Get the room's reverb type.
		// @function Room:GetReverbType
		// @treturn Objects.RoomReverb room's reverb type
		ScriptReserved_GetPosition, &Room::GetReverbType,

		/// Set the room's reverb type.
		// @function Room:SetReverbType
		// @tparam Objects.RoomReverb new reverb type of the room 
		ScriptReserved_SetReverbType, &Room::SetReverbType,

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
		// @tparam Objects.RoomFlagID flagID The room's flag ID
		// @treturn bool the room's specified flag value
		ScriptReserved_GetFlag, &Room::GetFlag,

		/// Set the room's specified flag value.
		// @function Room:SetFlag
		// @tparam Objects.RoomFlagID flagID The room's flag ID
		// @tparam bool the room's new flag value
		ScriptReserved_SetFlag, &Room::SetFlag,

		/// Checks if specified tag is set for this room.
		// @function Room:IsTagPresent
		// @tparam string tag A text tag to check (case sensitive)
		// @treturn bool true if tag is present, false if not
		ScriptReserved_IsTagPresent, &Room::IsTagPresent);
}

bool Room::GetActive() const
{
	return m_room.Active();
}

int Room::GetRoomNumber() const
{
	return m_room.RoomNumber;
}

ScriptColor Room::GetColor() const
{
	return ScriptColor{ m_room.ambient };
}

ReverbType Room::GetReverbType() const
{
	return m_room.reverbType;
}

void Room::SetReverbType(ReverbType reverb)
{
	m_room.reverbType = reverb;
}

std::string Room::GetName() const
{
	return m_room.Name;
}

void Room::SetName(const std::string& name)
{
	if (!ScriptAssert(!name.empty(), "Unable to set name. Name cannot be blank."))
		return;

	// Remove old name if it already exists.
	if (s_callbackSetName(name, m_room))
	{
		s_callbackRemoveName(m_room.Name);
		m_room.Name = name;
	}
	else
	{
		ScriptAssertF(false, "Could not add name {} - does an object with this name already exist?", name);
		TENLog("Name will not be set", LogLevel::Warning, LogConfig::All);
	}
}

bool Room::GetFlag(RoomEnvFlags flag) const
{
	return ((m_room.flags & flag) == flag);
}

void Room::SetFlag(RoomEnvFlags flag, bool value)
{
	if (value)
	{
		m_room.flags |= flag;
	}
	else
	{
		m_room.flags &= ~flag;
	}
}

bool Room::IsTagPresent(const std::string& tag) const
{
	if (m_room.Tags.empty())
		return false;

	return std::any_of(
		m_room.Tags.begin(), m_room.Tags.end(),
		[&tag](const std::string& value) { return (value == tag); });
}
