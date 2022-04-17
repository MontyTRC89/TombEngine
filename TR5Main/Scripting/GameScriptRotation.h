#pragma once

#include "framework.h"

namespace sol {
	class state;
}
class PoseData;

class GameScriptRotation {
public:
	short								x{ 0 };
	short								y{ 0 };
	short								z{ 0 };

	GameScriptRotation() = default;
	GameScriptRotation(int x, int y, int z);
	GameScriptRotation(PoseData const& pos);

	std::string ToString() const;

	void StoreInPHDPos(PoseData& pos) const;

	static void Register(sol::state*);
};
