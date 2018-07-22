#pragma once

#include "..\Global\global.h"

#define TIME_Init ((void (__cdecl*)(void)) 0x004D19D0)
#define ResetSoundThings ((void (__cdecl*)(void)) 0x004A857A)
#define GameClose ((__int32 (__cdecl*)(void)) 0x004A8575)
#define LoadGameflow ((__int32 (__cdecl*)(void)) 0x00434800)
#define InitialiseTitleOptionsMaybe ((void (__cdecl*)(__int32, __int32)) 0x004CA720)
#define TriggerTitleSpotcam ((void (__cdecl*)(__int32)) 0x004284A0)

void __cdecl DoTitleFMV();
void __cdecl LoadScreen(__int32 index, __int32 num);
void __cdecl DoTitle(__int32 index);
__int32 __cdecl ControlPhase(__int32 a, __int32 b);

unsigned __stdcall GameMain(void*);

void Inject_Game();