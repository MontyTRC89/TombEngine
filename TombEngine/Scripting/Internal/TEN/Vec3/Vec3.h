#pragma once

class GameVector;
class Pose;

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
	Vec3(float x, float y, float z);
	Vec3(const Pose& pose);
	Vec3(const Vector3& pos);

	void StoreInPose(Pose& pos) const;
	void StoreInGameVector(GameVector& vector) const;

	// Setters
	void SetLength(float length);

	// Meta functions
	[[nodiscard]] std::string ToString() const;
	static Vec3 Add(const Vec3& vector0, const Vec3& vector1);
	static Vec3 Subtract(const Vec3& vector0, const Vec3& vector1);
	static Vec3 Multiply(const Vec3& vector0, const Vec3& vector1);
	static Vec3 MultiplyScale(const Vec3& vector, float scale);
	static Vec3 DivideScale(const Vec3& vector, float scale);
	static Vec3 UnaryMinus(const Vec3& vector);

	// Operators
	operator Vector3() const;
};
