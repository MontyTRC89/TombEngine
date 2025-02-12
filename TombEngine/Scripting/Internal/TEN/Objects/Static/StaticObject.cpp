#include "framework.h"

#include "Game/effects/debris.h"
#include "Objects/game_object_ids.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/ScriptAssert.h"
#include "Scripting/Internal/TEN/Objects/Static/StaticObject.h"
#include "Scripting/Internal/TEN/Types/Color/Color.h"
#include "Scripting/Internal/TEN/Types/Rotation/Rotation.h"
#include "Scripting/Internal/TEN/Types/Vec3/Vec3.h"

using namespace TEN::Scripting::Types;

/// Represents a static object in the game world.
//
// @tenclass Objects.Static
// @pragma nostrip

namespace TEN::Scripting
{
	static auto IndexError = IndexErrorMaker(Static, ScriptReserved_Static);
	static auto NewIndexError = NewIndexErrorMaker(Static, ScriptReserved_Static);

	void Static::Register(sol::table& parent)
	{
		// Register type.
		parent.new_usertype<Static>(
			ScriptReserved_Static,
			sol::no_constructor, // TODO: Add feature to spawn statics.
			sol::meta_function::index, IndexError,
			sol::meta_function::new_index, NewIndexError,

			// Getters
			ScriptReserved_StaticGetName, &Static::GetName,
			ScriptReserved_StaticGetObjectId, &Static::GetObjectId,
			ScriptReserved_StaticGetPosition, &Static::GetPosition,
			ScriptReserved_StaticGetRotation, &Static::GetRotation,
			ScriptReserved_StaticGetScale, &Static::GetScale,
			ScriptReserved_StaticGetColor, &Static::GetColor,
			ScriptReserved_StaticGetHitPoints, &Static::GetHitPoints,
			ScriptReserved_StaticGetActive, &Static::GetActiveStatus, // TODO: Deprecate. Rename Lua func to GetActiveStatus.
			ScriptReserved_StaticGetSolid, &Static::GetSolidStatus, // TODO: Deprecate. Rename Lua func to GetSolidStatus.

			// Setters
			ScriptReserved_StaticSetName, &Static::SetName,
			ScriptReserved_StaticSetObjectId, &Static::SetObjectId,
			ScriptReserved_StaticSetPosition, &Static::SetPosition,
			ScriptReserved_StaticSetRotation, &Static::SetRotation,
			ScriptReserved_StaticSetScale, sol::overload(
				(void(Static::*)(const Vec3&))(&Static::SetScale),
				(void(Static::*)(float))(&Static::SetScale)), // COMPATIBILITY
			ScriptReserved_StaticSetColor, &Static::SetColor,
			ScriptReserved_StaticSetHitPoints, &Static::SetHitPoints,
			ScriptReserved_StaticSetSolid, &Static::SetSolidStatus, // TODO: Deprecate. Rename Lua func to SetSolidStatus.
			
			// Utilities
			ScriptReserved_StaticEnable, &Static::Enable,
			ScriptReserved_StaticDisable, &Static::Disable,
			ScriptReserved_StaticShatter, &Static::Shatter,
			
			// COMPATIBILITY
			"GetSlot", &Static::GetObjectId,
			"GetHP", &Static::GetHitPoints,
			"SetSlot", &Static::SetObjectId,
			"SetHP", &Static::SetHitPoints);
	}

	Static::Static(MESH_INFO& staticObj) :
		_static(staticObj)
	{
	};

	/// Get this static's unique string identifier.
	// @function Static:GetName
	// @treturn string Name string.
	std::string Static::GetName() const
	{
		return _static.Name;
	}

	/// Get this static's object ID.
	// @function Static:GetObjectID
	// @treturn Objects.ObjID Object ID.
	GAME_OBJECT_ID Static::GetObjectId() const
	{
		return _static.ObjectId;
	}

	/// Get this static's world position.
	// @function Static:GetPosition
	// @treturn Vec3 World position.
	Vec3 Static::GetPosition() const
	{
		return Vec3(_static.Transform.Position);
	}

	/// Get this static's world rotation.
	// @function Static:GetRotation
	// @treturn Rotation World rotation.
	Rotation Static::GetRotation() const
	{
		return Rotation(_static.Transform.Orientation);
	}

