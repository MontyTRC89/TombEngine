#pragma once

namespace TEN { namespace Math { class Vector2i; } }
namespace sol { class state; }

class Vec2
{
public:
	int x = 0;
	int y = 0;

	Vec2(int x, int y);
	Vec2(const TEN::Math::Vector2i& pos);

	operator TEN::Math::Vector2i() const;

	[[nodiscard]] std::string ToString() const;

	void ToLength(float newLength);

	static void Register(sol::table&);
};

Vec2 AddVec2s(const Vec2& vector0, const Vec2& vector1);
Vec2 SubtractVec2s(const Vec2& vector0, const Vec2& vector1);
Vec2 MultiplyVec2Number(const Vec2& vector, float scale);
Vec2 UnaryMinusVec2(const Vec2& vector);
