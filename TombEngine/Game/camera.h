#pragma once
#include "Game/items.h"
#include "Math/Math.h"

enum class CameraType
{
	Chase,
	Fixed,
	Look,
	Combat,
	Heavy,
	Object
};

struct CAMERA_INFO
{
	GameVector pos;
	GameVector target;
	CameraType type;
	CameraType oldType;
	int shift;
	int flags;
	bool fixedCamera;
	bool underwater;
	int numberFrames;
	int bounce;
	int targetDistance;
	short targetAngle;
	short targetElevation;
	short actualElevation;
	short actualAngle;
	short laraNode;
	short box;
	short number;
	short last;
	short timer;
	short speed;
	short targetspeed;
	ItemInfo* item;
	ItemInfo* lastItem;
	int mikeAtLara;
	Vector3i mikePos;
};

struct ObjectCameraInfo
{
	GameVector LastAngle;
	bool ItemCameraOn;
};

enum CAMERA_FLAGS
{
	CF_NONE			 = 0,
	CF_FOLLOW_CENTER = 1,
	CF_NO_CHUNKY	 = 2,
	CF_CHASE_OBJECT	 = 3,
};

constexpr auto FADE_SCREEN_SPEED = 16.0f / 255.0f;
constexpr auto DEFAULT_FOV = 80.0f;

extern CAMERA_INFO Camera;
extern GameVector ForcedFixedCamera;
extern int UseForcedFixedCamera;
extern int BinocularRange;
extern bool BinocularOn;
extern CameraType BinocularOldCamera;
extern bool LaserSight;
extern short CurrentFOV;
extern short LastFOV;

extern bool  ScreenFadedOut;
extern bool  ScreenFading;
extern float ScreenFadeSpeed;
extern float ScreenFadeStart;
extern float ScreenFadeEnd;
extern float ScreenFadeCurrent;
extern float CinematicBarsDestinationHeight;
extern float CinematicBarsHeight;
extern float CinematicBarsSpeed;

void DoLookAround(ItemInfo* item, bool invertVerticalAxis = true);
void ClearLookAroundActions(ItemInfo* item);
void DoThumbstickCamera();
void LookCamera(ItemInfo* item);

void LookAt(CAMERA_INFO* cam, short roll);
void AlterFOV(short value, bool store = true);
short GetCurrentFOV();
void InitialiseCamera();
void MoveCamera(GameVector* ideal, int speed);
void ChaseCamera(ItemInfo* item);
void UpdateCameraElevation();
void CombatCamera(ItemInfo* item);
bool CameraCollisionBounds(GameVector* ideal, int push, bool yFirst);
void FixedCamera(ItemInfo* item);
void BounceCamera(ItemInfo* item, short bounce, short maxDistance);
void BinocularCamera(ItemInfo* item);
void ConfirmCameraTargetPos();
void CalculateCamera();
void ResetLook(ItemInfo* item, float alpha = 0.9f);
void RumbleScreen();
bool TestBoundsCollideCamera(const GameBoundingBox& bounds, const Pose& pose, short radius);
void ItemPushCamera(GameBoundingBox* bounds, Pose* pos, short radius);
void ItemsCollideCamera();
void ObjCamera(ItemInfo* camSlotId, int camMeshID, ItemInfo* targetItem, int targetMeshID, bool cond);
void MoveObjCamera(GameVector* ideal, ItemInfo* camSlotId, int camMeshID, ItemInfo* targetItem, int targetMeshID);
void RefreshFixedCamera(short camNumber);

void SetScreenFadeOut(float speed);
void SetScreenFadeIn(float speed);
void SetCinematicBars(float height, float speed);
void ClearCinematicBars();
void UpdateFadeScreenAndCinematicBars();
void HandleOptics(ItemInfo* item);
void UpdateMikePos(ItemInfo* item);
void ClearObjCamera();

float GetParticleDistanceFade(Vector3i position);
