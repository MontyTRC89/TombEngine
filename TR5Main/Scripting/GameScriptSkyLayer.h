#pragma once

#include "framework.h"
#include "GameScriptSkyLayer.h"

namespace sol {
	class state;
}

struct GameScriptSkyLayer
{
	bool Enabled{ false };
	byte R{ 0 };
	byte G{ 0 };
	byte B{ 0 };
	short CloudSpeed{ 0 };

	GameScriptSkyLayer() = default;
	GameScriptSkyLayer(byte r, byte g, byte b, short speed);

	static void Register(sol::state *);
};

