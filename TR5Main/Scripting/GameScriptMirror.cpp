#include "framework.h"
#include "GameScriptMirror.h"

void GameScriptMirror::Register(sol::state* lua)
{
	lua->new_usertype<GameScriptMirror>("Mirror",
		sol::constructors<GameScriptMirror(short, int, int, int, int)>(),
		"room", &GameScriptMirror::Room,
		"startX", &GameScriptMirror::StartX,
		"endX", &GameScriptMirror::EndX,
		"startZ", &GameScriptMirror::StartZ,
		"endZ", &GameScriptMirror::EndZ
		);
}

GameScriptMirror::GameScriptMirror(short room, int startX, int endX, int startZ, int endZ)
{
	Room = room;
	StartX = startX;
	EndX = endX;
	StartZ = startZ;
	EndZ = endZ;
}
