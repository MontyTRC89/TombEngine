#pragma once

#include "..\Global\global.h"

#define DrawAnimatingItem ((void (__cdecl*)(ITEM_INFO*)) 0x0042B900)
#define GetBoundsAccurate ((short* (__cdecl*)(ITEM_INFO*)) 0x0042CF80)
#define GetBestFrame ((short* (__cdecl*)(ITEM_INFO*)) 0x0042D020)
#define GetFrame_D2 ((__int32 (__cdecl*)(ITEM_INFO*, __int16**, __int32*)) 0x0042CEB0)
#define CalcLaraMatrices ((void (__cdecl*)(__int32)) 0x0041E120)
#define phd_PushUnitMatrix ((void (__cdecl*)()) 0x0048FA90)

extern Renderer* g_Renderer;

__int32 __cdecl DrawPhaseGame();
void __cdecl DoBootScreen(__int32 language);

void Inject_Draw();