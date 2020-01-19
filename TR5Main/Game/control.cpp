#include "control.h"

#include "..\Global\global.h"
 
#include "pickup.h"
#include "spotcam.h"
#include "Camera.h"
#include "Lara.h"
#include "hair.h"
#include "items.h"
#include "effect2.h"
#include "draw.h"
#include "inventory.h"
#include "gameflow.h"
#include "lot.h"
#include "pickup.h"
#include "draw.h"
#include "healt.h"
#include "savegame.h"
#include "sound.h"
#include "spotcam.h"
#include "box.h"
#include "objects.h"
#include "switch.h"
#include "laramisc.h"
#include "rope.h"
#include "tomb4fx.h"
#include "traps.h"
#include "effects.h"
#include "sphere.h"
#include "debris.h"
#include "larafire.h"

#include "footprint.h"
#include "..\Specific\roomload.h"
#include "..\Specific\input.h"
#include "..\Specific\init.h"
#include "..\Specific\winmain.h"

#include <process.h>
#include <stdio.h>

int KeyTriggerActive;
PENDULUM CurrentPendulum;
int number_los_rooms;
short los_rooms[20];
int ClosestItem;
int ClosestDist;
PHD_VECTOR ClosestCoord;
int rand_1 = -747505337;
int rand_2 = -747505337;
int RumbleTimer = 0;
int InGameCnt = 0;
byte IsAtmospherePlaying = 0;
byte FlipStatus = 0;
int FlipStats[255];
int FlipMap[255];
bool InItemControlLoop;
short ItemNewRoomNo;
short ItemNewRooms[512];
short NextFxActive;
short NextFxFree;
short NextItemActive;
short NextItemFree;

#define _NormalizeVector ((PHD_VECTOR* (__cdecl*)(PHD_VECTOR*)) 0x0046DE10)

extern GameFlow* g_GameFlow;
extern GameScript* g_GameScript;
extern Inventory* g_Inventory;
extern int SplashCount;
extern void(*effect_routines[59])(ITEM_INFO* item);
extern short FXType;
extern vector<AudioTrack> g_AudioTracks;
extern std::deque<FOOTPRINT_STRUCT> footprints;

GAME_STATUS ControlPhase(int numFrames, int demoMode)
{
	GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

	RegeneratePickups();

	if (numFrames > 10)
		numFrames = 10;

	if (TrackCameraInit)
		UseSpotCam = false;

	SetDebounce = true;

	for (FramesCount += numFrames; FramesCount > 0; FramesCount -= 2)
	{
		GlobalCounter++;

		UpdateSky();
		
		// Poll the keyboard and update input variables
		if (CurrentLevel != 0)
		{
			if (S_UpdateInput() == -1)
				return GAME_STATUS_NONE;
		}

		// Has Lara control been disabled?
		if (DisableLaraControl || CurrentLevel == 0)
		{
			if (CurrentLevel != 0)
				DbInput = 0;
			TrInput &= 0x200;
		}

		// If cutscene has been triggered then clear input
		if (CutSeqTriggered)
			TrInput = 0;

		// Does the player want to enter inventory?
		SetDebounce = false;
		if (CurrentLevel != 0 && !g_Renderer->IsFading())
		{
			if ((DbInput & IN_DESELECT || g_Inventory->GetEnterObject() != NO_ITEM) && !CutSeqTriggered && LaraItem->hitPoints > 0)
			{ 
				// Stop all sounds
				SOUND_Stop();
				int inventoryResult = g_Inventory->DoInventory();
				switch (inventoryResult)
				{
				case INV_RESULT_LOAD_GAME:
					return GAME_STATUS_LOAD_GAME;
				case INV_RESULT_EXIT_TO_TILE:
					return GAME_STATUS_EXIT_TO_TITLE;
				}
			}
		}

		// Has level been completed?
		if (CurrentLevel != 0 && LevelComplete)
			return GAME_STATUS_LEVEL_COMPLETED;

		int oldInput = TrInput;
		 
		// Is Lara dead?
		if (CurrentLevel != 0 && (Lara.deathCount > 300 || Lara.deathCount > 60 && TrInput))
		{
			int inventoryResult = g_Inventory->DoInventory();
			switch (inventoryResult)
			{
			case INV_RESULT_NEW_GAME:
				return GAME_STATUS_NEW_GAME;
			case INV_RESULT_LOAD_GAME:
				return GAME_STATUS_LOAD_GAME;
			case INV_RESULT_EXIT_TO_TILE:
				return GAME_STATUS_EXIT_TO_TITLE;
			}
		}

		if (demoMode && TrInput == -1)
		{
			oldInput = 0;
			TrInput = 0;
		}

		if (CurrentLevel == 0)
			TrInput = 0;

		// Handle lasersight and binocular
		if (CurrentLevel != 0)
		{
			if (!(TrInput & IN_LOOK) || SniperCameraActive || UseSpotCam || TrackCameraInit ||
				((LaraItem->currentAnimState != STATE_LARA_STOP || LaraItem->animNumber != ANIMATION_LARA_STAY_IDLE)
					&& (!Lara.isDucked
						|| TrInput & IN_DUCK
						|| LaraItem->animNumber != ANIMATION_LARA_CROUCH_IDLE
						|| LaraItem->goalAnimState != STATE_LARA_CROUCH_IDLE)))
			{
				if (BinocularRange == 0)
				{
					if (SniperCameraActive || UseSpotCam || TrackCameraInit)
						TrInput &= ~IN_LOOK;
				}
				else
				{
					if (LaserSight)
					{
						BinocularRange = 0;
						LaserSight = false;
						AlterFOV(ANGLE(80));
						LaraItem->meshBits = 0xFFFFFFFF;
						Lara.busy = false;
						Camera.type = BinocularOldCamera;

						Lara.headYrot = 0;
						Lara.headXrot = 0;

						Lara.torsoYrot = 0;
						Lara.torsoXrot = 0;

						Camera.bounce = 0;
						BinocularOn = -8;

						TrInput &= ~IN_LOOK;
					}
					else
					{
						TrInput |= IN_LOOK;
					}
				}

				Infrared = false;
			}
			else if (BinocularRange == 0)
			{
				if (Lara.gunStatus == LG_READY
					&& ((Lara.gunType == WEAPON_REVOLVER && g_LaraExtra.Weapons[WEAPON_REVOLVER].HasLasersight)
						|| (Lara.gunType == WEAPON_HK)
						|| (Lara.gunType == WEAPON_CROSSBOW && g_LaraExtra.Weapons[WEAPON_CROSSBOW].HasLasersight)))
				{
					BinocularRange = 128;
					BinocularOldCamera = Camera.oldType;

					Lara.busy = true;
					LaserSight = true;

					/*if (!(gfLevelFlags & GF_LVOP_TRAIN))
						InfraRed = TRUE;
					else*
						InfraRed = false;*/
					Infrared = true;
				}
				else
					Infrared = false;
			}
			else
			{
				if (LaserSight)
				{
					/*if (!(gfLevelFlags & GF_LVOP_TRAIN))
						InfraRed = TRUE;
					else
						InfraRed = false;*/
					Infrared = true;
				}
				else
				{
					/*if ((gfLevelFlags & GF_LVOP_TRAIN) && (inputBusy & IN_ACTION))
						InfraRed = TRUE;
					else
						InfraRed = false;*/
					Infrared = false;
				}
			}
		}

		// Clear dynamic lights
		ClearDynamicLights();
		ClearFires();
		g_Renderer->ClearDynamicLights();

		GotLaraSpheres = false;

		// Update all items
		InItemControlLoop = true;

		short itemNum = NextItemActive;
		while (itemNum != NO_ITEM)
		{
			ITEM_INFO* item = &Items[itemNum];
			short nextItem = item->nextActive;

			if (item->afterDeath <= 128)
			{
				if (Objects[item->objectNumber].control)
					Objects[item->objectNumber].control(itemNum);

				if (item->afterDeath < 128 && item->afterDeath > 0 && !(Wibble & 3))
					item->afterDeath++;
				if (item->afterDeath == 128)
					KillItem(itemNum);
			}
			else
			{
				KillItem(itemNum);
			}

			itemNum = nextItem;
		}

		InItemControlLoop = false;
		KillMoveItems();

		// Update all effects
		InItemControlLoop = true;

		short fxNum = NextFxActive;
		while (fxNum != NO_ITEM)
		{
			short nextFx = Effects[fxNum].nextActive;
			FX_INFO* fx = &Effects[fxNum];
			if (Objects[fx->objectNumber].control)
				Objects[fx->objectNumber].control(fxNum);
			fxNum = nextFx;
		}

		InItemControlLoop = false;
		KillMoveEffects();

		// Update some effect timers
		if (SmokeCountL)
			SmokeCountL--;
		if (SmokeCountR)
			SmokeCountR--;
		if (SplashCount)
			SplashCount--;
		if (WeaponDelay)
			WeaponDelay--;
		if (WeaponEnemyTimer)
			WeaponEnemyTimer--;

		if (CurrentLevel != 0)
		{
			if (Lara.hasFired)
			{
				AlertNearbyGuards(LaraItem);
				Lara.hasFired = false;
			}

			// Is Lara poisoned?
			if (Lara.poisoned)
			{
				if (Lara.poisoned <= 4096)
				{
					if (Lara.dpoisoned)
						++Lara.dpoisoned;
				}
				else
				{
					Lara.poisoned = 4096;
				}
				if ((gfLevelFlags & 0x80u) != 0 && !Lara.gassed)
				{
					if (Lara.dpoisoned)
					{
						Lara.dpoisoned -= 8;
						if (Lara.dpoisoned < 0)
							Lara.dpoisoned = 0;
					}
				}
				if (Lara.dpoisoned >= 256 && !(Wibble & 0xFF))
				{
					LaraItem->hitPoints -= Lara.poisoned >> (8 - Lara.gassed);
					PoisonFlags = 16;
				}
			}

			// Control Lara
			InItemControlLoop = true;
			Lara.skelebob = NULL;
			LaraControl(Lara.itemNumber);
			InItemControlLoop = false;
			KillMoveItems();

			// Update Lara's ponytails
			HairControl(0, 0, 0);
			if (level->LaraType == LARA_YOUNG)
				HairControl(0, 1, 0);
		}

		if (UseSpotCam)
		{
			// Draw flyby cameras
			//if (CurrentLevel != 0)
			//	g_Renderer->EnableCinematicBars(true);
			CalculateSpotCameras();
		}
		else
		{
			// Do the standard camera
			//g_Renderer->EnableCinematicBars(false);
			TrackCameraInit = false;
			CalculateCamera();
		}
		    
		//WTF: what is this? It's used everywhere so it has to stay
		Wibble = (Wibble + 4) & 0xFC;
		
		// Update special effects
		TriggerLaraDrips();

		if (SmashedMeshCount)
		{
			do
			{
				SmashedMeshCount--;

				FLOOR_INFO* floor = GetFloor(
					SmashedMesh[SmashedMeshCount]->x,
					SmashedMesh[SmashedMeshCount]->y,
					SmashedMesh[SmashedMeshCount]->z,
					&SmashedMeshRoom[SmashedMeshCount]);

				int height = GetFloorHeight(
					floor,
					SmashedMesh[SmashedMeshCount]->x,
					SmashedMesh[SmashedMeshCount]->y,
					SmashedMesh[SmashedMeshCount]->z);

				TestTriggers(TriggerIndex, 1, 0);

				floor->stopper = false;
				SmashedMesh[SmashedMeshCount] = 0;
			} while (SmashedMeshCount != 0);
		}
		
		UpdateSparks();
		UpdateFireSparks();
		UpdateSmoke();
		UpdateBlood();
		UpdateBubbles();
		UpdateDebris();
		UpdateGunShells();
		updateFootprints();
		UpdateSplashes();
		UpdateDrips();
		UpdateRats();
		UpdateBats();
		UpdateSpiders();
		UpdateShockwaves();
		UpdateLightning();
		UpdatePulseColor();
		AnimateWaterfalls();

		if (level->Rumble)
			RumbleScreen();

		SoundEffects();

		HealtBarTimer--;

		GameTimer++;
	}

	return GAME_STATUS_NONE;
}

