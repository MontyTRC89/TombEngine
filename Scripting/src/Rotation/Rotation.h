#pragma once

namespace sol {
	class state;
}
struct PHD_3DPOS;

class Rotation {
public:
	short								x{ 0 };
	short								y{ 0 };
	short								z{ 0 };

	Rotation() = default;
	Rotation(int x, int y, int z);
	Rotation(PHD_3DPOS const& pos);

	std::string ToString() const;

	void StoreInPHDPos(PHD_3DPOS& pos) const;

	static void Register(sol::table & parent);
};
