#include "framework.h"
#include "LensFlare.h"
#include <Specific\level.h>

/***
LensFlare

@tenclass Flow.LensFlare
@pragma nostrip
*/

void LensFlare::Register(sol::table& parent)
{
	using ctors = sol::constructors<LensFlare(Vec2 const&, ScriptColor const&)>;
	parent.new_usertype<LensFlare>("LensFlare",
		ctors(),
		sol::call_constructor, ctors(),

		/// (@{Color}) RGB lens flare color
		//@mem lensFlareColor
		"color", sol::property(&LensFlare::GetColor, &LensFlare::SetColor),

		/*** (@{Vec2}) Lens flare orientation.

		This is the position of the lens flare in the sky. The X value is the horizontal position, and the Y value is the vertical position. Angles must be specified in degrees.

		@mem lensFlarePosition*/
		"position", sol::property(&LensFlare::GetPosition, &LensFlare::SetPosition)
	);
}

/***
@tparam Vec2 yawPitchInDegrees Position of the lens flare (yaw and pitch) in degrees
@tparam Color color RGB color
@treturn LensFlare A lens flare object.
@function LensFlare
*/
LensFlare::LensFlare(Vec2 const& yawPitchInDegrees, ScriptColor const& col)
{
	SetColor(col);
	SetPosition(yawPitchInDegrees);
	Enabled = true;
}

void LensFlare::SetColor(ScriptColor const& col)
{
	R = col.GetR();
	G = col.GetG();
	B = col.GetB();
}


ScriptColor LensFlare::GetColor() const
{
	return ScriptColor{ R, G, B };
}

void LensFlare::SetPosition(Vec2 const& yawPitchInDegrees)
{
	Yaw = yawPitchInDegrees.x;
	Pitch = yawPitchInDegrees.y;
}


Vec2 LensFlare::GetPosition() const
{
	return Vec2{ Yaw, Pitch };
}

bool LensFlare::GetEnabled() const
{
	return Enabled;
}

void LensFlare::SetSunSpriteID(int const& spriteIndex)
{
	assertion(spriteIndex >= 0 && spriteIndex < g_Level.Sprites.size(), "Sprite Index must be in a valid range");

	SunSpriteID = spriteIndex;
}

int LensFlare::GetSunSpriteID() const
{
	return SunSpriteID;
}