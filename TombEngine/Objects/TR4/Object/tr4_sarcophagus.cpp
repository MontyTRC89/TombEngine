#include "framework.h"
#include "tr4_sarcophagus.h"
#include "Specific/level.h"
#include "Specific/Input/Input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/items.h"
#include "Game/pickup/pickup.h"
#include "Specific/setup.h"
#include "Game/health.h"
#include "Game/collision/collide_item.h"

using namespace TEN::Input;

const auto SarcophagusPosition = Vector3i(0, 0, -300);
const InteractionBasis SarcophagusBounds =
{
	GameBoundingBox(
		-SECTOR(0.5f), SECTOR(0.5f),
		-100, 100,
		-SECTOR(0.5f), 0
	),
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), 0),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), 0)
		)
};

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
		if (TestPlayerEntityInteract(sarcItem, laraItem, SarcophagusBounds))
		{
			if (AlignPlayerToEntity(sarcItem, laraItem, SarcophagusPosition))
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
		CollectMultiplePickups(sarcItem->Index);
	}
}
