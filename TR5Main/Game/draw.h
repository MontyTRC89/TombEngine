#pragma once

#include "..\Global\global.h"

#define DrawAnimatingItem ((void (__cdecl*)(ITEM_INFO*)) 0x0042B900)
#define GetBoundsAccurate ((short* (__cdecl*)(ITEM_INFO*)) 0x0042CF80)
#define GetBestFrame ((short* (__cdecl*)(ITEM_INFO*)) 0x0042D020)
//#define GetFrame_D2 ((__int32 (__cdecl*)(ITEM_INFO*, __int16**, __int32*)) 0x0042CEB0)
#define CalcLaraMatrices ((void (__cdecl*)(__int32)) 0x0041E120)
#define phd_PushUnitMatrix ((void (__cdecl*)()) 0x0048FA90)
#define Sync ((__int32 (__cdecl*)()) 0x004D1A40)
#define GetRoomBounds ((void (__cdecl*)()) 0x0042D4F0) 
#define UpdateStorm ((void (__cdecl*)()) 0x0042A310)  
#define Sub_0042A050 ((void (__cdecl*)()) 0x0042A050)  
#define IsRoomOutside ((__int32 (__cdecl*)(__int32, __int32, __int32)) 0x0044C050)  

__int32 __cdecl DrawPhaseGame();
__int32 DrawGame();
__int32 DrawInventory();
void __cdecl DoBootScreen(__int32 language);
__int32 __cdecl GetFrame_D2(ITEM_INFO* item, __int16* framePtr[], __int32* rate);

extern Renderer11* g_Renderer;

void Inject_Draw();