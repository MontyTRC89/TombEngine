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

#include "..\Specific\roomload.h"
#include "..\Specific\input.h"
#include "..\Specific\init.h"
#include "..\Specific\winmain.h"

#include <process.h>
#include <stdio.h>

__int32 __cdecl ControlPhase(__int32 numFrames, __int32 demoMode)
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

		// UpdateSky();

		// Poll the keyboard and update input variables
		if (S_UpdateInput() == -1)
			return 0;
		
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
				//S_SoundStopAllSamples();
				if (g_Inventory->DoInventory())
					return 2;
			}
		}

		if (DbInput & 0x2000)
		{
			DoPauseMenu();
		}

		// Has level been completed?
		if (LevelComplete)
			return 3;

		__int32 oldInput = TrInput;
		 
		if (ResetFlag || Lara.deathCount > 300 || Lara.deathCount > 60 && TrInput)
		{
			if (GameFlow->DemoDisc && ResetFlag)
			{
				ResetFlag = 0;
				return 4;
			}
			else
			{
				ResetFlag = 0;
				return 1;
			}
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
		InItemControlLoop = 1;
		__int16 itemNum = NextItemActive;
		while (itemNum != NO_ITEM)
		{
			__int16 nextItem = Items[itemNum].nextActive;
			if (Objects[Items[itemNum].objectNumber].control)
				(*Objects[Items[itemNum].objectNumber].control)(itemNum);
			itemNum = nextItem;
		}
		InItemControlLoop = 0;

		InItemControlLoop = true;
		Lara.skelebob = NULL;
		LaraControl();
		InItemControlLoop = false;
		KillMoveItems();

		j_HairControl(0, 0, 0);

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
	}
}

unsigned __stdcall GameMain(void*)
{
	DB_Log(2, "GameMain - DLL");
	printf("GameMain\n");

	MatrixPtr = MatrixStack;
	DxMatrixPtr = (byte*)malloc(48 * 40);

	InitGameMalloc();
	TIME_Init();
	//ResetSoundThings();
	//SOUND_Init();
	LoadNewStrings();
	DoGameflow();
	GameClose();
	//ResetSoundThings();
	PostMessageA((HWND)WindowsHandle, 0x10u, 0, 0);
	_endthreadex(1);

	return 1;   
}

void __cdecl DoTitle(__int32 index)
{
	DB_Log(2, "DoTitle - DLL");
	printf("DoTitle\n");
	DoLevel(3);
	return;

	S_LoadLevelFile(0);

	g_Inventory->DoTitleInventory();
}

void __cdecl DoLevel(__int32 index)
{
	CreditsDone = false;
	//j_DoTitleFMV();
	CanLoad = false;

	//InitialiseTitleOptionsMaybe(255, 0);

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

	//num_fmvs = 0;
	//fmv_to_play[1] = 0;
	//fmv_to_play[0] = 0;

	//IsLevelLoading = true;
	S_LoadLevelFile(index);
	//while (IsLevelLoading);

	printf("Starting rendering\n");

	//while(true)
	//	TestRenderer();

	GlobalLastInventoryItem = -1;
	DelCutSeqPlayer = 0;

	InitSpotCamSequences();

	TitleControlsLockedOut = false;
	IsAtmospherePlaying = false;

	InitialiseFXArray(true);
	InitialiseLOTarray(true);
	InitialisePickUpDisplay();
	InitialiseCamera();
	printf("InitialiseCamera OK\n");

	printf("Initialised\n");
	//while (true)
	//TriggerTitleSpotcam(1);

	InitialiseHair();

	ControlPhase(2, 0);
	printf("After control\n");

	__int32 nframes = 2;
	GameStatus = ControlPhase(nframes, 0);
	//JustLoaded = 0;
	while (!GameStatus || true)
	{
		nframes = DrawPhaseGame();
		GameStatus = ControlPhase(nframes, 0);
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