#include "framework.h"
#include "Game/control/trigger.h"

#include "Game/camera.h"
#include "Game/collision/floordata.h"
#include "Game/control/flipeffect.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/control/volume.h"
#include "Game/effects/item_fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_climb.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Game/room.h"
#include "Game/savegame.h"
#include "Game/Setup.h"
#include "Game/spotcam.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Objects/Generic/puzzles_keys.h"
#include "Objects/objectslist.h"
#include "Objects/TR3/Vehicles/kayak.h"
#include "Sound/sound.h"
#include "Specific/clock.h"

using namespace TEN::Effects::Items;
using namespace TEN::Entities::Switches;

int TriggerTimer;
int KeyTriggerActive;

int TriggerActive(ItemInfo* item)
{
	int flag = (~item->Flags & IFLAG_REVERSE) >> 14;
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

bool GetKeyTrigger(ItemInfo* item)
{
	short* triggerIndexPtr = GetTriggerIndex(item);
	if (triggerIndexPtr == nullptr)
		return false;

	short* triggerPtr = triggerIndexPtr;
	if (*triggerPtr & END_BIT)
		return false;

	for (short* j = &triggerPtr[2]; (*j >> 8) & 0x3C || item != &g_Level.Items[*j & VALUE_BITS]; j++)
	{
		if (*j & END_BIT)
			return false;
	}

	return true;
}

// NOTE: attatchedToSwitch parameter unused.
int GetSwitchTrigger(ItemInfo* item, short* itemNumbersPtr, int attatchedToSwitch)
{
	short* triggerIndexPtr = GetTriggerIndex(item);
	if (triggerIndexPtr == nullptr)
		return 0;

	short* trigger = triggerIndexPtr;
	if (*trigger & END_BIT)
		return 0;

	trigger += 2;
	short* currentPtr = itemNumbersPtr;
	int k = 0;

	do
	{
		if (TRIG_BITS(*trigger) == TO_OBJECT && item != &g_Level.Items[*trigger & VALUE_BITS])
		{
			currentPtr[k] = *trigger & VALUE_BITS;
			++k;
		}

		if (*trigger & END_BIT)
			break;

		++trigger;

	} while (true);

	return k;
}

int SwitchTrigger(short itemNumber, short timer)
{
	auto& item = g_Level.Items[itemNumber];
	const auto& player = Lara;

	// Handle reusable receptacles.
	if (item.ObjectNumber >= ID_PUZZLE_DONE1 && item.ObjectNumber <= ID_PUZZLE_DONE16 &&
		item.ItemFlags[1] != 0)
	{
		item.Flags |= IFLAG_ACTIVATION_MASK;
		item.Status = ITEM_ACTIVE;
		item.ItemFlags[1] = false;
	
		return 1;
	}

	if (item.ObjectNumber >= ID_PUZZLE_HOLE1 && item.ObjectNumber <= ID_PUZZLE_HOLE16 &&
		item.ItemFlags[1] != 0)
	{
		item.Flags |= IFLAG_ACTIVATION_MASK;
		item.Status = ITEM_DEACTIVATED;
		item.ItemFlags[1] = false;
	
		return 1;
	}

	if ((item.ObjectNumber >= ID_PUZZLE_DONE1 && item.ObjectNumber <= ID_PUZZLE_DONE16) ||
		(item.ObjectNumber >= ID_PUZZLE_HOLE1 && item.ObjectNumber <= ID_PUZZLE_HOLE16))
	{
		return 0;
	}

	// Handle reusable receptacles.
	if (item.ObjectNumber >= ID_KEY_HOLE1 && item.ObjectNumber <= ID_KEY_HOLE16 &&
		item.ItemFlags[1] != 0 &&
		(item.ItemFlags[5] == (int)ReusableReceptacleState::Empty || item.ItemFlags[5] == (int)ReusableReceptacleState::None) &&
		player.Control.HandStatus != HandStatus::Busy)
	{
		item.Flags |= IFLAG_ACTIVATION_MASK;
		item.Status = ITEM_ACTIVE;
		item.ItemFlags[5] = (int)ReusableReceptacleState::Done;
		item.ItemFlags[1] = false;
		return 1;
	}

	if (item.ObjectNumber >= ID_KEY_HOLE1 && item.ObjectNumber <= ID_KEY_HOLE16 && 
		item.ItemFlags[1] != 0 && item.ItemFlags[5] == (int)ReusableReceptacleState::Done &&
		player.Control.HandStatus != HandStatus::Busy)
	{
		item.Flags |= IFLAG_ACTIVATION_MASK;
		item.Status = ITEM_DEACTIVATED;
		item.ItemFlags[5] = (int)ReusableReceptacleState::Empty;
		item.ItemFlags[1] = false;
		return 1;
	}

	if (item.ObjectNumber >= ID_KEY_HOLE1 && item.ObjectNumber <= ID_KEY_HOLE16)
		return 0;

	// Handle switches.
	if (item.Status == ITEM_DEACTIVATED)
	{
		if (((item.Animation.ActiveState == SWITCH_OFF && item.ObjectNumber != ID_JUMP_SWITCH) ||
			 (item.Animation.ActiveState == SWITCH_ON && item.ObjectNumber == ID_JUMP_SWITCH)) &&
			timer > 0)
		{
			item.Timer = timer;
			item.Status = ITEM_ACTIVE;

			if (timer != 1)
				item.Timer = FPS * timer;

			return 1;
		}
	
		if (item.TriggerFlags >= 0 || item.Animation.ActiveState != SWITCH_OFF)
		{
			RemoveActiveItem(itemNumber);

			item.Status = ITEM_NOT_ACTIVE;
			if (!item.ItemFlags[0] == 0)
				item.Flags |= ONESHOT;

			return 1;
		}
		else
		{
			item.Status = ITEM_ACTIVE;
			return 1;
		}
	}
	else if (item.Status != ITEM_NOT_ACTIVE)
	{
		if (item.ObjectNumber == ID_AIRLOCK_SWITCH &&
			item.Animation.AnimNumber == GetAnimIndex(item, 2) &&
			item.Animation.FrameNumber == GetFrameIndex(&item, 0))
		{
			return 1;
		}

		return ((item.Flags & ONESHOT) >> 8);
	}

	return 0;
}

int KeyTrigger(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	const auto& player = Lara;

	if ((item->Status != ITEM_ACTIVE || player.Control.HandStatus == HandStatus::Busy) &&
		(!KeyTriggerActive || player.Control.HandStatus != HandStatus::Busy))
	{
		return -1;
	}

	int oldkey = KeyTriggerActive;

	if (!oldkey)
		item->Status = ITEM_DEACTIVATED;

	KeyTriggerActive = false;

	return oldkey;
}

bool PickupTrigger(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (((item->Flags & IFLAG_CLEAR_BODY) && (item->Flags & IFLAG_KILLED)) ||
		item->Status != ITEM_INVISIBLE || 
		item->ItemFlags[3] != 1 || 
		item->TriggerFlags & 0x80)
	{
		return false;
	}

	KillItem(itemNumber);
	item->Flags |= IFLAG_CLEAR_BODY;

	return true;
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
			Camera.item = nullptr;

	if (Camera.number == -1 && Camera.timer > 0)
		Camera.timer = -1;
}

short* GetTriggerIndex(FloorInfo* floor, int x, int y, int z)
{
	auto bottomBlock = GetCollision(x, y, z, floor->RoomNumber).BottomBlock; 

	if (bottomBlock->TriggerIndex == -1)
		return nullptr;

	return &g_Level.FloorData[bottomBlock->TriggerIndex];
}

short* GetTriggerIndex(ItemInfo* item)
{
	auto roomNumber = item->RoomNumber;
	auto floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
	return GetTriggerIndex(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
}

void Antitrigger(short const value, short const flags)
{
	ItemInfo* item = &g_Level.Items[value];
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
		DisableEntityAI(value);
		RemoveActiveItem(value, false);
		item->Active = false;
		item->Status = ITEM_INVISIBLE;
	}
}

void Trigger(short const value, short const flags)
{
	ItemInfo* item = &g_Level.Items[value];
	item->Flags |= TRIGGERED;

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
					item->TouchBits = NO_JOINT_BITS;
					if (EnableEntityAI(value, false))
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
				item->TouchBits = NO_JOINT_BITS;
				item->Status = ITEM_ACTIVE;
				AddActiveItem(value);
				EnableEntityAI(value, true);
			}
		}
		else
		{
			item->TouchBits = NO_JOINT_BITS;
			AddActiveItem(value);
			item->Status = ITEM_ACTIVE;
		}
	}
}

