#include "framework.h"
#include "Game/misc.h"

#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Setup.h"
#include "Specific/level.h"

using std::vector;

CreatureInfo* GetCreatureInfo(ItemInfo* item)
{
	return (CreatureInfo*)item->Data;
}

void TargetNearestEntity(ItemInfo* item, CreatureInfo* creature, std::vector<GAME_OBJECT_ID> ignoredItemIds)
{
	float nearestDistance = INFINITY;

	for (int i = 0; i < g_Level.NumItems; i++)
	{
		auto* targetEntity = &g_Level.Items[i];
		if (targetEntity == nullptr || (targetEntity->Index == item->Index)) // Ignore itself !
			continue;

		bool checkPassed = true;
		for (auto& itemId : ignoredItemIds)
		{
			if (itemId == ID_NO_OBJECT) // NOTE: if there is any ID_NO_OBJECT then ignore the other itemids and attack the target !
			{
				checkPassed = true; // just-in-case checkPassed was false !
				break;
			}
			else if (targetEntity->ObjectNumber == itemId)
				checkPassed = false;
		}

		if ((targetEntity != item &&
			 targetEntity->HitPoints > 0 &&
			 targetEntity->Status != ITEM_INVISIBLE) && checkPassed)
		{
			float distance = Vector3i::Distance(item->Pose.Position, targetEntity->Pose.Position);
			if (distance < nearestDistance)
			{
				creature->Enemy = targetEntity;
				nearestDistance = distance;
			}
		}
	}
}

bool IsNextSectorValid(const ItemInfo& item, const Vector3& dir, float dist)
{
	auto projectedPos = Geometry::TranslatePoint(item.Pose.Position, dir, dist);
	auto pointColl = GetCollision(item.Pose.Position, item.RoomNumber, dir, dist);
	int height = GameBoundingBox(&item).GetHeight();

	// Test for wall.
	if (pointColl.Block->IsWall(projectedPos.x, projectedPos.z))
		return false;

	// Test for slippery slope.
	if (pointColl.Position.FloorSlope)
		return false;

	// Flat floor.
	if ((abs(pointColl.FloorTilt.x) == 0 && abs(pointColl.FloorTilt.y) == 0))
	{
		// Test for step.
		int relFloorHeight = abs(pointColl.Position.Floor - item.Pose.Position.y);
		if (relFloorHeight >= CLICK(1) && item.Pose.Position.y >= pointColl.Position.Floor)
			return false;
	}
	// Sloped floor.
	else
	{
		// Half block.
		int relFloorHeight = abs(pointColl.Position.Floor - item.Pose.Position.y);
		if (relFloorHeight > CLICK(1))
			return false;

		short slopeAngle = ANGLE(0.0f);
		if (pointColl.FloorTilt.x > 0)
		{
			slopeAngle = ANGLE(-90.0f);
		}
		else if (pointColl.FloorTilt.x < 0)
		{
			slopeAngle = ANGLE(90.0f);
		}

		if (pointColl.FloorTilt.y > 0 && pointColl.FloorTilt.y > abs(pointColl.FloorTilt.x))
		{
			slopeAngle = ANGLE(180.0f);
		}
		else if (pointColl.FloorTilt.y < 0 && -pointColl.FloorTilt.y > abs(pointColl.FloorTilt.x))
		{
			slopeAngle = ANGLE(0.0f);
		}

		short dirAngle = phd_atan(dir.z, dir.x);
		short alignAngle = Geometry::GetShortestAngle(slopeAngle, dirAngle);

		// Test if slope aspect is aligned with direction.
		if (alignAngle != 0 && alignAngle != ANGLE(180.0f))
			return false;
	}

	// Check for diagonal split.
	if (pointColl.Position.DiagonalStep)
		return false;

	// Test ceiling height.
	int relCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);

	if (relCeilHeight <= height)
		return false;

	// Check for blocked grey box.
	if (g_Level.Boxes[pointColl.Block->Box].flags & BLOCKABLE)
	{
		if (g_Level.Boxes[pointColl.Block->Box].flags & BLOCKED)
			return false;
	}

	// Check for inaccessible sector.
	if (pointColl.Block->Box == NO_ZONE)
		return false;

	// Check for stopper flag.
	if (pointColl.Block->Stopper)
		return false;

	return true;
}