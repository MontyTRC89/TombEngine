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

	item->ItemFlags[0] = item->Position.yPos;
	item->ItemFlags[1] = 1;
	UpdateBridgeItem(itemNumber);
}

void TwoBlocksPlatformControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->TriggerFlags)
		{
			if (item->Position.yPos > (item->ItemFlags[0] - 16 * (int) (item->TriggerFlags & 0xFFFFFFF0)))
			{
				item->Position.yPos -= item->TriggerFlags & 0xF;
			}

			short roomNumber = item->RoomNumber;
			FLOOR_INFO* floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
			item->Floor = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);

			if (roomNumber != item->RoomNumber)
			{
				UpdateBridgeItem(itemNumber, true);
				ItemNewRoom(itemNumber, roomNumber);
				UpdateBridgeItem(itemNumber);
			}
		}
		else
		{
			bool onObject = false;

			int height = LaraItem->Position.yPos + 1;
			if (GetBridgeItemIntersect(itemNumber, LaraItem->Position.xPos, LaraItem->Position.yPos, LaraItem->Position.zPos, false).has_value())
			{
				if (LaraItem->Position.yPos <= item->Position.yPos + 32)
				{
					if (item->Position.yPos < height)
					{
						onObject = true;
					}
				}
			}

			if (onObject && LaraItem->AnimNumber != LA_HOP_BACK_CONTINUE)
				item->ItemFlags[1] = 1;
			else
				item->ItemFlags[1] = -1;

			if (item->ItemFlags[1] < 0)
			{
				if (item->Position.yPos <= item->ItemFlags[0])
				{
					item->ItemFlags[1] = 1;
				}
				else
				{
					SoundEffect(SFX_TR4_RUMBLE_NEXTDOOR, &item->Position, 0);
					item->Position.yPos -= 4;
				}
			}
			else if (item->ItemFlags[1] > 0)
			{
				if (item->Position.yPos >= item->ItemFlags[0] + 128)
				{
					item->ItemFlags[1] = -1;
				}
				else
				{
					SoundEffect(SFX_TR4_RUMBLE_NEXTDOOR, &item->Position, 0);
					item->Position.yPos += 4;
				}
			}
		}
	}
}

std::optional<int> TwoBlocksPlatformFloor(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!item->MeshBits)
		return std::nullopt;

	return GetBridgeItemIntersect(itemNumber, x, y, z, false);
}

std::optional<int> TwoBlocksPlatformCeiling(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!item->MeshBits)
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