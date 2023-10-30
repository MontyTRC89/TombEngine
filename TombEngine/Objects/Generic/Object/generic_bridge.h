#pragma once
#include "Math/Math.h"
#include "Specific/level.h"
#include "Game/items.h"
#include "Game/collision/floordata.h"

using namespace TEN::Collision::Floordata;

void InitializeBridge(short itemNumber);
int GetOffset(short angle, int x, int z);

template <int tiltGrade>
std::optional<int> BridgeFloor(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];

	auto boxHeight = GetBridgeItemIntersect(Vector3i(x, y, z), itemNumber, false);
	if (boxHeight.has_value() && tiltGrade != 0)
	{
		int height = item.Pose.Position.y + (tiltGrade * ((GetOffset(item.Pose.Orientation.y, x, z) / 4) + (BLOCK(1) / 8)));
		return height;
	}

	return boxHeight;
}

template <int tiltGrade>
std::optional<int> BridgeCeiling(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];

	auto boxHeight = GetBridgeItemIntersect(Vector3i(x, y, z), itemNumber, true);
	if (boxHeight.has_value() && tiltGrade != 0)
	{
		int height = item.Pose.Position.y + (tiltGrade * ((GetOffset(item.Pose.Orientation.y, x, z) / 4) + (BLOCK(1) / 8)));
		return (height + CLICK(1));
	}

	return boxHeight;
}

template <int tiltGrade>
int BridgeFloorBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, false);
}

template <int tiltGrade>
int BridgeCeilingBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, true);
}
