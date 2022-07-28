#include "framework.h"
#include "Color.h"
#include <cmath>

/***
An RGBA or RGB color.
Components are specified in bytes; all values are clamped to [0, 255].

@tenprimitive Color
@pragma nostrip
*/

void ScriptColor::Register(sol::table & parent)
{
	using ctors = sol::constructors<ScriptColor(byte, byte, byte), ScriptColor(byte, byte, byte, byte)>;
	parent.new_usertype<ScriptColor>("Color",
		ctors(),
		sol::call_constructor, ctors(),	
		sol::meta_function::to_string, &ScriptColor::ToString,

/// (int) red component
//@mem r
		"r", sol::property(&ScriptColor::GetR, &ScriptColor::SetR),

/// (int) green component
//@mem g
		"g", sol::property(&ScriptColor::GetG, &ScriptColor::SetG),

/// (int) blue component
//@mem b
		"b", sol::property(&ScriptColor::GetB, &ScriptColor::SetB),

/// (int) alpha component (255 is opaque, 0 is invisible)
//@mem a
		"a", sol::property(&ScriptColor::GetA, &ScriptColor::SetA)
	);
}

/*** 
@int R red component
@int G green component
@int B blue component
@return A Color object.
@function Color
*/
ScriptColor::ScriptColor(byte r, byte g, byte b) :
m_color(r, g, b)
{
}

/*** 
@int R red component
@int G green component
@int B blue component
@int A alpha component (255 is opaque, 0 is invisible)
@return A Color object.
@function Color
*/
ScriptColor::ScriptColor(byte r, byte g, byte b, byte a) : ScriptColor(r, g, b)
{
	SetA(a);
}

ScriptColor::ScriptColor(Vector3 const& col) :
	m_color(col)
{
}

ScriptColor::ScriptColor(Vector4 const& col) :
	m_color(col)
{
}

ScriptColor::ScriptColor(D3DCOLOR col) : 
	m_color(col)
{
}

ScriptColor::operator Vector3() const
{
	return m_color;
}

ScriptColor::operator Vector4() const
{
	return m_color;
}

// D3DCOLOR is 32 bits and is laid out as ARGB.
ScriptColor::operator D3DCOLOR() const
{	
	return m_color;
}

ScriptColor::operator RGBAColor8Byte() const
{
	return m_color;
}

byte ScriptColor::GetR() const
{
	return m_color.GetR();
}

void ScriptColor::SetR(byte v)
{
	m_color.SetR(v);
}

byte ScriptColor::GetG() const
{
	return m_color.GetG();
}

void ScriptColor::SetG(byte v)
{
	m_color.SetG(v);
}

byte ScriptColor::GetB() const
{
	return m_color.GetB();
}

void ScriptColor::SetB(byte v)
{
	m_color.SetB(v);
}

byte ScriptColor::GetA() const
{
	return m_color.GetA();
}

void ScriptColor::SetA(byte v)
{
	m_color.SetA(v);
}

/***
@tparam Color color this color
@treturn string A string showing the r, g, b, and a values of the color
@function __tostring
*/
std::string ScriptColor::ToString() const
{
	return "{" + std::to_string(GetR()) + ", " + std::to_string(GetG()) + ", " + std::to_string(GetB()) +  ", " + std::to_string(GetA()) + "}";
}
