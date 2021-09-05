#include "framework.h"
#include "generic_doors.h"
#include "level.h"
#include "control.h"
#include "box.h"
#include "items.h"
#include "lot.h"
#include "newinv2.h"
#include "input.h"
#include "pickup.h"
#include "sound.h"
#include "draw.h"
#include "sphere.h"
#include "lara_struct.h"
#include "lara.h"
#include "trmath.h"
#include "misc.h"
#include "pushpull_kick_door.h"

namespace TEN::Entities::Doors
{
	PHD_VECTOR PullDoorPos(-201, 0, 322);
	PHD_VECTOR PushDoorPos(201, 0, -702);
	PHD_VECTOR KickDoorPos(0, 0, -917);

	OBJECT_COLLISION_BOUNDS PushPullKickDoorBounds =
	{
		-384, 384,
		0, 0,
		-1024, 512,
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10),
	};

	void PushPullKickDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];
		if (TrInput & IN_ACTION
			&& l->currentAnimState == LS_STOP
			&& l->animNumber == LA_STAND_IDLE
			&& item->status != ITEM_ACTIVE
			&& !(l->hitStatus)
			&& !Lara.gunStatus
			|| Lara.isMoving && Lara.generalPtr == (void*)itemNum)
		{
			bool applyRot = false;

			if (l->roomNumber == item->roomNumber)
			{
				item->pos.yRot ^= ANGLE(180);
				applyRot = true;
			}

			if (!TestLaraPosition(&PushPullKickDoorBounds, item, l))
			{
				if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_NO_ARMS;
				}
				if (applyRot)
					item->pos.yRot ^= ANGLE(180);
				return;
			}

			if (applyRot)
			{
				if (!MoveLaraPosition(&PullDoorPos, item, l))
				{
					Lara.generalPtr = (void*)itemNum;
					item->pos.yRot ^= ANGLE(180);
					return;
				}

				l->animNumber = LA_DOOR_OPEN_PULL;
				l->frameNumber = GF(LA_DOOR_OPEN_PULL, 0);
				item->goalAnimState = 3;

				AddActiveItem(itemNum);

				item->status = ITEM_ACTIVE;
				l->currentAnimState = LS_MISC_CONTROL;
				l->goalAnimState = LS_STOP;
				Lara.isMoving = false;
				Lara.gunStatus = LG_HANDS_BUSY;

				if (applyRot)
					item->pos.yRot ^= ANGLE(180);
				return;
			}

			if (item->objectNumber >= ID_KICK_DOOR1)
			{
				if (MoveLaraPosition(&KickDoorPos, item, l))
				{
					l->animNumber = LA_DOOR_OPEN_KICK;
					l->frameNumber = GF(LA_DOOR_OPEN_KICK, 0);
					item->goalAnimState = 2;

					AddActiveItem(itemNum);

					item->status = ITEM_ACTIVE;
					l->currentAnimState = LS_MISC_CONTROL;
					l->goalAnimState = LS_STOP;
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_BUSY;

					if (applyRot)
						item->pos.yRot ^= ANGLE(180);
					return;
				}
			}
			else if (item->objectNumber == ID_PUSHPULL_DOOR1 || item->objectNumber == ID_PUSHPULL_DOOR2)
			{
				if (MoveLaraPosition(&PushDoorPos, item, l))
				{
					l->animNumber = LA_DOOR_OPEN_PUSH;
					l->frameNumber = GF(LA_DOOR_OPEN_PUSH, 0);
					item->goalAnimState = 2;

					AddActiveItem(itemNum);

					item->status = ITEM_ACTIVE;
					l->currentAnimState = LS_MISC_CONTROL;
					l->goalAnimState = LS_STOP;
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_BUSY;

					if (applyRot)
						item->pos.yRot ^= ANGLE(180);
				}
				return;
			}

			Lara.generalPtr = (void*)itemNum;
			return;
		}

		if (!item->currentAnimState)
			DoorCollision(itemNum, l, coll);
	}

	void PushPullKickDoorControl(short itemNumber)
	{
		ITEM_INFO* item;
		DOOR_DATA* door;

		item = &g_Level.Items[itemNumber];
		door = (DOOR_DATA*)item->data;

		if (!door->opened)
		{
			OpenThatDoor(&door->d1, door);
			OpenThatDoor(&door->d2, door);
			OpenThatDoor(&door->d1flip, door);
			OpenThatDoor(&door->d2flip, door);
			door->opened = true;
		}

		AnimateItem(item);
	}
}