unsigned __stdcall GameMain(void*)
{
	DB_Log(2, "GameMain - DLL");
	printf("GameMain\n");

	// We still need legacy matrices because control routines use them
	MatrixPtr = MatrixStack;
	DxMatrixPtr = (byte*)malloc(48 * 40);

	// Initialise legacy memory buffer and game timer
	InitGameMalloc();
	TIME_Init();

	// Do a fixed time title image
	g_Renderer->DoTitleImage();

	// Execute the LUA gameflow and play the game
	g_GameFlow->DoGameflow();

	DoTheGame = false;

	// Finish the thread
	PostMessageA((HWND)WindowsHandle, 0x10u, 0, 0);
	_endthreadex(1);
	
	return 1;   
}   

GAME_STATUS DoTitle(int index)
{
	DB_Log(2, "DoTitle - DLL");
	printf("DoTitle\n");

	CreditsDone = false;
	CanLoad = false;

	// Load the level
	S_LoadLevelFile(index);

	int inventoryResult;

	if (g_GameFlow->TitleType == TITLE_FLYBY)
	{
		// Initialise items, effects, lots, camera
		InitialiseFXArray(true);
		InitialisePickupDisplay();
		InitialiseCamera();
		SOUND_Stop();

		RequiredStartPos = false;
		if (InitialiseGame)
		{
			GameTimer = 0;
			RequiredStartPos = false;
			InitialiseGame = false;
		}

		Savegame.Level.Timer = 0;
		if (CurrentLevel == 1)
			Savegame.TLCount = 0;

		LastInventoryItem = -1;
		DelCutSeqPlayer = 0;
		TitleControlsLockedOut = false;
		GameMode = 1;

		// Initialise flyby cameras
		InitSpotCamSequences();

		InitialiseSpotCam(2);
		CurrentAtmosphere = 83;
		UseSpotCam = true;

		// Play background music
		//CurrentAtmosphere = ambient;
		S_CDPlay(CurrentAtmosphere, 1);
		IsAtmospherePlaying = true;

		// Initialise ponytails
		InitialiseHair();

		ControlPhase(2, 0);
		inventoryResult = g_Inventory->DoTitleInventory();
	}
	else
	{
		inventoryResult = g_Inventory->DoTitleInventory();
	}

	UseSpotCam = false;
	S_CDStop();

	switch (inventoryResult)
	{
	case INV_RESULT_NEW_GAME:
		return GAME_STATUS_NEW_GAME;
	case INV_RESULT_LOAD_GAME:
		return GAME_STATUS_LOAD_GAME;
	case INV_RESULT_EXIT_GAME:
		return GAME_STATUS_EXIT_GAME;
	}

	return GAME_STATUS_NEW_GAME;
}

GAME_STATUS DoLevel(int index, int ambient, bool loadFromSavegame)
{
	CreditsDone = false;
	CanLoad = false;

	// If not loading a savegame, then clear all the infos
	if (!loadFromSavegame)
	{
		Savegame.Level.Timer = 0;
		Savegame.Level.Distance = 0;
		Savegame.Level.AmmoUsed = 0;
		Savegame.Level.AmmoHits = 0;
		Savegame.Level.Kills = 0;
	}

	// Load the level
	S_LoadLevelFile(index);

	// Initialise items, effects, lots, camera
	InitialiseFXArray(true);
	InitialisePickupDisplay();
	InitialiseCamera();
	SOUND_Stop();

	// Restore the game?
	if (loadFromSavegame)
	{
		char fileName[255];
		ZeroMemory(fileName, 255);
		sprintf(fileName, "savegame.%d", g_GameFlow->SelectedSaveGame);
		SaveGame::Load(fileName);

		Camera.pos.x = LaraItem->pos.xPos + 256;
		Camera.pos.y = LaraItem->pos.yPos + 256;
		Camera.pos.z = LaraItem->pos.zPos + 256;

		Camera.target.x = LaraItem->pos.xPos;
		Camera.target.y = LaraItem->pos.yPos;
		Camera.target.z = LaraItem->pos.zPos;

		int x = Lara.weaponItem;

		RequiredStartPos = false;
		InitialiseGame = false;
		g_GameFlow->SelectedSaveGame = 0;
	}
	else
	{
		RequiredStartPos = false;
		if (InitialiseGame)
		{
			GameTimer = 0;
			RequiredStartPos = false;
			InitialiseGame = false;
		}

		Savegame.Level.Timer = 0;
		if (CurrentLevel == 1)
			Savegame.TLCount = 0;
	}

	LastInventoryItem = -1;
	DelCutSeqPlayer = 0;
	TitleControlsLockedOut = false;
	g_Inventory->SetEnterObject(NO_ITEM);
	g_Inventory->SetSelectedObject(NO_ITEM);

	// Initialise flyby cameras
	InitSpotCamSequences();

	// Play background music
	CurrentAtmosphere = ambient;
	S_CDPlay(CurrentAtmosphere, 1);
	IsAtmospherePlaying = true;

	// Initialise ponytails
	InitialiseHair();

	int nframes = 2;
	GAME_STATUS result = ControlPhase(nframes, 0);
	g_Renderer->FadeIn();

	// The game loop, finally!
	while (true)
	{
		nframes = DrawPhaseGame();
		result = ControlPhase(nframes, 0); //printf("LastSpotCam: %d\n", LastSpotCam);

		if (result == GAME_STATUS_EXIT_TO_TITLE ||
			result == GAME_STATUS_LOAD_GAME ||
			result == GAME_STATUS_LEVEL_COMPLETED)
		{
			// Here is the only way for exiting from the loop
			SOUND_Stop();
			S_CDStop();

			return result;
		}

		Sound_UpdateScene();
	}
}

