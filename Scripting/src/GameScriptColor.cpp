#include "frameworkandsol.h"
#include "GameScriptColor.h"
#include <cmath>

/***
An RGBA or RGB color.
Components are specified in bytes; all values are clamped to [0, 255].

@miscclass Color
@pragma nostrip
*/

static byte FloatComponentToByte(float v)
{
	//todo look into what these actually do AND TEST THEM
	//todo like, see if these are actually not undefined or some shit
	auto lval = std::lroundf((v / 2.0f) * 255.0f);
	return static_cast<byte>(lval);
}

static float ByteComponentToFloat(byte b)
{
	//todo look into what these actually do AND TEST THEM
	//todo like, see if these are actually not undefined or some shit
	float f = b;
	f = (f / 255.0f) * 2.0f;
	return f;
}

void GameScriptColor::Register(sol::state* state)
{
	state->new_usertype<GameScriptColor>("Color",
		sol::constructors<GameScriptColor(byte, byte, byte), GameScriptColor(byte, byte, byte, byte)>(),
		sol::meta_function::to_string, &GameScriptColor::ToString,

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

GameScriptColor::GameScriptColor(Vector3 const& col) :
	r(FloatComponentToByte(col.x)),
	g(FloatComponentToByte(col.y)),
	b(FloatComponentToByte(col.z))
{
}

GameScriptColor::GameScriptColor(Vector4 const& col) :
	r(FloatComponentToByte(col.x)),
	g(FloatComponentToByte(col.y)),
	b(FloatComponentToByte(col.z)),
	a(FloatComponentToByte(col.w))
{
}

GameScriptColor::GameScriptColor(D3DCOLOR col)
{
	b = col & 0xFF;
	col >>= 8;
	g = col & 0xFF;
	col >>= 8;
	r = col & 0xFF;
	col >>= 8;
	a = col & 0xFF;
}

GameScriptColor::operator Vector3() const
{
	return Vector3{ ByteComponentToFloat(r), ByteComponentToFloat(g), ByteComponentToFloat(b) };
}

GameScriptColor::operator Vector4() const
{
	return Vector4{ ByteComponentToFloat(r), ByteComponentToFloat(g), ByteComponentToFloat(b), ByteComponentToFloat(a) };
}

// D3DCOLOR is 32 bits and is layed out as ARGB.
GameScriptColor::operator D3DCOLOR() const
{	
	D3DCOLOR col = a;
	col <<= 8;
	col += r;
	col <<= 8;
	col += g;
	col <<= 8;
	col += b;

	return col;
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
