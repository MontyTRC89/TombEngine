#include "framework.h"
#include "pulley_switch.h"
#include "control/control.h"
#include "input.h"
#include "lara.h"
#include "generic_switch.h"
#include "Sound\sound.h"
#include "pickup.h"
#include "level.h"
#include "collide.h"
#include "items.h"

namespace TEN::Entities::Switches
{
	OBJECT_COLLISION_BOUNDS PulleyBounds = 
	{
		-256, 256,
		0, 0,
		-512, 512,
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	PHD_VECTOR PulleyPos = { 0, 0, -148 }; 

	void InitialisePulleySwitch(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		item->itemFlags[3] = item->triggerFlags;
		item->triggerFlags = abs(item->triggerFlags);

		if (item->status == ITEM_INVISIBLE)
		{
			item->itemFlags[1] = 1;
			item->status = ITEM_NOT_ACTIVE;
		}
	}

	void PulleySwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if ((TrInput & IN_ACTION)
			&& Lara.gunStatus == LG_NO_ARMS
			&& l->currentAnimState == LS_STOP
			&& l->animNumber == LA_STAND_IDLE
			&& l->gravityStatus == false
			|| Lara.isMoving && Lara.interactedItem == itemNum)
		{
			short oldYrot = item->pos.yRot;
			item->pos.yRot = l->pos.yRot;
			if (TestLaraPosition(&PulleyBounds, item, l))
			{
				if (item->itemFlags[1])
				{
					if (OldPickupPos.x != l->pos.xPos || OldPickupPos.y != l->pos.yPos || OldPickupPos.z != l->pos.zPos)
					{
						OldPickupPos.x = l->pos.xPos;
						OldPickupPos.y = l->pos.yPos;
						OldPickupPos.z = l->pos.zPos;
						SayNo();
					}
				}
				else if (MoveLaraPosition(&PulleyPos, item, l))
				{
					l->animNumber = LA_PULLEY_GRAB;
					l->currentAnimState = LS_PULLEY;
					l->frameNumber = g_Level.Anims[l->animNumber].frameBase;

					AddActiveItem(itemNum);

					item->pos.yRot = oldYrot;
					item->status = ITEM_ACTIVE;

					Lara.isMoving = false;
					Lara.headYrot = 0;
					Lara.headXrot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.gunStatus = LG_HANDS_BUSY;
					Lara.interactedItem = itemNum;
				}
				else
				{
					Lara.interactedItem = itemNum;
				}
				item->pos.yRot = oldYrot;
			}
			else
			{
				if (Lara.isMoving && Lara.interactedItem == itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_NO_ARMS;
				}
				item->pos.yRot = oldYrot;
			}
		}
		else if (l->currentAnimState != LS_PULLEY)
		{
			ObjectCollision(itemNum, l, coll);
		}
	}
}