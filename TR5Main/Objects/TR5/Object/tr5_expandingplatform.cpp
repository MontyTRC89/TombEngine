#include "framework.h"
#include "tr5_expandingplatform.h"
#include "items.h"
#include "level.h"
#include "setup.h"
#include "control/control.h"
#include "control/box.h"
#include "animation.h"
#include "Sound/sound.h"
#include "camera.h"
#include "lara.h"
#include "collision/collide_room.h"
#include "collision/floordata.h"

using namespace TEN::Floordata;

void InitialiseExpandingPlatform(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	g_Level.Boxes[floor->Box].flags &= ~BLOCKED;

	// Set mutators to default
	ExpandingPlatformUpdateMutators(itemNumber);

	if (item->triggerFlags < 0)
	{
		item->aiBits |= ALL_AIOBJ;
		AddActiveItem(itemNumber);
		item->status = ITEM_ACTIVE;
	}

	TEN::Floordata::UpdateBridgeItem(itemNumber);
}

bool IsOnExpandingPlatform(int itemNumber, int x, int z)
{
	auto item = &g_Level.Items[itemNumber];

	if (item->itemFlags[1] <= 0)
		return false;

	int xb = x / SECTOR(1);
	int zb = z / SECTOR(1);
	int itemxb = item->pos.xPos / SECTOR(1);
	int itemzb = item->pos.zPos / SECTOR(1);

	auto bounds = GetBoundsAccurate(item);
	auto halfWidth = abs(bounds->Z2 - bounds->Z1) / 2;

	if (item->pos.yRot == ANGLE(90))
	{
		auto xBorder = item->pos.xPos + halfWidth - SECTOR(1) * item->itemFlags[1] / 4096;
		if (x < xBorder || zb != itemzb || xb != itemxb)
			return false;
	}
	else if (item->pos.yRot == ANGLE(270))
	{
		auto xBorder = item->pos.xPos - halfWidth + SECTOR(1) * item->itemFlags[1] / 4096;
		if (x > xBorder || zb != itemzb || xb != itemxb)
			return false;
	}
	else if (item->pos.yRot == 0)
	{
		auto zBorder = item->pos.zPos + halfWidth - SECTOR(1) * item->itemFlags[1] / 4096;
		if (z < zBorder || zb != itemzb || xb != itemxb)
			return false;
	}
	else if (item->pos.yRot == ANGLE(180))
	{
		auto zBorder = item->pos.zPos - halfWidth + SECTOR(1) * item->itemFlags[1] / 4096;
		if (z > zBorder || zb != itemzb || xb != itemxb)
			return false;
	}

	return GetBridgeItemIntersect(itemNumber, x, item->pos.yPos, z, false).has_value();
}

bool IsInFrontOfExpandingPlatform(int itemNumber, int x, int y, int z, int margin)
{
	auto item = &g_Level.Items[itemNumber];

	if (item->itemFlags[1] <= 0)
		return false;

	const auto topHeight = GetBridgeBorder(itemNumber, false);
	const auto bottomHeight = GetBridgeBorder(itemNumber, true);

	if (LaraItem->pos.yPos < topHeight - 32 || LaraItem->pos.yPos > bottomHeight + 32)
		return false;

	auto bounds = GetBoundsAccurate(item);
	auto halfWidth = abs(bounds->Z2 - bounds->Z1) / 2;
	
	int xb = x / SECTOR(1);
	int zb = z / SECTOR(1);
	int itemxb = item->pos.xPos / SECTOR(1);
	int itemzb = item->pos.zPos / SECTOR(1);

	if (item->pos.yRot == ANGLE(90))
	{
		auto xBorder = item->pos.xPos + halfWidth - margin - SECTOR(1) * item->itemFlags[1] / 4096;
		auto xBorder2 = item->pos.xPos + halfWidth;
		if (x < xBorder || zb != itemzb || x > xBorder2)
			return false;
	}
	else if (item->pos.yRot == ANGLE(270))
	{
		auto xBorder = item->pos.xPos - halfWidth + margin + SECTOR(1) * item->itemFlags[1] / 4096;
		auto xBorder2 = item->pos.xPos - halfWidth;
		if (x > xBorder || zb != itemzb || x < xBorder2)
			return false;
	}
	else if (item->pos.yRot == 0)
	{
		auto zBorder = item->pos.zPos + halfWidth - margin - SECTOR(1) * item->itemFlags[1] / 4096;
		auto zBorder2 = item->pos.zPos + halfWidth;
		if (z < zBorder || xb != itemxb || z > zBorder2)
			return false;
	}
	else if (item->pos.yRot == ANGLE(180))
	{
		auto zBorder = item->pos.zPos - halfWidth + margin + SECTOR(1) * item->itemFlags[1] / 4096;
		auto zBorder2 = item->pos.zPos - halfWidth;
		if (z > zBorder || xb != itemxb || z < zBorder2)
			return false;
	}

	return GetBridgeItemIntersect(itemNumber, x, item->pos.yPos, z, false).has_value();
}

