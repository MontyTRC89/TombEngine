#pragma once

#include "framework.h"

namespace sol {
	class state;
}
struct PHD_3DPOS;

class GameScriptRotation {
public:
	short								x;
	short								y;
	short								z;

	GameScriptRotation(int x, int y, int z);
	GameScriptRotation(PHD_3DPOS const& pos);
	void StoreInPHDPos(PHD_3DPOS& pos) const;

	static void Register(sol::state*);
};
