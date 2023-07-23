#pragma once

namespace sol { class state; }
namespace TEN::Math { class Vector2i; }
class Vec2;

using namespace TEN::Math;

class Vec2i
{
public:
	static void Register(sol::table& parent);

	// Members
	int x = 0;
	int y = 0;

	// Constructors
	Vec2i(int x, int y);
	Vec2i(const Vector2i& pos);

	// Setters
	void SetLength(float length);

	// Converters
	Vec2 ToVec2();

	// Meta functions
	[[nodiscard]] std::string ToString() const;
	static Vec2i Add(const Vec2i& vector0, const Vec2i& vector1);
	static Vec2i AddVec2(const Vec2i& vector0, const Vec2& vector1);
	static Vec2i Subtract(const Vec2i& vector0, const Vec2i& vector1);
	static Vec2i SubtractVec2(const Vec2i& vector0, const Vec2& vector1);
	static Vec2i Multiply(const Vec2i& vector0, const Vec2i& vector1);
	static Vec2i MultiplyVec2(const Vec2i& vector0, const Vec2& vector1);
	static Vec2i MultiplyScale(const Vec2i& vector, float scale);
	static Vec2i DivideScale(const Vec2i& vector, float scale);
	static Vec2i UnaryMinus(const Vec2i& vector);

	// Operators
	operator Vector2i() const;
};
