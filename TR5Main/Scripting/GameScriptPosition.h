#pragma once

#include "framework.h"

namespace sol {
	class state;
}
class PoseData;

class GameScriptPosition {
public:
	int x;
	int y;
	int z;

	GameScriptPosition(int x, int y, int z);
	GameScriptPosition(PoseData const& pos);

	std::string ToString() const;

	void StoreInPHDPos(PoseData& pos) const;

	static void Register(sol::state*);
};
