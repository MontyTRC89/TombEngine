#pragma once

#include "framework.h"

namespace sol {
	class state;
}
struct PHD_3DPOS;

class GameScriptPosition {
public:
	int x;
	int y;
	int z;

public:
	GameScriptPosition(int x, int y, int z);
	GameScriptPosition(PHD_3DPOS const& pos);
	static void Register(sol::state*);
	void StoreInPHDPos(PHD_3DPOS& pos) const;
};
