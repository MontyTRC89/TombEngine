#pragma once

namespace sol { class state; }
namespace TEN::Math { class Vector2i; }

using namespace TEN::Math;

class Vec2
{
public:
	int x = 0;
	int y = 0;

	Vec2(int x, int y);
	Vec2(const Vector2i& pos);

	operator Vector2i() const;

	[[nodiscard]] std::string ToString() const;

	void ToLength(int newLength);

	static void Register(sol::table&);
};

Vec2 AddVec2s(const Vec2& vector0, const Vec2& vector1);
Vec2 SubtractVec2s(const Vec2& vector0, const Vec2& vector1);
Vec2 MultiplyVec2Number(const Vec2& vector, float scale);
Vec2 UnaryMinusVec2(const Vec2& vector);
