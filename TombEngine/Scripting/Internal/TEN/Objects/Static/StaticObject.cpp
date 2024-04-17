#pragma once
#include "framework.h"

#include "Game/effects/debris.h"
#include "Game/Setup.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptAssert.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/Objects/Static/StaticObject.h"
#include "Scripting/Internal/TEN/Rotation/Rotation.h"
#include "Scripting/Internal/TEN/Vec3/Vec3.h"

/// Static object.
// @tenclass Objects.Static
// @pragma nostrip

static auto IndexError = index_error_maker(Static, ScriptReserved_Static);
static auto NewIndexError = newindex_error_maker(Static, ScriptReserved_Static);

Static::Static(StaticObject& staticObj) : _static(staticObj)
{
};

void Static::Register(sol::table& parent)
{
	parent.new_usertype<Static>(
		ScriptReserved_Static,
		sol::no_constructor, // TODO: Ability to spawn new statics could be added later.
		sol::meta_function::index, IndexError,
		sol::meta_function::new_index, NewIndexError,

		ScriptReserved_GetName, &Static::GetName,
		ScriptReserved_GetSlot, &Static::GetSlotID,
		ScriptReserved_GetPosition, &Static::GetPosition,
		ScriptReserved_GetRotation, &Static::GetRotation,
		ScriptReserved_GetScale, &Static::GetScale,
		ScriptReserved_GetColor, &Static::GetColor,
		ScriptReserved_GetHP, &Static::GetHitPoints,
		ScriptReserved_GetSolid, &Static::GetSolid,
		ScriptReserved_GetActive, &Static::GetActive,

		ScriptReserved_SetName, &Static::SetName,
		ScriptReserved_SetSlot, &Static::SetSlotID,
		ScriptReserved_SetPosition, &Static::SetPosition,
		ScriptReserved_SetRotation, &Static::SetRotation,
		ScriptReserved_SetScale, &Static::SetScale,
		ScriptReserved_SetColor, &Static::SetColor,
		ScriptReserved_SetHP, &Static::SetHitPoints,
		ScriptReserved_SetSolid, &Static::SetSolid,

		ScriptReserved_Enable, &Static::Enable,
		ScriptReserved_Disable, &Static::Disable,
		ScriptReserved_Shatter, &Static::Shatter);
}

/// Get this static's unique string identifier.
// @function Static:GetName()
// @treturn string Name.
std::string Static::GetName() const
{
	return _static.Name;
}

/// Get this static's slot ID.
// @function Static:GetSlot()
// @treturn string Slot ID.
int Static::GetSlotID() const
{
	return _static.AssetPtr->ID;
}

/// Get this static's position.
// @function Static:GetPosition()
// @treturn Vec3 Position.
Vec3 Static::GetPosition() const
{
	return Vec3(_static.Pose.Position);
}

/// Get this static's rotation.
// @function Static:GetRotation()
// @treturn Rotation Rotation.
// Doesn't guarantee the returned value will be identical to a value set via SetRotation(),
// only that the angle measures will be mathematically equal. E.g. 90 degrees = -270 degrees = 450 degrees.
Rotation Static::GetRotation() const
{
	return 
	{
		TO_DEGREES(_static.Pose.Orientation.x),
		TO_DEGREES(_static.Pose.Orientation.y),
		TO_DEGREES(_static.Pose.Orientation.z)
	};
}

/// Get this static's scale.
// @function Static:GetScale()
// @treturn float Scale.
float Static::GetScale() const
{
	return _static.Scale;
}

/// Get this static's color.
// @function Static:GetColor()
// @treturn Color Color.
ScriptColor Static::GetColor() const
{
	return ScriptColor(_static.Color);
}

/// Get this static's hit points. Used only with shatterable statics.
// @function Static:GetHP()
// @treturn int Hit points.
int Static::GetHitPoints() const
{
	return _static.HitPoints;
}

/// Get this static's solid collision status.
// @function Static:GetSolid()
// @treturn bool Solid collision status.
bool Static::GetSolid() const
{
	return ((_static.Flags & StaticMeshFlags::SM_SOLID) != 0);
}

/// Get this static's visibility status.
// @function Static:GetActive()
// @treturn bool Visibility status.
bool Static::GetActive() const
{
	return ((_static.Flags & StaticMeshFlags::SM_VISIBLE) != 0);
}

/// Set this static's unique string identifier.
// @function Static:SetName()
// @tparam string name New name.
void Static::SetName(const std::string& name) 
{
	if (!ScriptAssert(!name.empty(), "Cannot set blank name."))
		return;

	// Remove previous name if it exists.
	if (s_callbackSetName(name, _static))
	{
		s_callbackRemoveName(_static.Name);
		_static.Name = name;
	}
	else
	{
		ScriptAssertF(false, "Could not set name {}. Object with identical name may already exist.", name);
		TENLog("Name not set.", LogLevel::Warning, LogConfig::All);
	}
}

/// Set this static's slot ID.
// @function Static:SetSlot()
// @tparam int slot New slot ID.
void Static::SetSlotID(int slotID)
{
	_static.AssetPtr = &GetStaticAsset(slotID);
	_static.IsDirty = true;
}

/// Set this static's position.
// @function Static:SetPosition()
// @tparam Vec3 pos New position.
void Static::SetPosition(const Vec3& pos)
{
	_static.Pose.Position = pos.ToVector3i();
	_static.IsDirty = true;
}

/// Set this static's rotation.
// @function Static:SetRotation()
// @tparam Rotation rot New rotation.
void Static::SetRotation(const Rotation& rot)
{
	_static.Pose.Orientation = rot.ToEulerAngles();
	_static.IsDirty = true;
}

/// Set this static's scale.
// @function Static:SetScale()
// @tparam Scale scale New scale.
void Static::SetScale(float scale)
{
	_static.Scale = scale;
	_static.IsDirty = true;
}

/// Set this static's color.
// @function Static:SetColor()
// @tparam Color color New color.
void Static::SetColor(const ScriptColor& color)
{
	_static.Color = Vector4(color);
	_static.IsDirty = true;
}

/// Set this static's hit points. Used only with shatterable statics.
// @function Static:SetHP()
// @tparam int hisPoints New hit points.
void Static::SetHitPoints(int hitPoints)
{
	_static.HitPoints = hitPoints;
}

/// Set this static's solid collision status.
// @function Static:SetSolid()
// @tparam bool isSolid Solid collision status.
void Static::SetSolid(bool isSolid)
{
	if (isSolid)
	{
		_static.Flags |= StaticMeshFlags::SM_SOLID;
	}
	else
	{
		_static.Flags &= ~StaticMeshFlags::SM_SOLID;
	}
}

/// Enable this static. used in cases where it was previously shattered or manually disabled.
// @function Static:Enable()
void Static::Enable()
{
	_static.Flags |= StaticMeshFlags::SM_VISIBLE;
}

/// Disable this static.
// @function Static:Disable()
void Static::Disable()
{
	_static.Flags &= ~StaticMeshFlags::SM_VISIBLE;
}

/// Shatter this static.
// @function Static:Shatter()
void Static::Shatter()
{
	ShatterObject(nullptr, &_static, -128, _static.RoomNumber, 0);
}
