#include "framework.h"
#include "GameScriptPosition.h"
#include <sol.hpp>
#include "phd_global.h"

void GameScriptPosition::Register(sol::state* state)
{
	state->new_usertype<GameScriptPosition>("Position",
		sol::constructors<GameScriptPosition(int, int, int)>(),
		"X", &GameScriptPosition::x,
		"Y", &GameScriptPosition::y,
		"Z", &GameScriptPosition::z
		);
}

GameScriptPosition::GameScriptPosition(int aX, int aY, int aZ)
{
	x = aX;
	y = aY;
	z = aZ;
}

GameScriptPosition::GameScriptPosition(PHD_3DPOS const& pos)
{
	x = pos.xPos;
	y = pos.yPos;
	z = pos.zPos;
}

void GameScriptPosition::StoreInPHDPos(PHD_3DPOS& pos) const
{
	pos.xPos = x;
	pos.yPos = y;
	pos.zPos = z;
}

