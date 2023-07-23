#pragma once

namespace sol { class state; }

class Vec2
{
public:
	float x = 0;
	float y = 0;

	static void Register(sol::table& parent);

	Vec2(float x, float y);
	Vec2(const Vector2& pos);

	[[nodiscard]] std::string ToString() const;
	void ToLength(float length);

	static Vec2 Add(const Vec2& vector0, const Vec2& vector1);
	static Vec2 Subtract(const Vec2& vector0, const Vec2& vector1);
	static Vec2 MultiplyByScale(const Vec2& vector, float scale);
	static Vec2 UnaryMinus(const Vec2& vector);

	operator Vector2() const;
};