void TestTriggers(short* data, int heavy, int HeavyFlags)
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

	HeavyTriggered = false;

	if (!heavy)
	{
		Lara.canMonkeySwing = false;
		Lara.climbStatus = false;
	}

	if (!data)
		return;

	// Burn Lara
	if ((*data & 0x1F) == LAVA_TYPE)
	{
		if (!heavy && (LaraItem->pos.yPos == LaraItem->floor || Lara.waterStatus))
			LavaBurn(LaraItem);

		if (*data & 0x8000)
			return;

		data++;
	}

	// Lara can climb
	if ((*data & 0x1F) == CLIMB_TYPE)
	{
		if (!heavy)
		{
			short quad = (unsigned short)(LaraItem->pos.yRot + ANGLE(45)) / ANGLE(90);
			if ((1 << (quad + 8)) & *data)
				Lara.climbStatus = true;
		}

		if (*data & 0x8000)
			return;

		data++;
	}

	// Lara can monkey
	if ((*data & 0x1F) == MONKEY_TYPE)
	{
		if (!heavy)
			Lara.canMonkeySwing = true;

		if (*data & 0x8000)
			return;

		data++;
	}

	// Trigger triggerer
	if ((*data & 0x1F) == TRIGTRIGGER_TYPE)
	{
		if (!(*data & 0x20) || *data & 0x8000)
			return;

		data++;
	}

	short triggerType = (*(data++) >> 8) & 0x3F;
	short flags = *(data++);
	short timer = flags & 0xFF;

	if (Camera.type != HEAVY_CAMERA)
		RefreshCamera(triggerType, data);

	if (heavy)
	{
		switch (triggerType)
		{
		case HEAVY:
		case HEAVYANTITRIGGER:
			break;

		case HEAVYSWITCH:
			if (!HeavyFlags)
				return;

			if (HeavyFlags >= 0)
			{
				flags &= 0x3E00u;
				if (flags != HeavyFlags)
					return;
			}
			else
			{
				flags |= 0x3E00u;
				flags += HeavyFlags;
			}
			break;

		default:
			// Enemies can only activate heavy triggers
			return;
		}
	}

	short value = 0;

	switch (triggerType)
	{
	case TRIGGER_TYPES::SWITCH:
		value = *(data++) & 0x3FF;

		if (flags & 0x100)
			Items[value].itemFlags[0] = 1;

		if (!SwitchTrigger(value, timer))
			return;

		objectNumber = Items[value].objectNumber;
		if (objectNumber >= ID_SWITCH_TYPE1 && objectNumber <= ID_SWITCH_TYPE6 && Items[value].triggerFlags == 5)
			switchFlag = 1;

		switchOff = (Items[value].currentAnimState == 1);

		break;

	case TRIGGER_TYPES::MONKEY:
		if (LaraItem->currentAnimState >= 75 &&
			(LaraItem->currentAnimState <= 79 ||
				LaraItem->currentAnimState == 82 ||
				LaraItem->currentAnimState == 83))
			break;
		return;

	case TRIGGER_TYPES::TIGHTROPE_T:
		if (LaraItem->currentAnimState >= 119 &&
			LaraItem->currentAnimState <= 127 &&
			LaraItem->currentAnimState != 126)
			break;
		return;

	case TRIGGER_TYPES::CRAWLDUCK_T:
		if (LaraItem->currentAnimState == 80 ||
			LaraItem->currentAnimState == 81 ||
			LaraItem->currentAnimState == 84 ||
			LaraItem->currentAnimState == 85 ||
			LaraItem->currentAnimState == 86 ||
			LaraItem->currentAnimState == 71 ||
			LaraItem->currentAnimState == 72 ||
			LaraItem->currentAnimState == 105 ||
			LaraItem->currentAnimState == 106)
			break;
		return;

	case TRIGGER_TYPES::CLIMB_T:
		if (LaraItem->currentAnimState == 10 ||
			LaraItem->currentAnimState == 56 ||
			LaraItem->currentAnimState == 57 ||
			LaraItem->currentAnimState == 58 ||
			LaraItem->currentAnimState == 59 ||
			LaraItem->currentAnimState == 60 ||
			LaraItem->currentAnimState == 61 ||
			LaraItem->currentAnimState == 75)
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

	case TRIGGER_TYPES::SKELETON_T:
		Lara.skelebob = 2;
		break;

	case TRIGGER_TYPES::HEAVY:
	case TRIGGER_TYPES::DUMMY:
	case TRIGGER_TYPES::HEAVYSWITCH:
	case TRIGGER_TYPES::HEAVYANTITRIGGER:
		return;

	default:
		break;
	}

	short targetType = 0;
	short trigger = 0;

	ITEM_INFO* item = NULL;
	ITEM_INFO* cameraItem = NULL;

	do
	{
		trigger = *(data++);
		value = trigger & 0x3FF;
		targetType = (trigger >> 10) & 0xF;

		switch (targetType)
		{
		case TO_OBJECT:
			item = &Items[value];

			if (keyResult >= 2 ||
				(triggerType == TRIGGER_TYPES::ANTIPAD ||
					triggerType == TRIGGER_TYPES::ANTITRIGGER ||
					triggerType == TRIGGER_TYPES::HEAVYANTITRIGGER) &&
				item->flags & 0x80)
				break;

			if (triggerType == TRIGGER_TYPES::SWITCH)
			{
				if (item->flags & 0x40)
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
				if (HeavyFlags >= 0)
				{
					if (switchFlag)
						item->flags |= (flags & 0x3E00);
					else
						item->flags ^= (flags & 0x3E00);

					if (flags & 0x100)
						item->flags |= 0x40;
				}
				else
				{
					if (((flags ^ item->flags) & 0x3E00) == 0x3E00)
					{
						item->flags ^= (flags & 0x3E00);
						if (flags & 0x100)
							item->flags |= 0x40;
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

				item->flags &= ~(0x3E00 | 0x4000);

				if (flags & 0x100)
					item->flags |= 0x80;

				if (item->active && Objects[item->objectNumber].intelligent)
				{
					item->hitPoints = -16384;
					DisableBaddieAI(value);
					KillItem(value);
				}
			}
			else if (flags & 0x3E00)
			{
				item->flags |= flags & 0x3E00;
			}

			if ((item->flags & 0x3E00) == 0x3E00)
			{
				item->flags |= 0x20;

				if (flags & 0x100)
					item->flags |= 1;

				if (!(item->active) && !(item->flags & IFLAG_KILLED))
				{
					if (Objects[item->objectNumber].intelligent)
					{
						if (item->status != ITEM_DEACTIVATED)
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
						HeavyTriggered = heavy;
					}
				}
			}
			break;

		case TO_CAMERA:
			trigger = *(data++);

			if (keyResult == 1)
				break;

			if (Camera.fixed[value].flags & 0x100)
				break;

			Camera.number = value;

			if (Camera.type == LOOK_CAMERA || Camera.type == COMBAT_CAMERA && !(Camera.fixed[value].flags & 3))
				break;

			if (triggerType == TRIGGER_TYPES::COMBAT)
				break;

			if (triggerType == TRIGGER_TYPES::SWITCH && timer && switchOff)
				break;

			if (Camera.number != Camera.last || triggerType == TRIGGER_TYPES::SWITCH)
			{
				Camera.timer = (trigger & 0xFF) * 30;

				if (trigger & 0x100)
					Camera.fixed[Camera.number].flags |= 0x100;

				Camera.speed = ((trigger & 0x3E00) >> 6) + 1;
				Camera.type = heavy ? HEAVY_CAMERA : FIXED_CAMERA;
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
					if (trigger & 0x100)
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
			cameraItem = &Items[value];
			break;

		case TO_SINK:
			Lara.currentActive = value + 1;
			break;

		case TO_FLIPMAP:
			flipAvailable = true;
			
			if (FlipMap[value] & 0x100)
				break;

			if (triggerType == TRIGGER_TYPES::SWITCH)
				FlipMap[value] ^= (flags & 0x3E00);
			else if (flags & 0x3E00)
				FlipMap[value] |= (flags & 0x3E00);
			
			if ((FlipMap[value] & 0x3E00) == 0x3E00)
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
			FlipMap[value] |= 0x3E00;
			if (!FlipStats[value])
				flip = value;
			break;

		case TO_FLIPOFF:
			flipAvailable = true;
			FlipMap[value] &= ~0x3E00;
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

		case TO_LUA_SCRIPT:
			trigger = *(data++);
			g_GameScript->ExecuteTrigger(trigger & 0x7FFF);

			break;

		default: 
			break;
		}

	} while (!(trigger & 0x8000));

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

void UpdateSky()
{
	GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

	if (level->Layer1.Enabled)
	{
		SkyPos1 += level->Layer1.CloudSpeed;
		if (SkyPos1 <= 9728)
		{
			if (SkyPos1 < 0)
				SkyPos1 += 9728;
		}
		else
		{
			SkyPos1 -= 9728;
		}
	}

	if (level->Layer1.Enabled)
	{
		SkyPos2 += level->Layer2.CloudSpeed;
		if (SkyPos2 <= 9728)
		{
			if (SkyPos2 < 0)
				SkyPos2 += 9728;
		}
		else
		{
			SkyPos2 -= 9728;
		}
	}
}

void ActivateKey()
{
	KeyTriggerActive = 1;
}

short GetDoor(FLOOR_INFO* floor)
{
	if (!floor->index)
		return NO_ROOM;

	short* data = &FloorData[floor->index];
	short type = *(data++);

	if (((type & DATA_TYPE) == TILT_TYPE)
		|| ((type & DATA_TYPE) == SPLIT1)
		|| ((type & DATA_TYPE) == SPLIT2)
		|| ((type & DATA_TYPE) == NOCOLF1B)
		|| ((type & DATA_TYPE) == NOCOLF1T)
		|| ((type & DATA_TYPE) == NOCOLF2B)
		|| ((type & DATA_TYPE) == NOCOLF2T))
	{
		if (type & END_BIT)
			return NO_ROOM;

		data++;
		type = *(data++);
	}

	if (((type & DATA_TYPE) == ROOF_TYPE)
		|| ((type & DATA_TYPE) == SPLIT3)
		|| ((type & DATA_TYPE) == SPLIT4)
		|| ((type & DATA_TYPE) == NOCOLC1B)
		|| ((type & DATA_TYPE) == NOCOLC1T)
		|| ((type & DATA_TYPE) == NOCOLC2B)
		|| ((type & DATA_TYPE) == NOCOLC2T))
	{
		if (type & END_BIT)
			return NO_ROOM;

		data++;
		type = *(data++);
	}

	if ((type & DATA_TYPE) == DOOR_TYPE)
		return (*data);

	return NO_ROOM;
}

void TranslateItem(ITEM_INFO* item, int x, int y, int z)
{
	int c = COS(item->pos.yRot);
	int s = SIN(item->pos.yRot);

	item->pos.xPos += (c * x + s * z) >> W2V_SHIFT;
	item->pos.yPos += y;
	item->pos.zPos += (-s * x + c * z) >> W2V_SHIFT;
}

int GetWaterSurface(int x, int y, int z, short roomNumber)
{
	ROOM_INFO* room = &Rooms[roomNumber];
	FLOOR_INFO* floor = &XZ_GET_SECTOR(room, x - room->x, z - room->z);

	if (room->flags & ENV_FLAG_WATER)
	{
		while (floor->skyRoom != NO_ROOM)
		{
			room = &Rooms[floor->skyRoom];
			if (!(room->flags & ENV_FLAG_WATER))
				return (floor->ceiling << 8);
			floor = &XZ_GET_SECTOR(room, x - room->x, z - room->z);
		}
		return NO_HEIGHT;
	}
	else
	{
		while (floor->pitRoom != NO_ROOM)
		{
			room = &Rooms[floor->pitRoom];
			if (room->flags & ENV_FLAG_WATER)
				return (floor->floor << 8);
			floor = &XZ_GET_SECTOR(room, x - room->x, z - room->z);
		}
	}

	return NO_HEIGHT;
}

void KillMoveItems()
{
	if (ItemNewRoomNo > 0)
	{
		for (int i = 0; i < ItemNewRoomNo; i++)
		{
			short itemNumber = ItemNewRooms[2 * i];
			if (itemNumber >= 0)
				ItemNewRoom(itemNumber, ItemNewRooms[2 * i + 1]);
			else
				KillItem(itemNumber & 0x7FFF);
		}
	}

	ItemNewRoomNo = 0;
}

void KillMoveEffects()
{
	if (ItemNewRoomNo > 0)
	{
		for (int i = 0; i < ItemNewRoomNo; i++)
		{
			short itemNumber = ItemNewRooms[2 * i];
			if (itemNumber >= 0)
				EffectNewRoom(itemNumber, ItemNewRooms[2 * i + 1]);
			else
				KillEffect(itemNumber & 0x7FFF);
		}
	}

	ItemNewRoomNo = 0;
}

int GetChange(ITEM_INFO* item, ANIM_STRUCT* anim)
{
	if (item->currentAnimState == item->goalAnimState)
		return 0;

	if (anim->numberChanges <= 0)
		return 0;

	for (int i = 0; i < anim->numberChanges; i++)
	{
		CHANGE_STRUCT* change = &Changes[anim->changeIndex + i];
		if (change->goalAnimState == item->goalAnimState)
		{
			for (int j = 0; j < change->numberRanges; j++)
			{
				RANGE_STRUCT* range = &Ranges[change->rangeIndex + j];
				if (item->frameNumber >= range->startFrame && item->frameNumber <= range->endFrame)
				{
					item->animNumber = range->linkAnimNum;
					item->frameNumber = range->linkFrameNum;
					
					return 1;
				}
			}
		}
	}

	return 0;
}

void AlterFloorHeight(ITEM_INFO* item, int height)
{
	FLOOR_INFO* floor;
	FLOOR_INFO* ceiling;
	BOX_INFO* box;
	short roomNumber;
	int flag = 0;

	if (abs(height))
	{
		flag = 1;
		if (height >= 0)
			height++;
		else
			height--;
	}

	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	ceiling = GetFloor(item->pos.xPos, height + item->pos.yPos - WALL_SIZE, item->pos.zPos, &roomNumber);

	if (floor->floor == NO_HEIGHT / STEP_SIZE)
	{
		floor->floor = ceiling->ceiling + height / STEP_SIZE;
	}
	else
	{
		floor->floor += height / STEP_SIZE;
		if (floor->floor == ceiling->ceiling && !flag)
			floor->floor = NO_HEIGHT / STEP_SIZE;
	}

	box = &Boxes[floor->box];
	if (box->overlapIndex & BLOCKABLE)
	{
		if (height >= 0)
			box->overlapIndex &= ~BLOCKED;
		else
			box->overlapIndex |= BLOCKED;
	}
}

FLOOR_INFO* GetFloor(int x, int y, int z, short* roomNumber)
{
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	short data;
	int xFloor = 0;
	int yFloor = 0;
	short roomDoor = 0;
	int retval;

	r = &Rooms[*roomNumber];
	do
	{
		xFloor = (z - r->z) >> WALL_SHIFT;
		yFloor = (x - r->x) >> WALL_SHIFT;

		if (xFloor <= 0)
		{
			xFloor = 0;
			if (yFloor < 1)
				yFloor = 1;
			else if (yFloor > r->ySize - 2)
					yFloor = r->ySize - 2;
		}
		else if (xFloor >= r->xSize - 1)
		{
			xFloor = r->xSize - 1;
			if (yFloor < 1)
				yFloor = 1;
			else if (yFloor > r->ySize - 2)
					yFloor = r->ySize - 2;
		}
		else if (yFloor >= 0)
		{
			if (yFloor >= r->ySize)
				yFloor = r->ySize - 1;
		}
		else
		{
			yFloor = 0;
		}

		floor = &r->floor[xFloor + (yFloor * r->xSize)];
		data = GetDoor(floor);
		if (data != NO_ROOM)
		{
			*roomNumber = data;
			r = &Rooms[data];
		}
	} while (data != NO_ROOM);

	if (y < floor->floor * 256)
	{
		if (y < floor->ceiling * 256 && floor->skyRoom != NO_ROOM)
		{
			do
			{
				int noCollision = CheckNoColCeilingTriangle(floor, x, z);
				if (noCollision == 1 || noCollision == -1 && y >= r->maxceiling)
					break;

				*roomNumber = floor->skyRoom;
				r = &Rooms[floor->skyRoom];
				floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);
				if (y >= floor->ceiling * 256)
					break;
			} while (floor->skyRoom != NO_ROOM);
		}
	}
	else if (floor->pitRoom != NO_ROOM)
	{
		do
		{
			int noCollision = CheckNoColFloorTriangle(floor, x, z);
			if (noCollision == 1 || noCollision == -1 && y < r->minfloor)
				break;

			*roomNumber = floor->pitRoom;
			r = &Rooms[floor->pitRoom];
			floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);
			if (y < floor->floor * 256)
				break;
		} while (floor->pitRoom != NO_ROOM);
	}

	return floor;
}

int CheckNoColFloorTriangle(FLOOR_INFO* floor, int x, int z)
{
	if (!floor->index)
		return 0;

	short* data = &FloorData[floor->index];
	short type = *(data) & DATA_TYPE;

	if (type == NOCOLF1T || type == NOCOLF1B || type == NOCOLF2T || type == NOCOLF2B)
	{
		int dx = x & 1023;
		int dz = z & 1023;

		if (type == NOCOLF1T && dx <= (SECTOR(1) - dz))
			return -1;
		else if (type == NOCOLF1B && dx > (SECTOR(1) - dz))
			return -1;
		else if (type == NOCOLF2T && dx <= dz)
			return -1;
		else if (type == NOCOLF2B && dx > dz)
			return -1;
		else
			return 1;
	}

	return 0;
}

int CheckNoColCeilingTriangle(FLOOR_INFO * floor, int x, int z)
{
	if (!floor->index)
		return 0;

	short* data = &FloorData[floor->index];
	short type = *(data) & DATA_TYPE;

	if (type == TILT_TYPE || type == SPLIT1 || type == SPLIT2 || type == NOCOLF1T || type == NOCOLF1B || type == NOCOLF2T || type == NOCOLF2B)	// gibby
	{
		if (*(data) & END_BIT)
			return 0;
		type = *(data + 2) & DATA_TYPE;
	}

	if (type == NOCOLC1T || type == NOCOLC1B || type == NOCOLC2T || type == NOCOLC2B)
	{
		int dx = x & 1023;
		int dz = z & 1023;

		if (type == NOCOLC1T && dx <= (SECTOR(1) - dz))
			return -1;
		else if (type == NOCOLC1B && dx > (SECTOR(1) - dz))
			return -1;
		else if (type == NOCOLC2T && dx <= dz)
			return -1;
		else if (type == NOCOLC2B && dx > dz)
			return -1;
		else
			return 1;
	}
	
	return 0;
}

int GetFloorHeight(FLOOR_INFO* floor, int x, int y, int z)
{
	TiltYOffset = 0;
	TiltXOffset = 0;
	OnObject = 0;
	HeightType = WALL;

	ROOM_INFO* r;
	while (floor->pitRoom != NO_ROOM)
	{
		if (CheckNoColFloorTriangle(floor, x, z) == 1)
			break;
		r = &Rooms[floor->pitRoom];
		floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);
	}

	int height = floor->floor * 256;
	if (height == NO_HEIGHT)
		return height;

	TriggerIndex = NULL;
	
	if (floor->index == 0)
		return height;

	short* data = &FloorData[floor->index];
	short type, hadj;

	int xOff, yOff, trigger;
	ITEM_INFO* item;
	OBJECT_INFO* obj;
	int tilts, t0, t1, t2, t3, t4, dx, dz, h1, h2;

	do
	{
		type = *(data++);

		switch (type & DATA_TYPE)
		{
		case DOOR_TYPE:
		case ROOF_TYPE:
		case SPLIT3:
		case SPLIT4:
		case NOCOLC1T:
		case NOCOLC1B:
		case NOCOLC2T:
		case NOCOLC2B:
			data++;
			break;

		case TILT_TYPE:
			TiltXOffset = xOff = (*data >> 8);
			TiltYOffset = yOff = *(char *) data;

			if ((abs(xOff)) > 2 || (abs(yOff)) > 2)
				HeightType = BIG_SLOPE;
			else
				HeightType = SMALL_SLOPE;

			if (xOff >= 0)
				height += (xOff * ((-1 - z) & 1023) >> 2);
			else
				height -= (xOff * (z & 1023) >> 2);

			if (yOff >= 0)
				height += yOff * ((-1 - x) & 1023) >> 2;
			else
				height -= yOff * (x & 1023) >> 2;

			data++;
			break;

		case TRIGGER_TYPE:
			if (!TriggerIndex)
				TriggerIndex = data - 1;

			data++;

			do
			{
				trigger = *(data++);

				if (TRIG_BITS(trigger) != TO_OBJECT)
				{
					if (TRIG_BITS(trigger) == TO_CAMERA || 
						TRIG_BITS(trigger) == TO_FLYBY)
					{
						trigger = *(data++);
					}
				}
				else
				{
					item = &Items[trigger & VALUE_BITS];
					obj = &Objects[item->objectNumber];

					if (obj->floor && !(item->flags & 0x8000))
					{
						(obj->floor)(item, x, y, z, &height);
					}
				}

			} while (!(trigger& END_BIT));
			break;

		case LAVA_TYPE:
			TriggerIndex = data - 1;
			break;

		case CLIMB_TYPE:
		case MONKEY_TYPE:
		case TRIGTRIGGER_TYPE:
			if (!TriggerIndex)
				TriggerIndex = data - 1;
			break;

		case SPLIT1:
		case SPLIT2:
		case NOCOLF1T:
		case NOCOLF1B:
		case NOCOLF2T:
		case NOCOLF2B:
			tilts = *data;
			t0 = tilts & 15;
			t1 = (tilts >> 4) & 15;
			t2 = (tilts >> 8) & 15;
			t3 = (tilts >> 12) & 15;

			dx = x & 1023;
			dz = z & 1023;

			xOff = yOff = 0;

			HeightType = SPLIT_TRI;

			if ((type & DATA_TYPE) == SPLIT1 ||
				(type & DATA_TYPE) == NOCOLF1T ||
				(type & DATA_TYPE) == NOCOLF1B)
			{
				if (dx <= (1024 - dz))	 
				{
					hadj = (type >> 10) & 0x1F;
					if (hadj & 0x10) 
						hadj |= 0xfff0;
					height += 256 * hadj;
					xOff = t2 - t1;
					yOff = t0 - t1;
				}
				else
				{
					hadj = (type >> 5) & 0x1F;
					if (hadj & 0x10) 
						hadj |= 0xFFF0;
					height += 256 * hadj;
					xOff = t3 - t0;
					yOff = t3 - t2;
				}
			}
			else 
			{
				if (dx <= dz) 
				{
					hadj = (type >> 10) & 0x1f;
					if (hadj & 0x10) 
						hadj |= 0xfff0;
					height += 256 * hadj;
					xOff = t2 - t1;
					yOff = t3 - t2;
				}
				else
				{
					hadj = (type >> 5) & 0x1f;
					if (hadj & 0x10) 
						hadj |= 0xfff0;
					height += 256 * hadj;
					xOff = t3 - t0;
					yOff = t0 - t1;

				}
			}

			TiltXOffset = xOff;
			TiltYOffset = yOff;

			if ((abs(xOff)) > 2 || (abs(yOff)) > 2)
				HeightType = DIAGONAL;
			else if (HeightType != SPLIT_TRI)
				HeightType = SMALL_SLOPE;

			if (xOff >= 0)
				height += xOff * ((-1 - z) & 1023) >> 2;
			else
				height -= xOff * (z & 1023) >> 2;
			
			if (yOff >= 0)
				height += yOff * ((-1 - x) & 1023) >> 2;
			else
				height -= yOff * (x & 1023) >> 2;

			data++;
			break;

		default:
			break;
		}
	} while (!(type & END_BIT));

	return height;
}

