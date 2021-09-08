#include "framework.h"
#include "jump_switch.h"
#include "control.h"
#include "input.h"
#include "lara.h"
#include "crowbar_switch.h"
#include "newinv2.h"
#include "sound.h"
#include "pickup.h"

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

		if ((!(TrInput & IN_ACTION) &&
#ifdef NEW_INV
			GLOBAL_inventoryitemchosen != ID_CROWBAR_ITEM
#else
			g_Inventory.GetSelectedObject() != ID_CROWBAR_ITEM
#endif
			|| l->currentAnimState != LS_STOP
			|| l->animNumber != LA_STAND_IDLE
			|| Lara.gunStatus
			|| item->itemFlags[0])
			&& (!Lara.isMoving || Lara.interactedItem !=itemNum))
		{
			ObjectCollision(itemNum, l, coll);
			return;
		}

		if (item->currentAnimState)
		{
			if (item->currentAnimState != 1)
			{
				ObjectCollision(itemNum, l, coll);
				return;
			}

			l->pos.yRot ^= (short)ANGLE(180);
			if (TestLaraPosition(&CrowbarBounds2, item, l))
			{
				if (Lara.isMoving ||
#ifdef NEW_INV
					GLOBAL_inventoryitemchosen == ID_CROWBAR_ITEM
#else
					g_Inventory.GetSelectedObject() == ID_CROWBAR_ITEM
#endif
					)
				{
					if (MoveLaraPosition(&CrowbarPos2, item, l))
					{
						l->animNumber = LA_CROWBAR_USE_ON_FLOOR;
						doSwitch = 1;
						l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
						item->goalAnimState = 0;
					}
					else
					{
						Lara.interactedItem = itemNum;
					}
#ifdef NEW_INV
					GLOBAL_inventoryitemchosen = NO_ITEM;
#else
					g_Inventory.SetSelectedObject(NO_ITEM);
#endif
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
			if (!TestLaraPosition(&CrowbarBounds, item, l))
			{
				if (Lara.isMoving && Lara.interactedItem == itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_NO_ARMS;
				}
				ObjectCollision(itemNum, l, coll);
				return;
			}

			if (!(Lara.isMoving &&
#ifdef NEW_INV
				GLOBAL_inventoryitemchosen != ID_CROWBAR_ITEM)
#else
				g_Inventory.GetSelectedObject() != ID_CROWBAR_ITEM)
#endif
				)
			{
				if (Lara.Crowbar)
#ifdef NEW_INV
					GLOBAL_inventoryitemchosen = ID_CROWBAR_ITEM;
#else
					g_Inventory.SetEnterObject(ID_CROWBAR_ITEM);
#endif
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
				return;
			}

			if (MoveLaraPosition(&CrowbarPos, item, l))
			{
				l->animNumber = LA_CROWBAR_USE_ON_FLOOR;
				doSwitch = 1;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				item->goalAnimState = 1;
			}
			else
			{
				Lara.interactedItem = itemNum;
			}
#ifdef NEW_INV
			GLOBAL_inventoryitemchosen = NO_ITEM;
#else
			g_Inventory.SetSelectedObject(NO_ITEM);
#endif
		}

		if (!doSwitch)
		{
			ObjectCollision(itemNum, l, coll);
			return;
		}

		if (doSwitch != -1)
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

			return;
		}

		if (Lara.Crowbar)
#ifdef NEW_INV
			GLOBAL_enterinventory = ID_CROWBAR_ITEM;
#else
			g_Inventory.SetEnterObject(ID_CROWBAR_ITEM);
#endif
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

}