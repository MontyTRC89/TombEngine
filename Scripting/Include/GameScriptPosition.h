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

	GameScriptPosition(int x, int y, int z);
	GameScriptPosition(PHD_3DPOS const& pos);

	std::string ToString() const;

	void StoreInPHDPos(PHD_3DPOS& pos) const;

	static void Register(sol::state*);
};
