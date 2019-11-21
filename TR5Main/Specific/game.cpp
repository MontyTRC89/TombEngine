#include "game.h"

#include "..\Game\gameflow.h"
#include "..\Game\spotcam.h"
#include "..\Game\box.h"
#include "..\Game\lot.h"
#include "..\Game\effects.h"
#include "..\Game\items.h"
#include "..\Game\healt.h"
#include "..\Game\Camera.h"
#include "..\Game\pickup.h"
#include "..\Game\draw.h"
#include "..\Game\hair.h"
#include "..\Game\inventory.h"

#include "roomload.h"
#include <process.h>
#include <stdio.h>

/*void __cdecl DoTitleFMV()
{

}

void __cdecl LoadScreen(__int32 index, __int32 num)
{

}*/

/*unsigned __stdcall GameMainThread(void*)
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

	r*eturn 1;
}*/

/*void __cdecl DoTitle(__int32 index)
{
	DB_Log(2, "DoTitle - DLL");
	printf("DoTitle\n");
	DoLevel(1);
	return;

	S_LoadLevelFile(0);

	g_Inventory->DoTitleInventory();
}*/

/*void __cdecl DoLevel(__int32 index)
{
	CreditsDone = false;
	//j_DoTitleFMV();
	CanLoad = false;

	InitialiseTitleOptionsMaybe(255, 0);

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

	LastInventoryItem = -1;
	DelCutSeqPlayer = 0;

	InitSpotCamSequences();

	TitleControlsLockedOut = false;
	IsAtmospherePlaying = false;

	InitialiseFXArray(true);
	InitialiseLOTarray(true);
	InitialisePickupDisplay();
	InitialiseCamera();
	printf("InitialiseCamera OK\n");

	printf("Initialised\n");
	//while (true)
	//TriggerTitleSpotcam(1);

	InitialiseHair();

	ControlPhase(2, 0);
	printf("After control\n");

	nFrames = 2;
	GameStatus = ControlPhase(2, 0);
	//JustLoaded = 0;
	while (!GameStatus || true)
	{
		Sound_UpdateScene();
		nFrames = DrawPhaseGame();
		GameStatus = ControlPhase(nFrames, 0);
	}
}*/

void Inject_Game()
{
	/*INJECT(0x004B2090, DoTitleFMV);
	INJECT(0x004AC810, LoadScreen);
	INJECT(0x00435C70, DoTitle);*/
}