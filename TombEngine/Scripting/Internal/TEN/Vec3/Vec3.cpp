#include "framework.h"
#include "Scripting/Internal/TEN/Vec3/Vec3.h"

#include "Math/Math.h"
#include "Scripting/Internal/ReservedScriptNames.h"

using namespace TEN::Math;

/***
Represents a 3D vector.
@tenprimitive Vec3
@pragma nostrip
*/

void Vec3::Register(sol::table& parent)
{
	using ctors = sol::constructors<Vec3(int, int, int)>;

	parent.new_usertype<Vec3>(
		ScriptReserved_Vec3,
		ctors(),
		sol::call_constructor, ctors(),
		sol::meta_function::to_string, &Vec3::ToString,
		sol::meta_function::addition, &AddVec3s,
		sol::meta_function::subtraction, &SubtractVec3s,
		sol::meta_function::unary_minus, &UnaryMinusVec3,
		sol::meta_function::multiplication, &MultiplyVec3Number,

/*** Modify this vector so that it becomes close to the requested length.

Note that since the engine uses integers instead of floating-point
numbers, this will be less accurate at smaller lengths.
For example, if you have the vector (100, 600, 700) and set it to
the length of 1, the vector SHOULD become approximately (0.11, 0.65, 0.75).
However, this function would return it as (0, 1, 1).
@tparam float length the new length to set the vector to.
@function Vec3:ToLength
*/
		ScriptReserved_ToLength, &Vec3::ToLength,

		/// (int) x coordinate
		//@mem x
		"x", &Vec3::x,

		/// (int) y coordinate
		//@mem y

		"y", &Vec3::y,
		/// (int) z coordinate
		//@mem z

		"z", &Vec3::z
		);
}

/*** 
@int X x coordinate
@int Y y coordinate
@int Z z coordinate
@treturn Vec3 A Vec3 object.
@function Vec3
*/
Vec3::Vec3(int aX, int aY, int aZ) : x{ aX }, y{ aY }, z{ aZ }
{
}

Vec3::Vec3(const Pose& pose) : x{ pose.Position.x }, y{ pose.Position.y }, z{ pose.Position.z }
{
}

Vec3::Vec3(const Vector3i& pos) : x{ pos.x }, y{ pos.y }, z{ pos.z }
{
}

Vec3::operator Vector3i() const
{
	return Vector3i(x, y, z);
};

void Vec3::StoreInPose(Pose& pose) const
{
	pose.Position.x = x;
	pose.Position.y = y;
	pose.Position.z = z;
}

void Vec3::StoreInGameVector(GameVector& pos) const
{
	pos.x = x;
	pos.y = y;
	pos.z = z;
}

/*** Metafunction; use tostring(myVector)
@tparam Vec3 Vec3 this Vec3
@treturn string A string showing the x, y, and z values of the Vec3
@function __tostring
*/
std::string Vec3::ToString() const
{
	return ("{" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "}");
}

Vec3 AddVec3s(const Vec3& vector0, const Vec3& vector1)
{
	return Vec3(vector0.x + vector1.x, vector0.y + vector1.y, vector0.z + vector1.z);
}

Vec3 SubtractVec3s(const Vec3& vector0, const Vec3& vector1)
{
	return Vec3(vector0.x - vector1.x, vector0.y - vector1.y, vector0.z - vector1.z);
}

Vec3 MultiplyVec3Number(const Vec3& vector, float scale)
{
	return Vec3(int(vector.x * scale), int(vector.y * scale), int(vector.z * scale));
}

Vec3 UnaryMinusVec3(const Vec3& vector)
{
	return Vec3(vector.x * -1, vector.y * -1, vector.z * -1);
}

void Vec3::ToLength(float newLength)
{
	float length = sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z));
	x = (int)round((x / length) * newLength);
	y = (int)round((y / length) * newLength);
	z = (int)round((z / length) * newLength);
}
