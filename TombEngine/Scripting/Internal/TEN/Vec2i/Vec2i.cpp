#include "framework.h"
#include "Scripting/Internal/TEN/Vec2i/Vec2i.h"

#include "Math/Math.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"

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
		sol::meta_function::addition, &Vec2i::AddVec2,
		sol::meta_function::subtraction, &Vec2i::Subtract,
		sol::meta_function::subtraction, &Vec2i::SubtractVec2,
		sol::meta_function::multiplication, &Vec2i::Multiply,
		sol::meta_function::multiplication, &Vec2i::MultiplyVec2,
		sol::meta_function::multiplication, &Vec2i::MultiplyScale,
		sol::meta_function::division, &Vec2i::DivideScale,
		sol::meta_function::unary_minus, &Vec2i::UnaryMinus,

		/*** Convert to Vec2 object.
		*/
		ScriptReserved_ToVec2, &Vec2i::ToVec2,

		/*** Modify to match input length.

		Note that since Vec2i is integer-based,
		this will be less accurate at shorter lengths.
		@tparam float length new length to set.
		@function Vec2i:ToLength
		*/
		ScriptReserved_ToLength, &Vec2i::SetLength,

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
	return "{ " + std::to_string(x) + ", " + std::to_string(y) + " }";
}

Vec2 Vec2i::ToVec2()
{
	return Vec2(x, y);
}

void Vec2i::SetLength(float length)
{
	float currentLength = sqrt(SQUARE(x) + SQUARE(y));
	x = (int)round((x / currentLength) * length);
	y = (int)round((y / currentLength) * length);
}

Vec2i Vec2i::Add(const Vec2i& vector0, const Vec2i& vector1)
{
	return Vec2i(vector0.x + vector1.x, vector0.y + vector1.y);
}

Vec2i Vec2i::AddVec2(const Vec2i& vector0, const Vec2& vector1)
{
	return Vec2i((int)round(vector0.x + vector1.x), (int)round(vector0.y + vector1.y));
}

Vec2i Vec2i::Subtract(const Vec2i& vector0, const Vec2i& vector1)
{
	return Vec2i(vector0.x - vector1.x, vector0.y - vector1.y);
}

Vec2i Vec2i::SubtractVec2(const Vec2i& vector0, const Vec2& vector1)
{
	return Vec2i(vector0.x - vector1.x, vector0.y - vector1.y);
}

Vec2i Vec2i::Multiply(const Vec2i& vector0, const Vec2i& vector1)
{
	return Vec2i(vector0.x * vector1.x, vector0.y * vector1.y);
}

Vec2i Vec2i::MultiplyVec2(const Vec2i& vector0, const Vec2& vector1)
{
	return Vec2i((int)round(vector0.x * vector1.x), (int)round(vector0.y * vector1.y));
}

Vec2i Vec2i::MultiplyScale(const Vec2i& vector, float scale)
{
	return Vec2i((int)round(vector.x * scale), (int)round(vector.y * scale));
}

Vec2i Vec2i::DivideScale(const Vec2i& vector, float scale)
{
	return Vec2i((int)round(vector.x / scale), (int)round(vector.y / scale));
}

Vec2i Vec2i::UnaryMinus(const Vec2i& vector)
{
	return Vec2i(vector.x * -1, vector.y * -1);
}

Vec2i::operator Vector2i() const
{
	return Vector2i(x, y);
};
