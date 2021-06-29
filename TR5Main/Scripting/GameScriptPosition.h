#pragma once

#include "framework.h"

class GameScriptPosition {
public:
	int x;
	int y;
	int z;

	GameScriptPosition(int x, int y, int z);

	int								GetX();
	void							SetX(int x);
	int								GetY();
	void							SetY(int y);
	int								GetZ();
	void							SetZ(int z);
};