	/// Get this static's world scale.
	// @function Static:GetScale
	// @treturn Vec3 World scale.
	Vec3 Static::GetScale() const
	{
		return Vec3(_static.Transform.Scale);
	}

	/// Get this static's color.
	// @function Static:GetColor
	// @treturn Color Color.
	ScriptColor Static::GetColor() const
	{
		return ScriptColor(_static.Color);
	}

	/// Get this static's hit points. Used only with shatterable statics.
	// @function Static:GetHitPoints
	// @treturn int Hit points.
	int Static::GetHitPoints() const
	{
		return _static.HitPoints;
	}

	/// Get this static's visibility status.
	// @function Static:GetActive
	// @treturn bool Status.  __true: visible__, __false: invisible__
	bool Static::GetActiveStatus() const
	{
		return ((_static.Flags & StaticMeshFlags::SM_VISIBLE) != 0);
	}

	/// Get this static's solid collision status.
	// @function Static:GetSolid
	// @treturn bool Solid Status. __true: solid__, __false: soft__
	bool Static::GetSolidStatus() const
	{
		return ((_static.Flags & StaticMeshFlags::SM_SOLID) != 0);
	}

	/// Set this static's unique identifier string.
	// @function Static:SetName
	// @tparam string name New name.
	void Static::SetName(const std::string& name)
	{
		if (!ScriptAssert(!name.empty(), "Name cannot be blank. Not setting name."))
			return;

		if (_callbackSetName(name, _static))
		{
			_callbackRemoveName(_static.Name);
			_static.Name = name;
		}
		else
		{
			ScriptAssertF(false, "Could not add name {} - an object with this name may already exist.", name);
			TENLog("Name will not be set.", LogLevel::Warning, LogConfig::All);
		}
	}

	/// Set this static's object ID.
	// @function Static:SetObjectID
	// @tparam Objects.ObjID slotID New object ID.
	void Static::SetObjectId(GAME_OBJECT_ID objectId)
	{
		_static.ObjectId = objectId;
		_static.Dirty = true;
	}

	/// Set this static's world position.
	// @function Static:SetPosition
	// @tparam Vec3 pos New world position.
	void Static::SetPosition(const Vec3& pos)
	{
		_static.Transform.Position = pos.ToVector3i();
		_static.Dirty = true;
	}

	/// Set this static's rotation.
	// @function Static:SetRotation
	// @tparam Rotation rot New rotation.
	void Static::SetRotation(const Rotation& rot)
	{
		_static.Transform.Orientation = rot.ToEulerAngles();
		_static.Dirty = true;
	}

	/// Set this static's world scale.
	// @function Static:SetScale
	// @tparam Vec3 scale New world scale.
	void Static::SetScale(const Vec3& scale)
	{
		_static.Transform.Scale = scale.ToVector3();
		_static.Dirty = true;
	}

	/// Set this static's color.
	// @function Static:SetColor
	// @tparam Color color New color.
	void Static::SetColor(ScriptColor const& col)
	{
		_static.Color = col;
		_static.Dirty = true;
	}

	/// Set this static's hit points. Used only with shatterable statics.
	// @function Static:SetHitPoints
	// @tparam int hitPoints New hit points.
	void Static::SetHitPoints(int hitPoints)
	{
		_static.HitPoints = hitPoints;
	}

	/// Set this static's solid collision status.
	// @function Static:SetSolid
	// @tparam bool status New status. __true: solid__, __false: soft__
	void Static::SetSolidStatus(bool status)
	{
		if (status)
		{
			_static.Flags |= StaticMeshFlags::SM_SOLID;
		}
		else
		{
			_static.Flags &= ~StaticMeshFlags::SM_SOLID;
		}
	}

	/// Enable this static. Used when previously shattered disabled manually.
	// @function Static:Enable
	void Static::Enable()
	{
		_static.Flags |= StaticMeshFlags::SM_VISIBLE;
	}

	/// Disable this static.
	// @function Static:Disable
	void Static::Disable()
	{
		_static.Flags &= ~StaticMeshFlags::SM_VISIBLE;
	}

	/// Shatter this static.
	// @function Static:Shatter
	void Static::Shatter()
	{
		ShatterObject(nullptr, &_static, -128, _static.RoomNumber, 0);
	}

	void Static::SetScale(float scale)
	{
		_static.Transform.Scale = Vector3(scale);
		_static.Dirty = true;
	}
}
