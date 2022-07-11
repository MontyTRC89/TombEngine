#pragma once
#include "framework.h"

#include "ScriptAssert.h"
#include "StaticObject.h"
#include "Vec3/Vec3.h"
#include "Rotation/Rotation.h"
#include "Color/Color.h"
#include "ScriptUtil.h"
#include "ReservedScriptNames.h"
/***
Statics

@tenclass Objects.Static
@pragma nostrip
*/

static auto index_error = index_error_maker(Static, ScriptReserved_Static);
static auto newindex_error = newindex_error_maker(Static, ScriptReserved_Static);

Static::Static(MESH_INFO & ref) : m_mesh{ref}
{};

void Static::Register(sol::table & parent)
{
	parent.new_usertype<Static>(ScriptReserved_Static,
		sol::no_constructor, // ability to spawn new ones could be added later
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// Get the static's position
		// @function Static:GetPosition
		// @treturn Vec3 a copy of the static's position
		ScriptReserved_GetPosition, &Static::GetPos,

		/// Set the static's position
		// @function Static:SetPosition
		// @tparam Vec3 position the new position of the static 
		ScriptReserved_SetPosition, &Static::SetPos,

		/// Get the static's rotation
		// @function Static:GetRotation
		// @treturn Rotation a copy of the static's rotation
		ScriptReserved_GetRotation, &Static::GetRot,

		/// Set the static's rotation
		// @function Static:SetRotation
		// @tparam Rotation rotation the static's new rotation
		ScriptReserved_SetRotation, &Static::SetRot,

		/// Get the static's unique string identifier
		// @function Static:GetName
		// @treturn string the static's name
		ScriptReserved_GetName, &Static::GetName,

		/// Set the static's name (its unique string identifier)
		// e.g. "my\_vase" or "oldrubble"
		// @function Static:SetName
		// @tparam string name The static's new name
		ScriptReserved_SetName, &Static::SetName,

		/// Get the static's slot number (as listed in Tomb Editor and WadTool)
		// @function Static:GetSlot
		// @treturn string the static's slot number
		ScriptReserved_GetSlot, &Static::GetSlot,

		/// Set the static's slot number (as listed in Tomb Editor and WadTool)
		// @function Static:SetSlot
		// @tparam int slot The static's slot number 
		ScriptReserved_SetSlot, &Static::SetSlot,

		/// Get the static's color
		// @function Static:GetColor
		// @treturn Color a copy of the static's color
		ScriptReserved_GetColor, &Static::GetColor,

		/// Set the static's color
		// @function Static:SetColor
		// @tparam Color color the new color of the static 
		ScriptReserved_SetColor, &Static::SetColor);
}

Vec3 Static::GetPos() const
{
	return Vec3{ m_mesh.pos.Position.x, m_mesh.pos.Position.y, m_mesh.pos.Position.z };
}

void Static::SetPos(Vec3 const& pos)
{
	m_mesh.pos.Position.x = pos.x;
	m_mesh.pos.Position.y = pos.y;
	m_mesh.pos.Position.z = pos.z;
}

// This does not guarantee that the returned value will be identical
// to a value written in via SetRot - only that the angle measures
// will be mathematically equal
// (e.g. 90 degrees = -270 degrees = 450 degrees)
Rotation Static::GetRot() const
{
	return {
		static_cast<int>(TO_DEGREES(m_mesh.pos.Orientation.x)) % 360,
		static_cast<int>(TO_DEGREES(m_mesh.pos.Orientation.y)) % 360,
		static_cast<int>(TO_DEGREES(m_mesh.pos.Orientation.z)) % 360
	};
}

void Static::SetRot(Rotation const& rot)
{
	m_mesh.pos.Orientation.x = FROM_DEGREES(rot.x);
	m_mesh.pos.Orientation.y = FROM_DEGREES(rot.y);
	m_mesh.pos.Orientation.z = FROM_DEGREES(rot.z);
}

std::string Static::GetName() const
{
	return m_mesh.luaName;
}

void Static::SetName(std::string const & name) 
{
	if (!ScriptAssert(!name.empty(), "Name cannot be blank. Not setting name."))
	{
		return;
	}

	if (s_callbackSetName(name, m_mesh))
	{
		// remove the old name if we have one
		s_callbackRemoveName(m_mesh.luaName);
		m_mesh.luaName = name;
	}
	else
	{
		ScriptAssertF(false, "Could not add name {} - does an object with this name already exist?", name);
		TENLog("Name will not be set", LogLevel::Warning, LogConfig::All);
	}
}

int Static::GetSlot() const
{
	return m_mesh.staticNumber;
}

void Static::SetSlot(int slot)
{
	m_mesh.staticNumber = slot;
}

ScriptColor Static::GetColor() const
{
	return ScriptColor{ m_mesh.color };
}

void Static::SetColor(ScriptColor const& col)
{
	m_mesh.color = col;
}