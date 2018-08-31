#pragma once

#include "..\Global\global.h"

#define DrawAnimatingItem ((void (__cdecl*)(ITEM_INFO*)) 0x0042B900)
#define GetBoundsAccurate ((short* (__cdecl*)(ITEM_INFO*)) 0x0042CF80)
#define GetBestFrame ((short* (__cdecl*)(ITEM_INFO*)) 0x0042D020)
#define GetFrame_D2 ((__int32 (__cdecl*)(ITEM_INFO*, __int16**, __int32*)) 0x0042CEB0)
#define CalcLaraMatrices ((void (__cdecl*)(__int32)) 0x0041E120)
#define phd_PushUnitMatrix ((void (__cdecl*)()) 0x0048FA90)
#define Sync ((__int32 (__cdecl*)()) 0x004D1A40)
#define WeaponObject ((__int16 (__cdecl*)(__int32)) 0x00453AE0)  
#define GetRoomBounds ((void (__cdecl*)()) 0x0042D4F0) 
#define UpdateStorm ((void (__cdecl*)()) 0x0042A310)  
#define Sub_0042A050 ((void (__cdecl*)()) 0x0042A050)  

extern Renderer* g_Renderer;

__int32 __cdecl DrawPhaseGame();
void __cdecl DoBootScreen(__int32 language);
void __cdecl DrawPistolsMeshes(__int32 weapon);
void __cdecl DrawShotgunMeshes(__int32 weapon);
void __cdecl UndrawPistolsMeshLeft(__int32 weapon);
void __cdecl UndrawPistolsMeshRight(__int32 weapon);
void __cdecl UndrawShotgunMeshes(__int32 weapon);

void Inject_Draw();