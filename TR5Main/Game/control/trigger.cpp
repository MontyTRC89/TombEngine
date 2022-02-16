#include "framework.h"
#include "Game/control/trigger.h"

#include "Game/camera.h"
#include "Game/collision/floordata.h"
#include "Game/control/flipeffect.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/lara_fx.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_climb.h"
#include "Game/Lara/lara_tests.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Game/spotcam.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Objects/objectslist.h"
#include "Sound/sound.h"
#include "Specific/setup.h"


using namespace TEN::Effects::Lara;
using namespace TEN::Entities::Switches;

int TriggerTimer;
int KeyTriggerActive;

int TriggerActive(ITEM_INFO* item)
{
	int flag;

	flag = (~item->Flags & IFLAG_REVERSE) >> 14;
	if ((item->Flags & IFLAG_ACTIVATION_MASK) != IFLAG_ACTIVATION_MASK)
	{
		flag = !flag;
	}
	else
	{
		if (item->Timer)
		{
			if (item->Timer > 0)
			{
				--item->Timer;
				if (!item->Timer)
					item->Timer = -1;
			}
			else if (item->Timer < -1)
			{
				++item->Timer;
				if (item->Timer == -1)
					item->Timer = 0;
			}
			if (item->Timer <= -1)
				flag = !flag;
		}
	}
	return flag;
}

bool GetKeyTrigger(ITEM_INFO* item)
{
	auto triggerIndex = GetTriggerIndex(item);

	if (triggerIndex == 0)
		return false;

	short* trigger = triggerIndex;

	if (*trigger & END_BIT)
		return false;

	for (short* j = &trigger[2]; (*j >> 8) & 0x3C || item != &g_Level.Items[*j & VALUE_BITS]; j++)
	{
		if (*j & END_BIT)
			return false;
	}

	return true;
}

int GetSwitchTrigger(ITEM_INFO* item, short* itemNos, int attatchedToSwitch)
{
	auto triggerIndex = GetTriggerIndex(item);

	if (triggerIndex == 0)
		return 0;

	short* trigger = triggerIndex;

	if (*trigger & END_BIT)
		return 0;

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

	return 0;
}

int SwitchTrigger(short itemNum, short timer)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	if (item->Status == ITEM_DEACTIVATED)
	{
		if ((!item->ActiveState && item->ObjectNumber != ID_JUMP_SWITCH || item->ActiveState == 1 && item->ObjectNumber == ID_JUMP_SWITCH) && timer > 0)
		{
			item->Timer = timer;
			item->Status = ITEM_ACTIVE;
			if (timer != 1)
				item->Timer = 30 * timer;
			return 1;
		}
		if (item->TriggerFlags != 6 || item->ActiveState)
		{
			RemoveActiveItem(itemNum);

			item->Status = ITEM_NOT_ACTIVE;
			if (!item->ItemFlags[0] == 0)
				item->Flags |= ONESHOT;
			if (item->ActiveState != 1)
				return 1;
			if (item->TriggerFlags != 5 && item->TriggerFlags != 6)
				return 1;
		}
		else
		{
			item->Status = ITEM_ACTIVE;
			return 1;
		}
	}
	else if (item->Status)
	{
		return (item->Flags & ONESHOT) >> 8;
	}
	else
	{
		return 0;
	}

	return 0;
}

int KeyTrigger(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	int oldkey;

	if ((item->Status != ITEM_ACTIVE || Lara.Control.HandStatus == HandStatus::Busy) && (!KeyTriggerActive || Lara.Control.HandStatus != HandStatus::Busy))
		return -1;

	oldkey = KeyTriggerActive;

	if (!oldkey)
		item->Status = ITEM_DEACTIVATED;

	KeyTriggerActive = false;

	return oldkey;
}

int PickupTrigger(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (item->Flags & IFLAG_KILLED
		|| (item->Status != ITEM_INVISIBLE
			|| item->ItemFlags[3] != 1
			|| item->TriggerFlags & 0x80))
	{
		return 0;
	}

	KillItem(itemNum);

	return 1;
}

