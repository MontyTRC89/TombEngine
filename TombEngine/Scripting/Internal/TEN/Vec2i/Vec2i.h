#pragma once

namespace sol { class state; }
namespace TEN::Math { class Vector2i; }

using namespace TEN::Math;

class Vec2i
{
public:
	int x = 0;
	int y = 0;

	static void Register(sol::table& parent);

	Vec2i(int x, int y);
	Vec2i(const Vector2i& pos);

	[[nodiscard]] std::string ToString() const;
	void ToLength(float length);

	static Vec2i Add(const Vec2i& vector0, const Vec2i& vector1);
	static Vec2i Subtract(const Vec2i& vector0, const Vec2i& vector1);
	static Vec2i MultiplyByScale(const Vec2i& vector, float scale);
	static Vec2i UnaryMinus(const Vec2i& vector);

	operator Vector2i() const;
};
