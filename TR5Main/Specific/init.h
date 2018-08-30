#pragma once

#include "..\Global\global.h"

#define TIME_Init ((void (__cdecl*)(void)) 0x004D19D0)
#define SOUND_Init ((__int32 (__cdecl*)(void)) 0x004790A0)

#define LoadSettings ((__int32 (__cdecl*)(void)) 0x004BDE20)

void Inject_Init();