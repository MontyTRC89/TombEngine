#include "frameworkandsol.h"
#include "Position.h"
#include "Specific/phd_global.h"

/***
Represents a position in the game world.
@tenprimitive Position
@pragma nostrip
*/

void Position::Register(sol::table & parent)
{
	parent.new_usertype<Position>("Position",
		sol::constructors<Position(int, int, int)>(),
		sol::meta_function::to_string, &Position::ToString,
		sol::meta_function::addition, &AddPositions,

		/// (int) x coordinate
		//@mem x
		"x", &Position::x,

		/// (int) y coordinate
		//@mem y

		"y", &Position::y,
		/// (int) z coordinate
		//@mem z

		"z", &Position::z
		);
}

/*** 
@int X x coordinate
@int Y y coordinate
@int Z z coordinate
@return A Position object.
@function Position.new
*/
Position::Position(int aX, int aY, int aZ) : x{aX}, y{aY}, z{aZ}
{
}

Position::Position(PHD_3DPOS const& pos) : x{pos.xPos}, y{pos.yPos}, z{pos.zPos}
{
}

void Position::StoreInPHDPos(PHD_3DPOS& pos) const
{
	pos.xPos = x;
	pos.yPos = y;
	pos.zPos = z;
}

void Position::StoreInGameVector(GAME_VECTOR& pos) const
{
	pos.x = x;
	pos.y = y;
	pos.z = z;
}


/***
@tparam Position position this position
@treturn string A string showing the x, y, and z values of the position
@function __tostring
*/
std::string Position::ToString() const
{
	return "{" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "}";
}

Position AddPositions(Position const & one, Position const & two)
{
	return Position { one.x + two.x, one.y + two.y, one.z + two.z };
}


