#pragma once

#include "..\Global\global.h"
#include <d3d9.h>
#include <d3dx9.h>

#define Camera						VAR_U_(0x00EEF940, CAMERA_INFO)
#define ForcedFixedCamera			VAR_U_(0x00EEFA20, GAME_VECTOR)
#define UseForcedFixedCamera		VAR_U_(0x00EEFA50, char)

//#define InitialiseCamera ((void (__cdecl*)()) 0x0040C690)
//#define MoveCamera ((int(__cdecl*)(GAME_VECTOR*,int)) 0x0040C7A0)
//#define ChaseCamera ((int(__cdecl*)(ITEM_INFO*)) 0x0040D150)
#define CombatCamera ((int(__cdecl*)(ITEM_INFO*)) 0x0040D640)
#define LookCamera ((void(__cdecl*)(ITEM_INFO*)) 0x0040DC10)
#define FixedCamera ((void(__cdecl*)()) 0x0040E890)
#define CalculateCamera ((void(__cdecl*)()) 0x0040ED30)
#define BinocularCamera ((void(__cdecl*)()) 0x0040FC20)
//#define RefreshCamera ((void(__cdecl*)(short,short*)) 0x004165E0)
#define CameraCollisionBounds ((int (__cdecl*)(GAME_VECTOR*,int,int)) 0x0040F5C0)
#define do_new_cutscene_camera ((void(__cdecl*)()) 0x00421480)
#define SaveD3DCameraMatrix ((void(__cdecl*)()) 0x00497280)
#define UnknownCamera ((void(__cdecl*)()) 0x004975D0)

extern PHD_VECTOR CurrentCameraPosition;

void ActivateCamera();
void LookAt(int posX, int posY, int posZ, int targetX, int targetY, int targetZ, short roll);
void AlterFOV(int value);
int mgLOS(GAME_VECTOR* start, GAME_VECTOR* target, int push);
void InitialiseCamera();
void MoveCamera(GAME_VECTOR* ideal, int speed);
void ChaseCamera(ITEM_INFO* item);
void UpdateCameraElevation();

void Inject_Camera();