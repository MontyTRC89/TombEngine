#include "framework.h"
#include "GameScriptColor.h"

/***
An RGBA or RGB color. Components are set in bytes, with 0 being
the minimum amount of that component, and 255 being the maximum.

@classmod Color
@pragma nostrip
*/

void GameScriptColor::Register(sol::state* state)
{
	state->new_usertype<GameScriptColor>("Color",
		sol::constructors<GameScriptColor(int, int, int)>(),

/// (int) red component
//@mem R
		"R", sol::property(&GameScriptColor::GetR, &GameScriptColor::SetR),

/// (int) green component
//@mem G
		"G", sol::property(&GameScriptColor::GetG, &GameScriptColor::SetG),

/// (int) blue component
//@mem B
		"B", sol::property(&GameScriptColor::GetB, &GameScriptColor::SetB),

/// (int) alpha component (255 is opaque, 0 is invisible)
//@mem A
		"A", sol::property(&GameScriptColor::GetA, &GameScriptColor::SetA)
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
	SetA(255);
}

/*** 
@int R red component
@int G green component
@int B blue component
@int A alpha component (255 is opaque, 0 is invisible)
@return A Color object.
@function Color.new
*/
GameScriptColor::GameScriptColor(byte r, byte g, byte b, byte a)
{
	SetR(r);
	SetG(g);
	SetB(b);
	SetA(a);
}

byte GameScriptColor::GetR()
{
	return r;
}

void GameScriptColor::SetR(byte v)
{
	r = std::clamp<byte>(v, 0, 255);
}

byte GameScriptColor::GetG()
{
	return g;
}

void GameScriptColor::SetG(byte v)
{
	g = std::clamp<byte>(v, 0, 255);
}

byte GameScriptColor::GetB()
{
	return b;
}

void GameScriptColor::SetB(byte v)
{
	b = std::clamp<byte>(v, 0, 255);
}

byte GameScriptColor::GetA()
{
	return a;
}

void GameScriptColor::SetA(byte v)
{
	a = std::clamp<byte>(v, 0, 255);
}