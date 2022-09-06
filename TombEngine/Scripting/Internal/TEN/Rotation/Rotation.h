#pragma once

namespace sol {
	class state;
}
struct PoseData;

class Rotation {
public:
	short								x{ 0 };
	short								y{ 0 };
	short								z{ 0 };

	Rotation() = default;
	Rotation(int x, int y, int z);
	Rotation(PoseData const& pos);

	std::string ToString() const;

	void StoreInPHDPos(PoseData& pos) const;

	static void Register(sol::table & parent);
};
