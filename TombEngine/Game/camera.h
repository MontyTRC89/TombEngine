#pragma once
#include "Game/items.h"
#include "Math/Math.h"

struct CollisionInfo;

using namespace TEN::Math;

constexpr auto LOOKCAM_ORIENT_CONSTRAINT = std::pair<EulerAngles, EulerAngles>(
	EulerAngles(ANGLE(-70.0f), ANGLE(-90.0f), 0),
	EulerAngles(ANGLE(60.0f), ANGLE(90.0f), 0));

enum class CameraType
{
	Chase,
	Fixed,
	Look,
	Combat,
	Heavy,
	Object
};

enum class CameraFlag
{
	None,
	FollowCenter,
	NoChunky,
	ChaseObject
};

struct CAMERA_INFO
{
	// Camera sphere
	GameVector pos			   = GameVector::Zero;
	GameVector target		   = GameVector::Zero; // LookAt
	short	   actualAngle	   = 0;				   // AzimuthAngle
	short	   targetAngle	   = 0;
	short	   actualElevation = 0;				   // AltitudeAngle
	short	   targetElevation = 0;
	float	   targetDistance  = 0.0f;

	CameraType type	= CameraType::Chase;
	CameraType oldType = CameraType::Chase;
	CameraFlag flags = CameraFlag::None;
	float speed;
	float targetspeed;
	int bounce;
	int shift;
	int numberFrames;

	int laraNode;
	int box;
	int number;
	int last;
	int timer;
	ItemInfo* item;
	ItemInfo* lastItem;
	int mikeAtLara;
	Vector3i mikePos;

	bool fixedCamera			 = false;
	bool underwater				 = false;
	bool IsControllingTankCamera = false;
};

struct ObjectCameraInfo
{
	GameVector LastAngle;
	bool ItemCameraOn;
};

constexpr auto FADE_SCREEN_SPEED = 16.0f / 255.0f;
constexpr auto DEFAULT_FOV = 80.0f;

extern CAMERA_INFO Camera;
extern GameVector ForcedFixedCamera;
extern int UseForcedFixedCamera;
extern CameraType BinocularOldCamera;
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

void LookCamera(const ItemInfo& item, const CollisionInfo& coll);

void LookAt(CAMERA_INFO* cam, short roll);
void AlterFOV(short value, bool store = true);
short GetCurrentFOV();
void InitializeCamera();
void MoveCamera(GameVector* ideal, float speed);
void ChaseCamera(const ItemInfo& playerItem);
void CombatCamera(const ItemInfo& playerItem);
void UpdateCameraSphere(const ItemInfo& playerItem);
bool CameraCollisionBounds(GameVector* ideal, int push, bool yFirst);
void FixedCamera();
void BounceCamera(ItemInfo* item, int bounce, float distMax);
void BinocularCamera(ItemInfo* item);
void ConfirmCameraTargetPos();
void CalculateCamera(ItemInfo& playerItem, const CollisionInfo& coll);
void RumbleScreen();
bool TestBoundsCollideCamera(const GameBoundingBox& bounds, const Pose& pose, float radius);
void ObjCamera(ItemInfo* item, int boneID, ItemInfo* targetItem, int targetBoneID, bool cond);
void MoveObjCamera(GameVector* ideal, ItemInfo* item, int boneID, ItemInfo* targetItem, int targetBoneID);
void RefreshFixedCamera(int cameraID);

void SetScreenFadeOut(float speed, bool force = false);
void SetScreenFadeIn(float speed, bool force = false);
void SetCinematicBars(float height, float speed);
void ClearCinematicBars();
void UpdateFadeScreenAndCinematicBars();
void UpdateMikePos(const ItemInfo& item);
void ClearObjCamera();

float GetParticleDistanceFade(const Vector3i& pos);
