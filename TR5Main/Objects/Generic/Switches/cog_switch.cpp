#include "framework.h"
#include "Objects/Generic/Switches/cog_switch.h"
#include "Game/control/control.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
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
				if (target->Data.is<DOOR_DATA>())
					door = (DOOR_DATA*)target->Data;
			}
		}

		// Door was not found, do ordinary collision and exit.

		if (door == nullptr)
		{
			ObjectCollision(itemNum, l, coll);
			return;
		}

		// Door is found, attach to it.

		if (item->Status == ITEM_NOT_ACTIVE)
		{
			if (!(item->Flags & ONESHOT)
				&& (TrInput & IN_ACTION
					&& !Lara.gunStatus
					&& !item->Airborne
					&& l->ActiveState == LS_IDLE
					&& l->AnimNumber == LA_STAND_IDLE
					|| Lara.Control.IsMoving
					&& Lara.interactedItem == itemNum))
			{
				if (TestLaraPosition(&CogSwitchBounds, item, l))
				{
					if (MoveLaraPosition(&CogSwitchPos, item, l))
					{
						Lara.Control.IsMoving = false;
						ResetLaraFlex(l);
						Lara.gunStatus = LG_HANDS_BUSY;
						Lara.interactedItem = targetItemNum;
						l->AnimNumber = LA_COGWHEEL_GRAB;
						l->TargetState = LS_COGWHEEL;
						l->ActiveState = LS_COGWHEEL;
						l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;

						AddActiveItem(itemNum);
						item->TargetState = SWITCH_ON;
						item->Status = ITEM_ACTIVE;

						if (door != NULL)
						{
							if (!door->opened)
							{
								AddActiveItem((target - g_Level.Items.data()));
								target->Status = ITEM_ACTIVE;
							}
						}
					}
					else
					{
						Lara.interactedItem = itemNum;
					}
					return;
				}
				else if (Lara.Control.IsMoving && Lara.interactedItem == itemNum)
				{
					Lara.Control.IsMoving = false;
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

		if (item->ActiveState == SWITCH_ON)
		{
			if (item->TargetState == SWITCH_ON && !(TrInput & IN_ACTION))
			{
				LaraItem->TargetState = LS_IDLE;
				item->TargetState = SWITCH_OFF;
			}

			if (LaraItem->AnimNumber == LA_COGWHEEL_PULL)
			{
				if (LaraItem->FrameNumber == g_Level.Anims[LaraItem->AnimNumber].frameBase + 10)
				{
					ITEM_INFO* doorItem = &g_Level.Items[Lara.interactedItem];
					doorItem->ItemFlags[0] = COG_DOOR_TURN;
				}
			}
		}
		else
		{
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
			{
				item->ActiveState = SWITCH_OFF;
				item->Status = ITEM_NOT_ACTIVE;

				RemoveActiveItem(itemNumber);

				LaraItem->AnimNumber = LA_STAND_SOLID;
				LaraItem->FrameNumber = g_Level.Anims[LaraItem->AnimNumber].frameBase;
				LaraItem->TargetState = LS_IDLE;
				LaraItem->ActiveState = LS_IDLE;
				Lara.gunStatus = LG_HANDS_FREE;
			}
		}
	}
}