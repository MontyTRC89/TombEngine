#include "framework.h"
#include "tr4_sarcophagus.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/items.h"
#include "Game/pickup/pickup.h"
#include "Specific/setup.h"
#include "Game/health.h"
#include "Game/collision/collide_item.h"

static PHD_VECTOR SarcophagusPosition(0, 0, -300);
OBJECT_COLLISION_BOUNDS SarcophagusBounds =
{ -512, 512, -100, 100, -512, 0, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), 0, 0 };

void InitialiseSarcophagus(short itemNum)
{

}

void SarcophagusCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (TrInput & IN_ACTION &&
		item->Status != ITEM_ACTIVE &&
		l->ActiveState == LS_IDLE &&
		l->AnimNumber == LA_STAND_IDLE &&
		Lara.Control.HandStatus == HandStatus::Free ||
		Lara.Control.IsMoving && Lara.interactedItem == itemNum)
	{
		if (TestLaraPosition(&SarcophagusBounds, item, l))
		{
			if (MoveLaraPosition(&SarcophagusPosition, item, l))
			{
				l->AnimNumber = LA_PICKUP_SARCOPHAGUS;
				l->ActiveState = LS_MISC_CONTROL;
				l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
				item->Flags |= IFLAG_ACTIVATION_MASK;

				AddActiveItem(itemNum);
				item->Status = ITEM_ACTIVE;

				Lara.Control.IsMoving = false;
				ResetLaraFlex(l);
				Lara.Control.HandStatus = HandStatus::Busy;
			}
			else
			{
				Lara.interactedItem = itemNum;
			}
		}
		else if (Lara.Control.IsMoving)
		{
			if (Lara.interactedItem == itemNum)
			{
				Lara.Control.IsMoving = false;
				Lara.Control.HandStatus = HandStatus::Free;
			}
		}
	}
	else if (l->AnimNumber != LA_PICKUP_SARCOPHAGUS || l->FrameNumber != g_Level.Anims[LA_PICKUP_SARCOPHAGUS].frameBase + 113)
	{
		ObjectCollision(itemNum, l, coll);
	}
	else
	{
		short linknum;
		for (linknum = g_Level.Items[g_Level.Rooms[item->RoomNumber].itemNumber].NextItem; linknum != NO_ITEM; linknum = g_Level.Items[linknum].NextItem)
		{
			ITEM_INFO* currentItem = &g_Level.Items[linknum];

			if (linknum != itemNum && currentItem->Position.xPos == item->Position.xPos && currentItem->Position.zPos == item->Position.zPos)
			{
				if (Objects[currentItem->ObjectNumber].isPickup)
				{
					PickedUpObject(static_cast<GAME_OBJECT_ID>(currentItem->ObjectNumber), 0);
					currentItem->Status = ITEM_ACTIVE;
					currentItem->ItemFlags[3] = 1;
					AddDisplayPickup(currentItem->ObjectNumber);
				}
			}
		}
	}
}