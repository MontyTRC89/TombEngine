#pragma once

#include "framework.h"

class GameScriptRotation {
private:
	int								x;
	int								y;
	int								z;

public:
	GameScriptRotation(int x, int y, int z);

	int								GetX();
	void							SetX(int x);
	int								GetY();
	void							SetY(int y);
	int								GetZ();
	void							SetZ(int z);
};