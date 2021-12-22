#include "framework.h"
#include "tr5_twoblockplatform.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Renderer/Renderer11.h"
using namespace TEN::Renderer;

using namespace TEN::Floordata;

void InitialiseTwoBlocksPlatform(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->itemFlags[0] = item->pos.yPos;
	item->itemFlags[1] = 1;
	UpdateBridgeItem(itemNumber);
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
			if (GetBridgeItemIntersect(itemNumber, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, false).has_value())
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