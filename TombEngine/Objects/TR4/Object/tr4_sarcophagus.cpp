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

using namespace TEN::Input;

static Vector3Int SarcophagusPosition(0, 0, -300);
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

void SarcophagusCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* sarcItem = &g_Level.Items[itemNumber];

	if (TrInput & IN_ACTION &&
		laraItem->Animation.ActiveState == LS_IDLE &&
		laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
		laraInfo->Control.HandStatus == HandStatus::Free &&
		sarcItem->Status != ITEM_ACTIVE ||
		laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
	{
		if (TestLaraPosition(&SarcophagusBounds, sarcItem, laraItem))
		{
			if (MoveLaraPosition(&SarcophagusPosition, sarcItem, laraItem))
			{
				laraItem->Animation.AnimNumber = LA_PICKUP_SARCOPHAGUS;
				laraItem->Animation.ActiveState = LS_MISC_CONTROL;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				sarcItem->Flags |= IFLAG_ACTIVATION_MASK;

				AddActiveItem(itemNumber);
				sarcItem->Status = ITEM_ACTIVE;

				laraInfo->Control.IsMoving = false;
				ResetLaraFlex(laraItem);
				laraInfo->Control.HandStatus = HandStatus::Busy;
			}
			else
				laraInfo->InteractedItem = itemNumber;
		}
		else if (laraInfo->Control.IsMoving)
		{
			if (laraInfo->InteractedItem == itemNumber)
			{
				laraInfo->Control.IsMoving = false;
				laraInfo->Control.HandStatus = HandStatus::Free;
			}
		}
	}
	else if (laraItem->Animation.AnimNumber != LA_PICKUP_SARCOPHAGUS ||
		laraItem->Animation.FrameNumber != g_Level.Anims[LA_PICKUP_SARCOPHAGUS].frameBase + 113)
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
				currentItem->Pose.Position.x == sarcItem->Pose.Position.x &&
				currentItem->Pose.Position.z == sarcItem->Pose.Position.z)
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
