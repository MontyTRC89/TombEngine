#include "framework.h"
#include "tr5_expandingplatform.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/control/control.h"
#include "Game/control/box.h"
#include "Game/animation.h"
#include "Sound/sound.h"
#include "Game/camera.h"
#include "Game/Lara/lara.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"

using namespace TEN::Floordata;

void InitialiseExpandingPlatform(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	short roomNumber = item->RoomNumber;
	FloorInfo* floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
	g_Level.Boxes[floor->Box].flags &= ~BLOCKED;

	// Set mutators to default
	ExpandingPlatformUpdateMutators(itemNumber);

	if (item->TriggerFlags < 0)
	{
		item->AIBits |= ALL_AIOBJ;
		AddActiveItem(itemNumber);
		item->Status = ITEM_ACTIVE;
	}

	TEN::Floordata::UpdateBridgeItem(itemNumber);
}

bool IsOnExpandingPlatform(int itemNumber, int x, int z)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->ItemFlags[1] <= 0)
		return false;

	int xb = x / SECTOR(1);
	int zb = z / SECTOR(1);
	int itemxb = item->Pose.Position.x / SECTOR(1);
	int itemzb = item->Pose.Position.z / SECTOR(1);

	auto bounds = GetBoundsAccurate(item);
	auto halfWidth = abs(bounds->Z2 - bounds->Z1) / 2;

	if (item->Pose.Orientation.y == Angle::DegToRad(90.0f))
	{
		int xBorder = item->Pose.Position.x + halfWidth - SECTOR(1) * item->ItemFlags[1] / 4096;
		if (x < xBorder || zb != itemzb || xb != itemxb)
			return false;
	}
	else if (item->Pose.Orientation.y == Angle::DegToRad(270.0f))
	{
		int xBorder = item->Pose.Position.x - halfWidth + SECTOR(1) * item->ItemFlags[1] / 4096;
		if (x > xBorder || zb != itemzb || xb != itemxb)
			return false;
	}
	else if (item->Pose.Orientation.y == 0)
	{
		int zBorder = item->Pose.Position.z + halfWidth - SECTOR(1) * item->ItemFlags[1] / 4096;
		if (z < zBorder || zb != itemzb || xb != itemxb)
			return false;
	}
	else if (item->Pose.Orientation.y == Angle::DegToRad(180.0f))
	{
		int zBorder = item->Pose.Position.z - halfWidth + SECTOR(1) * item->ItemFlags[1] / 4096;
		if (z > zBorder || zb != itemzb || xb != itemxb)
			return false;
	}

	return GetBridgeItemIntersect(itemNumber, x, item->Pose.Position.y, z, false).has_value();
}

bool IsInFrontOfExpandingPlatform(int itemNumber, int x, int y, int z, int margin)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->ItemFlags[1] <= 0)
		return false;

	const auto topHeight = GetBridgeBorder(itemNumber, false);
	const auto bottomHeight = GetBridgeBorder(itemNumber, true);

	if (LaraItem->Pose.Position.y < topHeight - 32 || LaraItem->Pose.Position.y > bottomHeight + 32)
		return false;

	auto bounds = GetBoundsAccurate(item);
	auto halfWidth = abs(bounds->Z2 - bounds->Z1) / 2;
	
	int xb = x / SECTOR(1);
	int zb = z / SECTOR(1);
	int itemxb = item->Pose.Position.x / SECTOR(1);
	int itemzb = item->Pose.Position.z / SECTOR(1);

	if (item->Pose.Orientation.y == Angle::DegToRad(90))
	{
		int xBorder = item->Pose.Position.x + halfWidth - margin - SECTOR(1) * item->ItemFlags[1] / 4096;
		int xBorder2 = item->Pose.Position.x + halfWidth;
		if (x < xBorder || zb != itemzb || x > xBorder2)
			return false;
	}
	else if (item->Pose.Orientation.y == Angle::DegToRad(270))
	{
		int xBorder = item->Pose.Position.x - halfWidth + margin + SECTOR(1) * item->ItemFlags[1] / 4096;
		int xBorder2 = item->Pose.Position.x - halfWidth;
		if (x > xBorder || zb != itemzb || x < xBorder2)
			return false;
	}
	else if (item->Pose.Orientation.y == 0)
	{
		int zBorder = item->Pose.Position.z + halfWidth - margin - SECTOR(1) * item->ItemFlags[1] / 4096;
		int zBorder2 = item->Pose.Position.z + halfWidth;
		if (z < zBorder || xb != itemxb || z > zBorder2)
			return false;
	}
	else if (item->Pose.Orientation.y == Angle::DegToRad(180))
	{
		int zBorder = item->Pose.Position.z - halfWidth + margin + SECTOR(1) * item->ItemFlags[1] / 4096;
		int zBorder2 = item->Pose.Position.z - halfWidth;
		if (z > zBorder || xb != itemxb || z < zBorder2)
			return false;
	}

	return GetBridgeItemIntersect(itemNumber, x, item->Pose.Position.y, z, false).has_value();
}

