#pragma once
#include "framework.h"

#include "ScriptAssert.h"
#include "VolumeObject.h"
#include "Vec3/Vec3.h"
#include "Rotation/Rotation.h"
#include "ScriptUtil.h"
#include "ReservedScriptNames.h"
#include "Specific/level.h"

/***
Volumes

@tenclass Objects.Volume
@pragma nostrip
*/

static auto index_error = index_error_maker(Volume, ScriptReserved_Volume);
static auto newindex_error = newindex_error_maker(Volume, ScriptReserved_Volume);

Volume::Volume(TriggerVolume& volume) : m_volume{ volume }
{};

void Volume::Register(sol::table& parent)
{
	parent.new_usertype<Volume>(ScriptReserved_Volume,
		sol::no_constructor, // ability to spawn new ones could be added later
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// Enable the volume.
		// @function Volume:Enable
		ScriptReserved_Enable, &Volume::Enable,

		/// Disable the volume.
		// @function Volume:Disable
		ScriptReserved_Disable, &Volume::Disable,

		/// Determine whether the volume is active or not 
		// @function Volume:GetActive
		// @treturn bool true if the volume is active
		ScriptReserved_GetName, & Volume::GetActive,

		/// Get the volume's position.
		// @function Volume:GetPosition
		// @treturn Vec3 a copy of the static's position
		ScriptReserved_GetPosition, &Volume::GetPos,

		/// Set the volume's position.
		// @function Volume:SetPosition
		// @tparam Vec3 position the new position of the static 
		ScriptReserved_SetPosition, &Volume::SetPos,

		/// Get the volume's rotation.
		// @function Volume:GetRotation
		// @treturn Rotation a copy of the static's rotation
		ScriptReserved_GetRotation, &Volume::GetRot,

		/// Set the volume's rotation.
		// @function Volume:SetRotation
		// @tparam Rotation rotation the static's new rotation
		ScriptReserved_SetRotation, &Volume::SetRot,

		/// Get the volume's scale (separately on all 3 axes).
		// @function Volume:GetScale
		// @treturn Vec3 current static scale
		ScriptReserved_GetScale, &Volume::GetScale,

		/// Set the volume's scale (separately on all 3 axes).
		// @function Volume:SetScale
		// @tparam Vec3 scale the volume's new scale
		ScriptReserved_SetScale, &Volume::SetScale,

		/// Get the volume's unique string identifier.
		// @function Volume:GetName
		// @treturn string the volume's name
		ScriptReserved_GetName, &Volume::GetName,

		/// Set the volume's name (its unique string identifier).
		// @function Volume:SetName
		// @tparam string name The volume's new name
		ScriptReserved_SetName, &Volume::SetName,

		/// Clear activator list for volumes (makes volume trigger everything again)
		// @function Volume:ClearActivators
		ScriptReserved_ClearActivators, &Volume::ClearActivators,

		/// Clear activator list for volumes (makes volume trigger everything again)
		// @function Volume:ClearActivators
		ScriptReserved_IsMoveableInside, &Volume::IsMoveableInside);
}

void Volume::Enable()
{
	m_volume.Enabled = true;
}

void Volume::Disable()
{
	ClearActivators();
	m_volume.Enabled = false;
}

bool Volume::GetActive() const
{
	return m_volume.Enabled;
}

Vec3 Volume::GetPos() const
{
	return Vec3{ (int)m_volume.Box.Center.x, (int)m_volume.Box.Center.y, (int)m_volume.Box.Center.z };
}

void Volume::SetPos(Vec3 const& pos)
{
	m_volume.Box.Center = m_volume.Sphere.Center = Vector3i(pos).ToVector3();
}

Rotation Volume::GetRot() const
{
	auto angles = EulerAngles(m_volume.Box.Orientation);
	return Rotation(TO_DEGREES(angles.x), TO_DEGREES(angles.y), TO_DEGREES(angles.z));
}

void Volume::SetRot(Rotation const& rot)
{
	auto angles = EulerAngles(ANGLE(rot.x), ANGLE(rot.y), ANGLE(rot.z));
	m_volume.Box.Orientation = angles.ToQuaternion();
}

Vec3 Volume::GetScale() const
{
	return Vec3((Vector3)m_volume.Box.Extents);
}

void Volume::SetScale(Vec3 const& scale)
{
	m_volume.Box.Extents = Vector3(scale.x, scale.y, scale.z);
	m_volume.Sphere.Radius = m_volume.Box.Extents.x;
}

std::string Volume::GetName() const
{
	return m_volume.Name;
}

void Volume::SetName(std::string const& name)
{
	if (!ScriptAssert(!name.empty(), "Name cannot be blank. Not setting name."))
	{
		return;
	}

	if (s_callbackSetName(name, m_volume))
	{
		// remove the old name if we have one
		s_callbackRemoveName(m_volume.Name);
		m_volume.Name = name;
	}
	else
	{
		ScriptAssertF(false, "Could not add name {} - does an object with this name already exist?", name);
		TENLog("Name will not be set", LogLevel::Warning, LogConfig::All);
	}
}

void Volume::ClearActivators()
{
	m_volume.StateQueue.clear();
}

bool Volume::IsMoveableInside(Moveable const& moveable)
{
	for (auto& entry : m_volume.StateQueue)
	{
		if (std::holds_alternative<short>(entry.Triggerer))
		{
			short id = std::get<short>(entry.Triggerer);
			auto& mov = std::make_unique<Moveable>(id);

			if (mov.get() == &moveable)
				return true;
		}			
	}

	return false;
}