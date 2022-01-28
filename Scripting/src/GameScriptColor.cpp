#include "frameworkandsol.h"
#include "GameScriptColor.h"
#include <cmath>

/***
An RGBA or RGB color.
Components are specified in bytes; all values are clamped to [0, 255].

@miscclass Color
@pragma nostrip
*/

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
GameScriptColor::GameScriptColor(byte r, byte g, byte b) :
m_color(r, g, b)
{
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
	m_color(col)
{
}

GameScriptColor::GameScriptColor(Vector4 const& col) :
	m_color(col)
{
}

GameScriptColor::GameScriptColor(D3DCOLOR col) : 
	m_color(col)
{
}

GameScriptColor::operator Vector3() const
{
	return m_color;
}

GameScriptColor::operator Vector4() const
{
	return m_color;
}

// D3DCOLOR is 32 bits and is layed out as ARGB.
GameScriptColor::operator D3DCOLOR() const
{	
	return m_color;
}

GameScriptColor::operator RGBAColor8Byte() const
{
	return m_color;
}

byte GameScriptColor::GetR() const
{
	return m_color.GetR();
}

void GameScriptColor::SetR(byte v)
{
	m_color.SetR(v);
}

byte GameScriptColor::GetG() const
{
	return m_color.GetG();
}

void GameScriptColor::SetG(byte v)
{
	m_color.SetG(v);
}

byte GameScriptColor::GetB() const
{
	return m_color.GetB();
}

void GameScriptColor::SetB(byte v)
{
	m_color.SetB(v);
}

byte GameScriptColor::GetA() const
{
	return m_color.GetA();
}

void GameScriptColor::SetA(byte v)
{
	m_color.SetA(v);
}

/***
@tparam Color color this color
@treturn string A string showing the r, g, b, and a values of the color
@function __tostring
*/
std::string GameScriptColor::ToString() const
{
	return "{" + std::to_string(GetR()) + ", " + std::to_string(GetG()) + ", " + std::to_string(GetB()) +  ", " + std::to_string(GetA()) + "}";
}
