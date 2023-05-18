#pragma once

class GameVector;
class Pose;
class Vector3i;
namespace sol { class state; }

class Vec3
{
public:
	int x = 0;
	int y = 0;
	int z = 0;

	Vec3(int x, int y, int z);
	Vec3(const Pose& pose);
	Vec3(const Vector3i& pos);

	operator Vector3i() const;

	[[nodiscard]] std::string ToString() const;

	void ToLength(float newLength);
	void StoreInPose(Pose& pos) const;
	void StoreInGameVector(GameVector& vector) const;

	static void Register(sol::table&);
};

Vec3 AddVec3s(const Vec3& vector0, const Vec3& vector1);
Vec3 SubtractVec3s(const Vec3& vector0, const Vec3& vector1);
Vec3 MultiplyVec3Number(const Vec3& vector, float scale);
Vec3 UnaryMinusVec3(const Vec3& vector);
