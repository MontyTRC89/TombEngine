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
			&& l->activeState == LS_IDLE
			&& l->animNumber == LA_STAND_IDLE
			&& Lara.gunStatus == LG_HANDS_FREE
			&& item->itemFlags[0] == 0)
			|| (Lara.isMoving && Lara.interactedItem == itemNum))
		{
			if (item->activeState == SWITCH_ON)
			{
				l->pos.yRot ^= (short)ANGLE(180);

				if (TestLaraPosition(&CrowbarBounds2, item, l))
				{
					if (Lara.isMoving || g_Gui.GetInventoryItemChosen() == ID_CROWBAR_ITEM)
					{
						if (MoveLaraPosition(&CrowbarPos2, item, l))
						{
							doSwitch = 1;
							l->animNumber = LA_CROWBAR_USE_ON_FLOOR;
							l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
							item->targetState = SWITCH_OFF;
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
				else if (Lara.isMoving && Lara.interactedItem == itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_FREE;
				}
				l->pos.yRot ^= (short)ANGLE(180);
			}
			else
			{
				if (TestLaraPosition(&CrowbarBounds, item, l))
				{
					if (Lara.isMoving || g_Gui.GetInventoryItemChosen() == ID_CROWBAR_ITEM)
					{
						if (MoveLaraPosition(&CrowbarPos, item, l))
						{
							doSwitch = 1;
							l->animNumber = LA_CROWBAR_USE_ON_FLOOR;
							l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
							item->targetState = SWITCH_ON;
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
				else if (Lara.isMoving && Lara.interactedItem == itemNum)
				{
					Lara.isMoving = false;
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
					if (OldPickupPos.x != l->pos.xPos || OldPickupPos.y != l->pos.yPos || OldPickupPos.z != l->pos.zPos)
					{
						OldPickupPos.x = l->pos.xPos;
						OldPickupPos.y = l->pos.yPos;
						OldPickupPos.z = l->pos.zPos;
						SayNo();
					}
				}
			}
			else
			{
				l->targetState = LS_SWITCH_DOWN;
				l->activeState = LS_SWITCH_DOWN;
				Lara.isMoving = false;
				ResetLaraFlex(l);
				Lara.gunStatus = LG_HANDS_BUSY;
				item->status = ITEM_ACTIVE;

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