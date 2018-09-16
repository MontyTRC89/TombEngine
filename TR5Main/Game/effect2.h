#pragma once

#include "..\Global\global.h"

#define DetatchSpark ((SPARKS* (__cdecl*)(__int32, __int32)) 0x0042E6A0)
#define AddWaterSparks ((void (__cdecl*)(__int32, __int32, __int32, __int32)) 0x00483180)
#define TriggerUnderwaterExplosion ((void (__cdecl*)(ITEM_INFO*)) 0x0044F500)
#define TriggerExplosionSparks ((void (__cdecl*)(__int32, __int32, __int32, __int32, __int32, __int32, __int16)) 0x0042F610)
#define ExplodingDeath ((void (__cdecl*)(__int16, __int32, __int32)) 0x00484080)
#define GetFreeSpark ((__int16 (__cdecl*)()) 0x0042E790)

void __cdecl TriggerDynamics(__int32 x, __int32 y, __int32 z, __int16 falloff, byte r, byte g, byte b);

void Inject_Effect2();