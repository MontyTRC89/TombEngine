#pragma once

namespace sol {
	class state;
}
struct PHD_3DPOS;

class Position {
public:
	int x;
	int y;
	int z;

	Position(int x, int y, int z);
	Position(PHD_3DPOS const& pos);

	std::string ToString() const;

	void StoreInPHDPos(PHD_3DPOS& pos) const;

	static void Register(sol::state*);
};
