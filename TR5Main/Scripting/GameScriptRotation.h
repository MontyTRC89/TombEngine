#pragma once

#include "framework.h"

namespace sol {
	class state;
}
struct PHD_3DPOS;

class GameScriptRotation {
public:
	short								x{ 0 };
	short								y{ 0 };
	short								z{ 0 };

	GameScriptRotation() = default;
	GameScriptRotation(int x, int y, int z);
	GameScriptRotation(PHD_3DPOS const& pos);

	std::string ToString() const;

	void StoreInPHDPos(PHD_3DPOS& pos) const;

	static void Register(sol::state*);
};
