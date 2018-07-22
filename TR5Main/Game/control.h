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

__int32 __cdecl DoPauseMenu();
__int32 __cdecl DoStatisticsMenu();
__int32 __cdecl DoSettingsMenu();
__int32 __cdecl ControlPhase(__int32 numFrames, __int32 demoMode);

void Inject_Control();