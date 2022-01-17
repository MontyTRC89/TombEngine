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
			&& !Lara.gunStatus
			&& (l->currentAnimState == LS_REACH || l->currentAnimState == LS_JUMP_UP)
			&& (l->status || l->gravityStatus)
			&& l->fallspeed > 0
			&& !item->currentAnimState)
		{
			if (TestLaraPosition(&JumpSwitchBounds, item, l))
			{
				AlignLaraPosition(&JumpSwitchPos, item, l);

				l->currentAnimState = LS_SWITCH_DOWN;
				l->animNumber = LA_JUMPSWITCH_PULL;
				l->fallspeed = 0;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				l->gravityStatus = false;
				Lara.gunStatus = LG_HANDS_BUSY;

				item->goalAnimState = SWITCH_ON;
				item->status = ITEM_ACTIVE;

				AddActiveItem(itemNum);
			}
		}
	}
}