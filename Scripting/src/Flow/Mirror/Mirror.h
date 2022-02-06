#pragma once

namespace sol {
	class state;
}

struct Mirror
{
	short Room{ -1 };
	int StartX{ 0 };
	int EndX{ 0 };
	int StartZ{ 0 };
	int EndZ{ 0 };

	static void Register(sol::table & parent);
	Mirror() = default;

	Mirror(short room, int startX, int endX, int startZ, int endZ);
};

