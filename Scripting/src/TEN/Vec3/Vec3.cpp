#include "frameworkandsol.h"
#include "Vec3.h"
#include "Specific/phd_global.h"

/***
Represents a 3-dimensional vector.
@tenprimitive Vec3
@pragma nostrip
*/

void Vec3::Register(sol::table & parent)
{
	parent.new_usertype<Vec3>("Vec3",
		sol::constructors<Vec3(int, int, int)>(),
		sol::meta_function::to_string, &Vec3::ToString,
		sol::meta_function::addition, &AddVec3s,
		sol::meta_function::subtraction, &SubtractVec3s,
		sol::meta_function::unary_minus, &UnaryMinusVec3,
		sol::meta_function::multiplication, &MultiplyVec3Number,

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
@return A Vec3 object.
@function Vec3.new
*/
Vec3::Vec3(int aX, int aY, int aZ) : x{aX}, y{aY}, z{aZ}
{
}

Vec3::Vec3(PHD_3DPOS const& pos) : x{pos.xPos}, y{pos.yPos}, z{pos.zPos}
{
}

void Vec3::StoreInPHDPos(PHD_3DPOS& pos) const
{
	pos.xPos = x;
	pos.yPos = y;
	pos.zPos = z;
}

void Vec3::StoreInGameVector(GAME_VECTOR& pos) const
{
	pos.x = x;
	pos.y = y;
	pos.z = z;
}


/***
@tparam Vec3 Vec3 this Vec3
@treturn string A string showing the x, y, and z values of the Vec3
@function __tostring
*/
std::string Vec3::ToString() const
{
	return "{" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "}";
}

Vec3 AddVec3s(Vec3 const & one, Vec3 const & two)
{
	return Vec3{ one.x + two.x, one.y + two.y, one.z + two.z };
}

Vec3 SubtractVec3s(Vec3 const & one, Vec3 const & two)
{
	return Vec3{ one.x - two.x, one.y - two.y, one.z - two.z };
}

Vec3 MultiplyVec3Number(Vec3 const& one, double const & two)
{
	return Vec3{ static_cast<int>(one.x * two), static_cast<int>(one.y * two), static_cast<int>(one.z * two) };
}

Vec3 UnaryMinusVec3(Vec3 const& one)
{
	return Vec3{ one.x * -1, one.y * -1, one.z * -1 };
}

