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
	Vec3(const Vector3i& pos);
	Vec3(const Vector3& pos);

	void StoreInPose(Pose& pos) const;
	void StoreInGameVector(GameVector& vector) const;
	
	// Utilities
	void		Normalize();
	static Vec3 StaticNormalize(const Vec3& vector);
	void		SetLength(float length);
	void		ClampLength(float lengthMax);
	void		Rotate(const Rotation& rot);
	static Vec3 StaticRotate(const Vec3& vector, const Rotation& rot);
	void		Lerp(const Vec3& vector, float alpha);
	static Vec3 StaticLerp(const Vec3& vector0, const Vec3& vector1, float alpha);
	void		Cross(const Vec3& vector);
	static Vec3 StaticCross(const Vec3& vector0, const Vec3& vector1);

	float		 Length() const;
	float		 Distance(const Vec3& vector) const;
	static float StaticDistance(const Vec3& vector0, const Vec3& vector1);
	float		 Dot(const Vec3& vector) const;
	static float StaticDot(const Vec3& vector0, const Vec3& vector1);

	// Meta functions
	[[nodiscard]] std::string ToString() const;
	static Vec3 Add(const Vec3& vector0, const Vec3& vector1);
	static Vec3 Subtract(const Vec3& vector0, const Vec3& vector1);
	static Vec3 Multiply(const Vec3& vector0, const Vec3& vector1);
	static Vec3 MultiplyScale(const Vec3& vector, float scale);
	static Vec3 DivideScale(const Vec3& vector, float scale);
	static Vec3 UnaryMinus(const Vec3& vector);
	static bool IsEqualTo(const Vec3& vector0, const Vec3& vector1);

	// Operators
	operator Vector3() const;
};