void UpdateBats()
{
	if (!Objects[ID_BATS].loaded)
		return;

	short* bounds = GetBoundsAccurate(LaraItem);
	
	int x1 = LaraItem->pos.xPos + bounds[0] - (bounds[0] >> 2);
	int x2 = LaraItem->pos.xPos + bounds[1] - (bounds[1] >> 2);

	int y1 = LaraItem->pos.yPos + bounds[2] - (bounds[2] >> 2);
	int y2 = LaraItem->pos.yPos + bounds[3] - (bounds[3] >> 2);

	int z1 = LaraItem->pos.zPos + bounds[4] - (bounds[4] >> 2);
	int z2 = LaraItem->pos.zPos + bounds[5] - (bounds[5] >> 2);
	
	int minDistance = 0xFFFFFFF; // v40
	int minIndex = -1;

	for (int i=0;i<64;i++)
	{
		BAT_STRUCT* bat = &Bats[i];

		if (!bat->on)
			continue;

		if ((Lara.burn || LaraItem->hitPoints <= 0) 
			&& bat->counter > 90 
			&& !(GetRandomControl() & 7))
			bat->counter = 90;

		if (!(--bat->counter))
		{
			bat->on = 0;
			continue;
		}

		if (!(GetRandomControl() & 7))
		{
			bat->laraTarget = GetRandomControl() % 640 + 128;
			bat->xTarget = (GetRandomControl() & 0x7F) - 64;
			bat->zTarget = (GetRandomControl() & 0x7F) - 64;
		}

		short angles[2];
		phd_GetVectorAngles(
			LaraItem->pos.xPos + 8 * bat->xTarget - bat->pos.xPos,
			LaraItem->pos.yPos - bat->laraTarget - bat->pos.yPos,
			LaraItem->pos.zPos + 8 * bat->zTarget - bat->pos.zPos,
			angles);

		int distance = SQUARE(LaraItem->pos.zPos - bat->pos.zPos) + SQUARE(LaraItem->pos.xPos - bat->pos.xPos);
		if (distance < minDistance)
		{
			minDistance = distance;
			minIndex = i;
		}

		distance = SQRT_ASM(distance) / 8;
		if (distance <= 128)
		{
			if (distance < 48)
				distance = 48;
		}
		else
		{
			distance = 128;
		}

		if (bat->speed < distance)
			bat->speed++;
		else if (bat->speed > distance)
			bat->speed--;

		if (bat->counter > 90)
		{
			int speed = bat->speed * 128;
			
			short xAngle = abs(angles[1] - bat->pos.yRot) >> 3;
			short yAngle = abs(angles[0] - bat->pos.yRot) >> 3;

			if (xAngle <= -speed)
				xAngle = -speed;
			else if (xAngle >= speed)
				xAngle = speed;

			if (yAngle <= -speed)
				yAngle = -speed;
			else if (yAngle >= speed)
				yAngle = speed;

			bat->pos.yRot += yAngle;
			bat->pos.xRot += xAngle;
		}

		int sp = bat->speed * SIN(bat->pos.xRot) >> W2V_SHIFT;

		bat->pos.xPos += sp * SIN(bat->pos.yRot) >> W2V_SHIFT;
		bat->pos.yPos += bat->speed * SIN(-bat->pos.xRot) >> W2V_SHIFT;
		bat->pos.zPos += sp * COS(bat->pos.yRot) >> W2V_SHIFT;
		
		if ((i % 2 == 0)
			&& bat->pos.xPos > x1
			&& bat->pos.xPos < x2
			&& bat->pos.yPos > y1
			&& bat->pos.yPos < y2
			&& bat->pos.zPos > z1
			&& bat->pos.zPos < z2)
		{
			TriggerBlood(bat->pos.xPos, bat->pos.yPos, bat->pos.zPos, 2 * GetRandomControl(), 2);
			if (LaraItem->hitPoints > 0)
				LaraItem->hitPoints -= 2;
		}
	}

	if (minIndex != -1)
	{
		BAT_STRUCT* bat = &Bats[minIndex];
		if (!(GetRandomControl() & 4))
			SoundEffect(SFX_BATS_1, &bat->pos, 0);
	}
}

void UpdateDebris()
{
	for (int i = 0; i < 256; i++)
	{
		DEBRIS_STRUCT* debris = &Debris[i];

		if (debris->on)
		{
			debris->yVel += debris->gravity;
			if (debris->yVel > 4096)
				debris->yVel = 4096;

			debris->speed -= debris->speed >> 4;

			debris->x += debris->speed * SIN(debris->dir) >> W2V_SHIFT;
			debris->y += debris->yVel >> 4;
			debris->z += debris->speed * COS(debris->dir) >> W2V_SHIFT;
			
			FLOOR_INFO* floor = GetFloor(debris->x, debris->y, debris->z, &debris->roomNumber);
			int height = GetFloorHeight(floor, debris->x, debris->y, debris->z);
			int ceiling = GetCeiling(floor, debris->x, debris->y, debris->z);

			if (debris->y >= height || debris->y < ceiling)
				debris->on--;
			else
			{
				debris->xRot += debris->yVel >> 6;
				
				if (debris->yVel)
					debris->yRot += debris->speed >> 5;

				if (Rooms[debris->roomNumber].flags & ENV_FLAG_WATER)
					debris->yVel -= debris->yVel >> 2;
			}
		}
	}
}

