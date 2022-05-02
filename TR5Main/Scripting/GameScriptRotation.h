#pragma once
#include "framework.h"

namespace sol
{
	class state;
}

struct PoseData;

class GameScriptRotation
{
public:
	float								x{ 0 };
	float								y{ 0 };
	float								z{ 0 };

	GameScriptRotation() = default;
	GameScriptRotation(float x, float y, float z);
	GameScriptRotation(PoseData const& pos);

	std::string ToString() const;

	void StoreInPHDPos(PoseData& pos) const;

	static void Register(sol::state*);
};
