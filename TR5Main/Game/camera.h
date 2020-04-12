#pragma once

#include "..\Global\global.h"
#include <d3d9.h>
#include <d3dx9.h>

extern PHD_VECTOR CurrentCameraPosition;
extern CAMERA_INFO Camera;
extern GAME_VECTOR ForcedFixedCamera;
extern int UseForcedFixedCamera;
extern int NumberCameras;
extern int SniperCameraActive;
extern int BinocularRange;
extern int BinocularOn;
extern CAMERA_TYPE BinocularOldCamera;
extern int LaserSight;
extern int SniperCount;
extern int ExitingBinocular;

void ActivateCamera();
void LookAt(int posX, int posY, int posZ, int targetX, int targetY, int targetZ, short roll);
void AlterFOV(int value);
int mgLOS(GAME_VECTOR* start, GAME_VECTOR* target, int push);
void InitialiseCamera();
void MoveCamera(GAME_VECTOR* ideal, int speed);
void ChaseCamera(ITEM_INFO* item);
void UpdateCameraElevation();
void CombatCamera(ITEM_INFO* item);
int CameraCollisionBounds(GAME_VECTOR* ideal, int push, int yFirst);
void FixedCamera(ITEM_INFO* item);
void LookCamera(ITEM_INFO* item);
void BounceCamera(ITEM_INFO* item, short bounce, short maxDistance);
void BinocularCamera(ITEM_INFO* item);
void LaraTorch(PHD_VECTOR* src, PHD_VECTOR* target, int rot, int color);
void ConfirmCameraTargetPos();
void CalculateCamera();

void Inject_Camera();