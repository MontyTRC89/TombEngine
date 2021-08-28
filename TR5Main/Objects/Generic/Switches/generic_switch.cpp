#include "framework.h"
#include "control.h"
#include "input.h"
#include "lara.h"
#include "generic_switch.h"
#include "sphere.h"
#include "draw.h"

namespace TEN::Entities::Switches
{
	OBJECT_COLLISION_BOUNDS SwitchBounds = 
	{
		0, 0,
		0, 0,
		0, 0,
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	PHD_VECTOR SwitchPos = { 0, 0, 0 };

	void SwitchControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		item->flags |= 0x3E00;

		if (!TriggerActive(item) && !(item->flags & ONESHOT))
		{
			if (item->objectNumber == ID_JUMP_SWITCH)
			{
				item->goalAnimState = SWITCH_OFF;
				item->timer = 0;
				AnimateItem(item);
			}
			else
			{
				item->goalAnimState = SWITCH_ON;
				item->timer = 0;
			}
		}

		AnimateItem(item);
	}

	void SwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];
		if (TrInput & IN_ACTION
			&& l->currentAnimState == LS_STOP
			&& l->animNumber == LA_STAND_IDLE
			&& !Lara.gunStatus
			&& item->status == ITEM_NOT_ACTIVE
			&& !(item->flags & 0x100)
			&& item->triggerFlags >= 0
			|| Lara.isMoving && Lara.generalPtr == (void*)itemNum)
		{
			BOUNDING_BOX* bounds = GetBoundsAccurate(item);

			if (item->triggerFlags == 3 && item->currentAnimState == SWITCH_ON
				|| item->triggerFlags >= 5 && item->triggerFlags <= 7 
					&& item->currentAnimState == SWITCH_OFF)
				return;

			SwitchBounds.boundingBox.X1 = bounds->X1 - 256;
			SwitchBounds.boundingBox.X2 = bounds->X2 + 256;

			if (item->triggerFlags)
			{
				SwitchBounds.boundingBox.Z1 = bounds->Z1 - 512;
				SwitchBounds.boundingBox.Z2 = bounds->Z2 + 512;

				if (item->triggerFlags == 3)
				{
					SwitchPos.z = bounds->Z1 - 256;
				}
				else
				{
					SwitchPos.z = bounds->Z1 - 128;
				}
			}
			else
			{
				SwitchBounds.boundingBox.Z1 = bounds->Z1 - 200;
				SwitchBounds.boundingBox.Z2 = bounds->Z2 + 200;
				SwitchPos.z = bounds->Z1 - 64;
			}

			if (TestLaraPosition(&SwitchBounds, item, l))
			{
				if (MoveLaraPosition(&SwitchPos, item, l))
				{
					if (item->currentAnimState == SWITCH_ON) /* Switch down */
					{
						if (item->triggerFlags)
						{
							l->animNumber = LA_HOLESWITCH_ACTIVATE;
							l->currentAnimState = LS_HOLE;
						}
						else
						{
							l->currentAnimState = LS_SWITCH_UP;
							l->animNumber = LA_WALLSWITCH_DOWN;
						}
						
						item->goalAnimState = SWITCH_OFF;

						// NOTE: TR5 OCBs, to evaluate if keep them
						/*if (item->triggerFlags)
						{
							if (item->triggerFlags >= 3)
							{
								if (item->triggerFlags == 4)
								{
									l->currentAnimState = LS_SWITCH_UP;
									l->animNumber = LA_SWITCH_SMALL_DOWN;
									item->goalAnimState = SWITCH_OFF;
								}
								else
								{
									if (item->triggerFlags >= 5 && item->triggerFlags <= 7)
									{
										if (item->triggerFlags == 6)
											DisableLaraControl = true;
										l->currentAnimState = LS_SWITCH_DOWN;
										l->animNumber = LA_BUTTON_SMALL_PUSH;
									}
									item->goalAnimState = SWITCH_OFF;
								}
							}
							else
							{
								l->animNumber = LA_HOLESWITCH_ACTIVATE;
								l->currentAnimState = LS_HOLE;
								item->goalAnimState = SWITCH_OFF;
							}
						}
						else
						{
							l->currentAnimState = LS_SWITCH_UP;
							l->animNumber = LA_WALLSWITCH_DOWN;
							item->goalAnimState = SWITCH_OFF;
						}*/
					}
					else /* Switch up */
					{
						if (item->triggerFlags)
						{
							if (item->triggerFlags == 3)
							{
								l->animNumber = LA_BUTTON_LARGE_PUSH;
							}
							else
							{
								l->animNumber = LA_HOLESWITCH_ACTIVATE;
								l->currentAnimState = LS_HOLE;
							}
						}
						else
						{
							l->currentAnimState = LS_SWITCH_DOWN;
							l->animNumber = LA_WALLSWITCH_UP;
						}

						item->goalAnimState = SWITCH_ON;

						// NOTE: TR5 OCBs, to evaluate if keep them
						/*if (item->triggerFlags)
						{
							if (item->triggerFlags == 3)
							{
								l->currentAnimState = LS_SWITCH_DOWN;
								l->animNumber = LA_BUTTON_LARGE_PUSH;
							}
							else if (item->triggerFlags == 4)
							{
								l->currentAnimState = LS_SWITCH_DOWN;
								l->animNumber = LA_SWITCH_SMALL_UP;
							}
							else if (item->triggerFlags < 8)
							{
								l->currentAnimState = LS_HOLE;
								l->animNumber = LA_HOLESWITCH_ACTIVATE;
							}
							else
							{
								l->currentAnimState = LS_SWITCH_DOWN;
								l->animNumber = LA_VALVE_TURN;
							}
						}
						else
						{
							l->currentAnimState = LS_SWITCH_DOWN;
							l->animNumber = LA_WALLSWITCH_UP;
						}

						item->goalAnimState = SWITCH_ON;*/
					}

					l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_BUSY;
					Lara.headYrot = 0;
					Lara.headXrot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;

					AddActiveItem(itemNum);
					item->status = ITEM_ACTIVE;
					AnimateItem(item);
				}
				else
				{
					Lara.generalPtr = (void*)itemNum;
				}
			}
			else if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}

			return;
		}

		if (l->currentAnimState != LS_SWITCH_DOWN && l->currentAnimState != LS_SWITCH_UP)
			ObjectCollision(itemNum, l, coll);
	}

	int GetKeyTrigger(ITEM_INFO* item)
	{
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);
		auto triggerIndex = GetTriggerIndex(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

		if (triggerIndex)
		{
			short* trigger = triggerIndex;
			for (short i = *triggerIndex; (i & 0x1F) != 4; trigger++)
			{
				if (i < 0)
					break;
				i = trigger[1];
			}
			if (*trigger & 4)
			{
				for (short* j = &trigger[2]; (*j / 256) & 0x3C || item != &g_Level.Items[*j & 0x3FF]; j++)
				{
					if (*j & 0x8000)
						return 0;
				}
				return 1;
			}
		}

		return 0;
	}

	int GetSwitchTrigger(ITEM_INFO* item, short* itemNos, int AttatchedToSwitch)
	{
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);
		auto triggerIndex = GetTriggerIndex(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

		if (triggerIndex)
		{
			short* trigger;
			for (trigger = triggerIndex; (*trigger & DATA_TYPE) != TRIGGER_TYPE; trigger++)
			{
				if (*trigger & END_BIT)
					break;
			}

			if (*trigger & 4)
			{
				trigger += 2;
				short* current = itemNos;
				int k = 0;
				do
				{
					if (TRIG_BITS(*trigger) == TO_OBJECT && item != &g_Level.Items[*trigger & VALUE_BITS])
					{
						current[k] = *trigger & VALUE_BITS;
						++k;
					}
					if (*trigger & END_BIT)
						break;
					++trigger;
				} while (true);

				return k;
			}
		}

		return 0;
	}

	int SwitchTrigger(short itemNum, short timer)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];
		if (item->status == ITEM_DEACTIVATED)
		{
			if ((!item->currentAnimState && item->objectNumber != ID_JUMP_SWITCH || item->currentAnimState == 1 && item->objectNumber == ID_JUMP_SWITCH) && timer > 0)
			{
				item->timer = timer;
				item->status = ITEM_ACTIVE;
				if (timer != 1)
					item->timer = 30 * timer;
				return 1;
			}
			if (item->triggerFlags != 6 || item->currentAnimState)
			{
				RemoveActiveItem(itemNum);

				item->status = ITEM_NOT_ACTIVE;
				if (!item->itemFlags[0] == 0)
					item->flags |= 0x100;
				if (item->currentAnimState != 1)
					return 1;
				if (item->triggerFlags != 5 && item->triggerFlags != 6)
					return 1;
			}
			else
			{
				item->status = ITEM_ACTIVE;
				return 1;
			}
		}
		else if (item->status)
		{
			return ((item->flags & 0x100u) / 256);
		}
		else
		{
			return 0;
		}

		return 0;
	}
}