#include "framework.h"
#include "generic_switch.h"
#include "input.h"
#include "lara.h"
#include "crowbar_switch.h"
#include "gui.h"
#include "Sound/sound.h"
#include "pickup.h"
#include "level.h"
#include "collide.h"
#include "animation.h"
#include "items.h"

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
			&& l->currentAnimState == LS_STOP
			&& l->animNumber == LA_STAND_IDLE
			&& Lara.gunStatus == LG_NO_ARMS
			&& item->itemFlags[0] == 0)
			|| (Lara.isMoving && Lara.interactedItem == itemNum))
		{
			if (item->currentAnimState == SWITCH_ON)
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
							item->goalAnimState = SWITCH_OFF;
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
					Lara.gunStatus = LG_NO_ARMS;
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
							item->goalAnimState = SWITCH_ON;
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
					Lara.gunStatus = LG_NO_ARMS;
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
				l->goalAnimState = LS_SWITCH_DOWN;
				l->currentAnimState = LS_SWITCH_DOWN;
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
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