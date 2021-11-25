#include "framework.h"
#include <sol.hpp>
#include "GameScriptMirror.h"

/***
A mirror effect.
As seen in TR4's Coastal Ruins and Sacred Lake levels.

__Not currently implemented.__ 

@pregameclass Mirror
@pragma nostrip
*/

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
