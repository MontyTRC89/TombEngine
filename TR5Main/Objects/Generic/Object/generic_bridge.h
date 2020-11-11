#pragma once
#include "phd_global.h"

int GetOffset(short angle, int x, int z);

template <int tilt, int displacement>
std::optional<int> BridgeHeight(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	if (abs(item.pos.xPos - x) <= SECTOR(1) / 2 && abs(item.pos.zPos - z) <= SECTOR(1) / 2)
		return std::optional{item.pos.yPos + tilt * (GetOffset(item.pos.yRot, x, z) / 4 + SECTOR(1) / 8) + displacement};
	return std::nullopt;
}