void UpdateRats()
{
	if (Objects[ID_RATS].loaded)
	{
		for (int i = 0; i < 32; i++)
		{
			RAT_STRUCT* rat = &Rats[i];

			if (rat->on)
			{
				int oldX = rat->pos.xPos;
				int oldY = rat->pos.yPos;
				int oldZ = rat->pos.zPos;

				rat->pos.xPos += rat->speed * SIN(rat->pos.yRot) >> W2V_SHIFT;
				rat->pos.yPos += rat->fallspeed;
				rat->pos.zPos += rat->speed * COS(rat->pos.yRot) >> W2V_SHIFT;

				rat->fallspeed += 6;

				int dx = LaraItem->pos.xPos - rat->pos.xPos;
				int dy = LaraItem->pos.yPos - rat->pos.yPos;
				int dz = LaraItem->pos.zPos - rat->pos.zPos;

				short angle;
				if (rat->flags >= 170)
					angle = rat->pos.yRot - ATAN(dz, dx);
				else
					angle = ATAN(dz, dx) - rat->pos.yRot;

				if (abs(dx) < 85 && abs(dy) < 85 && abs(dz) < 85)
				{
					LaraItem->hitPoints--;
					LaraItem->hitStatus = true;
				}

				if (rat->flags & 1)
				{
					if (abs(dz) + abs(dx) <= 1024)
					{
						if (rat->speed & 1)
							rat->pos.yRot += 512;
						else
							rat->pos.yRot -= 512;
						rat->speed = 48 - (abs(angle) >> 10);
					}
					else
					{
						if (rat->speed < (i & 0x1F) + 24)
							rat->speed++;

						if (abs(angle) >= 2048)
						{
							if (angle >= 0)
								rat->pos.yRot += 1024;
							else
								rat->pos.yRot -= 1024;
						}
						else
						{
							rat->pos.yRot += 8 * (Wibble - i);
						}
					}
				}

				__int16 oldRoomNumber = rat->roomNumber;

				FLOOR_INFO* floor = GetFloor(rat->pos.xPos, rat->pos.yPos, rat->pos.zPos, &rat->roomNumber);
				int height = GetFloorHeight(floor, rat->pos.xPos, rat->pos.yPos, rat->pos.zPos);
				if (height < rat->pos.yPos - 1280 || height == NO_HEIGHT)
				{
					if (rat->flags > 170)
					{
						rat->on = false;
						NextRat = 0;
					}
					if (angle <= 0)
						rat->pos.yRot -= ANGLE(90);
					else
						rat->pos.yRot += ANGLE(90);
					
					rat->pos.xPos = oldX;
					rat->pos.yPos = oldY;
					rat->pos.zPos = oldZ;

					rat->fallspeed = 0;
				}
				else
				{
					if (height >= rat->pos.yPos - 64)
					{
						if (rat->pos.yPos <= height)
						{
							if (rat->fallspeed >= 500 || rat->flags >= 200)
							{
								rat->on = 0;
								NextRat = 0;
							}
							else
							{
								rat->pos.xRot = -128 * rat->fallspeed;
							}
						}
						else
						{
							rat->pos.yPos = height;
							rat->fallspeed = 0;
							rat->flags |= 1;
						}
					}
					else
					{
						rat->pos.xRot = 14336;
						rat->pos.xPos = oldX;
						rat->pos.yPos = oldY - 24;
						rat->pos.zPos = oldZ;
						rat->fallspeed = 0;
					}
				}

				if (!(Wibble & 0x3C))
					rat->flags += 2;

				ROOM_INFO* r = &Rooms[rat->roomNumber];
				if (r->flags & ENV_FLAG_WATER)
				{
					rat->fallspeed = 0;
					rat->speed = 16;
					rat->pos.yPos = r->maxceiling + 50;
					
					if (Rooms[oldRoomNumber].flags & ENV_FLAG_WATER)
					{
						if (!(GetRandomControl() & 0xF))
						{
							SetupRipple(rat->pos.xPos, r->maxceiling, rat->pos.zPos, (GetRandomControl() & 3) + 48, 2);
						}
					}
					else
					{
						AddWaterSparks(rat->pos.xPos, r->maxceiling, rat->pos.zPos, 16);
						SetupRipple(rat->pos.xPos, r->maxceiling, rat->pos.zPos, (GetRandomControl() & 3) + 48, 2);
						SoundEffect(SFX_RATSPLASH, &rat->pos, 0);
					}
				}

				if (!i && !(GetRandomControl() & 4))
					SoundEffect(SFX_RATS_1, &rat->pos, 0);
			}
		}
	}
}

/*char __usercall UpdateSpiders@<al > (signed int a1@<eax > )
{
	if (Objects[ID_SPIDER].loaded)
	{
		for (int i = 0; i < 64; i++)
		{
			SPIDER_STRUCT* spider = &Spiders[i];
			if (spider->on)
			{

			}
		}
	}

	LOBYTE(a1) = *(&objects[95] + 50);
	if (*(&objects[95] + 50) & 1)
	{
		v1 = Spiders;
		v23 = 0;
		do
		{
			if (BYTE2(v1[1].yPos))
			{
				LOWORD(a1) = v1->yRot;
				v2 = SHIWORD(v1[1].xPos);
				v3 = v1->yPos;
				v4 = (a1 >> 3) & 0x1FFE;
				v25 = v1->xPos;
				v5 = v1->zPos;
				v6 = v1[1].yPos;
				v1->xPos += v2 * 4 * rcossin_tbl[v4] >> 14;
				v26 = v3;
				v1->yPos = v3 + v6;
				v27 = v5;
				v7 = v5 + (v2 * 4 * rcossin_tbl[v4 + 1] >> 14);
				LOWORD(v1[1].yPos) = v6 + 6;
				v1->zPos = v7;
				v8 = v3 + v6;
				v9 = LaraItem->pos.zPos - v1->zPos;
				v10 = LaraItem->pos.xPos - v1->xPos;
				v11 = LaraItem->pos.yPos - v8;
				v12 = phd_atan(v9, v10) - v1->yRot;
				v24 = v12;
				v13 = abs(v9);
				if (v13 < 85 && abs(v11) < 85 && abs(v10) < 85)
				{
					LaraItem->hit_points -= 3;
					LaraItem->_bf15ea |= 0x10u;
					TriggerBlood(v1->xPos, v1->yPos, v1->zPos, v1->yRot, 1);
					v12 = v24;
				}
				if (HIBYTE(v1[1].yPos))
				{
					if (v13 + abs(v10) <= 768)
					{
						if (v1[1].xPos & 0x10000)
							v1->yRot += 512;
						else
							v1->yRot -= 512;
						HIWORD(v1[1].xPos) = 48 - (abs(v12) >> 10);
					}
					else
					{
						v14 = HIWORD(v1[1].xPos);
						if (v14 < (v23 & 0x1F) + 24)
							HIWORD(v1[1].xPos) = v14 + 1;
						if (abs(v12) >= 2048)
						{
							if (v12 >= 0)
								v1->yRot += 1024;
							else
								v1->yRot -= 1024;
						}
						else
						{
							v1->yRot += 8 * (wibble - v23);
						}
					}
				}
				v15 = v1 + 1;
				v16 = GetFloor(v1->xPos, v1->yPos, v1->zPos, &v1[1]);
				v17 = GetHeight(v16, v1->xPos, v1->yPos, v1->zPos);
				v18 = v1->yPos;
				if (v17 >= v18 - 1280 || v17 == -32512)
				{
					if (v17 >= v18 - 64)
					{
						if (v18 <= v17)
						{
							if (SLOWORD(v1[1].yPos) >= 500)
							{
								BYTE2(v1[1].yPos) = 0;
								NextSpider = 0;
							}
							else
							{
								v1->xRot = -128 * LOWORD(v1[1].yPos);
							}
						}
						else
						{
							v1->yPos = v17;
							LOWORD(v1[1].yPos) = 0;
							HIBYTE(v1[1].yPos) = 1;
						}
					}
					else
					{
						v1->xRot = 14336;
						v1->xPos = v25;
						v1->yPos = v26 - 8;
						if (!(GetRandomControl() & 0x1F))
							v1->yRot += -32768;
						LOWORD(v1[1].yPos) = 0;
						v1->zPos = v27;
					}
				}
				else
				{
					if (v24 <= 0)
						v1->yRot -= 0x4000;
					else
						v1->yRot += 0x4000;
					v1->xPos = v25;
					v1->yPos = v26;
					v1->zPos = v27;
					LOWORD(v1[1].yPos) = 0;
				}
				if (v1->yPos < room[SLOWORD(v15->xPos)].maxceiling + 50)
				{
					v19 = room[SLOWORD(v15->xPos)].maxceiling;
					LOWORD(v1[1].yPos) = 1;
					v1->yRot += -32768;
					v1->yPos = v19 + 50;
				}
				if (!v23 && !(GetRandomControl() & 4))
					SoundEffect(276, v1, 0);
			}
			v1 = (v1 + 26);
			a1 = v23 + 1;
			v21 = __OFSUB__(v23 + 1, 64);
			v20 = v23++ - 63 < 0;
		} while (v20 ^ v21);
	}
	return a1;
}*/

int LOS(GAME_VECTOR* start, GAME_VECTOR* end)
{
	int result1, result2;

	end->roomNumber = start->roomNumber;
	if (abs(end->z - start->z) > abs(end->x - start->x))
	{
		result1 = xLOS(start, end);
		result2 = zLOS(start, end);
	}
	else
	{
		result1 = zLOS(start, end);
		result2 = xLOS(start, end);
	}
	if (result2)
	{
		GetFloor(end->x, end->y, end->z, &end->roomNumber);
		if (ClipTarget(start, end) && result1 == 1 && result2 == 1)
		{
			return 1;
		}
	}
	return 0;
}

int xLOS(GAME_VECTOR* start, GAME_VECTOR* end)
{
	int dx, dy, dz, x, y, z, flag;
	short room, room2;
	FLOOR_INFO* floor;

	dx = end->x - start->x;
	if (!dx)
		return 1;
	dy = (end->y - start->y << 10) / dx;
	dz = (end->z - start->z << 10) / dx;
	number_los_rooms = 1;
	los_rooms[0] = start->roomNumber;
	room = start->roomNumber;
	room2 = start->roomNumber;
	flag = 1;
	if (dx < 0)
	{
		x = start->x & 0xFFFFFC00;
		y = ((x - start->x) * dy >> 10) + start->y;
		z = ((x - start->x) * dz >> 10) + start->z;
		while (x > end->x)
		{
			floor = GetFloor(x, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
			{
				flag = -1;
				break;
			}
			floor = GetFloor(x - 1, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x - 1, y, z) || y < GetCeiling(floor, x - 1, y, z))
			{
				flag = 0;
				break;
			}
			x -= 1024;
			y -= dy;
			z -= dz;
		}
		if (flag != 1)
		{
			end->x = x;
			end->y = y;
			end->z = z;
		}
		end->roomNumber = flag ? room : room2;
	}
	else
	{
		x = start->x | 0x3FF;
		y = ((x - start->x) * dy >> 10) + start->y;
		z = ((x - start->x) * dz >> 10) + start->z;
		while (x < end->x)
		{
			floor = GetFloor(x, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
			{
				flag = -1;
				break;
			}
			floor = GetFloor(x + 1, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x + 1, y, z) || y < GetCeiling(floor, x + 1, y, z))
			{
				flag = 0;
				break;
			}
			x += 1024;
			y += dy;
			z += dz;
		}
		if (flag != 1)
		{
			end->x = x;
			end->y = y;
			end->z = z;
		}
		end->roomNumber = flag ? room : room2;
	}
	return flag;
}

int zLOS(GAME_VECTOR* start, GAME_VECTOR* end)
{
	int dx, dy, dz, x, y, z, flag;
	short room, room2;
	FLOOR_INFO* floor;

	dz = end->z - start->z;
	if (!dz)
		return 1;
	dx = (end->x - start->x << 10) / dz;
	dy = (end->y - start->y << 10) / dz;
	number_los_rooms = 1;
	los_rooms[0] = start->roomNumber;
	room = start->roomNumber;
	room2 = start->roomNumber;
	flag = 1;
	if (dz < 0)
	{
		z = start->z & 0xFFFFFC00;
		x = ((z - start->z) * dx >> 10) + start->x;
		y = ((z - start->z) * dy >> 10) + start->y;
		while (z > end->z)
		{
			floor = GetFloor(x, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
			{
				flag = -1;
				break;
			}
			floor = GetFloor(x, y, z - 1, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x, y, z - 1) || y < GetCeiling(floor, x, y, z - 1))
			{
				flag = 0;
				break;
			}
			z -= 1024;
			x -= dx;
			y -= dy;
		}
		if (flag != 1)
		{
			end->x = x;
			end->y = y;
			end->z = z;
		}
		end->roomNumber = flag ? room : room2;
	}
	else
	{
		z = start->z | 0x3FF;
		x = ((z - start->z) * dx >> 10) + start->x;
		y = ((z - start->z) * dy >> 10) + start->y;
		while (z < end->z)
		{
			floor = GetFloor(x, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
			{
				flag = -1;
				break;
			}
			floor = GetFloor(x, y, z + 1, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x, y, z + 1) || y < GetCeiling(floor, x, y, z + 1))
			{
				flag = 0;
				break;
			}
			z += 1024;
			x += dx;
			y += dy;
		}
		if (flag != 1)
		{
			end->x = x;
			end->y = y;
			end->z = z;
		}
		end->roomNumber = flag ? room : room2;
	}
	return flag;
}

