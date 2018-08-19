#pragma once

#include "..\Global\global.h"

#define DetatchSpark ((SPARKS* (__cdecl*)(int, int)) 0x0042E6A0)

void __cdecl TriggerDynamics(__int32 x, __int32 y, __int32 z, __int16 falloff, byte r, byte g, byte b);

void Inject_Effect2();