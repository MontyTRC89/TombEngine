#pragma once

#include "framework.h"

/***
Represents a rotation as a combination of individual
angles, in degrees, about each axis.
All values will be clamped to [-32768, 32767].
@classmod Rotation
@pragma nostrip
*/
namespace sol {
	class state;
}
struct PHD_3DPOS;

class GameScriptRotation {
public:
/// (int) rotation about x axis
//@mem X

/// (int) rotation about y axis
//@mem Y

/// (int) rotation about z axis
//@mem Z

	short								x;
	short								y;
	short								z;

/*** 
@int X rotation about x axis
@int Y rotation about y axis
@int Z rotation about z axis
@return A Rotation object.
@function Rotation.new
*/

	GameScriptRotation(int x, int y, int z);
	GameScriptRotation(PHD_3DPOS const& pos);
	void StoreInPHDPos(PHD_3DPOS& pos) const;

	static void Register(sol::state*);
};
