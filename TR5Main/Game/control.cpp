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

#include "..\Specific\roomload.h"
#include "..\Specific\input.h"
#include "..\Specific\init.h"
#include "..\Specific\winmain.h"

#include <process.h>
#include <stdio.h>

int KeyTriggerActive;
PENDULUM CurrentPendulum;

extern GameFlow* g_GameFlow;
extern GameScript* g_GameScript;
extern Inventory* g_Inventory;

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
						Lara.isDucked = false;
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
		ClearDynamics();
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
			if (Objects[Effects[fxNum].objectNumber].control)
				Objects[Effects[fxNum].objectNumber].control(fxNum);
			fxNum = nextFx;
		}

		InItemControlLoop = false;
		//KillMoveEffects();

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
			if (CurrentLevel != 0)
				g_Renderer->EnableCinematicBars(true);
			CalculateSpotCameras();
		}
		else
		{
			// Do the standard camera
			g_Renderer->EnableCinematicBars(false);
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

			if ((item->flags & 0x3E00) & 0x3E00)
			{
				item->flags |= 0x20;

				if (flags & 0x100)
					item->flags |= 1;

				if (!(item->active))
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
				if (!FlipStatus)
					flip = value;
			}
			else if (FlipStatus)
				flip = value;
			break;

		case TO_FLIPON:
			flipAvailable = true;
			if ((FlipMap[value] & 0x3E00) == 0x3E00 && !FlipStatus)
				flip = value;
			break;

		case TO_FLIPOFF:
			flipAvailable = true;
			if ((FlipMap[value] & 0x3E00) == 0x3E00 && FlipStatus)
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

void ActivateCamera()
{
	KeyTriggerActive = 2;
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

int CheckNoColFloorTriangle(FLOOR_INFO* floor, short x, short z)
{
	if (!floor->index)
		return 0;

	short func = FloorData[floor->index] & 0x1F;
	if (func != NOCOLF1T && func != NOCOLF1B && func != NOCOLF2T && func != NOCOLF2B)
		return 0;

	int dx = x & 0x3FF;
	int dz = z & 0x3FF;

	switch (func)
	{
	case NOCOLF1T:
		if (dx <= 1024 - dz)
			return -1;
		break;

	case NOCOLF1B:
		if (dx > 1024 - dz)
			return -1;
		break;

	case NOCOLF2T:
		if (dx <= dz)
			return -1;
		break;

	case NOCOLF2B:
		if (dx > dz)
			return -1;
		break;

	}

	return 1;
}

int GetWaterSurface(int x, int y, int z, short roomNumber)
{
	ROOM_INFO* room = &Rooms[roomNumber];
	FLOOR_INFO* floor = &room->floor[((z - room->z) >> WALL_SHIFT) + ((x - room->x) >> WALL_SHIFT)* room->xSize];

	if (room->flags & ENV_FLAG_WATER)
	{
		while (floor->skyRoom != NO_ROOM)
		{
			room = &Rooms[floor->skyRoom];
			if (!(room->flags & ENV_FLAG_WATER))
				return (floor->ceiling << 8);
			floor = &room->floor[((z - room->z) >> WALL_SHIFT) + ((x - room->x) >> WALL_SHIFT)* room->xSize];
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
			floor = &room->floor[((z - room->z) >> WALL_SHIFT) + ((x - room->x) >> WALL_SHIFT)* room->xSize];
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
	int flag = 0;
	if (abs(height))
	{
		flag = 1;
		if (height >= 0)
			height++;
		else
			height--;
	}

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	FLOOR_INFO* ceiling = GetFloor(item->pos.xPos, height + item->pos.yPos - 1024, item->pos.zPos, &roomNumber);
	
	if (floor->floor == -127)
	{
		floor->floor = ceiling->ceiling + (((height >> 31) + height) >> 8);
	}
	else
	{
		floor->floor += (((height >> 31) + height) >> 8);
		if (floor->floor == ceiling->ceiling && !flag)
			floor->floor = -127;
	}

	BOX_INFO* box = &Boxes[floor->box];
	if (box->overlapIndex & 0x8000)
	{
		if (height >= 0)
			box->overlapIndex &= ~0x4000;
		else
			box->overlapIndex |= 0x4000;
	}
}

FLOOR_INFO* GetFloor(int x, signed int y, int z, short* roomNumber)
{
	FLOOR_INFO* floor;

	int xFloor = 0;
	int yFloor = 0;

	short roomDoor = 0;

	ROOM_INFO* r = &Rooms[*roomNumber];

	short data;
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

		floor = &r->floor[xFloor + yFloor * r->xSize];
		data = GetDoor(floor);
		if (data != NO_ROOM)
		{
			*roomNumber = data;
			r = &Rooms[*roomNumber];
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

		if (type == NOCOLF1T && dx <= (1024 - dz))
			return -1;
		else if (type == NOCOLF1B && dx > (1024 - dz))
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

		if (type == NOCOLC1T && dx <= (1024 - dz))
			return -1;
		else if (type == NOCOLC1B && dx > (1024 - dz))
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
	HeightType = 0;

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
	short type;

	int xOff, yOff, trigger;
	ITEM_INFO* item;
	OBJECT_INFO* obj;
	int tilts, t0, t1, t2, t3, t4, hadj, dx, dz, h1, h2;

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
			TiltXOffset = xOff = (*data >> 8) & 0xFF;
			TiltYOffset = yOff = *data & 0xFF;

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

void Inject_Control()
{
	INJECT(0x00416760, TestTriggers);
	INJECT(0x004167B0, TestTriggers);
	INJECT(0x00415960, TranslateItem);
	INJECT(0x00415B20, GetFloor);
}