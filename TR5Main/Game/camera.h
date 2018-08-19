#pragma once

#include "..\Global\global.h"
#include <d3d9.h>
#include <d3dx9.h>

#define Camera						VAR_U_(0x00EEF940, CAMERA_INFO)
#define ForcedFixedCamera			VAR_U_(0x00EEFA20, GAME_VECTOR)
#define UseForcedFixedCamera		VAR_U_(0x00EEFA50, __int8)

#define InitialiseCamera ((void (__cdecl*)()) 0x0040C690)
#define CalculateCamera ((void (__cdecl*)()) 0x0040ED30)

void __cdecl phd_LookAt(__int32 posX, __int32 posY, __int32 posZ,
						__int32 targetX, __int32 targetY, __int32 targetZ,
						__int16 roll);
void __cdecl phd_AlterFOV(__int32 value);
void __cdecl j_CalculateCamera();

void Inject_Camera();