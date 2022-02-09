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
{ -1408, -1408, 0, 0, -512, 512, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), ANGLE(-10), ANGLE(10) };

void ScalesControl(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (item->FrameNumber != g_Level.Anims[item->AnimNumber].frameEnd)
	{
		AnimateItem(item);
		return;
	}

	if (item->ActiveState == 1 || item->ItemFlags[1])
	{
		if (Objects[item->ObjectNumber].animIndex)
		{
			RemoveActiveItem(itemNum);
			item->Status = ITEM_NOT_ACTIVE;
			item->ItemFlags[1] = 0;

			AnimateItem(item);
			return;
		}

		if (RespawnAhmet(Lara.interactedItem))
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

			item->TargetState = 1;
		}

		AnimateItem(item);
	}

	int flags = 0;

	if (item->ActiveState == 2)
	{
		flags = -512;
		RemoveActiveItem(itemNum);
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

void ScalesCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (TestBoundsCollide(item, l, LARA_RAD))
	{
		if (l->AnimNumber != LA_WATERSKIN_POUR_LOW && l->AnimNumber != LA_WATERSKIN_POUR_HIGH || item->ActiveState != 1)
		{
			GlobalCollisionBounds.X1 = 640;
			GlobalCollisionBounds.X2 = 1280;
			GlobalCollisionBounds.Y1 = -1280;
			GlobalCollisionBounds.Y2 = 0;
			GlobalCollisionBounds.Z1 = -256;
			GlobalCollisionBounds.Z2 = 384;

			ItemPushItem(item, l, coll, 0, 2);

			GlobalCollisionBounds.X1 = -256;
			GlobalCollisionBounds.X2 = 256;

			ItemPushItem(item, l, coll, 0, 2);

			GlobalCollisionBounds.X1 = -1280;
			GlobalCollisionBounds.X2 = -640;

			ItemPushItem(item, l, coll, 0, 2);
		}
		else
		{
			short rotY = item->Position.yRot;
			item->Position.yRot = (short)(l->Position.yRot + ANGLE(45)) & 0xC000;

			ScalesBounds.boundingBox.X1 = -1408;
			ScalesBounds.boundingBox.X2 = -640;
			ScalesBounds.boundingBox.Z1 = -512;
			ScalesBounds.boundingBox.Z2 = 0;

			if (TestLaraPosition(&ScalesBounds, item, l))
			{
				l->AnimNumber = LA_WATERSKIN_POUR_HIGH;
				l->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				item->Position.yRot = rotY;
			}
			else if (l->FrameNumber == g_Level.Anims[LA_WATERSKIN_POUR_HIGH].frameBase + 51)
			{
				SoundEffect(SFX_TR4_POUR, &l->Position, 0);
				item->Position.yRot = rotY;
			}
			else if (l->FrameNumber == g_Level.Anims[LA_WATERSKIN_POUR_HIGH].frameBase + 74)
			{
				AddActiveItem(itemNum);
				item->Status = ITEM_ACTIVE;

				if (l->ItemFlags[3] < item->TriggerFlags)
				{
					item->TargetState = 4;
					item->Position.yRot = rotY;
				}
				else if (l->ItemFlags[3] == item->TriggerFlags)
				{
					item->TargetState = 2;
					item->Position.yRot = rotY;
				}
				else
				{
					item->TargetState = 3;
				}
			}
			else
			{
				item->Position.yRot = rotY;
			}
		}
	}
	
	if (l->FrameNumber >= g_Level.Anims[LA_WATERSKIN_POUR_LOW].frameBase + 44 &&
		l->FrameNumber <= g_Level.Anims[LA_WATERSKIN_POUR_LOW].frameBase + 72 ||
		l->FrameNumber >= g_Level.Anims[LA_WATERSKIN_POUR_HIGH].frameBase + 51 &&
		l->FrameNumber <= g_Level.Anims[LA_WATERSKIN_POUR_HIGH].frameBase + 74)
	{
		PHD_VECTOR pos;
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;

		GetLaraJointPosition(&pos, LM_LHAND);

		DRIP_STRUCT* drip = &Drips[GetFreeDrip()];
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
		drip->roomNumber = l->RoomNumber;
	}
}