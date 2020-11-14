#include "framework.h"
#include "tr5_raisingblock.h"
#include "items.h"
#include "level.h"
#include "control.h"
#include "box.h"
#include "objectslist.h"
#include "sound.h"
#include "camera.h"

void InitialiseRaisingBlock(short itemNumber)
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
}

void ControlRaisingBlock(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (!item->itemFlags[2])
		{
			if (item->objectNumber == ID_RAISING_BLOCK1)
			{
				if (item->triggerFlags == -1)
				{
					//AlterFloorHeight(item, -255);
				}
				else if (item->triggerFlags == -3)
				{
					//AlterFloorHeight(item, -1023);
				}
				else
				{
					//AlterFloorHeight(item, -item->itemFlags[7]);
				}
			}
			else
			{
				//AlterFloorHeight(item, -item->itemFlags[7]);
			}

			item->itemFlags[2] = 1;
		}

		if (item->triggerFlags < 0)
		{
			item->itemFlags[1] = 1;
		}
		else if (item->itemFlags[1] < 4096)
		{
			SoundEffect(SFX_BLK_PLAT_RAISE_LOW, &item->pos, 0);

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
					//AlterFloorHeight(item, 255);
					item->itemFlags[2] = 0;
				}
				else if (item->triggerFlags == -3)
				{
					//AlterFloorHeight(item, 1023);
					item->itemFlags[2] = 0;
				}
				else
				{
					//AlterFloorHeight(item, item->itemFlags[7]);
				}
			}
			else
			{
				//AlterFloorHeight(item, item->itemFlags[7]);
			}

			item->itemFlags[2] = 0;
		}
	}
	else
	{
		SoundEffect(SFX_BLK_PLAT_RAISE_LOW, &item->pos, 0);

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

std::tuple<std::optional<int>, bool> RaisingBlockFloor(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	if (abs(item.pos.xPos - x) <= SECTOR(1) / 2 && abs(item.pos.zPos - z) <= SECTOR(1) / 2)
	{
		auto height = item.pos.yPos - item.itemFlags[7] * item.itemFlags[1] / 4096;
		return std::make_tuple(std::optional{height}, y > height && y < item.pos.yPos);
	}
	return std::make_tuple(std::nullopt, false);
}

std::tuple<std::optional<int>, bool> RaisingBlockCeiling(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	if (abs(item.pos.xPos - x) <= SECTOR(1) / 2 && abs(item.pos.zPos - z) <= SECTOR(1) / 2)
	{
		auto height = item.pos.yPos - item.itemFlags[7] * item.itemFlags[1] / 4096;
		return std::make_tuple(std::optional{item.pos.yPos}, y > height && y < item.pos.yPos);
	}
	return std::make_tuple(std::nullopt, false);
}
