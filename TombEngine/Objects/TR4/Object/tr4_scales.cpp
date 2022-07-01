#include "framework.h"
#include "tr4_scales.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Specific/setup.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/effects/tomb4fx.h"
#include "tr4_ahmet.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Game/collision/collide_item.h"

using namespace TEN::Entities::Switches;
using namespace TEN::Entities::TR4;

OBJECT_COLLISION_BOUNDS ScalesBounds =
{
	-1408, -1408,
	0, 0,
	-512, 512,
	ANGLE(-10.0f), ANGLE(10.0f),
	ANGLE(-30.0f), ANGLE(30.0f),
	ANGLE(-10.0f), ANGLE(10.0f)
};

void ScalesControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->Animation.FrameNumber != g_Level.Anims[item->Animation.AnimNumber].frameEnd)
	{
		AnimateItem(item);
		return;
	}

	if (item->Animation.ActiveState == 1 || item->ItemFlags[1])
	{
		if (Objects[item->ObjectNumber].animIndex)
		{
			RemoveActiveItem(itemNumber);
			item->Status = ITEM_NOT_ACTIVE;
			item->ItemFlags[1] = 0;

			AnimateItem(item);
			return;
		}

		if (RespawnAhmet(Lara.InteractedItem))
		{
			short itemNos[8];
			int sw = GetSwitchTrigger(item, itemNos, 0);

			if (sw > 0)
			{
				while (g_Level.Items[itemNos[sw]].ObjectNumber == ID_FLAME_EMITTER2)
				{
					if (--sw <= 0)
						break;
				}

				g_Level.Items[itemNos[sw]].Flags = 1024;
			}

			item->Animation.TargetState = 1;
		}

		AnimateItem(item);
	}

	int flags = 0;

	if (item->Animation.ActiveState == 2)
	{
		flags = -512;
		RemoveActiveItem(itemNumber);
		item->Status = ITEM_NOT_ACTIVE;
	}
	else
	{
		flags = -1024;
		item->ItemFlags[1] = 1;
	}

	TestTriggers(item, true, flags);
	AnimateItem(item);
}

void ScalesCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	ItemInfo* item = &g_Level.Items[itemNumber];

	if (TestBoundsCollide(item, laraItem, LARA_RADIUS))
	{
		if (laraItem->Animation.AnimNumber != LA_WATERSKIN_POUR_LOW && laraItem->Animation.AnimNumber != LA_WATERSKIN_POUR_HIGH || item->Animation.ActiveState != 1)
		{
			GlobalCollisionBounds.X1 = 640;
			GlobalCollisionBounds.X2 = 1280;
			GlobalCollisionBounds.Y1 = -1280;
			GlobalCollisionBounds.Y2 = 0;
			GlobalCollisionBounds.Z1 = -256;
			GlobalCollisionBounds.Z2 = 384;

			ItemPushItem(item, laraItem, coll, 0, 2);

			GlobalCollisionBounds.X1 = -256;
			GlobalCollisionBounds.X2 = 256;

			ItemPushItem(item, laraItem, coll, 0, 2);

			GlobalCollisionBounds.X1 = -1280;
			GlobalCollisionBounds.X2 = -640;

			ItemPushItem(item, laraItem, coll, 0, 2);
		}
		else
		{
			short rotY = item->Pose.Orientation.y;
			item->Pose.Orientation.y = (short)(laraItem->Pose.Orientation.y + ANGLE(45.0f)) & 0xC000;

			ScalesBounds.boundingBox.X1 = -1408;
			ScalesBounds.boundingBox.X2 = -640;
			ScalesBounds.boundingBox.Z1 = -512;
			ScalesBounds.boundingBox.Z2 = 0;

			if (TestLaraPosition(&ScalesBounds, item, laraItem))
			{
				laraItem->Animation.AnimNumber = LA_WATERSKIN_POUR_HIGH;
				laraItem->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Pose.Orientation.y = rotY;
			}
			else if (laraItem->Animation.FrameNumber == g_Level.Anims[LA_WATERSKIN_POUR_HIGH].frameBase + 51)
			{
				SoundEffect(SFX_TR4_POUR_WATER, &laraItem->Pose);
				item->Pose.Orientation.y = rotY;
			}
			else if (laraItem->Animation.FrameNumber == g_Level.Anims[LA_WATERSKIN_POUR_HIGH].frameBase + 74)
			{
				AddActiveItem(itemNumber);
				item->Status = ITEM_ACTIVE;

				if (laraItem->ItemFlags[3] < item->TriggerFlags)
				{
					item->Animation.TargetState = 4;
					item->Pose.Orientation.y = rotY;
				}
				else if (laraItem->ItemFlags[3] == item->TriggerFlags)
				{
					item->Animation.TargetState = 2;
					item->Pose.Orientation.y = rotY;
				}
				else
					item->Animation.TargetState = 3;
			}
			else
				item->Pose.Orientation.y = rotY;
		}
	}
	
	if (laraItem->Animation.FrameNumber >= g_Level.Anims[LA_WATERSKIN_POUR_LOW].frameBase + 44 &&
		laraItem->Animation.FrameNumber <= g_Level.Anims[LA_WATERSKIN_POUR_LOW].frameBase + 72 ||
		laraItem->Animation.FrameNumber >= g_Level.Anims[LA_WATERSKIN_POUR_HIGH].frameBase + 51 &&
		laraItem->Animation.FrameNumber <= g_Level.Anims[LA_WATERSKIN_POUR_HIGH].frameBase + 74)
	{
		Vector3Int pos = { 0, 0, 0 };
		GetLaraJointPosition(&pos, LM_LHAND);

		auto* drip = &Drips[GetFreeDrip()];
		drip->x = pos.x;
		drip->y = pos.y;
		drip->z = pos.z;
		drip->on = 1;
		drip->r = (GetRandomControl() & 0xF) + 24;
		drip->g = (GetRandomControl() & 0xF) + 44;
		drip->b = (GetRandomControl() & 0xF) + 56;
		drip->yVel = (GetRandomControl() & 0x1F) + 32;
		drip->gravity = (GetRandomControl() & 0x1F) + 32;
		drip->life = (GetRandomControl() & 0x1F) + 16;
		drip->roomNumber = laraItem->RoomNumber;
	}
}
