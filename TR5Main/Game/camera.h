#pragma once

#include "..\Global\global.h"
#include <d3d9.h>
#include <d3dx9.h>

#define Camera						VAR_U_(0x00EEF940, CAMERA_INFO)
#define ForcedFixedCamera			VAR_U_(0x00EEFA20, GAME_VECTOR)
#define UseForcedFixedCamera		VAR_U_(0x00EEFA50, char)

#define InitialiseCamera ((void (__cdecl*)()) 0x0040C690)
#define CalculateCamera ((void (__cdecl*)()) 0x0040ED30)
#define CameraCollisionBounds ((int (__cdecl*)(GAME_VECTOR*,int,int)) 0x0040F5C0)

void __cdecl LookAt(int posX, int posY, int posZ, int targetX, int targetY, int targetZ, short roll);
void __cdecl AlterFOV(int value);
void __cdecl j_CalculateCamera();

void Inject_Camera();