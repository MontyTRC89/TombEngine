#pragma once
#include "frameworkandsol.h"
#include "GameScriptAIObject.h"
#include "ScriptAssert.h"
#include "GameScriptPosition.h"
#include "ScriptUtil.h"
/***
AI object

@entityclass AIObject
@pragma nostrip
*/


constexpr auto LUA_CLASS_NAME{ "AIObject" };

static auto index_error = index_error_maker(GameScriptAIObject, LUA_CLASS_NAME);
static auto newindex_error = newindex_error_maker(GameScriptAIObject, LUA_CLASS_NAME);

GameScriptAIObject::GameScriptAIObject(AI_OBJECT & ref, bool temp) : m_aiObject{ref}, m_temporary{ temp }
{};

GameScriptAIObject::~GameScriptAIObject() {
	if (m_temporary)
	{
		s_callbackRemoveName(m_aiObject.luaName);
	}
}

void GameScriptAIObject::Register(sol::state* state)
{
	state->new_usertype<GameScriptAIObject>(LUA_CLASS_NAME,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// (@{Position}) position in level
		// @mem pos
		"pos", sol::property(&GameScriptAIObject::GetPos, &GameScriptAIObject::SetPos),

		/// (int) y-axis rotation
		// @mem yRot
		"yRot", sol::property(&GameScriptAIObject::GetYRot, &GameScriptAIObject::SetYRot),

		/// (string) unique string identifier.
		// e.g. "door_back_room" or "cracked_greek_statue"
		// @mem name
		"name", sol::property(&GameScriptAIObject::GetName, &GameScriptAIObject::SetName),

		/// (int) room number
		// @mem room
		"room", sol::property(&GameScriptAIObject::GetRoom, &GameScriptAIObject::SetRoom),

		/// (@{ObjID}) object ID
		// @mem objID
		"objID", sol::property(&GameScriptAIObject::GetObjID, &GameScriptAIObject::SetObjID),

		/// (short) flags
		// @mem flags
		"flags", sol::property(&GameScriptAIObject::GetFlags, &GameScriptAIObject::SetFlags),

		/// (short) trigger flags
		// @mem triggerFlags
		"triggerFlags", sol::property(&GameScriptAIObject::GetTriggerFlags, &GameScriptAIObject::SetTriggerFlags),

		/// (short) box number
		// @mem boxNumber
		"boxNumber", sol::property(&GameScriptAIObject::GetBoxNumber, &GameScriptAIObject::SetBoxNumber)
		);
}

GameScriptPosition GameScriptAIObject::GetPos() const
{
	return GameScriptPosition{ m_aiObject.x, m_aiObject.y, m_aiObject.z };
}

void GameScriptAIObject::SetPos(GameScriptPosition const& pos)
{
	m_aiObject.x = pos.x;
	m_aiObject.y = pos.y;
	m_aiObject.z = pos.z;
}

GAME_OBJECT_ID GameScriptAIObject::GetObjID() const
{
	return m_aiObject.objectNumber;
}

void GameScriptAIObject::SetObjID(GAME_OBJECT_ID objNum)
{
	m_aiObject.objectNumber = objNum;
}

short GameScriptAIObject::GetYRot() const
{
	return m_aiObject.yRot;
}

void GameScriptAIObject::SetYRot(short yRot)
{
	m_aiObject.yRot = yRot;
}

std::string GameScriptAIObject::GetName() const
{
	return m_aiObject.luaName;
}

void GameScriptAIObject::SetName(std::string const & id) 
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

short GameScriptAIObject::GetRoom() const
{
	return m_aiObject.roomNumber;
}

void GameScriptAIObject::SetRoom(short room)
{
	m_aiObject.roomNumber = room;
}

short GameScriptAIObject::GetTriggerFlags() const
{

	return m_aiObject.triggerFlags;
}

void GameScriptAIObject::SetTriggerFlags(short tf)
{
	m_aiObject.triggerFlags = tf;
}

short GameScriptAIObject::GetFlags() const
{
	return m_aiObject.flags;
}

void GameScriptAIObject::SetFlags(short tf)
{
	m_aiObject.flags = tf;
}

short GameScriptAIObject::GetBoxNumber() const
{
	return m_aiObject.boxNumber;
}

void GameScriptAIObject::SetBoxNumber(short bn)
{
	m_aiObject.boxNumber = bn;
}

