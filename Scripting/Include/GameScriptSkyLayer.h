#pragma once

#include "GameScriptColor.h"

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
	GameScriptSkyLayer(GameScriptColor const & col, short speed);
	void SetColor(GameScriptColor const & col);

	static void Register(sol::state *);
};

