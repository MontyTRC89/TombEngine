#include "control.h"

#include "..\Global\global.h"

#include "pickup.h"
#include "spotcam.h"
#include "camera.h"
#include "lara.h"
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

#include "..\Specific\roomload.h"
#include "..\Specific\input.h"
#include "..\Specific\init.h"
#include "..\Specific\winmain.h"

#include <process.h>
#include <stdio.h>

extern GameScript* g_Script;

GAME_STATUS __cdecl ControlPhase(__int32 numFrames, __int32 demoMode)
{
	GameScriptLevel* level = g_Script->GetLevel(CurrentLevel);

	RegeneratePickups();

	if (numFrames > 10)
		numFrames = 10;

	if (TrackCameraInit)
		UseSpotCam = 0;

	SetDebounce = 1;

	for (FramesCount += numFrames; FramesCount > 0; FramesCount -= 2)
	{
		GlobalCounter++;

		UpdateSky();
		
		// Poll the keyboard and update input variables
		if (S_UpdateInput() == -1)
			return GAME_STATUS::GAME_STATUS_NONE;
		
		// Has Lara control been disabled?
		if (DisableLaraControl && false)
		{
			if (CurrentLevel != 0)
				DbInput = 0;
			TrInput &= 0x200;
		}

		// If cutscene has been triggered then clear input
		if (CutSeqTriggered)
			TrInput = 0;

		// Does the player want to enter inventory?
		SetDebounce = 0;
		if (CurrentLevel != 0 || true)
		{
			if ((DbInput & 0x200000 || GlobalEnterInventory != -1) && !CutSeqTriggered && LaraItem->hitPoints > 0)
			{ 
				// Stop all sounds
				INVENTORY_RESULT inventoryResult = g_Inventory->DoInventory();
				switch (inventoryResult)
				{
				case INVENTORY_RESULT::INVENTORY_RESULT_LOAD_GAME:
					return GAME_STATUS::GAME_STATUS_LOAD_GAME;
				case INVENTORY_RESULT::INVENTORY_RESULT_EXIT_TO_TILE:
					return GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE;
				}
			}
		}

		// Has level been completed?
		if (LevelComplete)
			return GAME_STATUS::GAME_STATUS_LEVEL_COMPLETED;

		__int32 oldInput = TrInput;
		 
		// Is Lara dead?
		if (Lara.deathCount > 300 || Lara.deathCount > 60 && TrInput)
		{
			INVENTORY_RESULT inventoryResult = g_Inventory->DoInventory();
			switch (inventoryResult)
			{
			case INVENTORY_RESULT::INVENTORY_RESULT_NEW_GAME:
				return GAME_STATUS::GAME_STATUS_NEW_GAME;
			case INVENTORY_RESULT::INVENTORY_RESULT_LOAD_GAME:
				return GAME_STATUS::GAME_STATUS_LOAD_GAME;
			case INVENTORY_RESULT::INVENTORY_RESULT_EXIT_TO_TILE:
				return GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE;
			}
		}

		if (demoMode && TrInput == -1)
		{
			oldInput = 0;
			TrInput = 0;
		}

		// CLear dynamic lights
		ClearDynamics();
		ClearFires();
		g_Renderer->ClearDynamicLights();

		GotLaraSpheres = false;

		// Update all items
		InItemControlLoop = true;

		__int16 itemNum = NextItemActive;
		while (itemNum != NO_ITEM)
		{
			ITEM_INFO* item = &Items[itemNum];
			__int16 nextItem = item->nextActive;

			if (item->afterDeath < 128)
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

		__int16 fxNum = NextFxActive;
		while (fxNum != NO_ITEM)
		{
			__int16 nextFx = Effects[fxNum].nextActive;
			if (Objects[Effects[fxNum].objectNumber].control)
				(*Objects[Effects[fxNum].objectNumber].control)(fxNum);
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

		// Control Lara
		InItemControlLoop = true;
		Lara.skelebob = NULL;
		LaraControl();
		InItemControlLoop = false;
		
		// Update Lara's ponytails
		HairControl(0, 0, 0);
		if (level->LaraType == LARA_DRAW_TYPE::LARA_YOUNG)
			HairControl(0, 1, 0);

		if (UseSpotCam)
		{
			// Draw flyby cameras
			g_Renderer->EnableCinematicBars(true);
			CalculateSpotCameras();
		}
		else
		{
			// Do the standard camera
			g_Renderer->EnableCinematicBars(false);
			CalculateCamera();
		}
		    
		//WTF: what is this? It's used everywhere so it has to stay
		Wibble = (Wibble + 4) & 0xFC;
		
		// Update special effects
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

		HealtBarTimer--;
	}

	return GAME_STATUS::GAME_STATUS_NONE;
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

	// TODO: deprecated, to remove because now we have LUA
	LoadNewStrings();
	
	// Execute the LUA gameflow and play the game
	g_Script->DoGameflow();

	// End the game and release some resources
	GameClose();

	// Finish the thread
	PostMessageA((HWND)WindowsHandle, 0x10u, 0, 0);
	_endthreadex(1);

	return 1;   
}   

GAME_STATUS __cdecl DoTitle(__int32 index)
{
	DB_Log(2, "DoTitle - DLL");
	printf("DoTitle\n");

	// Load the title level
	S_LoadLevelFile(0);
	
	INVENTORY_RESULT inventoryResult = g_Inventory->DoTitleInventory();
	switch (inventoryResult)
	{
	case INVENTORY_RESULT::INVENTORY_RESULT_NEW_GAME:
		return GAME_STATUS::GAME_STATUS_NEW_GAME;
	case INVENTORY_RESULT::INVENTORY_RESULT_LOAD_GAME:
		return GAME_STATUS::GAME_STATUS_LOAD_GAME;
	case INVENTORY_RESULT::INVENTORY_RESULT_EXIT_GAME:
		return GAME_STATUS::GAME_STATUS_EXIT_GAME;
	}

	return GAME_STATUS::GAME_STATUS_NEW_GAME;
}

GAME_STATUS __cdecl DoLevel(__int32 index, __int32 ambient, bool loadFromSavegame)
{
	CreditsDone = false;
	CanLoad = false;

	// If not loading a savegame, then clear all the infos
	if (!loadFromSavegame)
	{
		Savegame.Level.Timer = 0;
		Savegame.Game.Timer = 0;
		Savegame.Level.Distance = 0;
		Savegame.Game.Distance = 0;
		Savegame.Level.AmmoUsed = 0;
		Savegame.Game.AmmoUsed = 0;
		Savegame.Level.AmmoHits = 0;
		Savegame.Game.AmmoHits = 0;
		Savegame.Level.Kills = 0;
		Savegame.Game.Kills = 0;
	}

	// If load from savegame, then restore the game
	if (loadFromSavegame)
	{
		RestoreGame();
		gfRequiredStartPos = false;
		gfInitialiseGame = false;
	}
	else
	{
		gfRequiredStartPos = false;
		if (gfInitialiseGame)
		{
			GameTimer = 0;
			gfRequiredStartPos = false;
			gfInitialiseGame = false;
		}

		Savegame.Level.Timer = 0;
		if (CurrentLevel == 1)
			Savegame.TLCount = 0;
	}
	
	// Load the level
	S_LoadLevelFile(index);

	// TODO: deprecated?
	GlobalLastInventoryItem = -1;
	DelCutSeqPlayer = 0;
	TitleControlsLockedOut = false;

	// Initialise flyby cameras
	InitSpotCamSequences();
		
	// Play background music
	CurrentAtmosphere = ambient;
	S_CDPlay(CurrentAtmosphere, 1);
	IsAtmospherePlaying = true;

	// Initialise items, effects, lots, camera
	InitialiseFXArray(true);
	InitialiseLOTarray(true);
	InitialisePickUpDisplay();
	InitialiseCamera();

	// Initialise ponytails
	InitialiseHair();

	__int32 nframes = 2;
	GAME_STATUS result = ControlPhase(nframes, 0);
	g_Renderer->FadeIn();

	// The game loop, finally!
	while (true)
	{
		nframes = DrawPhaseGame();
		result = ControlPhase(nframes, 0);

		if (result == GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE ||
			result == GAME_STATUS::GAME_STATUS_LOAD_GAME ||
			result == GAME_STATUS::GAME_STATUS_LEVEL_COMPLETED)
		{
			// Here is the only way for exiting from the loop
			SOUND_Stop();
			S_CDStop();

			return result;
		}

		Sound_UpdateScene();
	}
}

void Inject_Control()
{

}