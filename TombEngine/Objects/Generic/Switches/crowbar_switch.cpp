#include "framework.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/crowbar_switch.h"
#include "Game/gui.h"
#include "Sound/sound.h"
#include "Game/pickup/pickup.h"
#include "Specific/level.h"
#include "Game/collision/collide_item.h"
#include "Game/animation.h"
#include "Game/items.h"

using namespace TEN::Input;

namespace TEN::Entities::Switches
{
	Vector3Int CrowbarPos = { -89, 0, -328 }; 

	OBJECT_COLLISION_BOUNDS CrowbarBounds = 
	{
		-256, 256,
		0, 0,
		-512, -256,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	Vector3Int CrowbarPos2 = { 89, 0, 328 }; 

	OBJECT_COLLISION_BOUNDS CrowbarBounds2 = 
	{
		-256, 256,
		0, 0,
		256, 512,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	void CrowbarSwitchCollision(short itemNumber, ItemInfo* laraitem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraitem);
		ItemInfo* switchItem = &g_Level.Items[itemNumber];

		int doSwitch = 0;

		if (((TrInput & IN_ACTION || g_Gui.GetInventoryItemChosen() == ID_CROWBAR_ITEM) &&
			laraitem->Animation.ActiveState == LS_IDLE &&
			laraitem->Animation.AnimNumber == LA_STAND_IDLE &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			switchItem->ItemFlags[0] == 0) ||
			(laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber))
		{
			if (switchItem->Animation.ActiveState == SWITCH_ON)
			{
				laraitem->Pose.Orientation.y ^= (short)ANGLE(180.0f);

				if (TestLaraPosition(&CrowbarBounds2, switchItem, laraitem))
				{
					if (laraInfo->Control.IsMoving || g_Gui.GetInventoryItemChosen() == ID_CROWBAR_ITEM)
					{
						if (MoveLaraPosition(&CrowbarPos2, switchItem, laraitem))
						{
							doSwitch = 1;
							laraitem->Animation.AnimNumber = LA_CROWBAR_USE_ON_FLOOR;
							laraitem->Animation.FrameNumber = g_Level.Anims[laraitem->Animation.AnimNumber].frameBase;
							switchItem->Animation.TargetState = SWITCH_OFF;
						}
						else
							laraInfo->InteractedItem = itemNumber;

						g_Gui.SetInventoryItemChosen(NO_ITEM);
					}
					else
						doSwitch = -1;
				}
				else if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}

				laraitem->Pose.Orientation.y ^= (short)ANGLE(180.0f);
			}
			else
			{
				if (TestLaraPosition(&CrowbarBounds, switchItem, laraitem))
				{
					if (laraInfo->Control.IsMoving || g_Gui.GetInventoryItemChosen() == ID_CROWBAR_ITEM)
					{
						if (MoveLaraPosition(&CrowbarPos, switchItem, laraitem))
						{
							doSwitch = 1;
							laraitem->Animation.AnimNumber = LA_CROWBAR_USE_ON_FLOOR;
							laraitem->Animation.FrameNumber = g_Level.Anims[laraitem->Animation.AnimNumber].frameBase;
							switchItem->Animation.TargetState = SWITCH_ON;
						}
						else
							laraInfo->InteractedItem = itemNumber;

						g_Gui.SetInventoryItemChosen(NO_ITEM);
					}
					else
						doSwitch = -1;
				}
				else if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}
			}
		}

		if (doSwitch)
		{
			if (doSwitch == -1)
			{
				if (laraInfo->Inventory.HasCrowbar)
					g_Gui.SetEnterInventory(ID_CROWBAR_ITEM);
				else
				{
					if (OldPickupPos.x != laraitem->Pose.Position.x || OldPickupPos.y != laraitem->Pose.Position.y || OldPickupPos.z != laraitem->Pose.Position.z)
					{
						OldPickupPos.x = laraitem->Pose.Position.x;
						OldPickupPos.y = laraitem->Pose.Position.y;
						OldPickupPos.z = laraitem->Pose.Position.z;
						SayNo();
					}
				}
			}
			else
			{
				ResetLaraFlex(laraitem);
				laraitem->Animation.TargetState = LS_SWITCH_DOWN;
				laraitem->Animation.ActiveState = LS_SWITCH_DOWN;
				laraInfo->Control.IsMoving = false;
				laraInfo->Control.HandStatus = HandStatus::Busy;
				switchItem->Status = ITEM_ACTIVE;

				AddActiveItem(itemNumber);
				AnimateItem(switchItem);
			}
		}
		else
			ObjectCollision(itemNumber, laraitem, coll);
	}
}