int ClipTarget(GAME_VECTOR* start, GAME_VECTOR* target)
{
	short room;
	int x, y, z, wx, wy, wz;

	room = target->roomNumber;
	if (target->y > GetFloorHeight(GetFloor(target->x, target->y, target->z, &room), target->x, target->y, target->z))
	{
		x = (7 * (target->x - start->x) >> 3) + start->x;
		y = (7 * (target->y - start->y) >> 3) + start->y;
		z = (7 * (target->z - start->z) >> 3) + start->z;
		for (int i = 3; i > 0; --i)
		{
			wx = ((target->x - x) * i >> 2) + x;
			wy = ((target->y - y) * i >> 2) + y;
			wz = ((target->z - z) * i >> 2) + z;
			if (wy < GetFloorHeight(GetFloor(wx, wy, wz, &room), wx, wy, wz))
				break;
		}
		target->x = wx;
		target->y = wy;
		target->z = wz;
		target->roomNumber = room;
		return 0;
	}
	room = target->roomNumber;
	if (target->y < GetCeiling(GetFloor(target->x, target->y, target->z, &room), target->x, target->y, target->z))
	{
		x = (7 * (target->x - start->x) >> 3) + start->x;
		y = (7 * (target->y - start->y) >> 3) + start->y;
		z = (7 * (target->z - start->z) >> 3) + start->z;
		for (int i = 3; i > 0; --i)
		{
			wx = ((target->x - x) * i >> 2) + x;
			wy = ((target->y - y) * i >> 2) + y;
			wz = ((target->z - z) * i >> 2) + z;
			if (wy > GetCeiling(GetFloor(wx, wy, wz, &room), wx, wy, wz))
				break;
		}
		target->x = wx;
		target->y = wy;
		target->z = wz;
		target->roomNumber = room;
		return 0;
	}
	return 1;
}

int GetTargetOnLOS(GAME_VECTOR* src, GAME_VECTOR* dest, int DrawTarget, int firing)
{
	GAME_VECTOR target;
	int result, flag, itemNumber, count;
	MESH_INFO* mesh;
	PHD_VECTOR vector;
	ITEM_INFO* item;
	short angle, room, triggerItems[8];

	target.x = dest->x;
	target.y = dest->y;
	target.z = dest->z;

	result = LOS(src, &target);

	GetFloor(target.x, target.y, target.z, &target.roomNumber);

	if (firing && LaserSight)
	{
		Lara.hasFired = true;
		Lara.fired = true;

		if (Lara.gunType == WEAPON_REVOLVER)
		{
			SoundEffect(SFX_REVOLVER, NULL, 0);
		}
	}

	flag = 0;
	itemNumber = ObjectOnLOS2(src, dest, &vector, &mesh);

	if (itemNumber != 999)
	{
		target.x = vector.x - (vector.x - src->x >> 5);
		target.y = vector.y - (vector.y - src->y >> 5);
		target.z = vector.z - (vector.z - src->z >> 5);
		
		GetFloor(target.x, target.y, target.z, &target.roomNumber);
		
		if (itemNumber >= 0)
			Lara.target = &Items[itemNumber];
		
		if (firing)
		{
			if (Lara.gunType != WEAPON_CROSSBOW)
			{
				if (itemNumber < 0)
				{
					if (mesh->staticNumber >= 50 && mesh->staticNumber < 58)
					{
						ShatterObject(NULL, mesh, 128, target.roomNumber, 0);
						SmashedMeshRoom[SmashedMeshCount] = target.roomNumber;
						SmashedMesh[SmashedMeshCount] = mesh;
						++SmashedMeshCount;
						mesh->Flags &= ~0x1;
						SoundEffect(ShatterSounds[CurrentLevel - 5][mesh->staticNumber], (PHD_3DPOS *) mesh, 0);
					}
					TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, 0);
					TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, 0);
				}
				else
				{
					item = &Items[itemNumber];
					if (item->objectNumber != ID_SHOOT_SWITCH1 && item->objectNumber != ID_SHOOT_SWITCH2)
					{
						if (Objects[item->objectNumber].explodableMeshbits & TargetMesh && LaserSight)
						{
							if (!Objects[item->objectNumber].intelligent)
							{
								ShatterObject(&ShatterItem, 0, 128, target.roomNumber, 0);
								item->meshBits &= ~TargetMesh;
								TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, 0);
							}
							else
							{
								if (item->objectNumber != ID_TWOGUN)
								{
									item->hitPoints -= 30;
									if (item->hitPoints < 0)
										item->hitPoints = 0;
									HitTarget(item, &target, WeaponsArray[Lara.gunType].damage, 0);
								}
								else
								{
									angle = ATAN(LaraItem->pos.zPos - item->pos.zPos, LaraItem->pos.xPos - item->pos.xPos) - item->pos.yRot;
									if (angle > -ANGLE(90) && angle < ANGLE(90))
									{
										item->hitPoints = 0;
										HitTarget(item, &target, WeaponsArray[Lara.gunType].damage, 0);
									}
								}
							}
						}
						else
						{
							if (DrawTarget && (Lara.gunType == WEAPON_REVOLVER || Lara.gunType == WEAPON_HK))
							{
								if (Objects[item->objectNumber].intelligent)
								{
									HitTarget(item, &target, WeaponsArray[Lara.gunType].damage, 0);
								}
								else
								{
									if (Objects[item->objectNumber].hitEffect == 3)
										TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, 0);
								}
							}
							else
							{
								if (item->objectNumber >= ID_SMASH_OBJECT1 && item->objectNumber <= ID_SMASH_OBJECT8)
								{
									SmashObject(itemNumber);
								}
								else
								{
									if (Objects[item->objectNumber].hitEffect == 1)
									{
										DoBloodSplat(target.x, target.y, target.z, (GetRandomControl() & 3) + 3, item->pos.yRot, item->roomNumber);
									}
									else if (Objects[item->objectNumber].hitEffect == 2)
									{
										TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, -5);
									}
									else if (Objects[item->objectNumber].hitEffect == 3)
									{
										TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, 0);
									}
									item->hitStatus = true;
									if (!Objects[item->objectNumber].undead)
									{
										item->hitPoints -= WeaponsArray[Lara.gunType].damage;
									}
								}
							}
						}
					}
					else
					{
						if (TargetMesh == 1 << Objects[item->objectNumber].nmeshes - 1)
						{
							if (!(item->flags & 0x40))
							{
								if (item->objectNumber == ID_SHOOT_SWITCH1)
									ExplodeItemNode(item, Objects[item->objectNumber].nmeshes - 1, 0, 64);
								if (item->triggerFlags == 444 && item->objectNumber == ID_SHOOT_SWITCH2)
								{
									ProcessExplodingSwitchType8(item);
								}
								else
								{
									if (item->flags & IFLAG_ACTIVATION_MASK && (item->flags & IFLAG_ACTIVATION_MASK) != IFLAG_ACTIVATION_MASK)
									{
										room = item->roomNumber;
										GetFloorHeight(GetFloor(item->pos.xPos, item->pos.yPos - 256, item->pos.zPos, &room), item->pos.xPos, item->pos.yPos - 256, item->pos.zPos);
										TestTriggers(TriggerIndex, 1, item->flags & IFLAG_ACTIVATION_MASK);
									}
									else
									{
										for (count = GetSwitchTrigger(item, triggerItems, 1); count > 0; --count)
										{
											AddActiveItem(triggerItems[count - 1]);
											Items[triggerItems[count - 1]].status = ITEM_ACTIVE;
											Items[triggerItems[count - 1]].flags |= IFLAG_ACTIVATION_MASK;
										}
									}
								}
							}
							if (item->status != ITEM_DEACTIVATED)
							{
								AddActiveItem(itemNumber);
								item->status = ITEM_ACTIVE;
								item->flags |= IFLAG_ACTIVATION_MASK | 0x40;
							}
						}
						TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, 0);
					}
				}
			}
			else
			{
				if (LaserSight && itemNumber >= 0)
				{
					item = &Items[itemNumber];
					if (item->objectNumber == ID_ENEMY_JEEP && item->meshBits & 1)
					{
						/* @FIXME This turns the LaserSight sprite of the Grappling Gun green and calls FireGrapplingBoltFromLasersight() */
					}
				}
			}
		}
		else
		{
			if (itemNumber >= 0)
			{
				item = &Items[itemNumber];
				if (item->objectNumber == ID_ENEMY_JEEP && Lara.gunType == WEAPON_CROSSBOW && item->meshBits & 1)
				{
					/* @FIXME This turns the LaserSight sprite of the Grappling Gun green */
				}
			}
		}
		flag = 1;
	}
	else
	{
		if (Lara.gunType != WEAPON_CROSSBOW)
		{
			target.x -= target.x - src->x >> 5;
			target.y -= target.y - src->y >> 5;
			target.z -= target.z - src->z >> 5;
			if (firing && !result)
				TriggerRicochetSpark(&target, LaraItem->pos.yRot, 8, 0);
		}
		else
		{
			if (firing && LaserSight)
			{
				/* @FIXME This calls FireGrapplingBoltFromLasersight() */
			}
		}
	}

	if (DrawTarget && (flag || !result))
	{
		TriggerDynamicLight(target.x, target.y, target.z, 64, 255, 0, 0);
		LaserSightActive = 1;
		LaserSightX = target.x;
		LaserSightY = target.y;
		LaserSightZ = target.z;
	}

	return flag;
}

int ObjectOnLOS2(GAME_VECTOR* start, GAME_VECTOR* end, PHD_VECTOR* vec, MESH_INFO** mesh)
{
	int r, m;
	ROOM_INFO* room;
	short linknum, *box;
	ITEM_INFO* item;
	PHD_3DPOS pos;
	MESH_INFO* meshp;

	ClosestItem = 999;
	ClosestDist = SQUARE(end->x - start->x) 
		+ SQUARE(end->y - start->y) 
		+ SQUARE(end->z - start->z);

	for (r = 0; r < number_los_rooms; ++r)
	{
		room = &Rooms[los_rooms[r]];

		for (m = 0; m < room->numMeshes; m++)
		{
			meshp = &room->mesh[m];

			if (meshp->Flags & 1)
			{
				pos.xPos = meshp->x;
				pos.yPos = meshp->y;
				pos.zPos = meshp->z;
				pos.yRot = meshp->yRot;

				if (DoRayBox(start, end, &StaticObjects[meshp->staticNumber].xMinc, &pos, vec, -1 - meshp->staticNumber))
				{
					*mesh = meshp;
					end->roomNumber = los_rooms[r];
				}
			}
		}

		for (linknum = room->itemNumber; linknum != NO_ITEM; linknum = Items[linknum].nextItem)
		{
			item = &Items[linknum];

			if (item->status != ITEM_DEACTIVATED 
				&& item->status != ITEM_INVISIBLE 
				&& (item->objectNumber != ID_LARA && Objects[item->objectNumber].collision != NULL
				|| item->objectNumber == ID_LARA && GetLaraOnLOS))
			{
				box = GetBoundsAccurate(item);

				pos.xPos = item->pos.xPos;
				pos.yPos = item->pos.yPos;
				pos.zPos = item->pos.zPos;
				pos.yRot = item->pos.yRot;

				if (DoRayBox(start, end, box, &pos, vec, linknum))
				{
					end->roomNumber = los_rooms[r];
				}
			}
		}
	}

	vec->x = ClosestCoord.x;
	vec->y = ClosestCoord.y;
	vec->z = ClosestCoord.z;

	return ClosestItem;
}

int GetRandomControl()
{
	rand_1 = 1103515245 * rand_1 + 12345;
	return rand_1 >> 10 & 0x7FFF;
}

void SeedRandomControl(int seed)
{
	rand_1 = seed;
}

int GetRandomDraw()
{
	rand_2 = 1103515245 * rand_2 + 12345;
	return rand_2 >> 10 & 0x7FFF;
}

void SeedRandomDraw(int seed)
{
	rand_2 = seed;
}

