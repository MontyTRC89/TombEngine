#pragma once

#include "..\Global\global.h"

#define BaddyObjects ((void (__cdecl*)()) 0x004737C0)
#define ObjectObjects ((void (__cdecl*)()) 0x00476360)
#define TrapObjects ((void (__cdecl*)()) 0x00475D40)
#define BaddyObjects ((void (__cdecl*)()) 0x004737C0)
#define InitialiseHairs ((void (__cdecl*)()) 0x00438BE0)
#define InitialiseSpecialEffects ((void (__cdecl*)()) 0x0043D8B0)

void __cdecl CustomObjects();
void __cdecl InitialiseObjects();

void Inject_Setup();