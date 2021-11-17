#include "framework.h"
#include "generic_doors.h"
#include "level.h"
#include "control/control.h"
#include "control/box.h"
#include "items.h"
#include "control/lot.h"
#include "gui.h"
#include "input.h"
#include "pickup.h"
#include "sound.h"
#include "animation.h"
#include "sphere.h"
#include "lara_struct.h"
#include "lara.h"
#include "trmath.h"
#include "misc.h"
#include "double_doors.h"
#include "collide.h"

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
			&& l->currentAnimState == LS_STOP
			&& l->animNumber == LA_STAND_IDLE
			&& !(item->status && item->gravityStatus)
			&& !(l->hitStatus)
			&& !Lara.gunStatus
			|| Lara.isMoving && Lara.interactedItem == itemNum)
		{
			item->pos.yRot ^= ANGLE(180);
			if (TestLaraPosition(&DoubleDoorBounds, item, l))
			{
				if (MoveLaraPosition(&DoubleDoorPos, item, l))
				{
					SetAnimation(l, LA_DOUBLEDOOR_OPEN_PUSH);

					AddActiveItem(itemNum);

					item->status = ITEM_ACTIVE;
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_BUSY;
					Lara.headYrot = 0;
					Lara.headXrot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
				}
				else
				{
					Lara.interactedItem = itemNum;
				}
				item->pos.yRot ^= ANGLE(180);
			}
			else
			{
				if (Lara.isMoving && Lara.interactedItem == itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_NO_ARMS;
				}
				item->pos.yRot ^= ANGLE(180);
			}
		}
	}
}