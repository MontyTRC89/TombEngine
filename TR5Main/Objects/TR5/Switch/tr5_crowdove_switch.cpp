#include "framework.h"
#include "tr5_crowdove_switch.h"
#include "Game/control/control.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/items.h"
#include "Game/collision/collide_item.h"

using namespace TEN::Entities::Switches;

namespace TEN::Entities::TR5
{
	OBJECT_COLLISION_BOUNDS CrowDoveBounds =
	{
		-256, 256,
		0, 0,
		-512, 512,
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	PHD_VECTOR CrowDovePos = { 0, 0, -400 }; 

	void InitialiseCrowDoveSwitch(short itemNumber)
	{
		g_Level.Items[itemNumber].MeshBits = 3;
	}

	void CrowDoveSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (item->Flags & ONESHOT
			|| !(item->MeshBits & 4)
			|| (!(TrInput & IN_ACTION)
				|| Lara.gunStatus
				|| l->ActiveState != LS_IDLE
				|| l->AnimNumber != LA_STAND_IDLE
				|| l->Airborne)
			&& (!Lara.Control.IsMoving || Lara.interactedItem != itemNum))
		{
			if (l->ActiveState != LS_DOVESWITCH)
				ObjectCollision(itemNum, l, coll);
		}
		else
		{
			int oldYrot = item->Position.yRot;
			item->Position.yRot = l->Position.yRot;
			if (TestLaraPosition(&CrowDoveBounds, item, l))
			{
				if (MoveLaraPosition(&CrowDovePos, item, l))
				{
					l->AnimNumber = LA_DOVESWITCH_TURN;
					l->ActiveState = LS_DOVESWITCH;
					l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;

					AddActiveItem(itemNum);

					// NOTE: In original TR5 the switch was used together with heavy switches.
					// This little fix make it usable normaly and less hardcoded.
					item->ItemFlags[0] = 0;

					item->Status = ITEM_ACTIVE;
					item->Position.yRot = oldYrot;
					Lara.Control.IsMoving = false;
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
				if (Lara.Control.IsMoving && Lara.interactedItem == itemNum)
				{
					Lara.Control.IsMoving = false;
					Lara.gunStatus = LG_HANDS_FREE;
				}
				item->Position.yRot = oldYrot;
			}
		}
	}

	void CrowDoveSwitchControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->MeshBits & 2) 
		{
			ExplodeItemNode(item, 1, 0, 256); 
			SoundEffect(SFX_TR5_RAVENSWITCH_EXP, &item->Position, 0);
			item->MeshBits = 5;	
			RemoveActiveItem(itemNumber);

			// NOTE: In original TR5 the switch was used together with heavy switches.
			// This little fix make it usable normaly and less hardcoded.
			item->ItemFlags[0] = 1; 
		}
		else if (item->ItemFlags[0] == 0)
		{
			if (item->ActiveState == SWITCH_OFF)
				item->TargetState = SWITCH_ON;

			AnimateItem(item);

			if (item->ActiveState == SWITCH_OFF)
				item->Position.yRot += ANGLE(90);
		}
	}
}