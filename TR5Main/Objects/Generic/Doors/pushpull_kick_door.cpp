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
#include "pushpull_kick_door.h"
#include "collide.h"
#include "itemdata/door_data.h"

namespace TEN::Entities::Doors
{
	enum STATES_PUSHPULL_KICK_DOOR
	{
		STATE_PUSHPULL_KICK_DOOR_OPEN = 0,
		STATE_PUSHPULL_KICK_DOOR_CLOSED = 1,
		STATE_PUSHPULL_KICK_DOOR_PUSH = 2,
		STATE_PUSHPULL_KICK_DOOR_PULL = 3
	};
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
			|| Lara.isMoving && Lara.interactedItem == itemNum)
		{
			bool pull = false;

			if (l->roomNumber == item->roomNumber)
			{
				item->pos.yRot ^= ANGLE(180);
				pull = true;
			}

			if (TestLaraPosition(&PushPullKickDoorBounds, item, l))
			{
				bool openTheDoor = false;

				if (pull)
				{
					if (MoveLaraPosition(&PullDoorPos, item, l))
					{
						SetAnimation(l, LA_DOOR_OPEN_PULL);
						item->goalAnimState = STATE_PUSHPULL_KICK_DOOR_PULL;
						openTheDoor = true;
					}
					else
					{
						Lara.interactedItem = itemNum;
					}
				}
				else
				{
					if (item->objectNumber >= ID_KICK_DOOR1)
					{
						if (MoveLaraPosition(&KickDoorPos, item, l))
						{
							SetAnimation(l, LA_DOOR_OPEN_KICK);
							item->goalAnimState = STATE_PUSHPULL_KICK_DOOR_PUSH;
							openTheDoor = true;
						}
						else
						{
							Lara.interactedItem = itemNum;
						}
					}
					else
					{
						if (MoveLaraPosition(&PushDoorPos, item, l))
						{
							SetAnimation(l, LA_DOOR_OPEN_PUSH);
							item->goalAnimState = STATE_PUSHPULL_KICK_DOOR_PUSH;
							openTheDoor = true;
						}
						else
						{
							Lara.interactedItem = itemNum;
						}
					}
				}

				if (openTheDoor)
				{
					AddActiveItem(itemNum);

					item->status = ITEM_ACTIVE;
					l->currentAnimState = LS_MISC_CONTROL;
					l->goalAnimState = LS_STOP;
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_BUSY;
				}
			}
			else if (Lara.isMoving && Lara.interactedItem == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}

			if (pull)
				item->pos.yRot ^= ANGLE(180);
		}
		else if (item->currentAnimState <= STATE_PUSHPULL_KICK_DOOR_CLOSED)
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