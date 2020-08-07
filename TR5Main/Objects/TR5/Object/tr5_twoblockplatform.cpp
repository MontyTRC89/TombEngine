#include "framework.h"
#include "tr5_twoblockplatform.h"
#include "level.h"
#include "control.h"
#include "items.h"
#include "lara.h"
#include "sound.h"

void InitialiseTwoBlocksPlatform(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->itemFlags[0] = item->pos.yPos;
	item->itemFlags[1] = 1;
}

BOOL IsOnTwoBlocksPlatform(ITEM_INFO* item, int x, int z)
{
	if (!item->meshBits)
		return false;

	short angle = item->pos.yRot;
	int xb = x >> WALL_SHIFT;
	int zb = z >> WALL_SHIFT;
	int itemxb = item->pos.xPos >> WALL_SHIFT;
	int itemzb = item->pos.zPos >> WALL_SHIFT;

	if (!angle && (xb == itemxb || xb == itemxb - 1) && (zb == itemzb || zb == itemzb + 1))
		return true;
	if (angle == -ANGLE(180) && (xb == itemxb || xb == itemxb + 1) && (zb == itemzb || zb == itemzb - 1))
		return true;
	if (angle == ANGLE(90) && (zb == itemzb || zb == itemzb - 1) && (xb == itemxb || xb == itemxb + 1))
		return true;
	if (angle == -ANGLE(90) && (zb == itemzb || zb == itemzb - 1) && (xb == itemxb || xb == itemxb - 1))
		return true;

	return false;
}

void TwoBlocksPlatformFloor(ITEM_INFO* item, int x, int y, int z, int* height)
{
	if (IsOnTwoBlocksPlatform(item, x, z))
	{
		if (y <= (item->pos.yPos + 32) && item->pos.yPos < *height)
		{
			*height = item->pos.yPos;
			OnFloor = 1;
			HeightType = WALL;
		}
	}
}

void TwoBlocksPlatformControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->triggerFlags)
		{
			if (item->pos.yPos > (item->itemFlags[0] - 16 * (item->triggerFlags & 0xFFFFFFF0)))
			{
				item->pos.yPos -= item->triggerFlags & 0xF;
			}

			short roomNumber = item->roomNumber;
			FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
			item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

			if (roomNumber != item->roomNumber)
				ItemNewRoom(itemNumber, roomNumber);
		}
		else
		{
			OnFloor = false;

			int height = LaraItem->pos.yPos + 1;
			TwoBlocksPlatformFloor(item, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, &height);

			if (OnFloor && LaraItem->animNumber != LA_HOP_BACK_CONTINUE)
				item->itemFlags[1] = 1;
			else
				item->itemFlags[1] = -1;

			if (item->itemFlags[1] <= 0)
			{
				if (item->itemFlags[1] <= 0)
				{
					if (item->pos.yPos <= item->itemFlags[0])
					{
						item->itemFlags[1] = 1;
					}
					else
					{
						SoundEffect(SFX_2GUNTEX_FALL_BIG, &item->pos, 0);
						item->pos.yPos -= 4;
					}
				}
			}
			else
			{
				if (item->pos.yPos >= item->itemFlags[0] + 128)
				{
					item->itemFlags[1] = -1;
				}
				else
				{
					SoundEffect(SFX_2GUNTEX_FALL_BIG, &item->pos, 0);
					item->pos.yPos += 4;
				}
			}
		}
	}
}

void TwoBlocksPlatformCeiling(ITEM_INFO* item, int x, int y, int z, int* height)
{
	if (IsOnTwoBlocksPlatform(item, x, z))
	{
		if (y > item->pos.yPos + 32 && item->pos.yPos > * height)
		{
			*height = item->pos.yPos + 256;
		}
	}
}