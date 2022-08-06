#pragma once
#include "framework.h"

#include "AIObject.h"
#include "ScriptAssert.h"
#include "Vec3/Vec3.h"
#include "ScriptUtil.h"
#include "ReservedScriptNames.h"
/***
AI object

@tenclass Objects.AIObject
@pragma nostrip
*/

static auto index_error = index_error_maker(AIObject, ScriptReserved_AIObject);
static auto newindex_error = newindex_error_maker(AIObject, ScriptReserved_AIObject);

AIObject::AIObject(AI_OBJECT & ref) : m_aiObject{ref}
{};

void AIObject::Register(sol::table & parent)
{
	parent.new_usertype<AIObject>(ScriptReserved_AIObject,
		sol::no_constructor, // ability to spawn new ones could be added later
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// Get the object's position
		// @function AIObject:GetPosition
		// @treturn Vec3 a copy of the object's position
		ScriptReserved_GetPosition, &AIObject::GetPos,

		/// Set the object's position
		// @function AIObject:SetPosition
		// @tparam Vec3 position the new position of the object 
		ScriptReserved_SetPosition, &AIObject::SetPos,

		/// Get the object's Y-axis rotation
		// To the best of my knowledge, the rotation of an AIObject has no effect.
		// @function AIObject:GetRotationY
		// @treturn number the object's Y-axis rotation 
		ScriptReserved_GetRotationY, &AIObject::GetYRot,

		/// Set the object's Y-axis rotation
		// To the best of my knowledge, the rotation of an AIObject has no effect.
		// @function AIObject:SetRotationY
		// @tparam number The object's new Y-axis rotation
		ScriptReserved_SetRotationY, &AIObject::SetYRot,

		/// Get the object's unique string identifier
		// @function AIObject:GetName
		// @treturn string the object's name
		ScriptReserved_GetName, &AIObject::GetName,

		/// Set the object's name (its unique string identifier)
		// @function AIObject:SetName
		// @tparam string name The object's new name
		ScriptReserved_SetName, &AIObject::SetName,

		/// Get the current room of the object
		// @function AIObject:GetRoom
		// @treturn int number representing the current room of the object
		ScriptReserved_GetRoom, &AIObject::GetRoom,

		/// Set room of object 
		// This is used in conjunction with SetPosition to teleport the object to a new room.
		// @function AIObject:SetRoom
		// @tparam int ID the ID of the new room 
		ScriptReserved_SetRoom, &AIObject::SetRoom,

		/// Retrieve the object ID
		// @function AIObject:GetObjectID
		// @treturn int a number representing the ID of the object
		ScriptReserved_GetObjectID, &AIObject::GetObjectID,

		/// Change the object's ID. This will change the type of AI object it is.
		// Note that a baddy will gain the behaviour of the tile it's on _before_ said baddy is triggered.
		// This means that changing the type of an AI object beneath a moveable will have no effect.
		// Instead, this function can be used to change an object that the baddy isn't standing on.
		// For example, you could have a pair of AI_GUARD objects, and change one or the other two
		// AI_PATROL_1 based on whether the player has a certain item or not.
		// @function AIObject:SetObjectID
		// @tparam ObjID ID the new ID 
		// @usage
		// aiObj = TEN.Objects.GetMoveableByName("ai_guard_sphinx_room")
		// aiObj:SetObjectID(TEN.Objects.ObjID.AI_PATROL1)
		ScriptReserved_SetObjectID, &AIObject::SetObjectID
		);
}

Vec3 AIObject::GetPos() const
{
	return Vec3{ m_aiObject.x, m_aiObject.y, m_aiObject.z };
}

void AIObject::SetPos(Vec3 const& pos)
{
	m_aiObject.x = pos.x;
	m_aiObject.y = pos.y;
	m_aiObject.z = pos.z;
}

GAME_OBJECT_ID AIObject::GetObjectID() const
{
	return m_aiObject.objectNumber;
}

void AIObject::SetObjectID(GAME_OBJECT_ID objNum)
{
	m_aiObject.objectNumber = objNum;
}

short AIObject::GetYRot() const
{
	return m_aiObject.yRot;
}

void AIObject::SetYRot(short yRot)
{
	m_aiObject.yRot = yRot;
}

std::string AIObject::GetName() const
{
	return m_aiObject.luaName;
}

void AIObject::SetName(std::string const & id) 
{
	if (!ScriptAssert(!id.empty(), "Name cannot be blank. Not setting name."))
	{
		return;
	}

	if (s_callbackSetName(id, m_aiObject))
	{
		// remove the old name if we have one
		s_callbackRemoveName(m_aiObject.luaName);
		m_aiObject.luaName = id;
	}
	else
	{
		ScriptAssertF(false, "Could not add name {} - does an object with this name already exist?", id);
		TENLog("Name will not be set", LogLevel::Warning, LogConfig::All);
	}
}

short AIObject::GetRoom() const
{
	return m_aiObject.roomNumber;
}

void AIObject::SetRoom(short room)
{
	const size_t nRooms = g_Level.Rooms.size();
	if (room < 0 || static_cast<size_t>(room) >= nRooms)
	{
		ScriptAssertF(false, "Invalid room number: {}. Value must be in range [0, {})", room, nRooms);
		TENLog("Room number will not be set", LogLevel::Warning, LogConfig::All);
		return;
	}

	m_aiObject.roomNumber = room;
}
