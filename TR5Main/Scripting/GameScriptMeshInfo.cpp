#pragma once
#include "framework.h"
#include "GameScriptMeshInfo.h"
#include "GameScriptPosition.h"
#include "GameScriptColor.h"
#include <sol.hpp>
/***
Mesh info

@classmod MeshInfo
@pragma nostrip
*/

extern bool const WarningsAsErrors;

constexpr auto LUA_CLASS_NAME{ "MeshInfo" };

static auto index_error = index_error_maker(GameScriptMeshInfo, LUA_CLASS_NAME);

void GameScriptMeshInfo::Register(sol::state* state)
{
	state->new_usertype<GameScriptMeshInfo>(LUA_CLASS_NAME,
		sol::meta_function::index, index_error,

		/// (@{Position}) position in level
		// @mem pos
		"pos", sol::property(&GameScriptMeshInfo::GetPos, &GameScriptMeshInfo::SetPos),

		/// (int) y-axis rotation
		// @mem yRot
		"yRot", sol::property(&GameScriptMeshInfo::GetRot, &GameScriptMeshInfo::SetRot),

		/// (string) unique string identifier.
		// e.g. "door_back_room" or "cracked_greek_statue"
		// @mem name
		"name", sol::property(&GameScriptMeshInfo::GetName, &GameScriptMeshInfo::SetName),

		/// (int) static number
		// @mem staticNumber
		"staticNumber", sol::property(&GameScriptMeshInfo::GetStaticNumber, &GameScriptMeshInfo::SetStaticNumber),

		/// (@{Color}) position in level
		// @mem color
		"color", sol::property(&GameScriptMeshInfo::GetColor, &GameScriptMeshInfo::SetColor),

		/// (int) hp
		// @mem HP
		"HP", sol::property(&GameScriptMeshInfo::GetHP, &GameScriptMeshInfo::SetHP),

		/// (string) name
		// @mem name
		"pos", sol::property(&GameScriptMeshInfo::GetName, &GameScriptMeshInfo::SetName)
		);
}

GameScriptPosition GameScriptMeshInfo::GetPos() const
{
	return GameScriptPosition{ m_mesh.x, m_mesh.y, m_mesh.z };
}

void GameScriptMeshInfo::SetPos(GameScriptPosition const& pos)
{
	m_mesh.x = pos.x;
	m_mesh.y = pos.y;
	m_mesh.z = pos.z;
}

int GameScriptMeshInfo::GetRot() const
{
	return m_mesh.yRot;
}

void GameScriptMeshInfo::SetRot(int yRot)
{
	m_mesh.yRot = yRot;
}

std::string GameScriptMeshInfo::GetName() const
{
	return m_mesh.luaName;
}

void GameScriptMeshInfo::SetName(std::string const & id) 
{
	if (id.empty() && WarningsAsErrors)
		throw std::runtime_error("Name cannot be blank");

	// remove the old name if we have one
	s_callbackRemoveName(m_mesh.luaName);

	// un-register any other objects using this name.
	// maybe we should throw an error if another object
	// already uses the name...
	s_callbackRemoveName(id);
	m_mesh.luaName = id;
	// todo add error checking
	s_callbackSetName(id, m_mesh);
}

int GameScriptMeshInfo::GetStaticNumber() const
{
	return m_mesh.staticNumber;
}

void GameScriptMeshInfo::SetStaticNumber(int staticNumber)
{
	m_mesh.staticNumber = staticNumber;
}

GameScriptColor GameScriptMeshInfo::GetColor() const
{

	return GameScriptColor{ m_mesh.color };
}

void GameScriptMeshInfo::SetColor(GameScriptColor const & col)
{
	m_mesh.color = col;
}

int GameScriptMeshInfo::GetHP() const
{
	return m_mesh.hitPoints;
}

void GameScriptMeshInfo::SetHP(int hp)
{
	m_mesh.hitPoints = hp;
}
