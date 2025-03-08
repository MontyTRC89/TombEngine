#include "framework.h"
#include "Scripting/Internal/TEN/Flow/Horizon/Horizon.h"

#include "Objects/game_object_ids.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/TEN/Types/Color/Color.h"
#include "Scripting/Internal/TEN/Types/Rotation/Rotation.h"

namespace TEN::Scripting
{
	/// Represents a horizon.
	//
	// @tenprimitive Flow.Horizon
	// @pragma nostrip

	void Horizon::Register(sol::table& parent)
	{
		using ctors = sol::constructors<
			Horizon(GAME_OBJECT_ID),
			Horizon(bool)>;

		// Register type.
		parent.new_usertype<Horizon>(
			ScriptReserved_Horizon,
			ctors(), sol::call_constructor, ctors(),

			// Getters
			ScriptReserved_HorizonGetEnabled, &Horizon::GetEnabled,
			ScriptReserved_HorizonGetObjectID, &Horizon::GetObjectID,
			ScriptReserved_HorizonGetPosition, &Horizon::GetPosition,
			ScriptReserved_HorizonGetRotation, &Horizon::GetRotation,
			ScriptReserved_HorizonGetTransparency, &Horizon::GetTransparency,

			// Setters
			ScriptReserved_HorizonSetEnabled, &Horizon::SetEnabled,
			ScriptReserved_HorizonSetObjectID, &Horizon::SetObjectID,
			ScriptReserved_HorizonSetPosition, &Horizon::SetPosition,
			ScriptReserved_HorizonSetRotation, &Horizon::SetRotation,
			ScriptReserved_HorizonSetTransparency, &Horizon::SetTransparency);
	}

	/// Create a horizon object.
	// @function Horizon
	// @tparam Objects.ObjID objectID Object ID for the horizon to use.
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
	// @tparam[opt] bool noInterpolation Disable interpolation with the previous frame's position. __default: false__
	void Horizon::SetPosition(const Vec3& pos, TypeOrNil<bool> noInterpolation)
	{
		bool convertedDisableInterp = ValueOr<bool>(noInterpolation, false);

		_prevPosition = convertedDisableInterp ? pos : _position;
		_position = pos;
	}

	/// Set the horizon's rotation.
	// @function SetRotation
	// @tparam Rotation rot New rotation.
	// @tparam[opt] bool noInterpolation Disable interpolation with the previous frame's rotation. __default: false__
	void Horizon::SetRotation(const Rotation& rot, TypeOrNil<bool> noInterpolation)
	{
		bool convertedDisableInterp = ValueOr<bool>(noInterpolation, false);

		_prevRotation = convertedDisableInterp ? rot : _rotation;
		_rotation = rot;
	}

	/// Set the horizon's transparency.
	// @function SetTransparency
	// @tparam float transparency New transparency alpha.
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
