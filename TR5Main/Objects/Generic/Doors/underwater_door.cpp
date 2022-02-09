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
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara.h"
#include "Specific/trmath.h"
#include "Game/misc.h"
#include "Objects/Generic/Doors/underwater_door.h"
#include "Game/collision/collide_item.h"

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
			&& l->activeState == LS_UNDERWATER_STOP
			&& !(item->status && item->Airborne)
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
					l->VerticalVelocity = 0;
					item->status = ITEM_ACTIVE;

					AddActiveItem(itemNum);

					item->targetState = LS_RUN_FORWARD;

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
					Lara.gunStatus = LG_HANDS_FREE;
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