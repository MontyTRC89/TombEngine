#pragma once

struct Vector3i;

namespace sol {
	class state;
}

struct PoseData;
struct GameVector;

class Vec3 {
public:
	int x;
	int y;
	int z;

	Vec3(int x, int y, int z);
	Vec3(PoseData const& pos);
	Vec3(Vector3i const& pos);

	operator Vector3i() const;

	[[nodiscard]] std::string ToString() const;

	void ToLength(int newLength);
	void StoreInPHDPos(PoseData& pos) const;
	void StoreInGameVector(GameVector& vec) const;

	static void Register(sol::table &);
};

Vec3 AddVec3s(Vec3 const& one, Vec3 const& two);
Vec3 SubtractVec3s(Vec3 const& one, Vec3 const& two);
Vec3 MultiplyVec3Number(Vec3 const& one, double const & two);
Vec3 UnaryMinusVec3(Vec3 const& one);
