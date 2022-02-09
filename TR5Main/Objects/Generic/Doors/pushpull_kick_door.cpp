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
#include "Objects/Generic/Doors/pushpull_kick_door.h"
#include "Game/collision/collide_item.h"
#include "Game/itemdata/door_data.h"

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
			&& l->ActiveState == LS_IDLE
			&& l->AnimNumber == LA_STAND_IDLE
			&& item->Status != ITEM_ACTIVE
			&& !(l->HitStatus)
			&& !Lara.gunStatus
			|| Lara.isMoving && Lara.interactedItem == itemNum)
		{
			bool pull = false;

			if (l->RoomNumber == item->RoomNumber)
			{
				item->Position.yRot ^= ANGLE(180);
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
						item->TargetState = STATE_PUSHPULL_KICK_DOOR_PULL;
						openTheDoor = true;
					}
					else
					{
						Lara.interactedItem = itemNum;
					}
				}
				else
				{
					if (item->ObjectNumber >= ID_KICK_DOOR1)
					{
						if (MoveLaraPosition(&KickDoorPos, item, l))
						{
							SetAnimation(l, LA_DOOR_OPEN_KICK);
							item->TargetState = STATE_PUSHPULL_KICK_DOOR_PUSH;
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
							item->TargetState = STATE_PUSHPULL_KICK_DOOR_PUSH;
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

					item->Status = ITEM_ACTIVE;
					l->ActiveState = LS_MISC_CONTROL;
					l->TargetState = LS_IDLE;
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_BUSY;
				}
			}
			else if (Lara.isMoving && Lara.interactedItem == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_HANDS_FREE;
			}

			if (pull)
				item->Position.yRot ^= ANGLE(180);
		}
		else if (item->ActiveState <= STATE_PUSHPULL_KICK_DOOR_CLOSED)
			DoorCollision(itemNum, l, coll);
	}

	void PushPullKickDoorControl(short itemNumber)
	{
		ITEM_INFO* item;
		DOOR_DATA* door;

		item = &g_Level.Items[itemNumber];
		door = (DOOR_DATA*)item->Data;

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