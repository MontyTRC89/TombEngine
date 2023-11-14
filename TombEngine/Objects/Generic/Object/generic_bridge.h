#pragma once
#include "Game/collision/floordata.h"
#include "Game/items.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;

class Vector3i;
struct ItemInfo;

void InitializeBridge(short itemNumber);
int GetOffset(short angle, int x, int z);

template <int tiltGrade>
std::optional<int> GetBridgeFloorHeight(const ItemInfo& item, const Vector3i& pos)
{
	auto boxHeight = GetBridgeItemIntersect(item, pos, false);
	if (boxHeight.has_value() && tiltGrade != 0)
	{
		int height = item.Pose.Position.y + (tiltGrade * ((GetOffset(item.Pose.Orientation.y, pos.x, pos.z) / 4) + (BLOCK(1 / 8.0f))));
		return height;
	}

	return boxHeight;
}

template <int tiltGrade>
std::optional<int> GetBridgeCeilingHeight(const ItemInfo& item, const Vector3i& pos)
{
	auto boxHeight = GetBridgeItemIntersect(item, pos, true);
	if (boxHeight.has_value() && tiltGrade != 0)
	{
		int height = item.Pose.Position.y + (tiltGrade * ((GetOffset(item.Pose.Orientation.y, pos.x, pos.z) / 4) + (BLOCK(1 / 8.0f))));
		return (height + CLICK(1));
	}

	return boxHeight;
}

template <int tiltGrade>
int GetBridgeFloorBorder(const ItemInfo& item)
{
	return GetBridgeBorder(item, false);
}

template <int tiltGrade>
int GetBridgeCeilingBorder(const ItemInfo& item)
{
	return GetBridgeBorder(item, true);
}
