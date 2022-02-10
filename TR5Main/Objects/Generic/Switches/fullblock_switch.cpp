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
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	PHD_VECTOR FullBlockSwitchPos = { 0, 0, 0 };

	byte SequenceUsed[6];
	byte SequenceResults[3][3][3];
	byte Sequences[3];
	byte CurrentSequence;

	void FullBlockSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if ((!(TrInput & IN_ACTION)
			|| item->Status
			|| item->Flags & 0x100
			|| CurrentSequence >= 3
			|| Lara.Control.HandStatus != HandStatus::Free
			|| l->ActiveState != LS_IDLE
			|| l->AnimNumber != LA_STAND_IDLE)
			&& (!Lara.Control.IsMoving || Lara.interactedItem !=itemNum))
		{
			ObjectCollision(itemNum, l, coll);
			return;
		}

		if (TestLaraPosition(&FullBlockSwitchBounds, item, l))
		{
			if (MoveLaraPosition(&FullBlockSwitchPos, item, l))
			{
				if (item->ActiveState == 1)
				{
					l->ActiveState = LS_SWITCH_DOWN;
					l->AnimNumber = LA_BUTTON_GIANT_PUSH;
					item->TargetState = 0;
				}
				l->TargetState = LS_IDLE;
				l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
				item->Status = ITEM_ACTIVE;

				AddActiveItem(itemNum);
				AnimateItem(item);

				Lara.Control.IsMoving = false;
				ResetLaraFlex(l);
				Lara.Control.HandStatus = HandStatus::Busy;
			}
			else
			{
				Lara.interactedItem = itemNum;
			}
		}
		else if (Lara.Control.IsMoving && Lara.interactedItem == itemNum)
		{
			Lara.Control.IsMoving = false;
			Lara.Control.HandStatus = HandStatus::Free;
		}
	}

	void FullBlockSwitchControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->AnimNumber != Objects[item->ObjectNumber].animIndex + 2
			|| CurrentSequence >= 3
			|| item->ItemFlags[0])
		{
			if (CurrentSequence >= 4)
			{
				item->ItemFlags[0] = 0;
				item->TargetState = SWITCH_ON;
				item->Status = ITEM_NOT_ACTIVE;
				if (++CurrentSequence >= 7)
					CurrentSequence = 0;
			}
		}
		else
		{
			item->ItemFlags[0] = 1;
			Sequences[CurrentSequence++] = item->TriggerFlags;
		}

		AnimateItem(item);
	}
}