void TestTriggers(int x, int y, int z, FloorInfo* floor, Activator activator, bool heavy, int heavyFlags)
{
	int flip = -1;
	int flipAvailable = 0;
	int newEffect = -1;
	int switchOff = 0;
	//int switchFlag = 0;
	short objectNumber = 0;
	int keyResult = 0;
	short cameraFlags = 0;
	short cameraTimer = 0;
	int spotCamIndex = 0;

	auto data = GetTriggerIndex(floor, x, y, z);

	if (!data)
		return;

	short triggerType = (*(data++) >> 8) & TRIGGER_BITS;
	short flags = *(data++);
	short timer = flags & TIMER_BITS;

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
			value = *(data++) & VALUE_BITS;

			if (flags & ONESHOT)
				g_Level.Items[value].ItemFlags[0] = 1;

			if (!SwitchTrigger(value, timer))
				return;

			objectNumber = g_Level.Items[value].ObjectNumber;
			//This disables the antitrigger of the Valve switch (ocb 5). I don't know the purpose of this in TR4.
			//if (objectNumber >= ID_SWITCH_TYPE1 && objectNumber <= ID_SWITCH_TYPE6 && g_Level.Items[value].TriggerFlags == 5)
				//switchFlag = 1;

			switchOff = (g_Level.Items[value].Animation.ActiveState == 1);

			break;

		case TRIGGER_TYPES::MONKEY:
			if (LaraItem->Animation.ActiveState >= LS_MONKEY_IDLE &&
				(LaraItem->Animation.ActiveState <= LS_MONKEY_TURN_180 ||
				 LaraItem->Animation.ActiveState == LS_MONKEY_TURN_LEFT ||
				 LaraItem->Animation.ActiveState == LS_MONKEY_TURN_RIGHT))
				break;
			return;

		case TRIGGER_TYPES::TIGHTROPE_T:
			if (LaraItem->Animation.ActiveState >= LS_TIGHTROPE_IDLE &&
				LaraItem->Animation.ActiveState <= LS_TIGHTROPE_RECOVER_BALANCE &&
				LaraItem->Animation.ActiveState != LS_DOVE_SWITCH)
				break;
			return;

		case TRIGGER_TYPES::CRAWLDUCK_T:
			if (LaraItem->Animation.ActiveState == LS_DOVE_SWITCH ||
				LaraItem->Animation.ActiveState == LS_CRAWL_IDLE ||
				LaraItem->Animation.ActiveState == LS_CRAWL_TURN_LEFT ||
				LaraItem->Animation.ActiveState == LS_CRAWL_TURN_RIGHT ||
				LaraItem->Animation.ActiveState == LS_CRAWL_BACK ||
				LaraItem->Animation.ActiveState == LS_CROUCH_IDLE ||
				LaraItem->Animation.ActiveState == LS_CROUCH_ROLL ||
				LaraItem->Animation.ActiveState == LS_CROUCH_TURN_LEFT ||
				LaraItem->Animation.ActiveState == LS_CROUCH_TURN_RIGHT)
				break;
			return;

		case TRIGGER_TYPES::CLIMB_T:
			if (LaraItem->Animation.ActiveState == LS_HANG ||
				LaraItem->Animation.ActiveState == LS_LADDER_IDLE ||
				LaraItem->Animation.ActiveState == LS_LADDER_UP ||
				LaraItem->Animation.ActiveState == LS_LADDER_LEFT ||
				LaraItem->Animation.ActiveState == LS_LADDER_STOP ||
				LaraItem->Animation.ActiveState == LS_LADDER_RIGHT ||
				LaraItem->Animation.ActiveState == LS_LADDER_DOWN)
				break;
			return;

		case TRIGGER_TYPES::PAD:
		case TRIGGER_TYPES::ANTIPAD:
			if (GetCollision(floor, x, y, z).Position.Floor == y)
				break;
			return;

		case TRIGGER_TYPES::KEY:
			value = *(data++) & VALUE_BITS;
			keyResult = KeyTrigger(value);
			if (keyResult != -1)
				break;
			return;

		case TRIGGER_TYPES::PICKUP:
			value = *(data++) & VALUE_BITS;
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

	ItemInfo* item = nullptr;
	ItemInfo* cameraItem = nullptr;

	do
	{
		trigger = *(data++);
		value = trigger & VALUE_BITS;
		targetType = (trigger >> 10) & FUNCTION_BITS;

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
				item->Timer = FPS * timer;

			if (triggerType == TRIGGER_TYPES::SWITCH ||
				triggerType == TRIGGER_TYPES::HEAVYSWITCH)
			{
				if (heavyFlags >= 0)
				{
					//if (switchFlag)
						//item->Flags |= (flags & CODE_BITS);
					//else

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
				Antitrigger(value, flags);
			}
			else if (flags & CODE_BITS)
			{
				item->Flags |= flags & CODE_BITS;
			}

			if ((item->Flags & CODE_BITS) == CODE_BITS)
			{
				Trigger(value, flags);
			}
			break;

		case TO_CAMERA:
			trigger = *(data++);

			if (keyResult == 1)
				break;

			if (g_Level.Cameras[value].Flags & ONESHOT)
				break;

			Camera.number = value;

			if (Camera.type == CameraType::Look || Camera.type == CameraType::Combat && !(g_Level.Cameras[value].Flags & 3))
				break;

			if (triggerType == TRIGGER_TYPES::COMBAT)
				break;

			if (triggerType == TRIGGER_TYPES::SWITCH && timer && switchOff)
				break;

			if (Camera.number != Camera.last || triggerType == TRIGGER_TYPES::SWITCH)
			{
				Camera.timer = (trigger & TIMER_BITS) * FPS;
				Camera.type = heavy ? CameraType::Heavy : CameraType::Fixed;
				if (trigger & ONESHOT)
					g_Level.Cameras[Camera.number].Flags |= ONESHOT;
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
						InitializeSpotCam(value);
					}
				}
			}
			break;

		case TO_TARGET:
			cameraItem = &g_Level.Items[value];
			break;

		case TO_SINK:
			Lara.Context.WaterCurrentActive = value + 1;
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
			if ((FlipMap[value] & CODE_BITS) == CODE_BITS && !FlipStats[value])
				flip = value;
			break;

		case TO_FLIPOFF:
			flipAvailable = true;
			if ((FlipMap[value] & CODE_BITS) == CODE_BITS && FlipStats[value])
				flip = value;
			break;

		case TO_FLIPEFFECT:
			TriggerTimer = timer;
			newEffect = value;
			break;

		case TO_FINISH:
			NextLevel = value ? value : CurrentLevel + 1;
			RequiredStartPos = timer;
			break;

		case TO_CD:
			PlaySoundTrack(value, flags);
			break;

		case TO_CUTSCENE:
			// TODO: not used for now
			break;

		case TO_SECRET:
			if (!(Statistics.Level.Secrets & (1 << value)))
			{
				PlaySecretTrack();
				Statistics.Level.Secrets |= (1 << value);
				Statistics.Game.Secrets++;
			}
			break;

		case TO_VOLUMEEVENT:
		case TO_GLOBALEVENT:
			trigger = *(data++);
			{
				auto& list = targetType == TO_VOLUMEEVENT ? g_Level.VolumeEventSets : g_Level.GlobalEventSets;

				if (list.size() <= value)
					continue;

				auto& set = list[value];

				if (targetType == TO_VOLUMEEVENT)
				{
					auto activatorType = heavy ? (int)ActivatorFlags::Flyby |
												 (int)ActivatorFlags::Moveable |
												 (int)ActivatorFlags::NPC : (int)ActivatorFlags::Player;

					if (!((int)set.Activators & activatorType))
						continue;
				}

				int eventType = trigger & TIMER_BITS;
				if (eventType >= (int)EventType::Count)
				{
					TENLog("Unknown volume event type encountered for legacy trigger " + std::to_string(eventType), LogLevel::Warning);
					continue;
				}

				HandleEvent(set.Events[eventType], activator);
			}
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

void TestTriggers(ItemInfo* item, bool isHeavy, int heavyFlags)
{
	auto roomNumber = item->RoomNumber;
	auto floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);

	TestTriggers(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, floor, item->Index, isHeavy, heavyFlags);
}

