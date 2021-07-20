#include "framework.h"
#include "GameScriptRotation.h"
#include "phd_global.h"

/***
Represents a rotation as a combination of individual
angles, in degrees, about each axis.
All values will be clamped to [-32768, 32767].
@classmod Rotation
@pragma nostrip
*/

void GameScriptRotation::Register(sol::state* state)
{
	state->new_usertype<GameScriptRotation>("Rotation",
		sol::constructors<GameScriptRotation(int, int, int)>(),

/// (int) rotation about x axis
//@mem X
		"X", &GameScriptRotation::x,

/// (int) rotation about x axis
//@mem Y
		"Y", &GameScriptRotation::y,

/// (int) rotation about x axis
//@mem Z
		"Z", &GameScriptRotation::z
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
