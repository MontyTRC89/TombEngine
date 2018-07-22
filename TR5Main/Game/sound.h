#pragma once

#include "..\Global\global.h"

extern "C" {
#define StopSoundEffect ((void (__cdecl*)(__int16)) 0x00478FE0)
#define SoundEffect ((void (__cdecl*)(__int32, PHD_3DPOS*, __int32)) 0x00478570)
#define DXCreateSampleADPCM ((__int32 (__cdecl*)(char*, __int32, __int32, __int32)) 0x004A3510)
}