void ShiftLaraOnPlatform(short itemNumber, bool isExpanding)
{
	auto* item = &g_Level.Items[itemNumber];

	float angle = item->Pose.Orientation.y;
	int xShift = 0;
	int zShift = 0;

	if (item->ItemFlags[1] <= 0)
		return;

	const auto height = GetBridgeBorder(itemNumber, false);

	if (IsOnExpandingPlatform(itemNumber, LaraItem->Pose.Position.x, LaraItem->Pose.Position.z))
	{
		//Slide Lara if on top of platform
		if (LaraItem->Pose.Position.y < height - 32 || LaraItem->Pose.Position.y > height + 32)
			return;
		if (angle == 0)
			zShift = isExpanding ? -16 : 16;
		else if (angle == Angle::DegToRad(180.0f))
			zShift = isExpanding ? 16 : -16;
		else if (angle == Angle::DegToRad(90.0f))
			xShift = isExpanding ? -16 : 16;
		else if (angle == Angle::DegToRad(-90.0f))
			xShift = isExpanding ? 16 : -16;
	} 
	else if (isExpanding && 
		IsInFrontOfExpandingPlatform(itemNumber, LaraItem->Pose.Position.x, LaraItem->Pose.Position.y, LaraItem->Pose.Position.z, LaraCollision.Setup.Radius))
	{
		//Push Lara if in front of expanding platform
		if (angle == 0)
			zShift = -LaraCollision.Setup.Radius / 6;
		else if (angle == Angle::DegToRad(180.0f))
			zShift = LaraCollision.Setup.Radius / 6;
		else if (angle == Angle::DegToRad(90.0f))
			xShift = -LaraCollision.Setup.Radius / 6;
		else if (angle == Angle::DegToRad(-90.0f))
			xShift = LaraCollision.Setup.Radius / 6;
	}

	auto coll = &LaraCollision;
	GetCollisionInfo(coll, LaraItem, Vector3Int(xShift, 0, zShift));

	if (coll->Middle.Ceiling >= 0 || coll->HitStatic)
		return;

	if (zShift != 0)
		LaraItem->Pose.Position.z += zShift;
	if (xShift != 0)
		LaraItem->Pose.Position.x += xShift;
}

void ControlExpandingPlatform(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (!item->ItemFlags[2])
			item->ItemFlags[2] = 1;

		if (item->TriggerFlags < 0)
			item->ItemFlags[1] = 1;
		else if (item->ItemFlags[1] < 4096)
		{
			SoundEffect(SFX_TR4_RAISING_BLOCK, &item->Pose);

			item->ItemFlags[1] += 64;
			ShiftLaraOnPlatform(itemNumber, true);

			if (item->TriggerFlags > 0)
			{
				if (abs(item->Pose.Position.x - Camera.pos.x) < 10240 &&
					abs(item->Pose.Position.x - Camera.pos.x) < 10240 &&
					abs(item->Pose.Position.x - Camera.pos.x) < 10240)
				{
					if (item->ItemFlags[1] == 64 || item->ItemFlags[1] == 4096)
						Camera.bounce = -32;
					else
						Camera.bounce = -16;
				}
			}
		}
	}
	else if (item->ItemFlags[1] <= 0 || item->TriggerFlags < 0)
	{
		if (item->ItemFlags[2])
		{
			item->ItemFlags[1] = 0;
			item->ItemFlags[2] = 0;
		}
	}
	else
	{
		SoundEffect(SFX_TR4_RAISING_BLOCK, &item->Pose);

		if (item->TriggerFlags >= 0)
		{
			if (abs(item->Pose.Position.x - Camera.pos.x) < 10240 &&
				abs(item->Pose.Position.x - Camera.pos.x) < 10240 &&
				abs(item->Pose.Position.x - Camera.pos.x) < 10240)
			{
				if (item->ItemFlags[1] == 64 || item->ItemFlags[1] == 4096)
					Camera.bounce = -32;
				else
					Camera.bounce = -16;
			}
		}

		item->ItemFlags[1] -= 64;
		ShiftLaraOnPlatform(itemNumber, false);
	}

	ExpandingPlatformUpdateMutators(itemNumber);
}

std::optional<int> ExpandingPlatformFloor(short itemNumber, int x, int y, int z)
{
	if (!IsOnExpandingPlatform(itemNumber, x, z))
		return std::nullopt;

	return GetBridgeItemIntersect(itemNumber, x, y, z, false);
}

std::optional<int> ExpandingPlatformCeiling(short itemNumber, int x, int y, int z)
{
	if (!IsOnExpandingPlatform(itemNumber, x, z))
		return std::nullopt;

	return GetBridgeItemIntersect(itemNumber, x, y, z, true);
}

int ExpandingPlatformFloorBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, false);
}

int ExpandingPlatformCeilingBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, true);
}

void ExpandingPlatformUpdateMutators(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	auto* bounds = GetBoundsAccurate(item);
	float normalizedThickness = item->ItemFlags[1] / 4096.0f;
	int width = abs(bounds->Z2 - bounds->Z1) / 2;
	float offset = width * normalizedThickness;

	// Update bone mutators
	float zTranslate = 0.0f;
	if (item->Pose.Orientation.y == 0)   zTranslate =  width  - offset;
	if (item->Pose.Orientation.y == Angle::DegToRad(90.0f))  zTranslate = -offset + width;
	if (item->Pose.Orientation.y == Angle::DegToRad(180.0f)) zTranslate = -offset + width;
	if (item->Pose.Orientation.y == Angle::DegToRad(270.0f)) zTranslate =  width  - offset;

	for (int i = 0; i < item->Animation.Mutator.size(); i++)
	{
		item->Animation.Mutator[i].Offset = Vector3(0, 0, zTranslate);
		item->Animation.Mutator[i].Scale = Vector3(1.0f, 1.0f, item->ItemFlags[1] / 4096.0f);
	}
}
