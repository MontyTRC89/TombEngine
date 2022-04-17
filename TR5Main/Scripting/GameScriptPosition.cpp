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

GameScriptPosition::GameScriptPosition(PoseData const& pos)
{
	x = pos.Position.x;
	y = pos.Position.y;
	z = pos.Position.z;
}

void GameScriptPosition::StoreInPHDPos(PoseData& pos) const
{
	pos.Position.x = x;
	pos.Position.y = y;
	pos.Position.z = z;
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

