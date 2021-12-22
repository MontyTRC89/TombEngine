#include "framework.h"
#include "GameScriptPosition.h"
#include <sol.hpp>
#include "Specific/phd_global.h"

/***
Represents a position in the game world.
@miscclass Position
@pragma nostrip
*/

void GameScriptPosition::Register(sol::state* state)
{
	state->new_usertype<GameScriptPosition>("Position",
		sol::constructors<GameScriptPosition(int, int, int)>(),
		sol::meta_function::to_string, &GameScriptPosition::ToString,

		/// (int) x coordinate
		//@mem x
		"x", &GameScriptPosition::x,

		/// (int) y coordinate
		//@mem y

		"y", &GameScriptPosition::y,
		/// (int) z coordinate
		//@mem z

		"z", &GameScriptPosition::z
		);
}

/*** 
@int X x coordinate
@int Y y coordinate
@int Z z coordinate
@return A Position object.
@function Position.new
*/
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

/***
@tparam Position position this position
@treturn string A string showing the x, y, and z values of the position
@function __tostring
*/
std::string GameScriptPosition::ToString() const
{
	return "{" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "}";
}