int GetCeiling(FLOOR_INFO* floor, int x, int y, int z)
{
	ROOM_INFO* room;
	FLOOR_INFO* floor2;
	int ceiling, t0, t1, t2, t3, dx, dz, xOff, yOff;
	short type, type2, function, cadj, trigger, *data;
	bool end;
	ITEM_INFO* item;

	floor2 = floor;
	while (floor2->skyRoom != NO_ROOM)
	{
		if (CheckNoColCeilingTriangle(floor2, x, z) == 1)
			break;
		room = &Rooms[floor2->skyRoom];
		floor2 = &XZ_GET_SECTOR(room, x - room->x, z - room->z);
	}
	ceiling = 256 * floor2->ceiling;
	if (ceiling != NO_HEIGHT)
	{
		if (floor2->index)
		{
			data = &FloorData[floor2->index];
			type = *data;
			function = type & DATA_TYPE;
			++data;
			end = false;
			if (function == TILT_TYPE || function == SPLIT1 || function == SPLIT2 || function == NOCOLF1T || function == NOCOLF1B || function == NOCOLF2T || function == NOCOLF2B)
			{
				++data;
				if (type & END_BIT)
					end = true;
				type = *data;
				function = type & DATA_TYPE;
				++data;
			}
			if (!end)
			{
				xOff = 0;
				yOff = 0;
				if (function != ROOF_TYPE)
				{
					if (function == SPLIT3 || function == SPLIT4 || function == NOCOLC1T || function == NOCOLC1B || function == NOCOLC2T || function == NOCOLC2B)
					{
						dx = x & 0x3FF;
						dz = z & 0x3FF;
						t0 = -(*data & DATA_TILT);
						t1 = -(*data >> 4 & DATA_TILT);
						t2 = -(*data >> 8 & DATA_TILT);
						t3 = -(*data >> 12 & DATA_TILT);
						if (function == SPLIT3 || function == NOCOLC1T || function == NOCOLC1B)
						{
							if (dx <= 1024 - dz)
							{
								cadj = type >> 10 & DATA_TYPE;
								if (cadj & 0x10)
									cadj |= 0xFFF0;
								ceiling += 256 * cadj;
								xOff = t2 - t1;
								yOff = t3 - t2;
							}
							else
							{
								cadj = type >> 5 & DATA_TYPE;
								if (cadj & 0x10)
									cadj |= 0xFFF0;
								ceiling += 256 * cadj;
								xOff = t3 - t0;
								yOff = t0 - t1;
							}
						}
						else
						{
							if (dx <= dz)
							{
								cadj = type >> 10 & DATA_TYPE;
								if (cadj & 0x10)
									cadj |= 0xFFF0;
								ceiling += 256 * cadj;
								xOff = t2 - t1;
								yOff = t0 - t1;
							}
							else
							{
								cadj = type >> 5 & DATA_TYPE;
								if (cadj & 0x10)
									cadj |= 0xFFF0;
								ceiling += 256 * cadj;
								xOff = t3 - t0;
								yOff = t3 - t2;
							}
						}
					}
				}
				else
				{
					xOff = *data >> 8;
					yOff = *(char *)data;
				}
				if (xOff < 0)
				{
					ceiling += (z & 0x3FF) * xOff >> 2;
				}
				else
				{
					ceiling -= (-1 - z & 0x3FF) * xOff >> 2;
				}
				if (yOff < 0)
				{
					ceiling += (-1 - x & 0x3FF) * yOff >> 2;
				}
				else
				{
					ceiling -= (x & 0x3FF) * yOff >> 2;
				}
			}
		}
		while (floor->pitRoom != NO_ROOM)
		{
			if (CheckNoColFloorTriangle(floor, x, z) == 1)
				break;
			room = &Rooms[floor->pitRoom];
			floor = &XZ_GET_SECTOR(room, x - room->x, z - room->z);
		}
		if (floor->index)
		{
			data = &FloorData[floor->index];
			do
			{
				type = *data;
				function = type & DATA_TYPE;
				++data;
				switch (function)
				{
				case DOOR_TYPE:
				case TILT_TYPE:
				case ROOF_TYPE:
				case SPLIT1:
				case SPLIT2:
				case SPLIT3:
				case SPLIT4:
				case NOCOLF1T:
				case NOCOLF1B:
				case NOCOLF2T:
				case NOCOLF2B:
				case NOCOLC1T:
				case NOCOLC1B:
				case NOCOLC2T:
				case NOCOLC2B:
					++data;
					break;
				case TRIGGER_TYPE:
					++data;
					do
					{
						type2 = *data;
						trigger = TRIG_BITS(type2);
						++data;
						if (trigger != TO_OBJECT)
						{
							if (trigger == TO_CAMERA || trigger == TO_FLYBY)
							{
								type2 = *data;
								++data;
							}
						}
						else
						{
							item = &Items[type2 & VALUE_BITS];
							if (Objects[item->objectNumber].ceiling && !(item->flags & 0x8000))
							{
								Objects[item->objectNumber].ceiling(item, x, y, z, &ceiling);
							}
						}
					}
					while (!(type2 & END_BIT));
				}
			}
			while (!(type & END_BIT));
		}
	}
	return ceiling;
}

PHD_VECTOR* NormalizeVector(PHD_VECTOR* vec)
{
	int x = vec->x >> 16;
	int y = vec->y >> 16;
	int z = vec->z >> 16;

	Vector3 v = Vector3(x, y, z);
	v.Normalize();

	vec->x = (int)(v.x * 16384);
	vec->y = (int)(v.y * 16384);
	vec->z = (int)(v.z * 16384);

	/*
	if (!x && !y && !z)
		return vec;

	int length = abs(SQUARE(x) + SQUARE(y) + SQUARE(z));

	length = 0x1000000 / (SQRT_ASM(length) << 16 >> 8) << 8 >> 8;

	vec->x = vec->x * length >> 16;
	vec->y = vec->y * length >> 16;
	vec->z = vec->z * length >> 16;
	*/

	return vec;
}

int DoRayBox(GAME_VECTOR* start, GAME_VECTOR* end, short* box, PHD_3DPOS* itemOrStaticPos, PHD_VECTOR* hitPos, short closesItemNumber)
{
	PHD_VECTOR p1, p2;

	p1.x = box[0] << 16;
	p2.x = box[1] << 16;
	p1.y = box[2] << 16;
	p2.y = box[3] << 16;
	p1.z = box[4] << 16;
	p2.z = box[5] << 16;

	int dx2 = end->x - itemOrStaticPos->xPos;
	int dy2 = end->y - itemOrStaticPos->yPos;
	int dz2 = end->z - itemOrStaticPos->zPos;

	phd_PushUnitMatrix();

	phd_RotY(-itemOrStaticPos->yRot);

	int a1 = MatrixPtr[M00] * dx2 + 
		MatrixPtr[M01] * dy2 + 
		MatrixPtr[M02] * dz2;

	int a2 = MatrixPtr[M10] * dx2 +
		MatrixPtr[M11] * dy2 +
		MatrixPtr[M12] * dz2;

	int a3 = MatrixPtr[M20] * dx2 +
		MatrixPtr[M21] * dy2 +
		MatrixPtr[M22] * dz2;

	int dx1 = start->x - itemOrStaticPos->xPos;
	int dy1 = start->y - itemOrStaticPos->yPos;
	int dz1 = start->z - itemOrStaticPos->zPos;

	int b1 = MatrixPtr[M00] * dx1 +
		MatrixPtr[M01] * dy1 +
		MatrixPtr[M02] * dz1;

	int b2 = MatrixPtr[M10] * dx1 +
		MatrixPtr[M11] * dy1 +
		MatrixPtr[M12] * dz1;

	int b3 = MatrixPtr[M20] * dx1 +
		MatrixPtr[M21] * dy1 +
		MatrixPtr[M22] * dz1;

	phd_PopMatrix();

	PHD_VECTOR vec;

	vec.x = ((a1 >> W2V_SHIFT) - (b1 >> W2V_SHIFT)) << 16;
	vec.y = ((a2 >> W2V_SHIFT) - (b2 >> W2V_SHIFT)) << 16;
	vec.z = ((a3 >> W2V_SHIFT) - (b3 >> W2V_SHIFT)) << 16;

	NormalizeVector(&vec);

	PHD_VECTOR pb;
	pb.x = b1 >> W2V_SHIFT << 16;
	pb.y = b2 >> W2V_SHIFT << 16;
	pb.z = b3 >> W2V_SHIFT << 16;

	vec.x <<= 8;
	vec.y <<= 8;
	vec.z <<= 8;

	if (!DoRayBox_sub_401523(&p1, &p2, &pb, &vec, hitPos))
		return 0;

	if (hitPos->x < box[0] 
		|| hitPos->y < box[2] 
		|| hitPos->z < box[4] 
		|| hitPos->x > box[1] 
		|| hitPos->y > box[3]
		|| hitPos->z > box[5])
		return 0;

	phd_PushUnitMatrix();

	phd_RotY(itemOrStaticPos->yRot);
	
	int c1 = MatrixPtr[M00] * hitPos->x +
		MatrixPtr[M01] * hitPos->y +
		MatrixPtr[M02] * hitPos->z;

	int c2 = MatrixPtr[M10] * hitPos->x +
		MatrixPtr[M11] * hitPos->y +
		MatrixPtr[M12] * hitPos->z;

	int c3 = MatrixPtr[M20] * hitPos->x +
		MatrixPtr[M21] * hitPos->y +
		MatrixPtr[M22] * hitPos->z;

	hitPos->x = (c1 >> W2V_SHIFT);
	hitPos->y = (c2 >> W2V_SHIFT);
	hitPos->z = (c3 >> W2V_SHIFT);
	
	phd_PopMatrix();
	
	short* meshPtr = NULL;
	int bit = 1;
	int sp = -2;
	int minDistance = 0x7FFFFFFF;

	if (closesItemNumber < 0)
	{
		sp = -1;
	}
	else
	{
		ITEM_INFO* item = &Items[closesItemNumber];
		OBJECT_INFO* obj = &Objects[item->objectNumber];

		GetSpheres(item, SphereList, 1);

		if (obj->nmeshes <= 0)
			return 0;

		meshPtr = Meshes[obj->meshIndex];

		for (int i = 0; i < obj->nmeshes; i++)
		{
			if (item->meshBits & (1 << i))
			{
				SPHERE* sphere = &SphereList[i];

				if (item->meshBits & (1 << i))
				{
					SPHERE* sphere = &SphereList[i];

					int dx = end->x - start->x;
					int dy = end->y - start->y;
					int dz = end->z - start->z;

					int d1 = dx * (sphere->x - start->x) + dy * (sphere->y - start->y) + dz * (sphere->z - start->z);
					int d2 = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);

					if ((d1 < 0 && d2 < 0) || (d1 > 0 && d2 > 0) || abs(d1) <= abs(d2))
					{
						int l;
						if (d2 >> 16)
							l = d1 / (d2 >> 16);
						else
							l = 0;

						int x = start->x + (l * dx >> 16);
						int y = start->y + (l * dy >> 16);
						int z = start->z + (l * dz >> 16);

						int d = SQUARE(x - sphere->x) + SQUARE(y - sphere->y) + SQUARE(z - sphere->z);

						if (d <= SQUARE(sphere->r))
						{
							int newDist = SQUARE(sphere->x - start->x) + SQUARE(sphere->y - start->y) + SQUARE(sphere->z - start->z);

							if (newDist < minDistance)
							{
								minDistance = newDist;
								meshPtr = Meshes[obj->meshIndex + 2 * i];
								bit = 1 << i;
								sp = i;
							}
						}
					}
				}
			}
		}

		if (sp < -1)
			return 0;
	}

	int distance = SQUARE(hitPos->x + itemOrStaticPos->xPos - start->x) 
		+ SQUARE(hitPos->y + itemOrStaticPos->yPos - start->y)
		+ SQUARE(hitPos->z + itemOrStaticPos->zPos - start->z);

	if (distance >= ClosestDist)
		return 0;
	
	ClosestCoord.x = hitPos->x + itemOrStaticPos->xPos;
	ClosestCoord.y = hitPos->y + itemOrStaticPos->yPos;
	ClosestCoord.z = hitPos->z + itemOrStaticPos->zPos;
	ClosestDist = distance;
	ClosestItem = closesItemNumber;

	if (sp >= 0)
	{
		ITEM_INFO* item = &Items[closesItemNumber];

		GetSpheres(item, SphereList, 3);

		ShatterItem.yRot = item->pos.yRot;
		ShatterItem.meshp = meshPtr;
		ShatterItem.sphere.x = SphereList[sp].x;
		ShatterItem.sphere.y = SphereList[sp].y;
		ShatterItem.sphere.z = SphereList[sp].z;
		ShatterItem.bit = bit;
		ShatterItem.flags = 0;
	}
	return 1;
}

