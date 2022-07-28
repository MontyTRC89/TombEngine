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

using namespace TEN::Input;

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

	Vector3Int FullBlockSwitchPos = { 0, 0, 0 };

	byte SequenceUsed[6];
	byte SequenceResults[3][3][3];
	byte Sequences[3];
	byte CurrentSequence;

	void FullBlockSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if ((!(TrInput & IN_ACTION) ||
			laraItem->Animation.ActiveState != LS_IDLE ||
			laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
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
				if (switchItem->Animation.ActiveState == 1)
				{
					laraItem->Animation.ActiveState = LS_SWITCH_DOWN;
					laraItem->Animation.AnimNumber = LA_BUTTON_GIANT_PUSH;
					switchItem->Animation.TargetState = 0;
				}

				laraItem->Animation.TargetState = LS_IDLE;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
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
		ItemInfo* switchItem = &g_Level.Items[itemNumber];

		if (switchItem->Animation.AnimNumber != Objects[switchItem->ObjectNumber].animIndex + 2 ||
			CurrentSequence >= 3 ||
			switchItem->ItemFlags[0])
		{
			if (CurrentSequence >= 4)
			{
				switchItem->ItemFlags[0] = 0;
				switchItem->Animation.TargetState = SWITCH_ON;
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
