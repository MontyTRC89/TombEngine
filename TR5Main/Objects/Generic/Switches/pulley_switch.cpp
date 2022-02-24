#include "framework.h"
#include "Objects/Generic/Switches/pulley_switch.h"
#include "Game/control/control.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Sound/sound.h"
#include "Game/pickup/pickup.h"
#include "Specific/level.h"
#include "Game/collision/collide_item.h"
#include "Game/items.h"

namespace TEN::Entities::Switches
{
	OBJECT_COLLISION_BOUNDS PulleyBounds = 
	{
		-256, 256,
		0, 0,
		-512, 512,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	PHD_VECTOR PulleyPos = { 0, 0, -148 }; 

	void InitialisePulleySwitch(short itemNumber)
	{
		auto* switchItem = &g_Level.Items[itemNumber];

		switchItem->ItemFlags[3] = switchItem->TriggerFlags;
		switchItem->TriggerFlags = abs(switchItem->TriggerFlags);

		if (switchItem->Status == ITEM_INVISIBLE)
		{
			switchItem->ItemFlags[1] = 1;
			switchItem->Status = ITEM_NOT_ACTIVE;
		}
	}

	void PulleySwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if (TrInput & IN_ACTION &&
			laraItem->ActiveState == LS_IDLE &&
			laraItem->AnimNumber == LA_STAND_IDLE &&
			laraItem->Airborne == false &&
			laraInfo->Control.HandStatus == HandStatus::Free ||
			laraInfo->Control.IsMoving && laraInfo->interactedItem == itemNumber)
		{
			short oldYrot = switchItem->Position.yRot;
			switchItem->Position.yRot = laraItem->Position.yRot;
			if (TestLaraPosition(&PulleyBounds, switchItem, laraItem))
			{
				if (switchItem->ItemFlags[1])
				{
					if (OldPickupPos.x != laraItem->Position.xPos || OldPickupPos.y != laraItem->Position.yPos || OldPickupPos.z != laraItem->Position.zPos)
					{
						OldPickupPos.x = laraItem->Position.xPos;
						OldPickupPos.y = laraItem->Position.yPos;
						OldPickupPos.z = laraItem->Position.zPos;
						SayNo();
					}
				}
				else if (MoveLaraPosition(&PulleyPos, switchItem, laraItem))
				{
					laraItem->AnimNumber = LA_PULLEY_GRAB;
					laraItem->ActiveState = LS_PULLEY;
					laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;

					AddActiveItem(itemNumber);

					switchItem->Position.yRot = oldYrot;
					switchItem->Status = ITEM_ACTIVE;

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
		else if (laraItem->ActiveState != LS_PULLEY)
			ObjectCollision(itemNumber, laraItem, coll);
	}
}
