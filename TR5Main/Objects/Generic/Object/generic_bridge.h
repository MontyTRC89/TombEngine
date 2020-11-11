#pragma once
#include "phd_global.h"

int GetOffset(short angle, int x, int z);

template <int tilt>
std::tuple<std::optional<int>, bool> BridgeFloor(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	if (abs(item.pos.xPos - x) <= SECTOR(1) / 2 && abs(item.pos.zPos - z) <= SECTOR(1) / 2)
	{
		auto height = item.pos.yPos + tilt * (GetOffset(item.pos.yRot, x, z) / 4 + SECTOR(1) / 8);
		return std::make_tuple(std::optional{height}, y > height && y < height + SECTOR(1) / 16);
	}
	return std::make_tuple(std::nullopt, false);
}

template <int tilt>
std::tuple<std::optional<int>, bool> BridgeCeiling(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	if (abs(item.pos.xPos - x) <= SECTOR(1) / 2 && abs(item.pos.zPos - z) <= SECTOR(1) / 2)
	{
		auto height = item.pos.yPos + tilt * (GetOffset(item.pos.yRot, x, z) / 4 + SECTOR(1) / 8);
		return std::make_tuple(std::optional{height + SECTOR(1) / 16}, y > height && y < height + SECTOR(1) / 16);
	}
	return std::make_tuple(std::nullopt, false);
}
