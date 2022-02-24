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
{
	-512, 512,
	-100, 100,
	-512, 0,
	ANGLE(-10.0f), ANGLE(10.0f),
	ANGLE(-30.0f), ANGLE(30.0f),
	0, 0
};

void InitialiseSarcophagus(short itemNumber)
{

}

void SarcophagusCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* sarcItem = &g_Level.Items[itemNumber];

	if (TrInput & IN_ACTION &&
		laraItem->ActiveState == LS_IDLE &&
		laraItem->AnimNumber == LA_STAND_IDLE &&
		laraInfo->Control.HandStatus == HandStatus::Free &&
		sarcItem->Status != ITEM_ACTIVE ||
		laraInfo->Control.IsMoving && laraInfo->interactedItem == itemNumber)
	{
		if (TestLaraPosition(&SarcophagusBounds, sarcItem, laraItem))
		{
			if (MoveLaraPosition(&SarcophagusPosition, sarcItem, laraItem))
			{
				laraItem->AnimNumber = LA_PICKUP_SARCOPHAGUS;
				laraItem->ActiveState = LS_MISC_CONTROL;
				laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
				sarcItem->Flags |= IFLAG_ACTIVATION_MASK;

				AddActiveItem(itemNumber);
				sarcItem->Status = ITEM_ACTIVE;

				laraInfo->Control.IsMoving = false;
				ResetLaraFlex(laraItem);
				laraInfo->Control.HandStatus = HandStatus::Busy;
			}
			else
				laraInfo->interactedItem = itemNumber;
		}
		else if (laraInfo->Control.IsMoving)
		{
			if (laraInfo->interactedItem == itemNumber)
			{
				laraInfo->Control.IsMoving = false;
				laraInfo->Control.HandStatus = HandStatus::Free;
			}
		}
	}
	else if (laraItem->AnimNumber != LA_PICKUP_SARCOPHAGUS ||
		laraItem->FrameNumber != g_Level.Anims[LA_PICKUP_SARCOPHAGUS].frameBase + 113)
	{
		ObjectCollision(itemNumber, laraItem, coll);
	}
	else
	{
		short linkNumber;
		for (linkNumber = g_Level.Items[g_Level.Rooms[sarcItem->RoomNumber].itemNumber].NextItem; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextItem)
		{
			auto* currentItem = &g_Level.Items[linkNumber];

			if (linkNumber != itemNumber &&
				currentItem->Position.xPos == sarcItem->Position.xPos &&
				currentItem->Position.zPos == sarcItem->Position.zPos)
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