void AnimateItem(ITEM_INFO* item)
{
	item->touchBits = 0;
	item->hitStatus = false;

	item->frameNumber++;

	ANIM_STRUCT* anim = &Anims[item->animNumber];
	if (anim->numberChanges > 0 && GetChange(item, anim))
	{
		anim = &Anims[item->animNumber];

		item->currentAnimState = anim->currentAnimState;

		if (item->requiredAnimState == item->currentAnimState)
			item->requiredAnimState = 0;
	}

	if (item->frameNumber > anim->frameEnd)
	{
		if (anim->numberCommands > 0)
		{
			short* cmd = &Commands[anim->commandIndex];
			for (int i = anim->numberCommands; i > 0; i--)
			{
				switch (*(cmd++))
				{
				case COMMAND_MOVE_ORIGIN:
					TranslateItem(item, cmd[0], cmd[1], cmd[2]);
					cmd += 3;
					break;

				case COMMAND_JUMP_VELOCITY:
					item->fallspeed = *(cmd++);
					item->speed = *(cmd++);
					item->gravityStatus = true;
					break;

				case COMMAND_DEACTIVATE:
					if (Objects[item->objectNumber].intelligent && !item->afterDeath)
						item->afterDeath = 1;
					item->status = ITEM_DEACTIVATED;
					break;

				case COMMAND_SOUND_FX:
				case COMMAND_EFFECT:
					cmd += 2;
					break;

				default:
					break;
				}
			}
		}

		item->animNumber = anim->jumpAnimNum;
		item->frameNumber = anim->jumpFrameNum;

		anim = &Anims[item->animNumber];
		
		if (item->currentAnimState != anim->currentAnimState)
		{
			item->currentAnimState = anim->currentAnimState;
			item->goalAnimState = anim->currentAnimState;
		}

		if (item->requiredAnimState == item->currentAnimState)
			item->requiredAnimState = 0;
	}

	if (anim->numberCommands > 0)
	{
		short* cmd = &Commands[anim->commandIndex];
		int flags;

		for (int i = anim->numberCommands; i > 0; i--)
		{
			switch (*(cmd++))
			{
			case COMMAND_MOVE_ORIGIN:
				cmd += 3;
				break;

			case COMMAND_JUMP_VELOCITY:
				cmd += 2;
				break;

			case COMMAND_SOUND_FX:
				if (item->frameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				flags = cmd[1] & 0xC000;

				if (!Objects[item->objectNumber].waterCreature)
				{
					if (item->roomNumber == NO_ROOM)
					{
						item->pos.xPos = LaraItem->pos.xPos;
						item->pos.yPos = LaraItem->pos.yPos - 762;
						item->pos.zPos = LaraItem->pos.zPos;
						
						SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
					}
					else if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
					{
						if (!flags 
							|| flags == SFX_WATERONLY 
							&& (Rooms[Camera.pos.roomNumber].flags & ENV_FLAG_WATER 
								|| Objects[item->objectNumber].intelligent))
						{
							SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
						}
					}
					else if (!flags || flags == SFX_LANDONLY && !(Rooms[Camera.pos.roomNumber].flags & ENV_FLAG_WATER))
					{
						SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
					}
				}
				else
				{
					if (Rooms[Camera.pos.roomNumber].flags & ENV_FLAG_WATER)
						SoundEffect(cmd[1] & 0x3FFF, &item->pos, 1);
					else
						SoundEffect(cmd[1] & 0x3FFF, &item->pos, 0);
				}
				break;

			case COMMAND_EFFECT:
				if (item->frameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				FXType = cmd[1] & 0xC000;
				(*effect_routines[(int)(cmd[1] & 0x3fff)])(item);

				cmd += 2;
				break;

			default:
				break;
			}
		}
	}

	int lateral = 0;

	if (item->gravityStatus)
	{
		item->fallspeed += (item->fallspeed >= 128 ? 1 : 6);
		item->pos.yPos += item->fallspeed;
	}
	else
	{
		int velocity = anim->velocity;
		if (anim->acceleration)
			velocity += anim->acceleration * (item->frameNumber - anim->frameBase);
		item->speed = velocity >> 16;

		lateral = anim->Xvelocity;
		if (anim->Xacceleration)
			lateral += anim->Xacceleration * (item->frameNumber - anim->frameBase);

		lateral >>= 16;
	}

	item->pos.xPos += item->speed * SIN(item->pos.yRot) >> W2V_SHIFT;
	item->pos.zPos += item->speed * COS(item->pos.yRot) >> W2V_SHIFT;

	item->pos.xPos += lateral * SIN(item->pos.yRot + ANGLE(90)) >> W2V_SHIFT;
	item->pos.zPos += lateral * COS(item->pos.yRot + ANGLE(90)) >> W2V_SHIFT;
}

void DoFlipMap(short group)
{
	ROOM_INFO temp ;
	
	for (int i = 0; i < NumberRooms; i++)
	{
		ROOM_INFO* r = &Rooms[i];

		if (r->flippedRoom >= 0 && r->flipNumber == group)
		{
			RemoveRoomFlipItems(r);

			ROOM_INFO* flipped = &Rooms[r->flippedRoom];
			
			memcpy(&temp, r, sizeof(temp));
			memcpy(r, flipped, sizeof(ROOM_INFO));
			memcpy(flipped, &temp, sizeof(ROOM_INFO));
			
			r->flippedRoom = flipped->flippedRoom;
			flipped->flippedRoom = -1;

			r->itemNumber = flipped->itemNumber;
			r->fxNumber = flipped->fxNumber;
			
			AddRoomFlipItems(r);

			g_Renderer->FlipRooms(i, r->flippedRoom);
		}
	}

	int status = FlipStats[group] == 0;
	FlipStats[group] = status;
	FlipStatus = status;
	
	for (int i = 0; i < NUM_SLOTS; i++)
	{
		BaddieSlots[i].LOT.targetBox = NO_BOX;
	}
}

void AddRoomFlipItems(ROOM_INFO* r)
{
	for (short linkNum = r->itemNumber; linkNum != NO_ITEM; linkNum = Items[linkNum].nextItem)
	{
		ITEM_INFO* item = &Items[linkNum];

		if (item->objectNumber == ID_RAISING_BLOCK1 && item->itemFlags[1])
			AlterFloorHeight(item, -1024);
		
		if (item->objectNumber == ID_RAISING_BLOCK2)
		{
			if (item->itemFlags[1])
				AlterFloorHeight(item, -2048);
		}
	}
}

void RemoveRoomFlipItems(ROOM_INFO* r)
{
	for (short linkNum = r->itemNumber; linkNum != NO_ITEM; linkNum = Items[linkNum].nextItem)
	{
		ITEM_INFO* item = &Items[linkNum];

		if (item->flags & 0x100
			&& Objects[item->objectNumber].intelligent
			&& item->hitPoints <= 0
			&& item->hitPoints != -16384)
		{
			KillItem(linkNum);
		}
	}
}

void PlaySoundTrack(short track, short flags)
{
	S_CDPlayEx(track, flags, 0);
}

void RumbleScreen()
{
	if (!(GlobalCounter & 0x1FF))
		SoundEffect(SFX_KLAXON, 0, 4104);

	if (RumbleTimer >= 0)
		RumbleTimer++;

	if (RumbleTimer > 450)
	{
		if (!(GetRandomControl() & 0x1FF))
		{
			InGameCnt = 0;
			RumbleTimer = -32 - (GetRandomControl() & 0x1F);
			return;
		}
	}

	if (RumbleTimer < 0)
	{
		if (InGameCnt >= abs(RumbleTimer))
		{
			Camera.bounce = -(GetRandomControl() % abs(RumbleTimer));
			RumbleTimer++;
		}
		else
		{
			InGameCnt++;
			Camera.bounce = -(GetRandomControl() % InGameCnt);
		}
	}
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

				if ((Camera.timer < 0)
					|| (Camera.type == LOOK_CAMERA)
					|| (Camera.type == COMBAT_CAMERA))
				{
					Camera.timer = -1;
					targetOk = 0;
					break;
				}
				Camera.type = FIXED_CAMERA;
				targetOk = 1;
			}
			else
				targetOk = 0;
			break;

		case TO_TARGET:
			if (Camera.type == LOOK_CAMERA || Camera.type == COMBAT_CAMERA)
				break;

			Camera.item = &Items[value];
			break;
		}
	} while (!(trigger & END_BIT));

	if (Camera.item)
		if (!targetOk || (targetOk == 2 && Camera.item->lookedAt && Camera.item != Camera.lastItem))
			Camera.item = NULL;

	if (Camera.number == -1 && Camera.timer > 0)
		Camera.timer = -1;
}

int ExplodeItemNode(ITEM_INFO* item, int Node, int NoXZVel, int bits)
{
	short Num;

	if (1 << Node & item->meshBits)
	{
		Num = bits;
		if (item->objectNumber == ID_SHOOT_SWITCH1 && (CurrentLevel == 4 || CurrentLevel == 7))
		{
			SoundEffect(SFX_SMASH_METAL, &item->pos, 0);
		}
		else if (Num == 256)
		{
			Num = -64;
		}
		GetSpheres(item, SphereList, 3);
		ShatterItem.yRot = item->pos.yRot;
		ShatterItem.bit = 1 << Node;
		ShatterItem.meshp = Meshes[Objects[item->objectNumber].meshIndex + 2 * Node];
		ShatterItem.sphere.x = SphereList[Node].x;
		ShatterItem.sphere.y = SphereList[Node].y;
		ShatterItem.sphere.z = SphereList[Node].z;
		ShatterItem.il = (ITEM_LIGHT *) &item->legacyLightData;
		ShatterItem.flags = item->objectNumber == ID_CROSSBOW_BOLT ? 0x400 : 0;
		ShatterObject(&ShatterItem, 0, Num, item->roomNumber, NoXZVel);
		item->meshBits &= ~ShatterItem.bit;
		return 1;
	}
	return 0;
}

int TriggerActive(ITEM_INFO* item)
{
	int flag;

	flag = item->flags & IFLAG_REVERSE ? 0 : 1;
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

int GetWaterHeight(int x, int y, int z, short roomNumber)
{
	ROOM_INFO* r = &Rooms[roomNumber];
	FLOOR_INFO* floor;
	short adjoiningRoom = NO_ROOM;

	do 
	{
		int xBlock = (x - r->x) >> WALL_SHIFT;
		int zBlock = (z - r->z) >> WALL_SHIFT;

		if (zBlock <= 0)
		{
			zBlock = 0;
			if (xBlock < 1)
				xBlock = 1;
			else if (xBlock > r->ySize - 2)
				xBlock = r->ySize - 2;
		}
		else if (zBlock >= r->xSize - 1)
		{
			zBlock = r->xSize - 1;
			if (xBlock < 1)
				xBlock = 1;
			else if (xBlock > r->ySize - 2)
				xBlock = r->ySize - 2;
		}
		else if (xBlock < 0)
			xBlock = 0;
		else if (xBlock >= r->ySize)
			xBlock = r->ySize - 1;

		floor = &r->floor[zBlock + xBlock * r->xSize];
		adjoiningRoom = GetDoor(floor);

		if (adjoiningRoom != NO_ROOM)
		{
			roomNumber = adjoiningRoom;
			r = &Rooms[adjoiningRoom];
		}
	} while (adjoiningRoom != NO_ROOM);

	if (r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
	{
		while (floor->skyRoom != NO_ROOM)
		{
			if (CheckNoColCeilingTriangle(floor, x, z) == 1)
				break;
			r = &Rooms[floor->skyRoom];
			if (!(r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP)))
				return r->minfloor;
			floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);
			if (floor->skyRoom == NO_ROOM)
				break;
		}
		
		return r->maxceiling;
	}
	else
	{
		while (floor->pitRoom != NO_ROOM)
		{
			if (CheckNoColFloorTriangle(floor, x, z) == 1)
				break;
			r = &Rooms[floor->pitRoom];
			if (r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
				return r->maxceiling;
			floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);
			if (floor->pitRoom == NO_ROOM)
				break;
		}
	}

	return NO_HEIGHT;
}

void Inject_Control()
{
	INJECT(0x00416760, TestTriggers);
	INJECT(0x004167B0, TestTriggers);
	INJECT(0x00415960, TranslateItem);
	INJECT(0x00415B20, GetFloor);
	INJECT(0x00417640, GetCeiling);
	INJECT(0x004A7C10, GetRandomControl);
	INJECT(0x004A7C40, GetRandomDraw);
	INJECT(0x004A7C70, SeedRandomControl);
	INJECT(0x004A7C90, SeedRandomDraw);
	INJECT(0x00415300, AnimateItem);
	INJECT(0x0041A170, GetTargetOnLOS);
	INJECT(0x00415DA0, GetWaterHeight);
}