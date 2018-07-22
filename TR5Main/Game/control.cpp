#include "control.h"

#include "..\Global\global.h"
#include "pickup.h"
#include "..\Specific\input.h"
#include "spotcam.h"
#include "camera.h"
#include "lara.h"
#include <stdio.h>
#include "hair.h"
#include "items.h"
#include "effect2.h"
#include "draw.h"
#include "..\Specific\input.h"

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
		if (CurrentLevel != 0)
		{
			if ((DbInput & 1 || GlobalEnterInventory != -1) && !CutSeqTriggered && LaraItem->hitPoints > 0)
			{
				//S_SoundStopAllSamples();
				//if (j_S_CallIventory2())
				//	return 2;
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

		//CalculateSpotCameras();
	}
}

__int32 __cdecl DoPauseMenu()
{
	// Dump the screen
	g_Renderer->DumpScreen();

	__int32 choice = 0;
	g_Renderer->DrawPauseMenu(choice, false);

	while (true)
	{
		g_Renderer->DrawPauseMenu(choice, false);

		SetDebounce = 1;
		S_UpdateInput();
		SetDebounce = 0;

		// Process input
		if (DbInput & 1 && choice > 0)
		{
			choice--;
			continue;
		}
		else if (DbInput & 2 && choice < 2)
		{
			choice++;
			continue;
		}
		else if (DbInput & 0x200000)
		{
			break;
		}
		else if (DbInput & 0x100000)
		{
			if (choice == 0)
				DoStatisticsMenu();
		}
	}

	return 0;
}

__int32 __cdecl DoStatisticsMenu()
{
	g_Renderer->DrawStatisticsMenu();

	while (true)
	{
		g_Renderer->DrawStatisticsMenu();

		SetDebounce = 1;
		S_UpdateInput();
		SetDebounce = 0;

		// Process input
		if (DbInput & 0x200000)
		{
			return 0;
		}
	}

	return 0;
}

__int32 __cdecl DoSettingsMenu()
{
	return 0;
}

void Inject_Control()
{
	INJECT(0x004147C0, ControlPhase);
}