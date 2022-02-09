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
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	PHD_VECTOR CrowbarPos2 = { 89, 0, 328 }; 

	OBJECT_COLLISION_BOUNDS CrowbarBounds2 = 
	{
		-256, 256,
		0, 0,
		256, 512,
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	void CrowbarSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		int doSwitch = 0;
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if ((((TrInput & IN_ACTION) || g_Gui.GetInventoryItemChosen() == ID_CROWBAR_ITEM)
			&& l->ActiveState == LS_IDLE
			&& l->AnimNumber == LA_STAND_IDLE
			&& Lara.gunStatus == LG_HANDS_FREE
			&& item->ItemFlags[0] == 0)
			|| (Lara.Control.IsMoving && Lara.interactedItem == itemNum))
		{
			if (item->ActiveState == SWITCH_ON)
			{
				l->Position.yRot ^= (short)ANGLE(180);

				if (TestLaraPosition(&CrowbarBounds2, item, l))
				{
					if (Lara.Control.IsMoving || g_Gui.GetInventoryItemChosen() == ID_CROWBAR_ITEM)
					{
						if (MoveLaraPosition(&CrowbarPos2, item, l))
						{
							doSwitch = 1;
							l->AnimNumber = LA_CROWBAR_USE_ON_FLOOR;
							l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
							item->TargetState = SWITCH_OFF;
						}
						else
						{
							Lara.interactedItem = itemNum;
						}

						g_Gui.SetInventoryItemChosen(NO_ITEM);
					}
					else
					{
						doSwitch = -1;
					}
				}
				else if (Lara.Control.IsMoving && Lara.interactedItem == itemNum)
				{
					Lara.Control.IsMoving = false;
					Lara.gunStatus = LG_HANDS_FREE;
				}
				l->Position.yRot ^= (short)ANGLE(180);
			}
			else
			{
				if (TestLaraPosition(&CrowbarBounds, item, l))
				{
					if (Lara.Control.IsMoving || g_Gui.GetInventoryItemChosen() == ID_CROWBAR_ITEM)
					{
						if (MoveLaraPosition(&CrowbarPos, item, l))
						{
							doSwitch = 1;
							l->AnimNumber = LA_CROWBAR_USE_ON_FLOOR;
							l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
							item->TargetState = SWITCH_ON;
						}
						else
						{
							Lara.interactedItem = itemNum;
						}

						g_Gui.SetInventoryItemChosen(NO_ITEM);
					}
					else
					{
						doSwitch = -1;
					}
				}
				else if (Lara.Control.IsMoving && Lara.interactedItem == itemNum)
				{
					Lara.Control.IsMoving = false;
					Lara.gunStatus = LG_HANDS_FREE;
				}
			}
		}

		if (doSwitch)
		{
			if (doSwitch == -1)
			{
				if (Lara.Crowbar)
					g_Gui.SetEnterInventory(ID_CROWBAR_ITEM);
				else
				{
					if (OldPickupPos.x != l->Position.xPos || OldPickupPos.y != l->Position.yPos || OldPickupPos.z != l->Position.zPos)
					{
						OldPickupPos.x = l->Position.xPos;
						OldPickupPos.y = l->Position.yPos;
						OldPickupPos.z = l->Position.zPos;
						SayNo();
					}
				}
			}
			else
			{
				l->TargetState = LS_SWITCH_DOWN;
				l->ActiveState = LS_SWITCH_DOWN;
				Lara.Control.IsMoving = false;
				ResetLaraFlex(l);
				Lara.gunStatus = LG_HANDS_BUSY;
				item->Status = ITEM_ACTIVE;

				AddActiveItem(itemNum);
				AnimateItem(item);
			}
		}
		else
		{
			ObjectCollision(itemNum, l, coll);
		}
	}
}