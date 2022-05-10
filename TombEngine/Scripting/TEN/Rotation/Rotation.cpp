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

void Rotation::Register(sol::table& parent)
{
	parent.new_usertype<Rotation>("Rotation",
		sol::constructors<Rotation(int, int, int)>(),
		sol::meta_function::to_string, &Rotation::ToString,

/// (int) rotation about x axis
//@mem x
		"x", &Rotation::x,

/// (int) rotation about x axis
//@mem y
		"y", &Rotation::y,

/// (int) rotation about x axis
//@mem z
		"z", &Rotation::z
	);
}

/*** 
@int X rotation about x axis
@int Y rotation about y axis
@int Z rotation about z axis
@return A Rotation object.
@function Rotation.new
*/
Rotation::Rotation(float aX, float aY, float aZ)
{
	x = aX;
	y = aY;
	z = aZ;
}

void Rotation::StoreInPHDPos(EulerAngles& orient)
{
	orient.Set(x, y, z);
}

Rotation::Rotation(EulerAngles orient)
{
	x = orient.GetX();
	y = orient.GetY();
	z = orient.GetZ();
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
