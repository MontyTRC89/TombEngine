#pragma once
#include "phd_global.h"

void InitialiseBridge(short itemNumber);
int GetOffset(short angle, int x, int z);

template <int tilt>
std::optional<int> BridgeFloor(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	const auto height = item.pos.yPos + tilt * (GetOffset(item.pos.yRot, x, z) / 4 + SECTOR(1) / 8);
	return std::optional{height};
}

template <int tilt>
std::optional<int> BridgeCeiling(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	const auto height = item.pos.yPos + tilt * (GetOffset(item.pos.yRot, x, z) / 4 + SECTOR(1) / 8);
	return std::optional{height + SECTOR(1) / 16};
}
