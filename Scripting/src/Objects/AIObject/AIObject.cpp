#pragma once
#include "frameworkandsol.h"
#include "Objects/AIObject/AIObject.h"
#include "ScriptAssert.h"
#include "Position/Position.h"
#include "ScriptUtil.h"
#include "ReservedScriptNames.h"
/***
AI object

@tenclass Objects.AIObject
@pragma nostrip
*/

static auto index_error = index_error_maker(AIObject, ScriptReserved_AIObject);
static auto newindex_error = newindex_error_maker(AIObject, ScriptReserved_AIObject);

AIObject::AIObject(AI_OBJECT & ref, bool temp) : m_aiObject{ref}, m_temporary{ temp }
{};

AIObject::~AIObject() {
	if (m_temporary)
	{
		s_callbackRemoveName(m_aiObject.luaName);
	}
}

void AIObject::Register(sol::table & parent)
{
	parent.new_usertype<AIObject>(ScriptReserved_AIObject,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// (@{Position}) position in level
		// @mem pos
		"pos", sol::property(&AIObject::GetPos, &AIObject::SetPos),

		/// (int) y-axis rotation
		// @mem yRot
		"yRot", sol::property(&AIObject::GetYRot, &AIObject::SetYRot),

		/// (string) unique string identifier.
		// e.g. "door_back_room" or "cracked_greek_statue"
		// @mem name
		"name", sol::property(&AIObject::GetName, &AIObject::SetName),

		/// (int) room number
		// @mem room
		"room", sol::property(&AIObject::GetRoom, &AIObject::SetRoom),

		/// (@{ObjID}) object ID
		// @mem objID
		"objID", sol::property(&AIObject::GetObjID, &AIObject::SetObjID),

		/// (short) flags
		// @mem flags
		"flags", sol::property(&AIObject::GetFlags, &AIObject::SetFlags),

		/// (short) trigger flags
		// @mem triggerFlags
		"triggerFlags", sol::property(&AIObject::GetTriggerFlags, &AIObject::SetTriggerFlags),

		/// (short) box number
		// @mem boxNumber
		"boxNumber", sol::property(&AIObject::GetBoxNumber, &AIObject::SetBoxNumber)
		);
}

Position AIObject::GetPos() const
{
	return Position{ m_aiObject.x, m_aiObject.y, m_aiObject.z };
}

void AIObject::SetPos(Position const& pos)
{
	m_aiObject.x = pos.x;
	m_aiObject.y = pos.y;
	m_aiObject.z = pos.z;
}

GAME_OBJECT_ID AIObject::GetObjID() const
{
	return m_aiObject.objectNumber;
}

void AIObject::SetObjID(GAME_OBJECT_ID objNum)
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
	ScriptAssert(!id.empty(), "Name cannot be blank", ERROR_MODE::TERMINATE);

	// remove the old name if we have one
	s_callbackRemoveName(m_aiObject.luaName);

	// un-register any other objects using this name.
	// maybe we should throw an error if another object
	// already uses the name...
	s_callbackRemoveName(id);
	m_aiObject.luaName = id;
	// todo add error checking
	s_callbackSetName(id, m_aiObject);
}

short AIObject::GetRoom() const
{
	return m_aiObject.roomNumber;
}

void AIObject::SetRoom(short room)
{
	m_aiObject.roomNumber = room;
}

short AIObject::GetTriggerFlags() const
{

	return m_aiObject.triggerFlags;
}

void AIObject::SetTriggerFlags(short tf)
{
	m_aiObject.triggerFlags = tf;
}

short AIObject::GetFlags() const
{
	return m_aiObject.flags;
}

void AIObject::SetFlags(short tf)
{
	m_aiObject.flags = tf;
}

short AIObject::GetBoxNumber() const
{
	return m_aiObject.boxNumber;
}

void AIObject::SetBoxNumber(short bn)
{
	m_aiObject.boxNumber = bn;
}

