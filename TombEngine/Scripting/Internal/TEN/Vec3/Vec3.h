#pragma once

namespace sol {
	class state;
}

struct PHD_3DPOS;
struct GameVector;

class Vec3 {
public:
	int x;
	int y;
	int z;

	Vec3(int x, int y, int z);
	Vec3(PHD_3DPOS const& pos);

	[[nodiscard]] std::string ToString() const;

	void ToLength(int newLength);
	void StoreInPHDPos(PHD_3DPOS& pos) const;
	void StoreInGameVector(GameVector& vec) const;

	static void Register(sol::table &);
};

Vec3 AddVec3s(Vec3 const& one, Vec3 const& two);
Vec3 SubtractVec3s(Vec3 const& one, Vec3 const& two);
Vec3 MultiplyVec3Number(Vec3 const& one, double const & two);
Vec3 UnaryMinusVec3(Vec3 const& one);
