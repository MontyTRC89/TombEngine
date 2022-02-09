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

		item->ItemFlags[3] = item->TriggerFlags;
		item->TriggerFlags = abs(item->TriggerFlags);

		if (item->Status == ITEM_INVISIBLE)
		{
			item->ItemFlags[1] = 1;
			item->Status = ITEM_NOT_ACTIVE;
		}
	}

	void PulleySwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if ((TrInput & IN_ACTION)
			&& Lara.gunStatus == LG_HANDS_FREE
			&& l->ActiveState == LS_IDLE
			&& l->AnimNumber == LA_STAND_IDLE
			&& l->Airborne == false
			|| Lara.isMoving && Lara.interactedItem == itemNum)
		{
			short oldYrot = item->Position.yRot;
			item->Position.yRot = l->Position.yRot;
			if (TestLaraPosition(&PulleyBounds, item, l))
			{
				if (item->ItemFlags[1])
				{
					if (OldPickupPos.x != l->Position.xPos || OldPickupPos.y != l->Position.yPos || OldPickupPos.z != l->Position.zPos)
					{
						OldPickupPos.x = l->Position.xPos;
						OldPickupPos.y = l->Position.yPos;
						OldPickupPos.z = l->Position.zPos;
						SayNo();
					}
				}
				else if (MoveLaraPosition(&PulleyPos, item, l))
				{
					l->AnimNumber = LA_PULLEY_GRAB;
					l->ActiveState = LS_PULLEY;
					l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;

					AddActiveItem(itemNum);

					item->Position.yRot = oldYrot;
					item->Status = ITEM_ACTIVE;

					Lara.isMoving = false;
					ResetLaraFlex(l);
					Lara.gunStatus = LG_HANDS_BUSY;
					Lara.interactedItem = itemNum;
				}
				else
				{
					Lara.interactedItem = itemNum;
				}
				item->Position.yRot = oldYrot;
			}
			else
			{
				if (Lara.isMoving && Lara.interactedItem == itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_FREE;
				}
				item->Position.yRot = oldYrot;
			}
		}
		else if (l->ActiveState != LS_PULLEY)
		{
			ObjectCollision(itemNum, l, coll);
		}
	}
}