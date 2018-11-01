#pragma once

#include "..\Global\global.h"

#define DetatchSpark ((SPARKS* (__cdecl*)(__int32, __int32)) 0x0042E6A0)
#define AddWaterSparks ((void (__cdecl*)(__int32, __int32, __int32, __int32)) 0x00483180)
#define TriggerUnderwaterExplosion ((void (__cdecl*)(ITEM_INFO*)) 0x0044F500)
#define TriggerExplosionSparks ((void (__cdecl*)(__int32, __int32, __int32, __int32, __int32, __int32, __int16)) 0x0042F610)
#define ExplodingDeath ((void (__cdecl*)(__int16, __int32, __int32)) 0x00484080)
#define GetFreeSpark ((__int16 (__cdecl*)()) 0x0042E790)
#define GetFreeSmokeSpark ((__int16 (__cdecl*)()) 0x00481D40)
#define TriggerDartSmoke ((void (__cdecl*)(__int32, __int32, __int32, __int32, __int32, __int32)) 0x00430D90)
#define TriggerGunShell ((void (__cdecl*)(__int16, __int32, __int32)) 0x00482A60)

void __cdecl TriggerDynamics(__int32 x, __int32 y, __int32 z, __int16 falloff, byte r, byte g, byte b);
void __cdecl TriggerGunSmoke(__int32 x, __int32 y, __int32 z, __int32 xv, __int32 yv, __int32 zv, __int32 initial, __int32 weapon, __int32 count);
void __cdecl TriggerRocketFlame(__int32 x, __int32 y, __int32 z, __int32 xv, __int32 yv, __int32 zv, __int32 itemNumber);
void __cdecl TriggerRocketSmoke(__int32 x, __int32 y, __int32 z, __int32 bodyPart);

void Inject_Effect2();