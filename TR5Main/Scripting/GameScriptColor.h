#pragma once

#include "framework.h"

class GameScriptColor {
public:
	byte r;
	byte g;
	byte b;
	byte a;

	GameScriptColor(byte r, byte g, byte b);
	GameScriptColor(byte r, byte g, byte b, byte a);

	byte								GetR();
	void								SetR(byte v);
	byte								GetG();
	void								SetG(byte v);
	byte								GetB();
	void								SetB(byte v);
	byte								GetA();
	void								SetA(byte v);
};