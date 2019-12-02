#pragma once

#include "..\Global\global.h"

#define TIME_Init ((void (__cdecl*)(void)) 0x004D19D0)
#define SOUND_Init ((int (__cdecl*)(void)) 0x004790A0)

#define LoadSettings ((int (__cdecl*)(void)) 0x004BDE20)

void Inject_Init();