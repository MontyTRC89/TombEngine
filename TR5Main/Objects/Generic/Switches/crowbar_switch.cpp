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

namespace TEN::Entities::Switches
{
	PHD_VECTOR CrowbarPos = { -89, 0, -328 }; 

	OBJECT_COLLISION_BOUNDS CrowbarBounds = 
	{
		-256, 256,
		0, 0,
		-512, -256,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	PHD_VECTOR CrowbarPos2 = { 89, 0, 328 }; 

	OBJECT_COLLISION_BOUNDS CrowbarBounds2 = 
	{
		-256, 256,
		0, 0,
		256, 512,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	void CrowbarSwitchCollision(short itemNumber, ITEM_INFO* laraitem, COLL_INFO* coll)
	{
		auto* laraInfo = GetLaraInfo(laraitem);
		ITEM_INFO* switchItem = &g_Level.Items[itemNumber];

		int doSwitch = 0;

		if (((TrInput & IN_ACTION || g_Gui.GetInventoryItemChosen() == ID_CROWBAR_ITEM) &&
			laraitem->ActiveState == LS_IDLE &&
			laraitem->AnimNumber == LA_STAND_IDLE &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			switchItem->ItemFlags[0] == 0) ||
			(laraInfo->Control.IsMoving && laraInfo->interactedItem == itemNumber))
		{
			if (switchItem->ActiveState == SWITCH_ON)
			{
				laraitem->Position.yRot ^= (short)ANGLE(180.0f);

				if (TestLaraPosition(&CrowbarBounds2, switchItem, laraitem))
				{
					if (laraInfo->Control.IsMoving || g_Gui.GetInventoryItemChosen() == ID_CROWBAR_ITEM)
					{
						if (MoveLaraPosition(&CrowbarPos2, switchItem, laraitem))
						{
							doSwitch = 1;
							laraitem->AnimNumber = LA_CROWBAR_USE_ON_FLOOR;
							laraitem->FrameNumber = g_Level.Anims[laraitem->AnimNumber].frameBase;
							switchItem->TargetState = SWITCH_OFF;
						}
						else
							laraInfo->interactedItem = itemNumber;

						g_Gui.SetInventoryItemChosen(NO_ITEM);
					}
					else
						doSwitch = -1;
				}
				else if (laraInfo->Control.IsMoving && laraInfo->interactedItem == itemNumber)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}

				laraitem->Position.yRot ^= (short)ANGLE(180.0f);
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
							laraitem->AnimNumber = LA_CROWBAR_USE_ON_FLOOR;
							laraitem->FrameNumber = g_Level.Anims[laraitem->AnimNumber].frameBase;
							switchItem->TargetState = SWITCH_ON;
						}
						else
							laraInfo->interactedItem = itemNumber;

						g_Gui.SetInventoryItemChosen(NO_ITEM);
					}
					else
						doSwitch = -1;
				}
				else if (laraInfo->Control.IsMoving && laraInfo->interactedItem == itemNumber)
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
				if (laraInfo->Crowbar)
					g_Gui.SetEnterInventory(ID_CROWBAR_ITEM);
				else
				{
					if (OldPickupPos.x != laraitem->Position.xPos || OldPickupPos.y != laraitem->Position.yPos || OldPickupPos.z != laraitem->Position.zPos)
					{
						OldPickupPos.x = laraitem->Position.xPos;
						OldPickupPos.y = laraitem->Position.yPos;
						OldPickupPos.z = laraitem->Position.zPos;
						SayNo();
					}
				}
			}
			else
			{
				ResetLaraFlex(laraitem);
				laraitem->TargetState = LS_SWITCH_DOWN;
				laraitem->ActiveState = LS_SWITCH_DOWN;
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
