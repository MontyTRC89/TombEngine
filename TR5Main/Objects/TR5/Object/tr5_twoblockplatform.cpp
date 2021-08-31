#include "framework.h"
#include "tr5_twoblockplatform.h"
#include "level.h"
#include "control.h"
#include "items.h"
#include "lara.h"
#include "sound.h"
#include "collide.h"
void InitialiseTwoBlocksPlatform(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->itemFlags[0] = item->pos.yPos;
	item->itemFlags[1] = 1;
	AddTwoBlocksPlatform(itemNumber);
}

void AddTwoBlocksPlatform(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	TEN::Floordata::AddBridge(itemNumber);
	switch (GetQuadrant(item->pos.yRot))
	{
	case NORTH:
		TEN::Floordata::AddBridge(itemNumber, -SECTOR(1), 0);
		TEN::Floordata::AddBridge(itemNumber, 0, SECTOR(1));
		TEN::Floordata::AddBridge(itemNumber, -SECTOR(1), SECTOR(1));
		break;
	case EAST:
		TEN::Floordata::AddBridge(itemNumber, SECTOR(1), 0);
		TEN::Floordata::AddBridge(itemNumber, 0, SECTOR(1));
		TEN::Floordata::AddBridge(itemNumber, SECTOR(1), SECTOR(1));
		break;
	case SOUTH:
		TEN::Floordata::AddBridge(itemNumber, SECTOR(1), 0);
		TEN::Floordata::AddBridge(itemNumber, 0, -SECTOR(1));
		TEN::Floordata::AddBridge(itemNumber, SECTOR(1), -SECTOR(1));
		break;
	case WEST:
		TEN::Floordata::AddBridge(itemNumber, -SECTOR(1), 0);
		TEN::Floordata::AddBridge(itemNumber, 0, -SECTOR(1));
		TEN::Floordata::AddBridge(itemNumber, -SECTOR(1), -SECTOR(1));
		break;
	}
}

void RemoveTwoBlocksPlatform(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	TEN::Floordata::RemoveBridge(itemNumber);
	switch (GetQuadrant(item->pos.yRot))
	{
	case NORTH:
		TEN::Floordata::RemoveBridge(itemNumber, -SECTOR(1), 0);
		TEN::Floordata::RemoveBridge(itemNumber, 0, SECTOR(1));
		TEN::Floordata::RemoveBridge(itemNumber, -SECTOR(1), SECTOR(1));
		break;
	case EAST:
		TEN::Floordata::RemoveBridge(itemNumber, SECTOR(1), 0);
		TEN::Floordata::RemoveBridge(itemNumber, 0, SECTOR(1));
		TEN::Floordata::RemoveBridge(itemNumber, SECTOR(1), SECTOR(1));
		break;
	case SOUTH:
		TEN::Floordata::RemoveBridge(itemNumber, SECTOR(1), 0);
		TEN::Floordata::RemoveBridge(itemNumber, 0, -SECTOR(1));
		TEN::Floordata::RemoveBridge(itemNumber, SECTOR(1), -SECTOR(1));
		break;
	case WEST:
		TEN::Floordata::RemoveBridge(itemNumber, -SECTOR(1), 0);
		TEN::Floordata::RemoveBridge(itemNumber, 0, -SECTOR(1));
		TEN::Floordata::RemoveBridge(itemNumber, -SECTOR(1), -SECTOR(1));
		break;
	}
}

BOOL IsOnTwoBlocksPlatform(ITEM_INFO* item, int x, int z)
{
	if (!item->meshBits)
		return false;

	short angle = item->pos.yRot;
	int xb = x / SECTOR(1);
	int zb = z / SECTOR(1);
	int itemxb = item->pos.xPos / SECTOR(1);
	int itemzb = item->pos.zPos / SECTOR(1);

	if (!angle && (xb == itemxb || xb == itemxb - 1) && (zb == itemzb || zb == itemzb + 1))
		return true;
	if (angle == -ANGLE(180) && (xb == itemxb || xb == itemxb + 1) && (zb == itemzb || zb == itemzb - 1))
		return true;
	if (angle == ANGLE(90) && (zb == itemzb || zb == itemzb + 1) && (xb == itemxb || xb == itemxb + 1))
		return true;
	if (angle == -ANGLE(90) && (zb == itemzb || zb == itemzb - 1) && (xb == itemxb || xb == itemxb - 1))
		return true;

	return false;
}

std::optional<int> TwoBlocksPlatformFloor(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!item->meshBits)
		return std::nullopt;

	int height = item->pos.yPos;
	return std::optional{ height };
}

void TwoBlocksPlatformControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->triggerFlags)
		{
			if (item->pos.yPos > (item->itemFlags[0] - 16 * (int) (item->triggerFlags & 0xFFFFFFF0)))
			{
				item->pos.yPos -= item->triggerFlags & 0xF;
			}

			short roomNumber = item->roomNumber;
			FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
			item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

			if (roomNumber != item->roomNumber)
			{
				RemoveTwoBlocksPlatform(itemNumber);
				ItemNewRoom(itemNumber, roomNumber);
				AddTwoBlocksPlatform(itemNumber);
			}
		}
		else
		{
			bool onObject = false;

			int height = LaraItem->pos.yPos + 1;
			if (IsOnTwoBlocksPlatform(item, LaraItem->pos.xPos, LaraItem->pos.zPos))
			{
				if (LaraItem->pos.yPos <= item->pos.yPos + 32)
				{
					if (item->pos.yPos < height)
					{
						onObject = true;
					}
				}
			}

			if (onObject && LaraItem->animNumber != LA_HOP_BACK_CONTINUE)
				item->itemFlags[1] = 1;
			else
				item->itemFlags[1] = -1;

			if (item->itemFlags[1] < 0)
			{
				if (item->pos.yPos <= item->itemFlags[0])
				{
					item->itemFlags[1] = 1;
				}
				else
				{
					SoundEffect(SFX_TR4_RUMBLE_NEXTDOOR, &item->pos, 0);
					item->pos.yPos -= 4;
				}
			}
			else if (item->itemFlags[1] > 0)
			{
				if (item->pos.yPos >= item->itemFlags[0] + 128)
				{
					item->itemFlags[1] = -1;
				}
				else
				{
					SoundEffect(SFX_TR4_RUMBLE_NEXTDOOR, &item->pos, 0);
					item->pos.yPos += 4;
				}
			}
		}
	}
}

std::optional<int> TwoBlocksPlatformCeiling(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!item->meshBits)
		return std::nullopt;

	//+ 256 is more accurate, but prevents a tall block from entering underneath
	int height = item->pos.yPos + 20;
	return std::optional{ height };
}

int TwoBlocksPlatformFloorBorder(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	return item->pos.yPos;
}

int TwoBlocksPlatformCeilingBorder(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	return (item->pos.yPos + 256);
}