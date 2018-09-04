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
				// stop all sounds

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

		if (DbInput & 0x2000)
		{
			DoPauseMenu();
		}

		// Has level been completed?
		if (LevelComplete)
			return GAME_STATUS::GAME_STATUS_LEVEL_COMPLETED;

		__int32 oldInput = TrInput;
		 
		if (ResetFlag || Lara.deathCount > 300 || Lara.deathCount > 60 && TrInput)
		{
			/*if (GameFlow->DemoDisc && ResetFlag)
			{
				ResetFlag = 0;
				return 4;
			}
			else
			{
				ResetFlag = 0;
				return 1;
			}*/
			// TODO: check this
			return GAME_STATUS::GAME_STATUS_LARA_DEAD;
		}

		if (demoMode && TrInput == -1)
		{
			oldInput = 0;
			TrInput = 0;
		}

		ClearDynamics();
		ClearFires();
		g_Renderer->ClearDynamicLights();

		GotLaraSpheres = false;

		InItemControlLoop = true;

		__int16 itemNum = NextItemActive;
		while (itemNum != NO_ITEM)
		{
			ITEM_INFO* item = &Items[itemNum];
			__int16 nextItem = item->nextActive;

			if (item->afterDeath > 127)
				KillItem(itemNum);
			else
			{
				if (Objects[Items[itemNum].objectNumber].control)
					(*Objects[Items[itemNum].objectNumber].control)(itemNum);
			}

			itemNum = nextItem;
		}

		InItemControlLoop = false;

		KillMoveItems();

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

		if (SmokeCountL)
			SmokeCountL--;
		if (SmokeCountR)
			SmokeCountR--;
		if (SplashCount)
			SplashCount--;
		if (WeaponDelay)
			WeaponDelay--;

		InItemControlLoop = true;
		Lara.skelebob = NULL;
		LaraControl();
		InItemControlLoop = false;
		
		j_HairControl(0, 0, 0);
		if (gfLevelFlags & 1)
			j_HairControl(0, 1, 0);

		if (UseSpotCam)
			CalculateSpotCameras();
		else
			CalculateCamera();
		    
		Wibble = (Wibble + 4) & 0xFC;
		
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

	MatrixPtr = MatrixStack;
	DxMatrixPtr = (byte*)malloc(48 * 40);

	InitGameMalloc();
	TIME_Init();

	LoadNewStrings();

	g_Script->DoGameflow();

	GameClose();

	PostMessageA((HWND)WindowsHandle, 0x10u, 0, 0);
	_endthreadex(1);

	return 1;   
}   

GAME_STATUS __cdecl DoTitle(__int32 index)
{
	DB_Log(2, "DoTitle - DLL");
	printf("DoTitle\n");

	g_Renderer->FreeRendererData();

	//gfLevelFlags |= 1;
	//DoLevel(3, 124);
	//return;

	S_LoadLevelFile(0);
	
	INVENTORY_RESULT inventoryResult = g_Inventory->DoTitleInventory();
	switch (inventoryResult)
	{
	case INVENTORY_RESULT::INVENTORY_RESULT_NEW_GAME:
		return GAME_STATUS::GAME_STATUS_NEW_GAME;
	case INVENTORY_RESULT::INVENTORY_RESULT_LOAD_GAME:
		return GAME_STATUS::GAME_STATUS_LOAD_GAME;
	case INVENTORY_RESULT::INVENTORY_RESULT_EXIT_TO_TILE:
		return GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE;
	}

	return GAME_STATUS::GAME_STATUS_NEW_GAME;
}

GAME_STATUS __cdecl DoLevel(__int32 index, __int32 ambient, bool loadFromSavegame)
{
	g_Renderer->FreeRendererData();

	CreditsDone = false;
	//j_DoTitleFMV();
	CanLoad = false;

	//InitialiseTitleOptionsMaybe(255, 0);

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
		
	//num_fmvs = 0;
	//fmv_to_play[1] = 0;
	//fmv_to_play[0] = 0;
	 
	//IsLevelLoading = true;
	//S_LoadLevelFile(0);
	//InitialiseFXArray(true);
	//InitialiseLOTarray(true);

	S_LoadLevelFile(index);
	//while (IsLevelLoading);

	printf("Starting rendering\n");

	//while(true)
	//	TestRenderer();

	GlobalLastInventoryItem = -1;
	DelCutSeqPlayer = 0;

	InitSpotCamSequences();

	TitleControlsLockedOut = false;

	CurrentAtmosphere = ambient;
	S_CDPlay(CurrentAtmosphere, 1);
	IsAtmospherePlaying = true;

	InitialiseFXArray(true);
	InitialiseLOTarray(true);
	InitialisePickUpDisplay();
	InitialiseCamera();
	printf("InitialiseCamera OK\n");

	printf("Initialised\n");
	//while (true)
	//TriggerTitleSpotcam(1);

	InitialiseHair();

	//ControlPhase(2, 0);
	//printf("After control\n");
	
	__int32 nframes = 2;
	GAME_STATUS result = ControlPhase(nframes, 0);
	//JustLoaded = 0;
	while (true)
	{
		nframes = DrawPhaseGame();
		result = ControlPhase(nframes, 0);
		if (result == GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE ||
			result == GAME_STATUS::GAME_STATUS_LOAD_GAME ||
			result == GAME_STATUS::GAME_STATUS_LEVEL_COMPLETED)
			return result;

		Sound_UpdateScene();
	}
}

/*void __cdecl DoTitleFMV()
{

}

void __cdecl LoadScreen(__int32 index, __int32 num)
{

}*/

void Inject_Control()
{
	//INJECT(0x004B2090, DoTitleFMV);
	//INJECT(0x004AC810, LoadScreen);
	INJECT(0x00435C70, DoTitle);
	INJECT(0x004147C0, ControlPhase);
}