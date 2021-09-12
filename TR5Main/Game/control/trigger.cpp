#include "framework.h"
#include "items.h"
#include "control.h"
#include "floordata.h"
#include "objectslist.h"
#include "box.h"
#include "camera.h"
#include "puzzles_keys.h"
#include "pickup.h"
#include "lot.h"
#include "spotcam.h"
#include "traps.h"
#include "lara.h"
#include "Lara\lara_climb.h"
#include "Objects\Generic\Switches\generic_switch.h"

using namespace TEN::Entities::Switches;

int TriggerActive(ITEM_INFO* item)
{
	int flag;

	flag = (~item->flags & IFLAG_REVERSE) >> 14;
	if ((item->flags & IFLAG_ACTIVATION_MASK) != IFLAG_ACTIVATION_MASK)
	{
		flag = !flag;
	}
	else
	{
		if (item->timer)
		{
			if (item->timer > 0)
			{
				--item->timer;
				if (!item->timer)
					item->timer = -1;
			}
			else if (item->timer < -1)
			{
				++item->timer;
				if (item->timer == -1)
					item->timer = 0;
			}
			if (item->timer <= -1)
				flag = !flag;
		}
	}
	return flag;
}

int GetKeyTrigger(ITEM_INFO* item)
{
	auto triggerIndex = GetTriggerIndex(item);

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
			for (short* j = &trigger[2]; (*j >> 8) & 0x3C || item != &g_Level.Items[*j & 0x3FF]; j++)
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
	auto triggerIndex = GetTriggerIndex(item);

	if (triggerIndex)
	{
		short* trigger = triggerIndex;

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
		return (item->flags & 0x100u) >> 8;
	}
	else
	{
		return 0;
	}

	return 0;
}

short* GetTriggerIndex(FLOOR_INFO* floor, int x, int y, int z)
{
	ROOM_INFO* r;
	while (floor->RoomBelow() != NO_ROOM)
	{
		if (CheckNoColFloorTriangle(floor, x, z) == 1)
			break;
		r = &g_Level.Rooms[floor->RoomBelow()];
		floor = XZ_GET_SECTOR(r, x - r->x, z - r->z);
	}

	if (floor->index == 0)
		return NULL;

	return &g_Level.FloorData[floor->index];
}