void ShiftLaraOnPlatform(short itemNumber, bool isExpanding)
{
	auto item = &g_Level.Items[itemNumber];
	short angle = item->pos.yRot;
	int xShift = 0;
	int zShift = 0;

	if (item->itemFlags[1] <= 0)
		return;

	const auto height = GetBridgeBorder(itemNumber, false);

	if (IsOnExpandingPlatform(itemNumber, LaraItem->pos.xPos, LaraItem->pos.zPos))
	{
		//Slide Lara if on top of platform
		if (LaraItem->pos.yPos < height - 32 || LaraItem->pos.yPos > height + 32)
			return;
		if (angle == ANGLE(0))
			zShift = isExpanding ? -16 : 16;
		else if (angle == ANGLE(180))
			zShift = isExpanding ? 16 : -16;
		else if (angle == ANGLE(90))
			xShift = isExpanding ? -16 : 16;
		else if (angle == -ANGLE(90))
			xShift = isExpanding ? 16 : -16;
	} 
	else if (isExpanding && 
		IsInFrontOfExpandingPlatform(itemNumber, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, LaraCollision.Setup.Radius))
	{
		//Push Lara if in front of expanding platform
		if (angle == ANGLE(0))
			zShift = -LaraCollision.Setup.Radius / 6;
		else if (angle == ANGLE(180))
			zShift = LaraCollision.Setup.Radius / 6;
		else if (angle == ANGLE(90))
			xShift = -LaraCollision.Setup.Radius / 6;
		else if (angle == -ANGLE(90))
			xShift = LaraCollision.Setup.Radius / 6;
	}

	auto coll = &LaraCollision;
	GetCollisionInfo(coll, LaraItem, PHD_VECTOR(xShift, 0, zShift));

	if (coll->Middle.Ceiling >= 0 || coll->HitStatic)
		return;

	if (zShift != 0)
		LaraItem->pos.zPos += zShift;
	if (xShift != 0)
		LaraItem->pos.xPos += xShift;
}

void ControlExpandingPlatform(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (!item->itemFlags[2])
		{
			item->itemFlags[2] = 1;
		}

		if (item->triggerFlags < 0)
		{
			item->itemFlags[1] = 1;
		}
		else if (item->itemFlags[1] < 4096)
		{
			SoundEffect(SFX_TR4_BLK_PLAT_RAISE_AND_LOW, &item->pos, 0);

			item->itemFlags[1] += 64;
			ShiftLaraOnPlatform(itemNumber, true);
			if (item->triggerFlags > 0)
			{
				if (abs(item->pos.xPos - Camera.pos.x) < 10240 &&
					abs(item->pos.xPos - Camera.pos.x) < 10240 &&
					abs(item->pos.xPos - Camera.pos.x) < 10240)
				{
					if (item->itemFlags[1] == 64 || item->itemFlags[1] == 4096)
						Camera.bounce = -32;
					else
						Camera.bounce = -16;
				}
			}
		}
	}
	else if (item->itemFlags[1] <= 0 || item->triggerFlags < 0)
	{
		if (item->itemFlags[2])
		{
			item->itemFlags[1] = 0;
			item->itemFlags[2] = 0;
		}
	}
	else
	{
		SoundEffect(SFX_TR4_BLK_PLAT_RAISE_AND_LOW, &item->pos, 0);

		if (item->triggerFlags >= 0)
		{
			if (abs(item->pos.xPos - Camera.pos.x) < 10240 &&
				abs(item->pos.xPos - Camera.pos.x) < 10240 &&
				abs(item->pos.xPos - Camera.pos.x) < 10240)
			{
				if (item->itemFlags[1] == 64 || item->itemFlags[1] == 4096)
					Camera.bounce = -32;
				else
					Camera.bounce = -16;
			}
		}

		item->itemFlags[1] -= 64;
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
	auto item = &g_Level.Items[itemNumber];

	auto bounds = GetBoundsAccurate(item);
	auto normalizedThickness = item->itemFlags[1] / 4096.0f;
	auto width = abs(bounds->Z2 - bounds->Z1) / 2;
	auto offset = width * normalizedThickness;

	// Update bone mutators
	float zTranslate = 0.0f;
	if (item->pos.yRot == ANGLE(0))   zTranslate =  width  - offset;
	if (item->pos.yRot == ANGLE(90))  zTranslate = -offset + width;
	if (item->pos.yRot == ANGLE(180)) zTranslate = -offset + width;
	if (item->pos.yRot == ANGLE(270)) zTranslate =  width  - offset;

	for (int i = 0; i < item->mutator.size(); i++)
	{
		item->mutator[i].Offset = Vector3(0, 0, zTranslate);
		item->mutator[i].Scale = Vector3(1.0f, 1.0f, item->itemFlags[1] / 4096.0f);
	}
}
