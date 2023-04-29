#include "framework.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"

#include "Math/Math.h"
#include "Scripting/Internal/ReservedScriptNames.h"

using namespace TEN::Math;

/***
Represents a 2D vector.
@tenprimitive Vec2
@pragma nostrip
*/

void Vec2::Register(sol::table& parent)
{
	using ctors = sol::constructors<Vec2(int, int)>;

	parent.new_usertype<Vec2>(
		ScriptReserved_Vec2,
		ctors(),
		sol::call_constructor, ctors(),
		sol::meta_function::to_string, &Vec2::ToString,
		sol::meta_function::addition, &AddVec2s,
		sol::meta_function::subtraction, &SubtractVec2s,
		sol::meta_function::unary_minus, &UnaryMinusVec2,
		sol::meta_function::multiplication, &MultiplyVec2Number,

/*** Modify this vector so that it becomes close to the requested length.

Note that since the engine uses integers instead of floating-point
numbers, this will be less accurate at smaller lengths.
@tparam int length the new length to set the vector to.
@function Vec3:GetNormalised
*/
		ScriptReserved_ToLength, &Vec2::ToLength,

		/// (int) x coordinate
		//@mem x
		"x", &Vec2::x,

		/// (int) y coordinate
		//@mem y

		"y", &Vec2::y
		);
}

/*** 
@int X x coordinate
@int Y y coordinate
@treturn Vec2 A Vec2 object.
@function Vec2
*/
Vec2::Vec2(int aX, int aY) : x{ aX }, y{ aY }
{
}

Vec2::Vec2(const Vector2i& pos) : x{ pos.x }, y{ pos.y }
{
}

Vec2::operator Vector2i() const
{
	return Vector2i(x, y);
};

/*** Metafunction; use tostring(myVector)
@tparam Vec2 Vec2 this Vec2
@treturn string A string showing the x, y, and z values of the Vec2
@function __tostring
*/
std::string Vec2::ToString() const
{
	return "{" + std::to_string(x) + ", " + std::to_string(y) + "}";
}

Vec2 AddVec2s(const Vec2& vector0, const Vec2& vector1)
{
	return Vec2(vector0.x + vector1.x, vector0.y + vector1.y);
}

Vec2 SubtractVec2s(const Vec2& vector0, const Vec2& vector1)
{
	return Vec2(vector0.x - vector1.x, vector0.y - vector1.y);
}

Vec2 MultiplyVec2Number(const Vec2& vector, float scale)
{
	return Vec2(int(vector.x * scale), int(vector.y * scale));
}

Vec2 UnaryMinusVec2(const Vec2& vector)
{
	return Vec2(vector.x * -1, vector.y * -1);
}

void Vec2::ToLength(int newLength)
{
	float length = sqrt(SQUARE(x) + SQUARE(y));
	x = (int)round((x / length) * newLength);
	y = (int)round((y / length) * newLength);
}
