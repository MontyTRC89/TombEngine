#pragma once

#include "..\Global\global.h"

#define GetFloor ((FLOOR_INFO* (__cdecl*)(int, int, int, short*)) 0x00415B20)
#define GetCeiling ((int (__cdecl*)(FLOOR_INFO*, int, int, int)) 0x00417640)
#define GetFloorHeight ((int (__cdecl*)(FLOOR_INFO*, int, int, int)) 0x00415FB0)
#define GetRandomControl ((int (__cdecl*)()) 0x004A7C10)
#define AnimateItem ((void (__cdecl*)(ITEM_INFO*)) 0x00415300)
#define GetWaterHeight ((__int32 (__cdecl*)(__int32, __int32, __int32, __int16)) 0x00415DA0)
#define TriggerActive ((__int32 (__cdecl*)(ITEM_INFO*)) 0x004175B0)
#define GetChange ((__int32 (__cdecl*)(ITEM_INFO*, ANIM_STRUCT*)) 0x00415890)
#define KillMoveItems ((void (__cdecl*)()) 0x00414620)
#define KillMoveEffects ((void (__cdecl*)()) 0x004146A0)
#define ClearDynamics ((void (__cdecl*)()) 0x00431530)
#define ClearFires ((void (__cdecl*)()) 0x00481B10)
#define UpdateSparks ((void (__cdecl*)()) 0x0042E8B0)	
#define UpdateFireSparks ((void (__cdecl*)()) 0x004813B0)	
#define UpdateSmoke ((void (__cdecl*)()) 0x00481DD0)	
#define UpdateBlood ((void (__cdecl*)()) 0x00482610)	
#define UpdateBubbles ((void (__cdecl*)()) 0x00483540)
#define UpdateSplashes ((void (__cdecl*)()) 0x00430710)
#define UpdateDebris ((void (__cdecl*)()) 0x0041D500)
#define UpdateDrips ((void (__cdecl*)()) 0x00483D90)
#define UpdateGunShells ((void (__cdecl*)()) 0x00482D80)
//#define UpdateSky ((void (__cdecl*)()) 0x00414720)
#define UpdateBats ((void (__cdecl*)()) 0x00407FD0)
#define UpdateRats ((void (__cdecl*)()) 0x0046AC70)
#define UpdateSpiders ((void (__cdecl*)()) 0x0047A340)
#define UpdateShockwaves ((void (__cdecl*)()) 0x004849A0)
#define UpdateLightning ((void (__cdecl*)()) 0x00484CB0)
#define UpdatePulseColor ((void (__cdecl*)()) 0x00480830)
#define RumbleScreen ((void (__cdecl*)()) 0x00442C90)
#define ExplodeItemNode ((__int32 (__cdecl*)(ITEM_INFO*, __int32, __int32, __int32)) 0x0041ABF0)
#define LavaBurn ((void (__cdecl*)(ITEM_INFO*)) 0x0048ADD0)
#define RefreshCamera ((void (__cdecl*)(__int16, __int16*)) 0x004165E0)
#define DoFlipMap ((void (__cdecl*)(__int16)) 0x00418910)
//#define SwitchTrigger ((__int32 (__cdecl*)(__int16, __int16)) 0x0047D670)
//#define KeyTrigger ((__int32 (__cdecl*)(__int16)) 0x00469550)
//#define PickupTrigger ((__int32 (__cdecl*)(__int16)) 0x004695E0)
#define PlaySoundTrack ((void (__cdecl*)(__int16, __int16)) 0x00418B90)
#define AlterFloorHeight ((void (__cdecl*)(ITEM_INFO*, __int32)) 0x004159F0)
#define SoundEffects ((void (__cdecl*)()) 0x00432640)
#define ObjectOnLOS2 ((__int32 (__cdecl*)(GAME_VECTOR*, GAME_VECTOR*, PHD_VECTOR*, BITE_INFO*)) 0x00419110)
//#define AnimateWaterfalls ((void (__cdecl*)()) 0x00465DF0)

GAME_STATUS __cdecl DoTitle(__int32 index);
GAME_STATUS __cdecl DoLevel(__int32 index, __int32 ambient, bool loadFromSavegame);
GAME_STATUS __cdecl ControlPhase(__int32 numFrames, __int32 demoMode);
void __cdecl UpdateSky();
void __cdecl AnimateWaterfalls();

unsigned __stdcall GameMain(void*);
void __cdecl TestTriggers(__int16* data, __int32 heavy, __int32 HeavyFlags);
void Inject_Control();