#include "framework.h"
#include "tr5_twoblockplatform.h"
#include "level.h"
#include "control/control.h"
#include "items.h"
#include "lara.h"
#include "Sound/sound.h"
#include "collide.h"
#include "floordata.h"
#include "Renderer11.h"
using namespace TEN::Renderer;

using namespace TEN::Floordata;

void InitialiseTwoBlocksPlatform(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->itemFlags[0] = item->pos.yPos;
	item->itemFlags[1] = 1;
	UpdateBridgeItem(itemNumber);
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
				UpdateBridgeItem(itemNumber, true);
				ItemNewRoom(itemNumber, roomNumber);
				UpdateBridgeItem(itemNumber);
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

std::optional<int> TwoBlocksPlatformFloor(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!item->meshBits)
		return std::nullopt;

	auto room = &g_Level.Rooms[item->roomNumber];
	// Run through all blocks enclosed in AABB
	for (int x = 0; x < room->xSize; x++)
		for (int z = 0; z < room->ySize; z++)
		{
			auto floor = &room->floor[room->xSize * z + x];

			if (floor->BridgeItem.count(itemNumber))
			{
				g_Renderer.addDebugSphere(Vector3(room->z + z * SECTOR(1) + 512, room->y - 256, room->x + x * SECTOR(1) + 512), 256, Vector4(1, 0, 0, 1), RENDERER_DEBUG_PAGE::DIMENSION_STATS);

			}
		}


	return GetBridgeItemIntersect(itemNumber, x, y, z, false);
}

std::optional<int> TwoBlocksPlatformCeiling(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!item->meshBits)
		return std::nullopt;

	return GetBridgeItemIntersect(itemNumber, x, y, z, true);
}

int TwoBlocksPlatformFloorBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, false);
}

int TwoBlocksPlatformCeilingBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, true);
}