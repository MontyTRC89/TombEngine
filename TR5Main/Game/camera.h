#pragma once
#include "Specific/phd_global.h"
#include "items.h"

enum class CAMERA_TYPE
{
	CHASE_CAMERA,
	FIXED_CAMERA,
	LOOK_CAMERA,
	COMBAT_CAMERA,
	HEAVY_CAMERA
};

struct CAMERA_INFO
{
	GAME_VECTOR pos; // size=16, offset=0
	GAME_VECTOR target; // size=16, offset=16
	CAMERA_TYPE type; // size=4, offset=32
	CAMERA_TYPE oldType; // size=4, offset=36
	int shift; // size=0, offset=40
	int flags; // size=0, offset=44
	bool fixedCamera; // size=0, offset=48
	bool underwater; // size=0, offset=60
	int numberFrames; // size=0, offset=52
	int bounce; // size=0, offset=56
	int targetDistance; // size=0, offset=64
	short targetAngle; // size=0, offset=68
	short targetElevation; // size=0, offset=70
	short actualElevation; // size=0, offset=72
	short actualAngle; // size=0, offset=74
	short laraNode; // size=0, offset=76
	short box; // size=0, offset=78
	short number; // size=0, offset=80
	short last; // size=0, offset=82
	short timer; // size=0, offset=84
	short speed; // size=0, offset=86
	short targetspeed; // size=0, offset=88
	ITEM_INFO* item; // size=144, offset=92
	ITEM_INFO* lastItem; // size=144, offset=96
	int mikeAtLara; // size=0, offset=104
	PHD_VECTOR mikePos; // size=12, offset=108
};

enum CAMERA_FLAGS
{
	CF_FOLLOW_CENTER = 1,
	CF_NO_CHUNKY = 2,
	CF_CHASE_OBJECT = 3,
};

constexpr auto MAX_CAMERA = 18;
constexpr auto NO_MINY = 0xFFFFFF;

extern PHD_VECTOR CurrentCameraPosition;
extern CAMERA_INFO Camera;
extern GAME_VECTOR ForcedFixedCamera;
extern int UseForcedFixedCamera;
extern int NumberCameras;
extern int BinocularRange;
extern int BinocularOn;
extern CAMERA_TYPE BinocularOldCamera;
extern bool LaserSight;
extern int PhdPerspective;
extern short CurrentFOV;

void LookAt(CAMERA_INFO* cam, short roll);
void AlterFOV(int value);
void InitialiseCamera();
void MoveCamera(GAME_VECTOR* ideal, int speed);
void ChaseCamera(ITEM_INFO* item);
void UpdateCameraElevation();
void CombatCamera(ITEM_INFO* item);
bool CameraCollisionBounds(GAME_VECTOR* ideal, int push, int yFirst);
void FixedCamera(ITEM_INFO* item);
void LookCamera(ITEM_INFO* item);
void BounceCamera(ITEM_INFO* item, short bounce, short maxDistance);
void BinocularCamera(ITEM_INFO* item);
void ConfirmCameraTargetPos();
void CalculateCamera();
void LookLeftRight();
void LookUpDown();
void ResetLook();
void RumbleScreen();
bool TestBoundsCollideCamera(BOUNDING_BOX* bounds, PHD_3DPOS* pos, short radius);
void ItemPushCamera(BOUNDING_BOX* bounds, PHD_3DPOS* pos, short radius);
void ItemsCollideCamera();
