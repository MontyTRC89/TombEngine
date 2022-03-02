#include "framework.h"
#include "tr5_crowdove_switch.h"
#include "Game/control/control.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/items.h"
#include "Game/collision/collide_item.h"

using namespace TEN::Entities::Switches;

namespace TEN::Entities::TR5
{
	OBJECT_COLLISION_BOUNDS CrowDoveBounds =
	{
		-256, 256,
		0, 0,
		-512, 512,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	PHD_VECTOR CrowDovePos = { 0, 0, -400 }; 

	void InitialiseCrowDoveSwitch(short itemNumber)
	{
		g_Level.Items[itemNumber].MeshBits = 3;
	}

	void CrowDoveSwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if (switchItem->Flags & ONESHOT ||
			!(switchItem->MeshBits & 4) ||
			(!(TrInput & IN_ACTION) ||
				laraItem->ActiveState != LS_IDLE ||
				laraItem->AnimNumber != LA_STAND_IDLE ||
				laraItem->Airborne ||
				laraInfo->Control.HandStatus != HandStatus::Free) &&
			(!laraInfo->Control.IsMoving || laraInfo->interactedItem != itemNumber))
		{
			if (laraItem->ActiveState != LS_DOVE_SWITCH)
				ObjectCollision(itemNumber, laraItem, coll);
		}
		else
		{
			int oldYrot = switchItem->Position.yRot;
			switchItem->Position.yRot = laraItem->Position.yRot;
			if (TestLaraPosition(&CrowDoveBounds, switchItem, laraItem))
			{
				if (MoveLaraPosition(&CrowDovePos, switchItem, laraItem))
				{
					laraItem->AnimNumber = LA_DOVESWITCH_TURN;
					laraItem->ActiveState = LS_DOVE_SWITCH;
					laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;

					AddActiveItem(itemNumber);

					// NOTE: In original TR5 the switch was used together with heavy switches.
					// This little fix make it usable normaly and less hardcoded.
					switchItem->ItemFlags[0] = 0;

					switchItem->Status = ITEM_ACTIVE;
					switchItem->Position.yRot = oldYrot;
					laraInfo->Control.IsMoving = false;
					ResetLaraFlex(laraItem);
					laraInfo->Control.HandStatus = HandStatus::Busy;
					laraInfo->interactedItem = itemNumber;
				}
				else
					laraInfo->interactedItem = itemNumber;
				
				switchItem->Position.yRot = oldYrot;
			}
			else
			{
				if (laraInfo->Control.IsMoving && laraInfo->interactedItem == itemNumber)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}

				switchItem->Position.yRot = oldYrot;
			}
		}
	}

	void CrowDoveSwitchControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->MeshBits & 2) 
		{
			ExplodeItemNode(item, 1, 0, 256); 
			SoundEffect(SFX_TR5_RAVENSWITCHEXPLODE, &item->Position, 0);
			item->MeshBits = 5;	
			RemoveActiveItem(itemNumber);

			// NOTE: In original TR5 the switch was used together with heavy switches.
			// This little fix make it usable normaly and less hardcoded.
			item->ItemFlags[0] = 1; 
		}
		else if (item->ItemFlags[0] == 0)
		{
			if (item->ActiveState == SWITCH_OFF)
				item->TargetState = SWITCH_ON;

			AnimateItem(item);

			if (item->ActiveState == SWITCH_OFF)
				item->Position.yRot += ANGLE(90.0f);
		}
	}
}
