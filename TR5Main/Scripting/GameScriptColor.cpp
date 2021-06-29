#include "framework.h"
#include "GameScriptColor.h"

GameScriptColor::GameScriptColor(byte r, byte g, byte b)
{
	SetR(r);
	SetG(g);
	SetB(b);
}

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