#include "framework.h"
#include "Scripting/Internal/TEN/Flow/LensFlare/LensFlare.h"

#include "Objects/game_object_ids.h"
#include "Scripting/Internal/TEN/Types/Color/Color.h"
#include "Scripting/Internal/TEN/Types/Rotation/Rotation.h"
#include "Specific/level.h"

using namespace TEN::Scripting::Types;

/// Represents a global lens flare (not to be confused with the lens flare object).
//
// @tenprimitive Flow.LensFlare
// @pragma nostrip

namespace TEN::Scripting
{
	void LensFlare::Register(sol::table& parent)
	{
		using ctors = sol::constructors<
			LensFlare(float, float, const ScriptColor&)>;

		// Register type.
		parent.new_usertype<LensFlare>(
			"LensFlare",
			ctors(), sol::call_constructor, ctors(),

			"GetSunSpriteID", &LensFlare::GetSunSpriteID,
			"GetPitch", &LensFlare::GetPitch,
			"GetYaw", &LensFlare::GetYaw,
			"GetColor", &LensFlare::GetColor,
			"GetEnabled", &LensFlare::GetEnabledStatus,

			"SetSunSpriteID", &LensFlare::SetSunSpriteID,
			"SetPitch", &LensFlare::SetPitch,
			"SetYaw", &LensFlare::SetYaw,
			"SetColor", &LensFlare::SetColor);
	}

	/// Create a LensFlare object.
	// @function LensFlare
	// @tparam float Pitch angle in degrees.
	// @tparam float Yaw angle in degrees.
	// @tparam Color Color.
	// @treturn LensFlare A new LensFlare object.
	LensFlare::LensFlare(float pitch, float yaw, const ScriptColor& color)
	{
		_isEnabled = true;
		_color = color;
		_rotation = Rotation(pitch, yaw, 0.0f);
	}

	/// Get this lens flare's sun sprite ID.
	// @function LensFlare:GetSunSpriteID
	// @treturn int Sprite ID.
	int LensFlare::GetSunSpriteID() const
	{
		return _sunSpriteID;
	}

	/// Get this lens flare's pitch angle in degrees.
	// @function LensFlare:GetPitch
	// @treturn float Pitch angle in degrees.
	float LensFlare::GetPitch() const
	{
		return _rotation.x;
	}
	
	/// Get this lens flare's yaw angle in degrees.
	// @function LensFlare:GetYaw
	// @treturn float Yaw angle in degrees.
	float LensFlare::GetYaw() const
	{
		return _rotation.y;
	}

	/// Get the lens flare's color.
	// @function LensFlare:GetColor
	ScriptColor LensFlare::GetColor() const
	{
		return _color;
	}

	/// Get this lens flare's enabled status.
	// @function LensFlare:GetEnabled
	// @treturn bool Enabled status. __true: enabled__, __false: disabled__
	bool LensFlare::GetEnabledStatus() const
	{
		return _isEnabled;
	}

	/// Set this lens flare's sun sprite ID.
	// @function LensFlare:SetSunSpriteID
	// @tparam int New sun sprite ID.
	void LensFlare::SetSunSpriteID(int spriteID)
	{
		// Sprite ID out of range; return early.
		if (spriteID < 0 || g_Level.Sprites.size() > spriteID)
		{
			TENLog("Sun sprite ID out of range.");
			return;
		}

		_sunSpriteID = spriteID;
	}

	/// Set this lens flare's pitch angle.
	// @function LensFlare:SetPitch
	// @tparam float New pitch angle in degrees.
	void LensFlare::SetPitch(float pitch)
	{
		_rotation.x = pitch;
	}

	/// Set this lens flare's yaw angle.
	// @function LensFlare:SetYaw
	// @tparam float New yaw angle in degrees.
	void LensFlare::SetYaw(float yaw)
	{
		_rotation.y = yaw;
	}
	
	/// Set this lens flare's color.
	// @function LensFlare:SetColor
	// @tparam Color New color.
	void LensFlare::SetColor(const ScriptColor& color)
	{
		_color = color;
	}
}
