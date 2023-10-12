#include "framework.h"
#include "Scripting/Internal/TEN/Vec3/Vec3.h"

#include "Math/Math.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/TEN/Rotation/Rotation.h"

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
		sol::meta_function::multiplication, &Vec3::MultiplyByScale,
		sol::meta_function::division, &Vec3::DivideByScale,
		sol::meta_function::unary_minus, &Vec3::UnaryMinus,
		sol::meta_function::equal_to, &Vec3::IsEqualTo,

		/*** Normalize this Vec3 to the length of 1.0.
		@function Vec3:Normalize()
		*/
		ScriptReserved_Vec3Normalize, &Vec3::Normalize,
		
		/*** Set the length of this Vec3 to the input length.
		@tparam float length New length.
		@function Vec3:ToLength(length)
		*/
		ScriptReserved_Vec3SetLength, &Vec3::SetLength,

		/*** Clamp the length of this Vec3 to within the input length (inclusive).
		@tparam float lengthMax Max length.
		@function Vec3:ClampLength(lengthMax)
		*/
		ScriptReserved_Vec3ClampLength, &Vec3::ClampLength,

		/*** Rotate this Vec3 by the input Rotation.
		@tparam Rotation rot Rotation object.
		@function Vec3:Rotate(rot)
		*/
		ScriptReserved_Vec3Rotate, &Vec3::Rotate,

		/*** Linearly interpolate this Vec3 toward the input Vec3 according to the provided alpha value in the range [0.0, 1.0].
		@tparam float alpha Interpolation alpha value.
		@function Vec3:Lerp(rot)
		*/
		ScriptReserved_Vec3Lerp, &Vec3::Lerp,
		
		/*** Set this Vec3 to the cross product of this Vec3 and the input Vec3.
		@tparam Vec3 vector Second vector.
		@function Vec3:Cross(vector)
		*/
		ScriptReserved_Vec3Cross, &Vec3::Cross,
		
		/*** Get the length of this Vec3.
		@function Vec3:Length()
		*/
		ScriptReserved_Vec3Length, &Vec3::Length,

		/*** Get the distance between this Vec3 and the input Vec3.
		@tparam Vec3 vector Second vector.
		@function Vec3:Distance(vector)
		*/
		ScriptReserved_Vec3Distance, &Vec3::Distance,
		
		/*** Get the dot product of this Vec3 and the input Vec3.
		@tparam Vec3 vector Second vector.
		@function Vec3:Dot(vector)
		*/
		ScriptReserved_Vec3Dot, &Vec3::Dot,
		
		/// (float) X component.
		//@mem x
		"x", &Vec3::x,

		/// (float) Y component.
		//@mem y
		"y", &Vec3::y,

		/// (float) Z component.
		//@mem z
		"z", &Vec3::z);
}

/*** 
@float x X component.
@float y Y component.
@float z Z component.
@treturn Vec3 A Vec3.
@function Vec3
*/
Vec3::Vec3(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vec3::Vec3(const Vector3i& pos)
{
	x = pos.x;
	y = pos.y;
	z = pos.z;
}

Vec3::Vec3(const Vector3& pos)
{
	x = pos.x;
	y = pos.y;
	z = pos.z;
}

void Vec3::StoreInPose(Pose& pose) const
{
	pose.Position = Vector3i(x, y, z);
}

void Vec3::StoreInGameVector(GameVector& pos) const
{
	pos.x = (int)x;
	pos.y = (int)y;
	pos.z = (int)z;
}

void Vec3::Normalize()
{
	SetLength(1.0f);
}

void Vec3::SetLength(float length)
{
	float currentLength = Length();

	x = (x / currentLength) * length;
	y = (y / currentLength) * length;
	z = (z / currentLength) * length;
}

void Vec3::ClampLength(float lengthMax)
{
	if (Length() > lengthMax)
		SetLength(lengthMax);
}

void Vec3::Rotate(const Rotation& rot)
{
	auto vector = Vector3(x, y, z);
	auto eulerRot = rot.ToEulerAngles();
	auto rotMatrix = eulerRot.ToRotationMatrix();

	*this = Vec3(Vector3::Transform(vector, rotMatrix));
}

void Vec3::Lerp(const Vec3& vector, float alpha)
{
	auto vector0 = Vector3(x, y, z);
	auto vector1 = Vector3(vector.x, vector.y, vector.z);

	*this = Vec3(Vector3::Lerp(vector0, vector1, alpha));
}

void Vec3::Cross(const Vec3& vector)
{
	auto vector0 = Vector3(x, y, z);
	auto vector1 = Vector3(vector.x, vector.y, vector.z);

	*this = vector0.Cross(vector1);
}

float Vec3::Length() const
{
	return Vector3(x, y, z).Length();
}

float Vec3::Distance(const Vec3& vector) const
{
	auto vector0 = Vector3(x, y, z);
	auto vector1 = Vector3(vector.x, vector.y, vector.z);

	return Vector3::Distance(vector0, vector1);
}

float Vec3::Dot(const Vec3& vector) const
{
	auto vector0 = Vector3(x, y, z);
	auto vector1 = Vector3(vector.x, vector.y, vector.z);

	return vector0.Dot(vector1);
}

/*** Metafunction; use tostring(myVector)
@tparam Vec3 Vec3 this Vec3.
@treturn string A string showing the X, Y, and Z components of the Vec3.
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

Vec3 Vec3::MultiplyByScale(const Vec3& vector, float scale)
{
	return Vec3(vector.x * scale, vector.y * scale, vector.z * scale);
}

Vec3 Vec3::DivideByScale(const Vec3& vector, float scale)
{
	return Vec3(vector.x / scale, vector.y / scale, vector.z / scale);
}

Vec3 Vec3::UnaryMinus(const Vec3& vector)
{
	return Vec3(vector.x * -1, vector.y * -1, vector.z * -1);
}

bool Vec3::IsEqualTo(const Vec3& vector0, const Vec3& vector1)
{
	if (vector0.Distance(vector1) <= EPSILON)
		return true;

	return false;
}

Vec3::operator Vector3() const
{
	return Vector3(x, y, z);
};
