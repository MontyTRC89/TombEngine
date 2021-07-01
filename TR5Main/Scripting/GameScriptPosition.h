#pragma once

#include "framework.h"

namespace sol {
	class state;
}

class GameScriptPosition {
private:
	int x;
	int y;
	int z;

public:
	GameScriptPosition(int x, int y, int z);
	static void Register(sol::state*);

	int								GetX() const;
	void							SetX(int x);
	int								GetY() const;
	void							SetY(int y);
	int								GetZ() const;
	void							SetZ(int z);
};