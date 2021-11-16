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
#include "underwater_door.h"
#include "collide.h"

namespace TEN::Entities::Doors
{
	PHD_VECTOR UnderwaterDoorPos(-251, -540, -46);

	OBJECT_COLLISION_BOUNDS UnderwaterDoorBounds =
	{
		-256, 256, 
		-1024, 0, 
		-1024, 0, 
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80)
	};

	void UnderwaterDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (TrInput & IN_ACTION
			&& l->currentAnimState == LS_UNDERWATER_STOP
			&& !(item->status && item->gravityStatus)
			&& Lara.waterStatus == LW_UNDERWATER
			&& !Lara.gunStatus
			|| Lara.isMoving && Lara.interactedItem == itemNum)
		{
			l->pos.yRot ^= ANGLE(180.0f);

			if (TestLaraPosition(&UnderwaterDoorBounds, item, l))
			{
				if (MoveLaraPosition(&UnderwaterDoorPos, item, l))
				{
					SetAnimation(l, LA_UNDERWATER_DOOR_OPEN);
					l->fallspeed = 0;
					item->status = ITEM_ACTIVE;

					AddActiveItem(itemNum);

					item->goalAnimState = LS_RUN_FORWARD;

					AnimateItem(item);

					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_BUSY;
				}
				else
				{
					Lara.interactedItem = itemNum;
				}
				l->pos.yRot ^= ANGLE(180);
			}
			else
			{
				if (Lara.isMoving && Lara.interactedItem == itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_NO_ARMS;
				}
				l->pos.yRot ^= ANGLE(180);
			}
		}
		else if (item->status == ITEM_ACTIVE)
		{
			ObjectCollision(itemNum, l, coll);
		}
	}
}