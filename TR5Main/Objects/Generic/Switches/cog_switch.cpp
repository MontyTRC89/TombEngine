#include "framework.h"
#include "Objects/Generic/Switches/cog_switch.h"
#include "Game/control/control.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Game/itemdata/door_data.h"
#include "Game/control/box.h"
#include "Objects/Generic/Doors/generic_doors.h"
#include "Game/collision/collide_item.h"
#include "Game/animation.h"
#include "Game/items.h"

using namespace TEN::Entities::Doors;

namespace TEN::Entities::Switches
{
	OBJECT_COLLISION_BOUNDS CogSwitchBounds =
	{
		-512, 512,
		0, 0,
		-1536, -512,
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};
	PHD_VECTOR CogSwitchPos(0, 0, -856);

	void CogSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		auto item = &g_Level.Items[itemNum];
		auto triggerIndex = GetTriggerIndex(item);

		int targetItemNum;
		ITEM_INFO* target = nullptr;
		DOOR_DATA* door   = nullptr;

		// Try to find first item in a trigger list, and if it is a door,
		// attach it to cog. If no object found or object is not door,
		// bypass further processing and do ordinary object collision.

		if (triggerIndex)
		{
			short* trigger = triggerIndex;
			targetItemNum = trigger[3] & VALUE_BITS;

			if (targetItemNum < g_Level.Items.size())
			{
				target = &g_Level.Items[targetItemNum];
				if (target->data.is<DOOR_DATA>())
					door = (DOOR_DATA*)target->data;
			}
		}

		// Door was not found, do ordinary collision and exit.

		if (door == nullptr)
		{
			ObjectCollision(itemNum, l, coll);
			return;
		}

		// Door is found, attach to it.

		if (item->status == ITEM_NOT_ACTIVE)
		{
			if (!(item->flags & ONESHOT)
				&& (TrInput & IN_ACTION
					&& !Lara.gunStatus
					&& !item->gravityStatus
					&& l->currentAnimState == LS_IDLE
					&& l->animNumber == LA_STAND_IDLE
					|| Lara.isMoving
					&& Lara.interactedItem == itemNum))
			{
				if (TestLaraPosition(&CogSwitchBounds, item, l))
				{
					if (MoveLaraPosition(&CogSwitchPos, item, l))
					{
						Lara.isMoving = false;
						Lara.headYrot = 0;
						Lara.headXrot = 0;
						Lara.torsoYrot = 0;
						Lara.torsoXrot = 0;
						Lara.gunStatus = LG_HANDS_BUSY;
						Lara.interactedItem = targetItemNum;
						l->animNumber = LA_COGWHEEL_GRAB;
						l->goalAnimState = LS_COGWHEEL;
						l->currentAnimState = LS_COGWHEEL;
						l->frameNumber = g_Level.Anims[l->animNumber].frameBase;

						AddActiveItem(itemNum);
						item->goalAnimState = SWITCH_ON;
						item->status = ITEM_ACTIVE;

						if (door != NULL)
						{
							if (!door->opened)
							{
								AddActiveItem((target - g_Level.Items.data()));
								target->status = ITEM_ACTIVE;
							}
						}
					}
					else
					{
						Lara.interactedItem = itemNum;
					}
					return;
				}
				else if (Lara.isMoving && Lara.interactedItem == itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_FREE;
				}
			}

			ObjectCollision(itemNum, l, coll);
		}
	}

	void CogSwitchControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		AnimateItem(item);

		if (item->currentAnimState == SWITCH_ON)
		{
			if (item->goalAnimState == SWITCH_ON && !(TrInput & IN_ACTION))
			{
				LaraItem->goalAnimState = LS_IDLE;
				item->goalAnimState = SWITCH_OFF;
			}

			if (LaraItem->animNumber == LA_COGWHEEL_PULL)
			{
				if (LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameBase + 10)
				{
					ITEM_INFO* doorItem = &g_Level.Items[Lara.interactedItem];
					doorItem->itemFlags[0] = COG_DOOR_TURN;
				}
			}
		}
		else
		{
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
			{
				item->currentAnimState = SWITCH_OFF;
				item->status = ITEM_NOT_ACTIVE;

				RemoveActiveItem(itemNumber);

				LaraItem->animNumber = LA_STAND_SOLID;
				LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
				LaraItem->goalAnimState = LS_IDLE;
				LaraItem->currentAnimState = LS_IDLE;
				Lara.gunStatus = LG_HANDS_FREE;
			}
		}
	}
}