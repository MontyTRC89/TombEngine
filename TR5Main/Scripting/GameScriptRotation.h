#pragma once

#include "framework.h"

class GameScriptRotation {
private:
	int								x;
	int								y;
	int								z;

public:
	GameScriptRotation(int x, int y, int z);

	int								GetX() const;
	void							SetX(int x);
	int								GetY() const;
	void							SetY(int y);
	int								GetZ() const;
	void							SetZ(int z);
};