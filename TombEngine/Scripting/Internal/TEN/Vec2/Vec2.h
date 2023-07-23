#pragma once

namespace sol { class state; }
class Vec2i;

class Vec2
{
public:
	static void Register(sol::table& parent);

	// Members
	float x = 0;
	float y = 0;

	// Constructors
	Vec2(float x, float y);
	Vec2(const Vector2& pos);

	// Setters
	void SetLength(float length);

	// Converters
	Vec2i ToVec2i();

	// Meta functions
	[[nodiscard]] std::string ToString() const;
	static Vec2 Add(const Vec2& vector0, const Vec2& vector1);
	static Vec2 AddVec2i(const Vec2& vector0, const Vec2i& vector1);
	static Vec2 Subtract(const Vec2& vector0, const Vec2& vector1);
	static Vec2 SubtractVec2i(const Vec2& vector0, const Vec2i& vector1);
	static Vec2 Multiply(const Vec2& vector0, const Vec2& vector1);
	static Vec2 MultiplyVec2i(const Vec2& vector0, const Vec2i& vector1);
	static Vec2 MultiplyScale(const Vec2& vector, float scale);
	static Vec2 DivideScale(const Vec2& vector, float scale);
	static Vec2 UnaryMinus(const Vec2& vector);

	// Operators
	operator Vector2() const;
};