void RefreshCamera(short type, short* data)
{
	short trigger, value, targetOk;

	targetOk = 2;

	do
	{
		trigger = *(data++);
		value = trigger & VALUE_BITS;

		switch (TRIG_BITS(trigger))
		{
		case TO_CAMERA:
			data++;

			if (value == Camera.last)
			{
				Camera.number = value;

				if ((Camera.timer < 0) || (Camera.type == CameraType::Look) || (Camera.type == CameraType::Combat))
				{
					Camera.timer = -1;
					targetOk = 0;
					break;
				}
				Camera.type = CameraType::Fixed;
				targetOk = 1;
			}
			else
				targetOk = 0;
			break;

		case TO_TARGET:
			if (Camera.type == CameraType::Look || Camera.type == CameraType::Combat)
				break;

			Camera.item = &g_Level.Items[value];
			break;
		}
	} while (!(trigger & END_BIT));

	if (Camera.item)
		if (!targetOk || (targetOk == 2 && Camera.item->LookedAt && Camera.item != Camera.lastItem))
			Camera.item = NULL;

	if (Camera.number == -1 && Camera.timer > 0)
		Camera.timer = -1;
}

short* GetTriggerIndex(FLOOR_INFO* floor, int x, int y, int z)
{
	auto bottomBlock = GetCollisionResult(x, y, z, floor->Room).BottomBlock; 

	if (bottomBlock->TriggerIndex == -1)
		return nullptr;

	return &g_Level.FloorData[bottomBlock->TriggerIndex];
}

