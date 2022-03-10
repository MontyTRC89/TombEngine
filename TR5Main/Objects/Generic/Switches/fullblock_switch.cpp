#include "framework.h"
#include "Objects/Generic/Switches/fullblock_switch.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Specific/setup.h"
#include "Game/collision/collide_item.h"
#include "Specific/level.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::Switches
{
	OBJECT_COLLISION_BOUNDS FullBlockSwitchBounds = 
	{
		-384, 384,
		0, 256,
		0, 512,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	PHD_VECTOR FullBlockSwitchPos = { 0, 0, 0 };

	byte SequenceUsed[6];
	byte SequenceResults[3][3][3];
	byte Sequences[3];
	byte CurrentSequence;

	void FullBlockSwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if ((!(TrInput & IN_ACTION) ||
			laraItem->ActiveState != LS_IDLE ||
			laraItem->AnimNumber != LA_STAND_IDLE ||
			laraInfo->Control.HandStatus != HandStatus::Free ||
			switchItem->Status ||
			switchItem->Flags & 0x100 ||
			CurrentSequence >= 3) &&
			(!laraInfo->Control.IsMoving || laraInfo->InteractedItem !=itemNumber))
		{
			ObjectCollision(itemNumber, laraItem, coll);
			return;
		}

		if (TestLaraPosition(&FullBlockSwitchBounds, switchItem, laraItem))
		{
			if (MoveLaraPosition(&FullBlockSwitchPos, switchItem, laraItem))
			{
				if (switchItem->ActiveState == 1)
				{
					laraItem->ActiveState = LS_SWITCH_DOWN;
					laraItem->AnimNumber = LA_BUTTON_GIANT_PUSH;
					switchItem->TargetState = 0;
				}

				laraItem->TargetState = LS_IDLE;
				laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
				switchItem->Status = ITEM_ACTIVE;

				AddActiveItem(itemNumber);
				AnimateItem(switchItem);

				ResetLaraFlex(laraItem);
				laraInfo->Control.IsMoving = false;
				laraInfo->Control.HandStatus = HandStatus::Busy;
			}
			else
				laraInfo->InteractedItem = itemNumber;
		}
		else if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
		{
			laraInfo->Control.IsMoving = false;
			laraInfo->Control.HandStatus = HandStatus::Free;
		}
	}

	void FullBlockSwitchControl(short itemNumber)
	{
		ITEM_INFO* switchItem = &g_Level.Items[itemNumber];

		if (switchItem->AnimNumber != Objects[switchItem->ObjectNumber].animIndex + 2 ||
			CurrentSequence >= 3 ||
			switchItem->ItemFlags[0])
		{
			if (CurrentSequence >= 4)
			{
				switchItem->ItemFlags[0] = 0;
				switchItem->TargetState = SWITCH_ON;
				switchItem->Status = ITEM_NOT_ACTIVE;

				if (++CurrentSequence >= 7)
					CurrentSequence = 0;
			}
		}
		else
		{
			switchItem->ItemFlags[0] = 1;
			Sequences[CurrentSequence++] = switchItem->TriggerFlags;
		}

		AnimateItem(switchItem);
	}
}