void TestTriggers(int x, int y, int z, short roomNumber, bool heavy, int heavyFlags)
{
	auto roomNum = roomNumber;
	auto floor = GetFloor(x, y, z, &roomNum);

	// Don't process legacy triggers if trigger triggerer wasn't used
	if (floor->Flags.MarkTriggerer && !floor->Flags.MarkTriggererActive)
		return;

	TestTriggers(x, y, z, floor, nullptr, heavy, heavyFlags);
}

void ProcessSectorFlags(ItemInfo* item)
{
	auto pointColl = GetCollision(item);
	auto* sectorPtr = GetCollision(item).BottomBlock;

	bool isPlayer = item->IsLara();

	// Monkeyswing and climb (only for Lara)
	if (isPlayer)
	{
		auto* lara = GetLaraInfo(item);

		// Set climb status
		if (TestLaraNearClimbableWall(item, sectorPtr))
			lara->Control.CanClimbLadder = true;
		else
			lara->Control.CanClimbLadder = false;

		// Set monkeyswing status
		lara->Control.CanMonkeySwing = sectorPtr->Flags.Monkeyswing;
	}

	// Burn or drown item
	if (sectorPtr->Flags.Death && item->Pose.Position.y == item->Floor)
	{
		if (isPlayer)
		{
			if (!IsJumpState((LaraState)item->Animation.ActiveState) || 
				GetLaraInfo(item)->Control.WaterStatus != WaterStatus::Dry)
			{
				// To allow both lava and rapids in same level, also check floor material flag.
				if (sectorPtr->GetSurfaceMaterial(pointColl.Coordinates.x, pointColl.Coordinates.z, true) == MaterialType::Water &&
					Objects[ID_KAYAK_LARA_ANIMS].loaded)
				{
					KayakLaraRapidsDrown(item);
				}
				else
				{
					LavaBurn(item);
				}
			}
		}
		else if (Objects[item->ObjectNumber].intelligent && item->HitPoints != NOT_TARGETABLE)
		{
			if (sectorPtr->GetSurfaceMaterial(pointColl.Coordinates.x, pointColl.Coordinates.z, true) == MaterialType::Water ||
				TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, sectorPtr->RoomNumber))
			{
				DoDamage(item, INT_MAX); // TODO: Implement correct rapids behaviour for other objects!
			}
			else
			{
				ItemBurn(item);
			}
		}
	}
}
