#pragma once

#include "framework.h"

/***
Represents a position in the game world.
@classmod Position
@pragma nostrip
*/
namespace sol {
	class state;
}
struct PHD_3DPOS;

class GameScriptPosition {
public:
/// (int) x coordinate
//@mem X

/// (int) y coordinate
//@mem Y

/// (int) z coordinate
//@mem Z
	int x;
	int y;
	int z;

/*** 
@int X x coordinate
@int Y y coordinate
@int Z z coordinate
@return A Position object.
@function Position.new
*/
	GameScriptPosition(int x, int y, int z);
	GameScriptPosition(PHD_3DPOS const& pos);
	void StoreInPHDPos(PHD_3DPOS& pos) const;

	static void Register(sol::state*);
};
