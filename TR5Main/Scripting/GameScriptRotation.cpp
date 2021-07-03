#include "framework.h"
#include "GameScriptRotation.h"
#include "phd_global.h"

void GameScriptRotation::Register(sol::state* state)
{
	state->new_usertype<GameScriptRotation>("Rotation",
		sol::constructors<GameScriptRotation(int, int, int)>(),
		"X", &GameScriptRotation::x,
		"Y", &GameScriptRotation::y,
		"Z", &GameScriptRotation::z
	);
}


GameScriptRotation::GameScriptRotation(int aX, int aY, int aZ)
{
	x = aX;
	y = aY;
	z = aZ;
}

void GameScriptRotation::StoreInPHDPos(PHD_3DPOS& pos) const
{
	pos.xRot = x;
	pos.yRot = y;
	pos.zRot = z;
}

GameScriptRotation::GameScriptRotation(PHD_3DPOS const & pos)
{
	x = pos.xRot;
	y = pos.yRot;
	z = pos.zRot;
}