short* GetTriggerIndex(ITEM_INFO* item)
{
	auto roomNumber = item->RoomNumber;
	auto floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
	return GetTriggerIndex(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
}

void TestTriggers(FLOOR_INFO* floor, int x, int y, int z, bool heavy, int heavyFlags)
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

	auto data = GetTriggerIndex(floor, x, y, z);

	if (!data)
		return;

	short triggerType = (*(data++) >> 8) & 0x3F;
	short flags = *(data++);
	short timer = flags & 0xFF;

	if (Camera.type != CameraType::Heavy)
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
				g_Level.Items[value].ItemFlags[0] = 1;

			if (!SwitchTrigger(value, timer))
				return;

			objectNumber = g_Level.Items[value].ObjectNumber;
			if (objectNumber >= ID_SWITCH_TYPE1 && objectNumber <= ID_SWITCH_TYPE6 && g_Level.Items[value].TriggerFlags == 5)
				switchFlag = 1;

			switchOff = (g_Level.Items[value].ActiveState == 1);

			break;

		case TRIGGER_TYPES::MONKEY:
			if (LaraItem->ActiveState >= LS_MONKEY_IDLE &&
				(LaraItem->ActiveState <= LS_MONKEY_TURN_180 ||
					LaraItem->ActiveState == LS_MONKEY_TURN_LEFT ||
					LaraItem->ActiveState == LS_MONKEY_TURN_RIGHT))
				break;
			return;

		case TRIGGER_TYPES::TIGHTROPE_T:
			if (LaraItem->ActiveState >= LS_TIGHTROPE_IDLE &&
				LaraItem->ActiveState <= LS_TIGHTROPE_RECOVER_BALANCE &&
				LaraItem->ActiveState != LS_DOVESWITCH)
				break;
			return;

		case TRIGGER_TYPES::CRAWLDUCK_T:
			if (LaraItem->ActiveState == LS_DOVESWITCH ||
				LaraItem->ActiveState == LS_CRAWL_IDLE ||
				LaraItem->ActiveState == LS_CRAWL_TURN_LEFT ||
				LaraItem->ActiveState == LS_CRAWL_TURN_RIGHT ||
				LaraItem->ActiveState == LS_CRAWL_BACK ||
				LaraItem->ActiveState == LS_CROUCH_IDLE ||
				LaraItem->ActiveState == LS_CROUCH_ROLL ||
				LaraItem->ActiveState == LS_CROUCH_TURN_LEFT ||
				LaraItem->ActiveState == LS_CROUCH_TURN_RIGHT)
				break;
			return;

		case TRIGGER_TYPES::CLIMB_T:
			if (LaraItem->ActiveState == LS_HANG ||
				LaraItem->ActiveState == LS_LADDER_IDLE ||
				LaraItem->ActiveState == LS_LADDER_UP ||
				LaraItem->ActiveState == LS_LADDER_LEFT ||
				LaraItem->ActiveState == LS_LADDER_STOP ||
				LaraItem->ActiveState == LS_LADDER_RIGHT ||
				LaraItem->ActiveState == LS_LADDER_DOWN)
				break;
			return;

		case TRIGGER_TYPES::PAD:
		case TRIGGER_TYPES::ANTIPAD:
			if (GetCollisionResult(floor, x, y, z).Position.Floor == y)
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
			if (Lara.Control.HandStatus == HandStatus::WeaponReady)
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
				item->Flags & ATONESHOT)
				break;

			if (triggerType == TRIGGER_TYPES::SWITCH && item->Flags & SWONESHOT)
				break;

			if (triggerType != TRIGGER_TYPES::SWITCH
				&& triggerType != TRIGGER_TYPES::ANTIPAD
				&& triggerType != TRIGGER_TYPES::ANTITRIGGER
				&& triggerType != TRIGGER_TYPES::HEAVYANTITRIGGER
				&& (item->Flags & ONESHOT))
				break;

			if (triggerType != TRIGGER_TYPES::ANTIPAD 
				&& triggerType != TRIGGER_TYPES::ANTITRIGGER 
				&& triggerType != TRIGGER_TYPES::HEAVYANTITRIGGER)
			{
				if (item->ObjectNumber == ID_DART_EMITTER && item->Active)
					break;
			}

			item->Timer = timer;
			if (timer != 1)
				item->Timer = 30 * timer;

			if (triggerType == TRIGGER_TYPES::SWITCH ||
				triggerType == TRIGGER_TYPES::HEAVYSWITCH)
			{
				if (heavyFlags >= 0)
				{
					if (switchFlag)
						item->Flags |= (flags & CODE_BITS);
					else
						item->Flags ^= (flags & CODE_BITS);

					if (flags & ONESHOT)
						item->Flags |= SWONESHOT;
				}
				else
				{
					if (((flags ^ item->Flags) & CODE_BITS) == CODE_BITS)
					{
						item->Flags ^= (flags & CODE_BITS);
						if (flags & ONESHOT)
							item->Flags |= SWONESHOT;
					}
				}
			}
			else if (triggerType == TRIGGER_TYPES::ANTIPAD ||
				triggerType == TRIGGER_TYPES::ANTITRIGGER ||
				triggerType == TRIGGER_TYPES::HEAVYANTITRIGGER)
			{
				if (item->ObjectNumber == ID_EARTHQUAKE)
				{
					item->ItemFlags[0] = 0;
					item->ItemFlags[1] = 100;
				}

				item->Flags &= ~(CODE_BITS | REVERSE);

				if (flags & ONESHOT)
					item->Flags |= ATONESHOT;

				if (item->Active && Objects[item->ObjectNumber].intelligent)
				{
					item->HitPoints = NOT_TARGETABLE;
					DisableBaddieAI(value);
					KillItem(value);
				}
			}
			else if (flags & CODE_BITS)
			{
				item->Flags |= flags & CODE_BITS;
			}

			if ((item->Flags & CODE_BITS) == CODE_BITS)
			{
				item->Flags |= 0x20;

				if (flags & ONESHOT)
					item->Flags |= ONESHOT;

				if (!(item->Active) && !(item->Flags & IFLAG_KILLED))
				{
					if (Objects[item->ObjectNumber].intelligent)
					{
						if (item->Status != ITEM_NOT_ACTIVE)
						{
							if (item->Status == ITEM_INVISIBLE)
							{
								item->TouchBits = 0;
								if (EnableBaddieAI(value, 0))
								{
									item->Status = ITEM_ACTIVE;
									AddActiveItem(value);
								}
								else
								{
									item->Status = ITEM_INVISIBLE;
									AddActiveItem(value);
								}
							}
						}
						else
						{
							item->TouchBits = 0;
							item->Status = ITEM_ACTIVE;
							AddActiveItem(value);
							EnableBaddieAI(value, 1);
						}
					}
					else
					{
						item->TouchBits = 0;
						AddActiveItem(value);
						item->Status = ITEM_ACTIVE;
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

			if (Camera.type == CameraType::Look || Camera.type == CameraType::Combat && !(g_Level.Cameras[value].flags & 3))
				break;

			if (triggerType == TRIGGER_TYPES::COMBAT)
				break;

			if (triggerType == TRIGGER_TYPES::SWITCH && timer && switchOff)
				break;

			if (Camera.number != Camera.last || triggerType == TRIGGER_TYPES::SWITCH)
			{
				Camera.timer = (trigger & 0xFF) * 30;
				Camera.type = heavy ? CameraType::Heavy : CameraType::Fixed;
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
						if (LastSpotCamSequence != value)
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
			Lara.Control.WaterCurrentActive = value + 1;
			break;

		case TO_FLIPMAP:
			flipAvailable = true;

			if (FlipMap[value] & ONESHOT)
				break;

			if (triggerType == TRIGGER_TYPES::SWITCH)
				FlipMap[value] ^= (flags & CODE_BITS);
			else if (flags & CODE_BITS)
				FlipMap[value] |= (flags & CODE_BITS);

			if ((FlipMap[value] & CODE_BITS) == CODE_BITS)
			{
				if (flags & ONESHOT)
					FlipMap[value] |= ONESHOT;
				if (!FlipStats[value])
					flip = value;
			}
			else if (FlipStats[value])
				flip = value;
			break;

		case TO_FLIPON:
			flipAvailable = true;
			if ((FlipMap[value] & 0x3E00) == 0x3E00 && !FlipStats[value])
				flip = value;
			break;

		case TO_FLIPOFF:
			flipAvailable = true;
			if ((FlipMap[value] & 0x3E00) == 0x3E00 && FlipStats[value])
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

	if (cameraItem && (Camera.type == CameraType::Fixed || Camera.type == CameraType::Heavy))
		Camera.item = cameraItem;

	if (flip != -1)
		DoFlipMap(flip);

	if (newEffect != -1 && (flip || !flipAvailable))
		FlipEffect = newEffect;
}

void TestTriggers(ITEM_INFO* item, bool heavy, int heavyFlags)
{
	TestTriggers(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber, heavy, heavyFlags);
}

void TestTriggers(int x, int y, int z, short roomNumber, bool heavy, int heavyFlags)
{
	auto roomNum = roomNumber;
	auto floor = GetFloor(x, y, z, &roomNum);

	// Don't process legacy triggers if trigger triggerer wasn't used
	if (floor->Flags.MarkTriggerer && !floor->Flags.MarkTriggererActive)
		return;

	TestTriggers(floor, x, y, z, heavy, heavyFlags);
}

void ProcessSectorFlags(ITEM_INFO* item)
{
	ProcessSectorFlags(GetCollisionResult(item).BottomBlock);
}

void ProcessSectorFlags(int x, int y, int z, short roomNumber)
{
	ProcessSectorFlags(GetCollisionResult(x, y, z, roomNumber).BottomBlock);
}

void ProcessSectorFlags(FLOOR_INFO* floor)
{
	// Monkeyswing
	Lara.Control.CanMonkeySwing = floor->Flags.Monkeyswing;

	// Burn Lara
	if (floor->Flags.Death &&
		(LaraItem->Position.yPos == LaraItem->Floor && !IsJumpState((LaraState)LaraItem->ActiveState) ||
			Lara.Control.WaterStatus != WaterStatus::Dry))
	{
		LavaBurn(LaraItem);
	}

	// Set climb status
	if ((1 << (GetQuadrant(LaraItem->Position.yRot) + 8)) & GetClimbFlags(floor))
		Lara.Control.CanClimbLadder = true;
	else
		Lara.Control.CanClimbLadder = false;
}
