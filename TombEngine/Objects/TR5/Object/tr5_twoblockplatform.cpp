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
	auto* item = &g_Level.Items[itemNumber];

	item->ItemFlags[0] = item->Pose.Position.y;
	item->ItemFlags[1] = 1;
	UpdateBridgeItem(itemNumber);
}

void TwoBlocksPlatformControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->TriggerFlags)
		{
			int goalHeight = (item->ItemFlags[0] - 16 * (int)(item->TriggerFlags & 0xFFFFFFF0));
			int speed = item->TriggerFlags & 0xF;

			if (item->Pose.Position.y > goalHeight)
				item->Pose.Position.y -= speed;
			else
				return;

			int DistanceToPortal = *&g_Level.Rooms[item->RoomNumber].maxceiling - item->Pose.Position.y;
			if (DistanceToPortal <= speed)
				UpdateBridgeItem(itemNumber);

			auto probe = GetCollision(item);

			item->Floor = probe.Position.Floor;

			if (probe.RoomNumber != item->RoomNumber)
			{
				UpdateBridgeItem(itemNumber, true);
				ItemNewRoom(itemNumber, probe.RoomNumber);
				UpdateBridgeItem(itemNumber);
			}
		}
		else
		{
			bool onObject = false;

			int height = LaraItem->Pose.Position.y + 1;
			if (GetBridgeItemIntersect(itemNumber, LaraItem->Pose.Position.x, LaraItem->Pose.Position.y, LaraItem->Pose.Position.z, false).has_value())
			{
				if (LaraItem->Pose.Position.y <= item->Pose.Position.y + 32)
				{
					if (item->Pose.Position.y < height)
						onObject = true;
				}
			}

			if (onObject && LaraItem->Animation.AnimNumber != LA_HOP_BACK_CONTINUE)
				item->ItemFlags[1] = 1;
			else
				item->ItemFlags[1] = -1;

			if (item->ItemFlags[1] < 0)
			{
				if (item->Pose.Position.y <= item->ItemFlags[0])
					item->ItemFlags[1] = 1;
				else
				{
					SoundEffect(SFX_TR4_RUMBLE_NEXTDOOR, &item->Pose);
					item->Pose.Position.y -= 4;
				}
			}
			else if (item->ItemFlags[1] > 0)
			{
				if (item->Pose.Position.y >= item->ItemFlags[0] + 128)
					item->ItemFlags[1] = -1;
				else
				{
					SoundEffect(SFX_TR4_RUMBLE_NEXTDOOR, &item->Pose);
					item->Pose.Position.y += 4;
				}
			}
		}
	}
}

std::optional<int> TwoBlocksPlatformFloor(short itemNumber, int x, int y, int z)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!item->MeshBits.TestAny())
		return std::nullopt;

	return GetBridgeItemIntersect(itemNumber, x, y, z, false);
}

std::optional<int> TwoBlocksPlatformCeiling(short itemNumber, int x, int y, int z)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!item->MeshBits.TestAny())
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
