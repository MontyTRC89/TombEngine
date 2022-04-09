#pragma once

namespace sol {
	class state;
}
struct PHD_3DPOS;
struct GAME_VECTOR;


class Position {
public:
	int x;
	int y;
	int z;

	Position(int x, int y, int z);
	Position(PHD_3DPOS const& pos);

	std::string ToString() const;

	void StoreInPHDPos(PHD_3DPOS& pos) const;
	void StoreInGameVector(GAME_VECTOR& vec) const;

	static void Register(sol::table &);
};

Position AddPositions(Position const& one, Position const& two);
