#include "framework.h"
#include "Scripting/Internal/TEN/Vec3/Vec3.h"

#include "Math/Math.h"
#include "Scripting/Internal/ReservedScriptNames.h"

using namespace TEN::Math;

/***
Represents a float-based 3D vector.
@tenprimitive Vec3
@pragma nostrip
*/

void Vec3::Register(sol::table& parent)
{
	using ctors = sol::constructors<Vec3(float, float, float)>;

	parent.new_usertype<Vec3>(
		ScriptReserved_Vec3,
		ctors(),
		sol::call_constructor, ctors(),
		sol::meta_function::to_string, &Vec3::ToString,
		sol::meta_function::addition, &Vec3::Add,
		sol::meta_function::subtraction, &Vec3::Subtract,
		sol::meta_function::multiplication, &Vec3::Multiply,
		sol::meta_function::multiplication, &Vec3::MultiplyScale,
		sol::meta_function::division, &Vec3::DivideScale,
		sol::meta_function::unary_minus, &Vec3::UnaryMinus,

		/*** Modify the Vec3 to match the input length.
		@tparam float length new length to set.
		@function Vec3:ToLength
		*/
		ScriptReserved_ToLength, &Vec3::SetLength,

		/// (float) x coordinate
		//@mem x
		"x", &Vec3::x,

		/// (float) y coordinate
		//@mem y
		"y", &Vec3::y,

		/// (float) z coordinate
		//@mem z
		"z", &Vec3::z);
}

/*** 
@float X x coordinate
@float Y y coordinate
@float Z z coordinate
@treturn Vec3 A Vec3 object.
@function Vec3
*/
Vec3::Vec3(float aX, float aY, float aZ) : x(aX), y(aY), z(aZ)
{
}

Vec3::Vec3(const Vector3i& pos) : x(float(pos.x)), y(float(pos.y)), z(float(pos.z))
{
}

Vec3::Vec3(const Vector3& pos) : x(pos.x), y(pos.y), z(pos.z)
{
}

void Vec3::StoreInPose(Pose& pose) const
{
	pose.Position.x = int(x);
	pose.Position.y = int(y);
	pose.Position.z = int(z);
}

void Vec3::StoreInGameVector(GameVector& pos) const
{
	pos.x = int(x);
	pos.y = int(y);
	pos.z = int(z);
}

void Vec3::SetLength(float newLength)
{
	float currentLength = sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z));
	x = (x / currentLength) * newLength;
	y = (y / currentLength) * newLength;
	z = (z / currentLength) * newLength;
}

/*** Metafunction; use tostring(myVector)
@tparam Vec3 Vec3 this Vec3
@treturn string A string showing the x, y, and z values of the Vec3
@function __tostring
*/
std::string Vec3::ToString() const
{
	return "{ " + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + " }";
}

Vec3 Vec3::Add(const Vec3& vector0, const Vec3& vector1)
{
	return Vec3(vector0.x + vector1.x, vector0.y + vector1.y, vector0.z + vector1.z);
}

Vec3 Vec3::Subtract(const Vec3& vector0, const Vec3& vector1)
{
	return Vec3(vector0.x - vector1.x, vector0.y - vector1.y, vector0.z - vector1.z);
}

Vec3 Vec3::Multiply(const Vec3& vector0, const Vec3& vector1)
{
	return Vec3(vector0.x * vector1.x, vector0.y * vector1.y, vector0.z * vector1.z);
}

Vec3 Vec3::MultiplyScale(const Vec3& vector, float scale)
{
	return Vec3(vector.x * scale, vector.y * scale, vector.z * scale);
}

Vec3 Vec3::DivideScale(const Vec3& vector, float scale)
{
	return Vec3(vector.x / scale, vector.y / scale, vector.z / scale);
}

Vec3 Vec3::UnaryMinus(const Vec3& vector)
{
	return Vec3(vector.x * -1, vector.y * -1, vector.z * -1);
}

Vec3::operator Vector3() const
{
	return Vector3(x, y, z);
};
