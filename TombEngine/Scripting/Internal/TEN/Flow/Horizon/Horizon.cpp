#include "framework.h"
#include "Scripting/Internal/TEN/Flow/Horizon/Horizon.h"

#include "Objects/game_object_ids.h"
#include "Scripting/Internal/TEN/Types/Color/Color.h"
#include "Scripting/Internal/TEN/Types/Rotation/Rotation.h"

/// Represents a horizon.
//
// @tenprimitive Flow.Horizon
// @pragma nostrip

namespace TEN::Scripting
{
	void Horizon::Register(sol::table& parent)
	{
		using ctors = sol::constructors<Horizon(GAME_OBJECT_ID), Horizon(bool)>;

		// Register type.
		parent.new_usertype<Horizon>(
			"Horizon",
			ctors(), sol::call_constructor, ctors(),

			"GetEnabled",		&Horizon::GetEnabled,
			"GetObjectID",		&Horizon::GetObjectID,
			"GetPosition",		&Horizon::GetPosition,
			"GetRotation",		&Horizon::GetRotation,
			"GetTransparency",	&Horizon::GetTransparency,

			"SetEnabled",		&Horizon::SetEnabled,
			"SetObjectID",		&Horizon::SetObjectID,
			"SetPosition",		&Horizon::SetPosition,
			"SetRotation",		&Horizon::SetRotation,
			"SetTransparency",	&Horizon::SetTransparency);
	}

	/// Create a horizon object with specified object ID.
	// @function Horizon
	// @tparam Objects.ObjID object ID for horizon to use.
	// @treturn Horizon A new Horizon object.
	Horizon::Horizon(GAME_OBJECT_ID objectID)
	{
		_enabled = true;
		_objectID = objectID;
	}

	// Legacy constructor to maintain backwards compatibility.
	Horizon::Horizon(bool enabled)
	{
		_enabled = enabled;
	}

	/// Get the horizon's enabled state.
	// @function GetEnabled
	// @treturn bool Enabled state.
	bool Horizon::GetEnabled() const
	{
		return _enabled;
	}

	/// Get the horizon's slot object ID.
	// @function GetObjectID
	// @treturn Objects.ObjID Object ID.
	GAME_OBJECT_ID Horizon::GetObjectID() const
	{
		return _objectID;
	}

	/// Get the horizon's world position.
	// @function GetPosition
	// @treturn Vec3 Position.
	const Vec3 Horizon::GetPosition() const
	{
		return _position;
	}

	/// Get the horizon's rotation.
	// @function GetRotation
	// @treturn Rotation Rotation.
	const Rotation Horizon::GetRotation() const
	{
		return _rotation;
	}

	/// Get the horizon's transparency.
	// @function GetTransparency
	// @treturn float Transparency.
	const float Horizon::GetTransparency() const
	{
		return _transparency;
	}

	/// Set the horizon's enabled state.
	// @function SetEnabled
	// @tparam bool enabled New enabled state.
	void Horizon::SetEnabled(bool value)
	{
		_enabled = value;
	}

	/// Set the horizon's object ID.
	// @function SetObjectID
	// @tparam Objects.ObjID objectID Object ID.
	void Horizon::SetObjectID(GAME_OBJECT_ID objectID)
	{
		_objectID = objectID;
	}

	/// Set the horizon's world position.
	// @function SetPosition
	// @tparam Vec3 pos New world position.
	// @tparam[opt] bool disableInterpolation Disable interpolation with old position.
	void Horizon::SetPosition(const Vec3& pos, sol::optional<bool> disableInterpolation)
	{
		_prevPosition = (disableInterpolation.has_value() && disableInterpolation.value()) ? pos : _position;
		_position = pos;
	}

	/// Set the horizon's rotation.
	// @function SetRotation
	// @tparam Rotation rot New rotation.
	// @tparam[opt] bool disableInterpolation Disable interpolation with old rotation.
	void Horizon::SetRotation(const Rotation& rot, sol::optional<bool> disableInterpolation)
	{
		_prevRotation = (disableInterpolation.has_value() && disableInterpolation.value()) ? rot : _rotation;
		_rotation = rot;
	}

	/// Set the horizon's transparency.
	// @function SetTransparency
	// @tparam bool transparency Transparency.
	void Horizon::SetTransparency(float value)
	{
		_transparency = value;
	}

	const Vec3 Horizon::GetPrevPosition() const
	{
		return _prevPosition;
	}

	const Rotation Horizon::GetPrevRotation() const
	{
		return _prevRotation;
	}
}