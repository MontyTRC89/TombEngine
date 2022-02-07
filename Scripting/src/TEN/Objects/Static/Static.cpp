#pragma once
#include "frameworkandsol.h"
#include "ScriptAssert.h"
#include "Static.h"
#include "Position/Position.h"
#include "Color/Color.h"
#include "ScriptUtil.h"
#include "ReservedScriptNames.h"
/***
Mesh info

@tenclass Objects.Static
@pragma nostrip
*/

static auto index_error = index_error_maker(Static, ScriptReserved_Static);
static auto newindex_error = newindex_error_maker(Static, ScriptReserved_Static);

Static::Static(MESH_INFO & ref, bool temp) : m_mesh{ref}, m_temporary{ temp }
{};

Static::~Static() {
	if (m_temporary)
	{
		s_callbackRemoveName(m_mesh.luaName);
	}
}

void Static::Register(sol::table & parent)
{
	parent.new_usertype<Static>(ScriptReserved_Static,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// (@{Position}) position in level
		// @mem pos
		"pos", sol::property(&Static::GetPos, &Static::SetPos),

		/// (int) y-axis rotation
		// @mem yRot
		"yRot", sol::property(&Static::GetRot, &Static::SetRot),

		/// (string) unique string identifier.
		// e.g. "my\_vase" or "oldrubble"
		// @mem name
		"name", sol::property(&Static::GetName, &Static::SetName),

		/// (int) static number
		// @mem staticNumber
		"staticNumber", sol::property(&Static::GetStaticNumber, &Static::SetStaticNumber),

		/// (@{Color}) color of mesh
		// @mem color
		"color", sol::property(&Static::GetColor, &Static::SetColor),

		/// (int) hp
		// @mem HP
		"HP", sol::property(&Static::GetHP, &Static::SetHP)
		);
}

Position Static::GetPos() const
{
	return Position{ m_mesh.pos.xPos, m_mesh.pos.yPos, m_mesh.pos.zPos };
}

void Static::SetPos(Position const& pos)
{
	m_mesh.pos.xPos = pos.x;
	m_mesh.pos.yPos = pos.y;
	m_mesh.pos.zPos = pos.z;
}

int Static::GetRot() const
{
	return m_mesh.pos.yRot;
}

void Static::SetRot(int yRot)
{
	m_mesh.pos.yRot = yRot;
}

std::string Static::GetName() const
{
	return m_mesh.luaName;
}

void Static::SetName(std::string const & id) 
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

int Static::GetStaticNumber() const
{
	return m_mesh.staticNumber;
}

void Static::SetStaticNumber(int staticNumber)
{
	m_mesh.staticNumber = staticNumber;
}

ScriptColor Static::GetColor() const
{
	return ScriptColor{ m_mesh.color };
}

void Static::SetColor(ScriptColor const & col)
{
	m_mesh.color = col;
}

int Static::GetHP() const
{
	return m_mesh.hitPoints;
}

void Static::SetHP(int hp)
{
	m_mesh.hitPoints = hp;
}
