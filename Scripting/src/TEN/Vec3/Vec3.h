#pragma once

namespace sol {
	class state;
}
struct PHD_3DPOS;
struct GAME_VECTOR;


class Vec3 {
public:
	int x;
	int y;
	int z;

	Vec3(int x, int y, int z);
	Vec3(PHD_3DPOS const& pos);

	std::string ToString() const;

	void StoreInPHDPos(PHD_3DPOS& pos) const;
	void StoreInGameVector(GAME_VECTOR& vec) const;

	static void Register(sol::table &);
};

Vec3 AddVec3s(Vec3 const& one, Vec3 const& two);
