#include "framework.h"
#include "Rotation.h"

#include "Math/Math.h"
#include "ReservedScriptNames.h"

/*** Represents a rotation.
Rotations are specifed as a combination of individual
angles, in degrees, about each axis.
All values will be clamped to [0, 360].
@tenprimitive Rotation
@pragma nostrip
*/

void Rotation::Register(sol::table & parent)
{
	using ctors = sol::constructors<Rotation(float, float, float)>;
	parent.new_usertype<Rotation>(ScriptReserved_Rotation,
		ctors(),
		sol::call_constructor, ctors(),
		sol::meta_function::to_string, &Rotation::ToString,

/// (float) rotation about x axis
//@mem x
		"x", &Rotation::x,

/// (float) rotation about y axis
//@mem y
		"y", &Rotation::y,

/// (float) rotation about z axis
//@mem z
		"z", &Rotation::z
	);
}

/*** 
@float X rotation about x axis
@float Y rotation about y axis
@float Z rotation about z axis
@treturn Rotation A Rotation object.
@function Rotation
*/
Rotation::Rotation(float aX, float aY, float aZ)
{
	x = aX;
	y = aY;
	z = aZ;
}

Rotation::Rotation(EulerAngles const& ang)
{
	x = TO_DEGREES(ang.x);
	y = TO_DEGREES(ang.y);
	z = TO_DEGREES(ang.z);
}

Rotation::Rotation(Pose const& pos)
{
	x = TO_DEGREES(pos.Orientation.x);
	y = TO_DEGREES(pos.Orientation.y);
	z = TO_DEGREES(pos.Orientation.z);
}

void Rotation::StoreInPHDPos(Pose& pos) const
{
	pos.Orientation.x = ANGLE(x);
	pos.Orientation.y = ANGLE(y);
	pos.Orientation.z = ANGLE(z);
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

