#include "framework.h"
#include "tr5_ExpandingPlatform.h"
#include "items.h"
#include "level.h"
#include "control.h"
#include "box.h"
#include "animation.h"
#include "Sound/sound.h"
#include "camera.h"
#include "lara.h"
#include "collide.h"

void InitialiseExpandingPlatform(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	g_Level.Boxes[floor->Box].flags &= ~BLOCKED;

	if (item->triggerFlags < 0)
	{
		item->aiBits |= ALL_AIOBJ;
		AddActiveItem(itemNumber);
		item->status = ITEM_ACTIVE;
	}

	// Get height from animations
	ANIM_FRAME* frame = &g_Level.Frames[g_Level.Anims[Objects[item->objectNumber].animIndex].framePtr];
	item->itemFlags[7] = (short)abs(frame->boundingBox.Y1 - frame->boundingBox.Y2);
	TEN::Floordata::AddBridge(itemNumber);
}

bool IsOnExpandingPlatform(ITEM_INFO item, int x, int z)
{
	if (item.itemFlags[1] <= 0)
		return false;
	int xb = x / SECTOR(1);
	int zb = z / SECTOR(1);
	int itemxb = item.pos.xPos / SECTOR(1);
	int itemzb = item.pos.zPos / SECTOR(1);
	if (item.pos.yRot == ANGLE(90))
	{
		auto xBorder = item.pos.xPos + CLICK(2) - SECTOR(1) * item.itemFlags[1] / 4096;
		if (x < xBorder || zb != itemzb || xb != itemxb)
			return false;
	}
	else if (item.pos.yRot == ANGLE(270))
	{
		auto xBorder = item.pos.xPos - CLICK(2) + SECTOR(1) * item.itemFlags[1] / 4096;
		if (x > xBorder || zb != itemzb || xb != itemxb)
			return false;
	}
	else if (item.pos.yRot == 0)
	{
		auto zBorder = item.pos.zPos + CLICK(2) - SECTOR(1) * item.itemFlags[1] / 4096;
		if (z < zBorder || zb != itemzb || xb != itemxb)
			return false;
	}
	else if (item.pos.yRot == ANGLE(180))
	{
		auto zBorder = item.pos.zPos - CLICK(2) + SECTOR(1) * item.itemFlags[1] / 4096;
		if (z > zBorder || zb != itemzb || xb != itemxb)
			return false;
	}
	return true;
}

bool IsInFrontOfExpandingPlatform(ITEM_INFO item, int x, int y, int z, int margin)
{
	if (item.itemFlags[1] <= 0)
		return false;
	const auto topHeight = item.pos.yPos - item.itemFlags[7] + CLICK(2);
	const auto bottomHeight = item.pos.yPos + CLICK(2);
	if (LaraItem->pos.yPos < topHeight - 32 || LaraItem->pos.yPos > bottomHeight + 32)
		return false;
	int xb = x / SECTOR(1);
	int zb = z / SECTOR(1);
	int itemxb = item.pos.xPos / SECTOR(1);
	int itemzb = item.pos.zPos / SECTOR(1);
	if (item.pos.yRot == ANGLE(90))
	{
		auto xBorder = item.pos.xPos + CLICK(2) - margin - SECTOR(1) * item.itemFlags[1] / 4096;
		auto xBorder2 = item.pos.xPos + CLICK(2);
		if (x < xBorder || zb != itemzb || x > xBorder2)
			return false;
	}
	else if (item.pos.yRot == ANGLE(270))
	{
		auto xBorder = item.pos.xPos - CLICK(2) + margin + SECTOR(1) * item.itemFlags[1] / 4096;
		auto xBorder2 = item.pos.xPos - CLICK(2);
		if (x > xBorder || zb != itemzb || x < xBorder2)
			return false;
	}
	else if (item.pos.yRot == 0)
	{
		auto zBorder = item.pos.zPos + CLICK(2) - margin - SECTOR(1) * item.itemFlags[1] / 4096;
		auto zBorder2 = item.pos.zPos + CLICK(2);
		if (z < zBorder || xb != itemxb || z > zBorder2)
			return false;
	}
	else if (item.pos.yRot == ANGLE(180))
	{
		auto zBorder = item.pos.zPos - CLICK(2) + margin + SECTOR(1) * item.itemFlags[1] / 4096;
		auto zBorder2 = item.pos.zPos - CLICK(2);
		if (z > zBorder || xb != itemxb || z < zBorder2)
			return false;
	}
	return true;
}

void ShiftLaraOnPlatform(short itemNumber, bool isExpanding)
{
	ITEM_INFO item = g_Level.Items[itemNumber];
	short angle = item.pos.yRot;
	int xShift = 0;
	int zShift = 0;
	if (item.itemFlags[1] <= 0)
		return;
	const auto height = item.pos.yPos - item.itemFlags[7] + CLICK(2);
	if (IsOnExpandingPlatform(item, LaraItem->pos.xPos, LaraItem->pos.zPos))
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
	} else if (isExpanding && IsInFrontOfExpandingPlatform(item, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, LaraCollision.Setup.Radius))
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
	auto coll = LaraCollision;
	GetCollisionInfo(&coll, LaraItem, PHD_VECTOR(xShift, 0, zShift));

	if (coll.Middle.Ceiling >= 0 || coll.HitStatic)
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
}

std::optional<int> ExpandingPlatformFloor(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	if (!IsOnExpandingPlatform(item, x, z))
		return std::nullopt;
	const auto height = item.pos.yPos - item.itemFlags[7] + CLICK(2);
	return std::optional{ height };
}

std::optional<int> ExpandingPlatformCeiling(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	if (!IsOnExpandingPlatform(item, x, z))
		return std::nullopt;
	return std::optional{ item.pos.yPos + CLICK(2) + 1 };
}

int ExpandingPlatformFloorBorder(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];
	const auto height = item.pos.yPos - item.itemFlags[7];
	return height;
}

int ExpandingPlatformCeilingBorder(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];
	return item.pos.yPos + 1;
}
