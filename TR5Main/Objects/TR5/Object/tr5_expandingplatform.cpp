#include "framework.h"
#include "tr5_ExpandingPlatform.h"
#include "items.h"
#include "level.h"
#include "control.h"
#include "box.h"
#include "objectslist.h"
#include "sound.h"
#include "camera.h"

void InitialiseExpandingPlatform(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	g_Level.Boxes[floor->box].flags &= ~BLOCKED;

	if (item->triggerFlags < 0)
	{
		item->aiBits |= ALL_AIOBJ;
		AddActiveItem(itemNumber);
		item->status = ITEM_ACTIVE;
	}

	// Get height from animations
	ANIM_FRAME* frame = &g_Level.Frames[g_Level.Anims[Objects[item->objectNumber].animIndex].framePtr];
	item->itemFlags[7] = (short)abs(frame->boundingBox.Y1 - frame->boundingBox.Y2);
	T5M::Floordata::AddBridge(itemNumber);
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

			if (item->objectNumber == ID_RAISING_BLOCK1)
			{
				if (item->triggerFlags == -1)
				{
					item->itemFlags[2] = 0;
				}
				else if (item->triggerFlags == -3)
				{
					item->itemFlags[2] = 0;
				}
			}

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
	}
}

std::optional<int> ExpandingPlatformFloor(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	if (item.itemFlags[1] <= 0)
		return std::nullopt;
	if (item.pos.yRot == ANGLE(90))
	{
		auto xBorder = item.pos.xPos + CLICK(2) - SECTOR(1) * item.itemFlags[1] / 4096;
		if(x < xBorder)
			return std::nullopt;
	} else if (item.pos.yRot == ANGLE(270))
	{
		auto xBorder = item.pos.xPos - CLICK(2) +  SECTOR(1) * item.itemFlags[1] / 4096;
		if (x > xBorder)
			return std::nullopt;
	} else if (item.pos.yRot == 0)
	{
		auto zBorder = item.pos.zPos + CLICK(2) - SECTOR(1) * item.itemFlags[1] / 4096;
		if (z < zBorder)
			return std::nullopt;
	}
	else if (item.pos.yRot == ANGLE(180))
	{
		auto zBorder = item.pos.zPos - CLICK(2) + SECTOR(1) * item.itemFlags[1] / 4096;
		if (z > zBorder)
			return std::nullopt;
	}
	const auto height = item.pos.yPos - item.itemFlags[7] + CLICK(2);
	return std::optional{ height };
}

std::optional<int> ExpandingPlatformCeiling(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
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
