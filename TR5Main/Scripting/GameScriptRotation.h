#pragma once

#include "framework.h"

namespace sol {
	class state;
}

class GameScriptRotation {
private:
	int								x;
	int								y;
	int								z;
	int								ConvertRotation(int a);

public:
	GameScriptRotation(int x, int y, int z);
	static void Register(sol::state*);

	int								GetX() const;
	void							SetX(int x);
	int								GetY() const;
	void							SetY(int y);
	int								GetZ() const;
	void							SetZ(int z);
};