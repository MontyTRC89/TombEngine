#include "framework.h"
#include "GameScriptRotation.h"
#include "Specific\phd_global.h"

/*** Represents a rotation.
Rotations are specifed as a combination of individual
angles, in degrees, about each axis.
All values will be clamped to [-32768, 32767].
@miscclass Rotation
@pragma nostrip
*/

void GameScriptRotation::Register(sol::state* state)
{
	state->new_usertype<GameScriptRotation>("Rotation",
		sol::constructors<GameScriptRotation(int, int, int)>(),

/// (int) rotation about x axis
//@mem x
		"x", &GameScriptRotation::x,

/// (int) rotation about x axis
//@mem y
		"y", &GameScriptRotation::y,

/// (int) rotation about x axis
//@mem z
		"z", &GameScriptRotation::z
	);
}

/*** 
@int X rotation about x axis
@int Y rotation about y axis
@int Z rotation about z axis
@return A Rotation object.
@function Rotation.new
*/
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

/***
@tparam Rotation rotation this rotation
@treturn string A string showing the x, y, and z values of the rotation
@function __tostring
*/
std::string GameScriptRotation::ToString() const
{
	return "{" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "}";
}