short* GetTriggerIndex(ITEM_INFO* item)
{
	auto roomNumber = item->roomNumber;
	auto floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	return GetTriggerIndex(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
}


void TestTriggers(short* data, bool heavy, int heavyFlags)
{
	int flip = -1;
	int flipAvailable = 0;
	int newEffect = -1;
	int switchOff = 0;
	int switchFlag = 0;
	short objectNumber = 0;
	int keyResult = 0;
	short cameraFlags = 0;
	short cameraTimer = 0;
	int spotCamIndex = 0;

	if (!data)
		return;

	short triggerType = (*(data++) >> 8) & 0x3F;
	short flags = *(data++);
	short timer = flags & 0xFF;

	if (Camera.type != HEAVY_CAMERA)
		RefreshCamera(triggerType, data);

	short value = 0;

	if (heavy)
	{
		switch (triggerType)
		{
		case TRIGGER_TYPES::HEAVY:
		case TRIGGER_TYPES::HEAVYANTITRIGGER:
			break;

		case TRIGGER_TYPES::HEAVYSWITCH:
			if (!heavyFlags)
				return;

			if (heavyFlags >= 0)
			{
				flags &= CODE_BITS;
				if (flags != heavyFlags)
					return;
			}
			else
			{
				flags |= CODE_BITS;
				flags += heavyFlags;
			}
			break;

		default:
			// Enemies can only activate heavy triggers
			return;
		}
	}
	else
	{
		switch (triggerType)
		{
		case TRIGGER_TYPES::SWITCH:
			value = *(data++) & 0x3FF;

			if (flags & ONESHOT)
				g_Level.Items[value].itemFlags[0] = 1;

			if (!SwitchTrigger(value, timer))
				return;

			objectNumber = g_Level.Items[value].objectNumber;
			if (objectNumber >= ID_SWITCH_TYPE1 && objectNumber <= ID_SWITCH_TYPE6 && g_Level.Items[value].triggerFlags == 5)
				switchFlag = 1;

			switchOff = (g_Level.Items[value].currentAnimState == 1);

			break;

		case TRIGGER_TYPES::MONKEY:
			if (LaraItem->currentAnimState >= LS_MONKEYSWING_IDLE &&
				(LaraItem->currentAnimState <= LS_MONKEYSWING_TURN_180 ||
					LaraItem->currentAnimState == LS_MONKEYSWING_TURN_LEFT ||
					LaraItem->currentAnimState == LS_MONKEYSWING_TURN_RIGHT))
				break;
			return;

		case TRIGGER_TYPES::TIGHTROPE_T:
			if (LaraItem->currentAnimState >= LS_TIGHTROPE_IDLE &&
				LaraItem->currentAnimState <= LS_TIGHTROPE_RECOVER_BALANCE &&
				LaraItem->currentAnimState != LS_DOVESWITCH)
				break;
			return;

		case TRIGGER_TYPES::CRAWLDUCK_T:
			if (LaraItem->currentAnimState == LS_DOVESWITCH ||
				LaraItem->currentAnimState == LS_CRAWL_IDLE ||
				LaraItem->currentAnimState == LS_CRAWL_TURN_LEFT ||
				LaraItem->currentAnimState == LS_CRAWL_TURN_RIGHT ||
				LaraItem->currentAnimState == LS_CRAWL_BACK ||
				LaraItem->currentAnimState == LS_CROUCH_IDLE ||
				LaraItem->currentAnimState == LS_CROUCH_ROLL ||
				LaraItem->currentAnimState == LS_CROUCH_TURN_LEFT ||
				LaraItem->currentAnimState == LS_CROUCH_TURN_RIGHT)
				break;
			return;

		case TRIGGER_TYPES::CLIMB_T:
			if (LaraItem->currentAnimState == LS_HANG ||
				LaraItem->currentAnimState == LS_LADDER_IDLE ||
				LaraItem->currentAnimState == LS_LADDER_UP ||
				LaraItem->currentAnimState == LS_LADDER_LEFT ||
				LaraItem->currentAnimState == LS_LADDER_STOP ||
				LaraItem->currentAnimState == LS_LADDER_RIGHT ||
				LaraItem->currentAnimState == LS_LADDER_DOWN ||
				LaraItem->currentAnimState == LS_MONKEYSWING_IDLE)
				break;
			return;

		case TRIGGER_TYPES::PAD:
		case TRIGGER_TYPES::ANTIPAD:
			if (LaraItem->pos.yPos == LaraItem->floor)
				break;
			return;

		case TRIGGER_TYPES::KEY:
			value = *(data++) & 0x3FF;
			keyResult = KeyTrigger(value);
			if (keyResult != -1)
				break;
			return;

		case TRIGGER_TYPES::PICKUP:
			value = *(data++) & 0x3FF;
			if (!PickupTrigger(value))
				return;
			break;

		case TRIGGER_TYPES::COMBAT:
			if (Lara.gunStatus == LG_READY)
				break;
			return;

		case TRIGGER_TYPES::HEAVY:
		case TRIGGER_TYPES::DUMMY:
		case TRIGGER_TYPES::HEAVYSWITCH:
		case TRIGGER_TYPES::HEAVYANTITRIGGER:
			return;

		default:
			break;
		}
	}

	short targetType = 0;
	short trigger = 0;

	ITEM_INFO* item = NULL;
	ITEM_INFO* cameraItem = NULL;

	do
	{
		trigger = *(data++);
		value = trigger & VALUE_BITS;
		targetType = (trigger >> 10) & 0xF;

		switch (targetType)
		{
		case TO_OBJECT:
			item = &g_Level.Items[value];

			if (keyResult >= 2 ||
				(triggerType == TRIGGER_TYPES::ANTIPAD ||
					triggerType == TRIGGER_TYPES::ANTITRIGGER ||
					triggerType == TRIGGER_TYPES::HEAVYANTITRIGGER) &&
				item->flags & ATONESHOT)
				break;

			if (triggerType == TRIGGER_TYPES::SWITCH)
			{
				if (item->flags & SWONESHOT)
					break;
				if (item->objectNumber == ID_DART_EMITTER && item->active)
					break;
			}

			item->timer = timer;
			if (timer != 1)
				item->timer = 30 * timer;

			if (triggerType == TRIGGER_TYPES::SWITCH ||
				triggerType == TRIGGER_TYPES::HEAVYSWITCH)
			{
				if (heavyFlags >= 0)
				{
					if (switchFlag)
						item->flags |= (flags & CODE_BITS);
					else
						item->flags ^= (flags & CODE_BITS);

					if (flags & ONESHOT)
						item->flags |= SWONESHOT;
				}
				else
				{
					if (((flags ^ item->flags) & CODE_BITS) == CODE_BITS)
					{
						item->flags ^= (flags & CODE_BITS);
						if (flags & ONESHOT)
							item->flags |= SWONESHOT;
					}
				}
			}
			else if (triggerType == TRIGGER_TYPES::ANTIPAD ||
				triggerType == TRIGGER_TYPES::ANTITRIGGER ||
				triggerType == TRIGGER_TYPES::HEAVYANTITRIGGER)
			{
				if (item->objectNumber == ID_EARTHQUAKE)
				{
					item->itemFlags[0] = 0;
					item->itemFlags[1] = 100;
				}

				item->flags &= ~(CODE_BITS | REVERSE);

				if (flags & ONESHOT)
					item->flags |= ATONESHOT;

				if (item->active && Objects[item->objectNumber].intelligent)
				{
					item->hitPoints = NOT_TARGETABLE;
					DisableBaddieAI(value);
					KillItem(value);
				}
			}
			else if (flags & CODE_BITS)
			{
				item->flags |= flags & CODE_BITS;
			}

			if ((item->flags & CODE_BITS) == CODE_BITS)
			{
				item->flags |= 0x20;

				if (flags & ONESHOT)
					item->flags |= ONESHOT;

				if (!(item->active) && !(item->flags & IFLAG_KILLED))
				{
					if (Objects[item->objectNumber].intelligent)
					{
						if (item->status != ITEM_NOT_ACTIVE)
						{
							if (item->status == ITEM_INVISIBLE)
							{
								item->touchBits = 0;
								if (EnableBaddieAI(value, 0))
								{
									item->status = ITEM_ACTIVE;
									AddActiveItem(value);
								}
								else
								{
									item->status == ITEM_INVISIBLE;
									AddActiveItem(value);
								}
							}
						}
						else
						{
							item->touchBits = 0;
							item->status = ITEM_ACTIVE;
							AddActiveItem(value);
							EnableBaddieAI(value, 1);
						}
					}
					else
					{
						item->touchBits = 0;
						AddActiveItem(value);
						item->status = ITEM_ACTIVE;
					}
				}
			}
			break;

		case TO_CAMERA:
			trigger = *(data++);

			if (keyResult == 1)
				break;

			if (g_Level.Cameras[value].flags & ONESHOT)
				break;

			Camera.number = value;

			if (Camera.type == LOOK_CAMERA || Camera.type == COMBAT_CAMERA && !(g_Level.Cameras[value].flags & 3))
				break;

			if (triggerType == TRIGGER_TYPES::COMBAT)
				break;

			if (triggerType == TRIGGER_TYPES::SWITCH && timer && switchOff)
				break;

			if (Camera.number != Camera.last || triggerType == TRIGGER_TYPES::SWITCH)
			{
				Camera.timer = (trigger & 0xFF) * 30;
				Camera.type = heavy ? HEAVY_CAMERA : FIXED_CAMERA;
				if (trigger & ONESHOT)
					g_Level.Cameras[Camera.number].flags |= ONESHOT;
			}
			break;

		case TO_FLYBY:
			trigger = *(data++);

			if (keyResult == 1)
				break;

			if (triggerType == TRIGGER_TYPES::ANTIPAD ||
				triggerType == TRIGGER_TYPES::ANTITRIGGER ||
				triggerType == TRIGGER_TYPES::HEAVYANTITRIGGER)
				UseSpotCam = false;
			else
			{
				spotCamIndex = 0;
				if (SpotCamRemap[value] != 0)
				{
					for (int i = 0; i < SpotCamRemap[value]; i++)
					{
						spotCamIndex += CameraCnt[i];
					}
				}

				if (!(SpotCam[spotCamIndex].flags & SCF_CAMERA_ONE_SHOT))
				{
					if (trigger & ONESHOT)
						SpotCam[spotCamIndex].flags |= SCF_CAMERA_ONE_SHOT;

					if (!UseSpotCam || CurrentLevel == 0)
					{
						UseSpotCam = true;
						if (LastSpotCam != value)
							TrackCameraInit = false;
						InitialiseSpotCam(value);
					}
				}
			}
			break;

		case TO_TARGET:
			cameraItem = &g_Level.Items[value];
			break;

		case TO_SINK:
			Lara.currentActive = value + 1;
			break;

		case TO_FLIPMAP:
			flipAvailable = true;

			if (FlipMap[value] & 0x100)
				break;

			if (triggerType == TRIGGER_TYPES::SWITCH)
				FlipMap[value] ^= (flags & CODE_BITS);
			else if (flags & CODE_BITS)
				FlipMap[value] |= (flags & CODE_BITS);

			if ((FlipMap[value] & CODE_BITS) == CODE_BITS)
			{

				if (flags & 0x100)
					FlipMap[value] |= 0x100;
				if (!FlipStats[value])
					flip = value;
			}
			else if (FlipStats[value])
				flip = value;
			break;

		case TO_FLIPON:
			flipAvailable = true;
			FlipMap[value] |= CODE_BITS;
			if (!FlipStats[value])
				flip = value;
			break;

		case TO_FLIPOFF:
			flipAvailable = true;
			FlipMap[value] &= ~CODE_BITS;
			if (FlipStats[value])
				flip = value;
			break;

		case TO_FLIPEFFECT:
			TriggerTimer = timer;
			newEffect = value;
			break;

		case TO_FINISH:
			RequiredStartPos = false;
			LevelComplete = CurrentLevel + 1;
			break;

		case TO_CD:
			PlaySoundTrack(value, flags);
			break;

		case TO_CUTSCENE:
			// TODO: not used for now
			break;

		default:
			break;
		}

	} while (!(trigger & END_BIT));

	if (cameraItem && (Camera.type == FIXED_CAMERA || Camera.type == HEAVY_CAMERA))
		Camera.item = cameraItem;

	if (flip != -1)
		DoFlipMap(flip);

	if (newEffect != -1 && (flip || !flipAvailable))
	{
		FlipEffect = newEffect;
		FlipTimer = 0;
	}
}

void TestTriggers(ITEM_INFO* item, bool heavy, int heavyFlags)
{
	TestTriggers(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, heavy, heavyFlags);
}

void TestTriggers(int x, int y, int z, short roomNumber, bool heavy, int heavyFlags)
{
	auto roomNum = roomNumber;
	auto floor = GetFloor(x, y, z, &roomNum);

	// Don't process legacy triggers if trigger triggerer wasn't used
	if (floor->Flags.MarkTriggerer && !floor->Flags.MarkTriggererActive)
		return;

	TestTriggers(GetTriggerIndex(floor, x, y, z), heavy, heavyFlags);
}

void ProcessSectorFlags(ITEM_INFO* item)
{
	ProcessSectorFlags(GetCollisionResult(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber).BottomBlock);
}

void ProcessSectorFlags(int x, int y, int z, short roomNumber)
{
	ProcessSectorFlags(GetCollisionResult(x, y, z, roomNumber).BottomBlock);
}

void ProcessSectorFlags(FLOOR_INFO* floor)
{
	// Monkeyswing
	Lara.canMonkeySwing = floor->Flags.Monkeyswing;

	// Burn Lara
	if (floor->Flags.Death && (LaraItem->pos.yPos == LaraItem->floor || Lara.waterStatus))
		LavaBurn(LaraItem);

	// Set climb status
	if ((1 << (GetQuadrant(LaraItem->pos.yRot) + 8)) & GetClimbFlags(floor))
		Lara.climbStatus = true;
	else
		Lara.climbStatus = false;
}