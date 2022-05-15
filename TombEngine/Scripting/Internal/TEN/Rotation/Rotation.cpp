#include "framework.h"
#include "Rotation.h"
#include "Specific/phd_global.h"

/*** Represents a rotation.
Rotations are specifed as a combination of individual
angles, in degrees, about each axis.
All values will be clamped to [-32768, 32767].
@tenprimitive Rotation
@pragma nostrip
*/

void Rotation::Register(sol::table & parent)
{
	using ctors = sol::constructors<Rotation(int, int, int)>;
	parent.new_usertype<Rotation>("Rotation",
		ctors(),
		sol::call_constructor, ctors(),
		sol::meta_function::to_string, &Rotation::ToString,

/// (int) rotation about x axis
//@mem x
		"x", &Rotation::x,

/// (int) rotation about y axis
//@mem y
		"y", &Rotation::y,

/// (int) rotation about z axis
//@mem z
		"z", &Rotation::z
	);
}

/*** 
@int X rotation about x axis
@int Y rotation about y axis
@int Z rotation about z axis
@return A Rotation object.
@function Rotation
*/
Rotation::Rotation(int aX, int aY, int aZ)
{
	x = aX;
	y = aY;
	z = aZ;
}

void Rotation::StoreInPHDPos(PHD_3DPOS& pos) const
{
	pos.Orientation.x = x;
	pos.Orientation.y = y;
	pos.Orientation.z = z;
}

Rotation::Rotation(PHD_3DPOS const & pos)
{
	x = pos.Orientation.x;
	y = pos.Orientation.y;
	z = pos.Orientation.z;
}

/***
@tparam Rotation rotation this rotation
@treturn string A string showing the x, y, and z values of the rotation
@function __tostring
*/
std::string Rotation::ToString() const
{
	return "{" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "}";
}

