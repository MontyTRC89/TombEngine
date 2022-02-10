#include "framework.h"
#include "Objects/Generic/Switches/jump_switch.h"
#include "Game/control/control.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Specific/level.h"
#include "Game/collision/collide_item.h"
#include "Game/items.h"

namespace TEN::Entities::Switches
{
	OBJECT_COLLISION_BOUNDS JumpSwitchBounds =  
	{
		-128, 128,
		-256, 256,
		384, 512,
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	PHD_VECTOR JumpSwitchPos = { 0, -208, 256 };  

	void JumpSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if ((TrInput & IN_ACTION)
			&& Lara.Control.HandStatus == HandStatus::Free
			&& (l->ActiveState == LS_REACH || l->ActiveState == LS_JUMP_UP)
			&& (l->Status || l->Airborne)
			&& l->VerticalVelocity > 0
			&& !item->ActiveState)
		{
			if (TestLaraPosition(&JumpSwitchBounds, item, l))
			{
				AlignLaraPosition(&JumpSwitchPos, item, l);

				l->ActiveState = LS_SWITCH_DOWN;
				l->AnimNumber = LA_JUMPSWITCH_PULL;
				l->VerticalVelocity = 0;
				l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
				l->Airborne = false;
				Lara.Control.HandStatus = HandStatus::Busy;

				item->TargetState = SWITCH_ON;
				item->Status = ITEM_ACTIVE;

				AddActiveItem(itemNum);
			}
		}
	}
}