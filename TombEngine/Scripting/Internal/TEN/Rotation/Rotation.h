#pragma once

namespace sol {
	class state;
}
class Pose;

class Rotation {
public:
	short								x{ 0 };
	short								y{ 0 };
	short								z{ 0 };

	Rotation() = default;
	Rotation(int x, int y, int z);
	Rotation(Pose const& pos);

	std::string ToString() const;

	void StoreInPHDPos(Pose& pos) const;

	static void Register(sol::table & parent);
};
