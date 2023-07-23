#include "framework.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"

#include "Math/Math.h"
#include "Scripting/Internal/ReservedScriptNames.h"

using namespace TEN::Math;

/***
Represents a float-based 2D vector.
@tenprimitive Vec2
@pragma nostrip
*/

void Vec2::Register(sol::table& parent)
{
	using ctors = sol::constructors<Vec2(float, float)>;

	parent.new_usertype<Vec2>(
		ScriptReserved_Vec2,
		ctors(),
		sol::call_constructor, ctors(),
		sol::meta_function::to_string, &Vec2::ToString,
		sol::meta_function::addition, &Vec2::Add,
		sol::meta_function::subtraction, &Vec2::Subtract,
		sol::meta_function::unary_minus, &Vec2::UnaryMinus,
		sol::meta_function::multiplication, &Vec2::MultiplyByScale,

		/*** Modify this vector so that it becomes close to the input length.
		@tparam float length new length to set.
		@function Vec2:ToLength
		*/
		ScriptReserved_ToLength, &Vec2::ToLength,

		/// (float) x coordinate
		//@mem x
		"x", &Vec2::x,

		/// (float) y coordinate
		//@mem y
		"y", &Vec2::y);
}

/*** 
@float X x coordinate
@float Y y coordinate
@treturn Vec2 A Vec2 object.
@function Vec2
*/
Vec2::Vec2(float aX, float aY) : x(aX), y(aY)
{
}

Vec2::Vec2(const Vector2& pos) : x(pos.x), y(pos.y)
{
}

/*** Metafunction; use tostring(myVector)
@tparam Vec2 Vec2 this Vec2
@treturn string A string showing the x, y, and z values of the Vec2
@function __tostring
*/
std::string Vec2::ToString() const
{
	return "{" + std::to_string(x) + ", " + std::to_string(y) + "}";
}

void Vec2::ToLength(float length)
{
	float currentLength = sqrt(SQUARE(x) + SQUARE(y));
	x = (x / currentLength) * length;
	y = (y / currentLength) * length;
}

Vec2 Vec2::Add(const Vec2& vector0, const Vec2& vector1)
{
	return Vec2(vector0.x + vector1.x, vector0.y + vector1.y);
}

Vec2 Vec2::Subtract(const Vec2& vector0, const Vec2& vector1)
{
	return Vec2(vector0.x - vector1.x, vector0.y - vector1.y);
}

Vec2 Vec2::MultiplyByScale(const Vec2& vector, float scale)
{
	return Vec2(vector.x * scale, vector.y * scale);
}

Vec2 Vec2::UnaryMinus(const Vec2& vector)
{
	return Vec2(vector.x * -1, vector.y * -1);
}

Vec2::operator Vector2() const
{
	return Vector2(x, y);
};
