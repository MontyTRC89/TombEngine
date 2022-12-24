#include "framework.h"
#include "Scripting/Internal/TEN/Rotation/Rotation.h"

#include "Math/Math.h"
#include "Scripting/Internal/ReservedScriptNames.h"

/*** Represents a rotation.
Rotations are specifed as a combination of individual
angles, in degrees, about each axis.
All values will be clamped to [0.0f, 360.0f].
@tenprimitive Rotation
@pragma nostrip
*/

void Rotation::Register(sol::table& parent)
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
@tparam float X rotation about x axis
@tparam float Y rotation about y axis
@tparam float Z rotation about z axis
@treturn Rotation A Rotation object.
@function Rotation
*/
Rotation::Rotation(float aX, float aY, float aZ)
{
	x = aX;
	y = aY;
	z = aZ;
}

Rotation::Rotation(const EulerAngles& eulers)
{
	x = TO_DEGREES(eulers.x);
	y = TO_DEGREES(eulers.y);
	z = TO_DEGREES(eulers.z);
}

Rotation::Rotation(const Pose& pose)
{
	x = TO_DEGREES(pose.Orientation.x);
	y = TO_DEGREES(pose.Orientation.y);
	z = TO_DEGREES(pose.Orientation.z);
}

void Rotation::StoreInPHDPos(Pose& pose) const
{
	pose.Orientation.x = ANGLE(x);
	pose.Orientation.y = ANGLE(y);
	pose.Orientation.z = ANGLE(z);
}

/***
@tparam Rotation rotation this rotation
@treturn string A string showing the x, y, and z values of the rotation
@function __tostring
*/
std::string Rotation::ToString() const
{
	return ("{" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "}");
}
