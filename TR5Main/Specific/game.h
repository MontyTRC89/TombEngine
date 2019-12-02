#pragma once

#include "..\Global\global.h"

/*#define TIME_Init ((void (__cdecl*)(void)) 0x004D19D0)
#define ResetSoundThings ((void (__cdecl*)(void)) 0x004A857A)
#define GameClose ((int (__cdecl*)(void)) 0x004A8575)
#define LoadGameflow ((int (__cdecl*)(void)) 0x00434800)
#define InitialiseTitleOptionsMaybe ((void (__cdecl*)(int, int)) 0x004CA720)
#define TriggerTitleSpotcam ((void (__cdecl*)(int)) 0x004284A0)*/

/*void DoTitleFMV();
void LoadScreen(int index, int num);
void DoTitle(int index);
void DoLevel(int index);*/
//int ControlPhase(int a, int b);

//unsigned __stdcall GameMainThread(void*);

void Inject_Game();