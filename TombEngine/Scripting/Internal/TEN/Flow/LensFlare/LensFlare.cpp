#include "framework.h"
#include "Scripting/Internal/TEN/Flow/LensFlare/LensFlare.h"

#include "Objects/game_object_ids.h"
#include "Scripting/Internal/TEN/Rotation/Rotation.h"
#include "Specific\level.h"

/// Represents a lens flare.
//
// @tenclass Flow.LensFlare
// @pragma nostrip

namespace TEN::Scripting
{
	void LensFlare::Register(sol::table& parent)
	{
		using ctors = sol::constructors<
			LensFlare(const Rotation& rot, const ScriptColor& color)>;

		// Register type.
		parent.new_usertype<LensFlare>(
			"LensFlare", ctors(), sol::call_constructor, ctors(),
			"GetEnabled", &LensFlare::GetEnabled,
			"GetSunSpriteID", &LensFlare::GetSunSpriteID,
			"GetRotation", &LensFlare::GetRotation,
			"GetColor", &LensFlare::GetColor,
			"SetSunSpriteID", &LensFlare::SetSunSpriteID,
			"SetRotation", &LensFlare::SetRotation,
			"SetColor", &LensFlare::SetColor);
	}

	/// Create a LensFlare object.
	// @function LensFlare()
	// @tparam Rotation Rotation.
	// @tparam Color Color.
	// @treturn LensFlare A new LensFlare object.
	LensFlare::LensFlare(const Rotation& rot, const ScriptColor& color)
	{
		_isEnabled = true;
		_color = color;
		_rotation = rot;
	}

	/// Get the lens flare's enabled status.
	// @function LensFlare:GetEnabled()
	// @treturn bool Lens flare's enabled status.
	bool LensFlare::GetEnabled() const
	{
		return _isEnabled;
	}

	/// Get the sun's sprite ID.
	// @function LensFlare:GetObjectID()
	// @treturn int Sprite ID.
	int LensFlare::GetSunSpriteID() const
	{
		return _sunSpriteID;
	}

	/// Get the lens flare's euler rotation.
	// @function LensFlare:GetRotation()
	// @treturn Rotation Rotation.
	Rotation LensFlare::GetRotation() const
	{
		return _rotation;
	}

	/// Get the lens flare's color.
	// @function LensFlare:SetColor()
	ScriptColor LensFlare::GetColor() const
	{
		return _color;
	}

	/// Set the lens flare's sun sprite ID.
	// @function LensFlare:SetSunbjectID()
	// @tparam int New sprite ID.
	void LensFlare::SetSunSpriteID(int spriteID)
	{
		// Sprite ID out of range; return early.
		if (spriteID < 0 || g_Level.Sprites.size() > spriteID)
		{
			TENLog("Sub sprite ID out of range.");
			return;
		}

		_sunSpriteID = spriteID;
	}

	/// Set the lens flare's euler rotation.
	// @function LensFlare:SetRotation(Rotation)
	// @tparam Rotation New euler rotation.
	void LensFlare::SetRotation(const Rotation& rot)
	{
		_rotation = rot;
	}

	/// Set the lens flare's color.
	// @function LensFlare:SetColor(Color)
	// @tparam Color New color.
	void LensFlare::SetColor(const ScriptColor& color)
	{
		_color = color;
	}
}
