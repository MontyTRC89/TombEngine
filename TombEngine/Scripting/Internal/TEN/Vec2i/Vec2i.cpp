#include "framework.h"
#include "Scripting/Internal/TEN/Vec2i/Vec2i.h"

#include "Math/Math.h"
#include "Scripting/Internal/ReservedScriptNames.h"

using namespace TEN::Math;

/***
Represents an integer-based 2D vector.
@tenprimitive Vec2i
@pragma nostrip
*/

void Vec2i::Register(sol::table& parent)
{
	using ctors = sol::constructors<Vec2i(int, int)>;

	parent.new_usertype<Vec2i>(
		ScriptReserved_Vec2i,
		ctors(),
		sol::call_constructor, ctors(),
		sol::meta_function::to_string, &Vec2i::ToString,
		sol::meta_function::addition, &Vec2i::Add,
		sol::meta_function::subtraction, &Vec2i::Subtract,
		sol::meta_function::unary_minus, &Vec2i::UnaryMinus,
		sol::meta_function::multiplication, &Vec2i::MultiplyByScale,

		/*** Modify this vector so that it becomes close to the input length.

		Note that since Vec2i is integer-based, meaning that
		this will be less accurate at smaller lengths.
		@tparam float length new length to set.
		@function Vec2i:ToLength
		*/
		ScriptReserved_ToLength, &Vec2i::ToLength,

		/// (int) x coordinate
		//@mem x
		"x", &Vec2i::x,

		/// (int) y coordinate
		//@mem y
		"y", &Vec2i::y);
}

/*** 
@int X x coordinate
@int Y y coordinate
@treturn Vec2i A Vec2i object.
@function Vec2i
*/
Vec2i::Vec2i(int aX, int aY) : x(aX), y(aY)
{
}

Vec2i::Vec2i(const Vector2i& pos) : x(pos.x), y(pos.y)
{
}

/*** Metafunction; use tostring(myVector)
@tparam Vec2i Vec2i this Vec2i
@treturn string A string showing the x, y, and z values of the Vec2i
@function __tostring
*/
std::string Vec2i::ToString() const
{
	return "{" + std::to_string(x) + ", " + std::to_string(y) + "}";
}

void Vec2i::ToLength(float length)
{
	float currentLength = sqrt(SQUARE(x) + SQUARE(y));
	x = (int)round((x / currentLength) * length);
	y = (int)round((y / currentLength) * length);
}

Vec2i Vec2i::Add(const Vec2i& vector0, const Vec2i& vector1)
{
	return Vec2i(vector0.x + vector1.x, vector0.y + vector1.y);
}

Vec2i Vec2i::Subtract(const Vec2i& vector0, const Vec2i& vector1)
{
	return Vec2i(vector0.x - vector1.x, vector0.y - vector1.y);
}

Vec2i Vec2i::MultiplyByScale(const Vec2i& vector, float scale)
{
	return Vec2i((int)round(vector.x * scale), (int)round(vector.y * scale));
}

Vec2i Vec2i::UnaryMinus(const Vec2i& vector)
{
	return Vec2i(vector.x * -1, vector.y * -1);
}

Vec2i::operator Vector2i() const
{
	return Vector2i(x, y);
};
