#pragma once

#include "framework.h"

namespace sol {
	class state;
}
struct PHD_3DPOS;

class GameScriptRotation {
public:
	int								x;
	int								y;
	int								z;

public:
	GameScriptRotation(int x, int y, int z);
	static void Register(sol::state*);
	void StoreInPHDPos(PHD_3DPOS& pos) const;
	GameScriptRotation(PHD_3DPOS const& pos);
};
