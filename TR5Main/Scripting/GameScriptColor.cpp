#include "framework.h"
#include "GameScriptColor.h"

/***
An RGBA or RGB color.
Components are specified in bytes; all values are clamped to [0, 255].

@classmod Color
@pragma nostrip
*/

void GameScriptColor::Register(sol::state* state)
{
	state->new_usertype<GameScriptColor>("Color",
		sol::constructors<GameScriptColor(byte, byte, byte), GameScriptColor(byte, byte, byte, byte)>(),

/// (int) red component
//@mem r
		"r", sol::property(&GameScriptColor::GetR, &GameScriptColor::SetR),

/// (int) green component
//@mem g
		"g", sol::property(&GameScriptColor::GetG, &GameScriptColor::SetG),

/// (int) blue component
//@mem b
		"b", sol::property(&GameScriptColor::GetB, &GameScriptColor::SetB),

/// (int) alpha component (255 is opaque, 0 is invisible)
//@mem a
		"a", sol::property(&GameScriptColor::GetA, &GameScriptColor::SetA)
	);
}

/*** 
@int R red component
@int G green component
@int B blue component
@return A Color object.
@function Color.new
*/
GameScriptColor::GameScriptColor(byte r, byte g, byte b)
{
	SetR(r);
	SetG(g);
	SetB(b);
}

/*** 
@int R red component
@int G green component
@int B blue component
@int A alpha component (255 is opaque, 0 is invisible)
@return A Color object.
@function Color.new
*/
GameScriptColor::GameScriptColor(byte r, byte g, byte b, byte a) : GameScriptColor(r, g, b)
{
	SetA(a);
}

GameScriptColor::GameScriptColor(Vector3 const& col) : GameScriptColor{ col.x, col.y, col.z } {}

GameScriptColor::GameScriptColor(Vector4 const& col) : GameScriptColor{ col.x, col.y, col.z, col.w } {}

GameScriptColor::operator Vector3() const
{
	return Vector3{ float(r), float(g), float(b) };
}

GameScriptColor::operator Vector4() const
{
	return Vector4{ float(r), float(g), float(b), float(a) };
}

byte GameScriptColor::GetR() const
{
	return r;
}

void GameScriptColor::SetR(byte v)
{
	r = std::clamp<byte>(v, 0, 255);
}

byte GameScriptColor::GetG() const
{
	return g;
}

void GameScriptColor::SetG(byte v)
{
	g = std::clamp<byte>(v, 0, 255);
}

byte GameScriptColor::GetB() const
{
	return b;
}

void GameScriptColor::SetB(byte v)
{
	b = std::clamp<byte>(v, 0, 255);
}

byte GameScriptColor::GetA() const
{
	return a;
}

void GameScriptColor::SetA(byte v)
{
	a = std::clamp<byte>(v, 0, 255);
}

/***
@tparam Color color this color
@treturn string A string showing the r, g, b, and a values of the color
@function __tostring
*/
std::string GameScriptColor::ToString() const
{
	return "{" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) +  ", " + std::to_string(a) + "}";
}
