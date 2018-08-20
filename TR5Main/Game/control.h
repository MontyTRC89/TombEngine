#pragma once

#include "..\Global\global.h"

#define GetFloor ((FLOOR_INFO* (__cdecl*)(int, int, int, short*)) 0x00415B20)
#define GetCeiling ((int (__cdecl*)(FLOOR_INFO*, int, int, int)) 0x00417640)
#define TrGetHeight ((int (__cdecl*)(FLOOR_INFO*, int, int, int)) 0x00415FB0)
#define GetRandomControl ((int (__cdecl*)()) 0x004A7C10)
#define AnimateItem ((void (__cdecl*)(ITEM_INFO*)) 0x00415300)
#define GetWaterHeight ((__int32 (__cdecl*)(__int32, __int32, __int32, __int16)) 0x00415DA0)
#define TestTriggers ((void (__cdecl*)(__int16*, __int32, __int32)) 0x00416760)
#define TriggerActive ((__int32 (__cdecl*)(ITEM_INFO*)) 0x004175B0)
#define GetChange ((__int32 (__cdecl*)(ITEM_INFO*, ANIM_STRUCT*)) 0x00415890)
#define KillMoveItems ((void (__cdecl*)()) 0x00414620)
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


void __cdecl DoTitle(__int32 index);
void __cdecl DoLevel(__int32 index);
__int32 __cdecl ControlPhase(__int32 numFrames, __int32 demoMode);

unsigned __stdcall GameMain(void*);

void Inject_Control();