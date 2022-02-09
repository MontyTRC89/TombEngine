#include "framework.h"
#include "Objects/Generic/Doors/generic_doors.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/control/lot.h"
#include "Game/gui.h"
#include "Specific/input.h"
#include "Game/pickup/pickup.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/collision/sphere.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Specific/trmath.h"
#include "Game/misc.h"
#include "Objects/Generic/Doors/double_doors.h"
#include "Game/collision/collide_item.h"

namespace TEN::Entities::Doors
{
	PHD_VECTOR DoubleDoorPos(0, 0, 220);

	OBJECT_COLLISION_BOUNDS DoubleDoorBounds =
	{
		-384, 384, 
		0, 0, 
		-1024, 512, 
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10),
	};

	void DoubleDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (TrInput & IN_ACTION
			&& l->ActiveState == LS_IDLE
			&& l->AnimNumber == LA_STAND_IDLE
			&& !(item->Status && item->Airborne)
			&& !(l->HitStatus)
			&& !Lara.gunStatus
			|| Lara.isMoving && Lara.interactedItem == itemNum)
		{
			item->Position.yRot ^= ANGLE(180);
			if (TestLaraPosition(&DoubleDoorBounds, item, l))
			{
				if (MoveLaraPosition(&DoubleDoorPos, item, l))
				{
					SetAnimation(l, LA_DOUBLEDOOR_OPEN_PUSH);

					AddActiveItem(itemNum);

					item->Status = ITEM_ACTIVE;
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_BUSY;
					ResetLaraFlex(l);
				}
				else
				{
					Lara.interactedItem = itemNum;
				}
				item->Position.yRot ^= ANGLE(180);
			}
			else
			{
				if (Lara.isMoving && Lara.interactedItem == itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_FREE;
				}
				item->Position.yRot ^= ANGLE(180);
			}
		}
	}
}