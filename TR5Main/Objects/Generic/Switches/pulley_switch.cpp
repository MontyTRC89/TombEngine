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
			&& Lara.gunStatus == LG_HANDS_FREE
			&& l->activeState == LS_IDLE
			&& l->animNumber == LA_STAND_IDLE
			&& l->airborne == false
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
					l->activeState = LS_PULLEY;
					l->frameNumber = g_Level.Anims[l->animNumber].frameBase;

					AddActiveItem(itemNum);

					item->pos.yRot = oldYrot;
					item->status = ITEM_ACTIVE;

					Lara.isMoving = false;
					ResetLaraFlex(l);
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
					Lara.gunStatus = LG_HANDS_FREE;
				}
				item->pos.yRot = oldYrot;
			}
		}
		else if (l->activeState != LS_PULLEY)
		{
			ObjectCollision(itemNum, l, coll);
		}
	}
}