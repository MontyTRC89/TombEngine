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

using namespace TEN::Input;

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

	Vector3Int PulleyPos = { 0, 0, -148 }; 

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

	void PulleySwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if (TrInput & IN_ACTION &&
			laraItem->Animation.ActiveState == LS_IDLE &&
			laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
			laraItem->Animation.IsAirborne == false &&
			laraInfo->Control.HandStatus == HandStatus::Free ||
			laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
		{
			short oldYrot = switchItem->Pose.Orientation.y;
			switchItem->Pose.Orientation.y = laraItem->Pose.Orientation.y;
			if (TestLaraPosition(&PulleyBounds, switchItem, laraItem))
			{
				if (switchItem->ItemFlags[1])
				{
					if (OldPickupPos.x != laraItem->Pose.Position.x || OldPickupPos.y != laraItem->Pose.Position.y || OldPickupPos.z != laraItem->Pose.Position.z)
					{
						OldPickupPos.x = laraItem->Pose.Position.x;
						OldPickupPos.y = laraItem->Pose.Position.y;
						OldPickupPos.z = laraItem->Pose.Position.z;
						SayNo();
					}
				}
				else if (MoveLaraPosition(&PulleyPos, switchItem, laraItem))
				{
					laraItem->Animation.AnimNumber = LA_PULLEY_GRAB;
					laraItem->Animation.ActiveState = LS_PULLEY;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

					AddActiveItem(itemNumber);

					switchItem->Pose.Orientation.y = oldYrot;
					switchItem->Status = ITEM_ACTIVE;

					laraInfo->Control.IsMoving = false;
					ResetLaraFlex(laraItem);
					laraInfo->Control.HandStatus = HandStatus::Busy;
					laraInfo->InteractedItem = itemNumber;
				}
				else
					laraInfo->InteractedItem = itemNumber;
				
				switchItem->Pose.Orientation.y = oldYrot;
			}
			else
			{
				if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}

				switchItem->Pose.Orientation.y = oldYrot;
			}
		}
		else if (laraItem->Animation.ActiveState != LS_PULLEY)
			ObjectCollision(itemNumber, laraItem, coll);
	}
}
