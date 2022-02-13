#pragma once
#include "frameworkandsol.h"
#include "ScriptAssert.h"
#include "Static.h"
#include "Position/Position.h"
#include "Rotation/Rotation.h"
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

		/// Get the static's position
		// @function GetPosition
		// @treturn Position a copy of the static's position
		ScriptReserved_GetPosition, &Static::GetPos,

		/// Set the static's position
		// @function SetPosition
		// @tparam Position position the new position of the static 
		ScriptReserved_SetPosition, &Static::SetPos,

		/// Get the static's rotation
		// @function GetRotation
		// @treturn Rotation a copy of the static's rotation
		ScriptReserved_GetRotation, &Static::GetRot,

		/// Set the static's rotation
		// @function SetRotation
		// @tparam Rotation The static's new rotation
		ScriptReserved_SetRotation, &Static::SetRot,

		/// (string) unique string identifier.
		// e.g. "my\_vase" or "oldrubble"
		// @mem name
		"name", sol::property(&Static::GetName, &Static::SetName),

		/// (int) static number
		// @mem staticNumber
		"staticNumber", sol::property(&Static::GetStaticNumber, &Static::SetStaticNumber),

		/// Get the static's color
		// @function GetColor
		// @treturn Color a copy of the static's color
		ScriptReserved_GetColor, &Static::GetColor,

		/// Set the static's color
		// @function SetColor
		// @tparam Color color the new color of the static 
		ScriptReserved_SetColor, &Static::SetColor,

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

// This does not guarantee that the returned value will be identical
// to a value written in via SetRot - only that the angle measures
// will be mathematically equal
// (e.g. 90 degrees = -270 degrees = 450 degrees)
Rotation Static::GetRot() const
{
	return Rotation(	int(TO_DEGREES(m_mesh.pos.xRot)) % 360,
						int(TO_DEGREES(m_mesh.pos.yRot)) % 360,
						int(TO_DEGREES(m_mesh.pos.zRot)) % 360);
}

void Static::SetRot(Rotation const& rot)
{
	m_mesh.pos.xRot = FROM_DEGREES(rot.x);
	m_mesh.pos.yRot = FROM_DEGREES(rot.y);
	m_mesh.pos.zRot = FROM_DEGREES(rot.z);
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
