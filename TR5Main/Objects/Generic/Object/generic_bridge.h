#pragma once
#include "Specific\phd_global.h"

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

template <int tilt>
int BridgeFloorBorder(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];
	return item.pos.yPos;
}

template <int tilt>
int BridgeCeilingBorder(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];
	const auto height = item.pos.yPos + tilt * SECTOR(1) / 4;
	return height + SECTOR(1) / 16;
}
