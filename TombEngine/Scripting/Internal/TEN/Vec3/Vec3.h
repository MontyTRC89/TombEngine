#pragma once

class GameVector;
class Pose;
class Rotation;
class Vector3i;
namespace sol { class state; }

class Vec3
{
public:
	static void Register(sol::table& parent);

	// Members
	float x = 0;
	float y = 0;
	float z = 0;

	// Constructors
	Vec3() {};
	Vec3(float x, float y, float z);
	Vec3(const Vector3& pos);
	Vec3(const Vector3i& pos);

	// Utilities
	Vec3  Normalize() const;
	Vec3  Rotate(const Rotation& rot) const;
	Vec3  Lerp(const Vec3& vector, float alpha) const;
	Vec3  Cross(const Vec3& vector) const;
	float Dot(const Vec3& vector) const;
	float Distance(const Vec3& vector) const;
	float Length() const;

	// Meta functions
	[[nodiscard]] std::string ToString() const;
	static Vec3 Add(const Vec3& vector0, const Vec3& vector1);
	static Vec3 Subtract(const Vec3& vector0, const Vec3& vector1);
	static Vec3 Multiply(const Vec3& vector0, const Vec3& vector1);
	static Vec3 MultiplyByValue(const Vec3& vector, float value);
	static Vec3 DivideByValue(const Vec3& vector, float value);
	static Vec3 UnaryMinus(const Vec3& vector);
	static bool IsEqualTo(const Vec3& vector0, const Vec3& vector1);

	// Converters
	Vector3	   ToVector3() const;
	Vector3i   ToVector3i() const;
	GameVector ToGameVector() const;

	// Operators
	operator Vector3() const;
};
