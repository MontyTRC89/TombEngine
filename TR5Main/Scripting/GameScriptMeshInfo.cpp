#pragma once
#include "framework.h"
#include "ScriptAssert.h"
#include "GameScriptMeshInfo.h"
#include "GameScriptPosition.h"
#include "GameScriptColor.h"
#include "ScriptUtil.h"
#include <sol.hpp>
/***
Mesh info

@entityclass MeshInfo
@pragma nostrip
*/

constexpr auto LUA_CLASS_NAME{ "MeshInfo" };

static auto index_error = index_error_maker(GameScriptMeshInfo, LUA_CLASS_NAME);
static auto newindex_error = newindex_error_maker(GameScriptMeshInfo, LUA_CLASS_NAME);

GameScriptMeshInfo::GameScriptMeshInfo(MESH_INFO & ref, bool temp) : m_mesh{ref}, m_temporary{ temp }
{};

GameScriptMeshInfo::~GameScriptMeshInfo() {
	if (m_temporary)
	{
		s_callbackRemoveName(m_mesh.luaName);
	}
}

void GameScriptMeshInfo::Register(sol::state* state)
{
	state->new_usertype<GameScriptMeshInfo>(LUA_CLASS_NAME,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// (@{Position}) position in level
		// @mem pos
		"pos", sol::property(&GameScriptMeshInfo::GetPos, &GameScriptMeshInfo::SetPos),

		/// (int) y-axis rotation
		// @mem yRot
		"yRot", sol::property(&GameScriptMeshInfo::GetRot, &GameScriptMeshInfo::SetRot),

		/// (string) unique string identifier.
		// e.g. "my\_vase" or "oldrubble"
		// @mem name
		"name", sol::property(&GameScriptMeshInfo::GetName, &GameScriptMeshInfo::SetName),

		/// (int) static number
		// @mem staticNumber
		"staticNumber", sol::property(&GameScriptMeshInfo::GetStaticNumber, &GameScriptMeshInfo::SetStaticNumber),

		/// (@{Color}) color of mesh
		// @mem color
		"color", sol::property(&GameScriptMeshInfo::GetColor, &GameScriptMeshInfo::SetColor),

		/// (int) hp
		// @mem HP
		"HP", sol::property(&GameScriptMeshInfo::GetHP, &GameScriptMeshInfo::SetHP)
		);
}

GameScriptPosition GameScriptMeshInfo::GetPos() const
{
	return GameScriptPosition{ m_mesh.pos.Position.x, m_mesh.pos.Position.y, m_mesh.pos.Position.z };
}

void GameScriptMeshInfo::SetPos(GameScriptPosition const& pos)
{
	m_mesh.pos.Position.x = pos.x;
	m_mesh.pos.Position.y = pos.y;
	m_mesh.pos.Position.z = pos.z;
}

int GameScriptMeshInfo::GetRot() const
{
	return m_mesh.pos.Orientation.GetY();
}

void GameScriptMeshInfo::SetRot(int yRot)
{
	m_mesh.pos.Orientation.y = yRot;
}

std::string GameScriptMeshInfo::GetName() const
{
	return m_mesh.luaName;
}

void GameScriptMeshInfo::SetName(std::string const & id) 
{
	ScriptAssert(!id.empty(), "Name cannot be blank", ERROR_MODE::TERMINATE);

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
	return m_mesh.HitPoints;
}

void GameScriptMeshInfo::SetHP(int hp)
{
	m_mesh.HitPoints = hp;
}
