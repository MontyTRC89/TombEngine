#pragma once

#include "GameScriptColor.h"

namespace sol {
	class state;
}

struct GameScriptFog
{
	bool Enabled{ false };
	byte R{ 0 };
	byte G{ 0 };
	byte B{ 0 };
	short MinDistance{ 0 };
	short MaxDistance{ 0 };

	GameScriptFog() = default;
	GameScriptFog(GameScriptColor const& col, short minDistance, short maxDistance);
	void SetColor(GameScriptColor const& col);

	static void Register(sol::state*);
};
