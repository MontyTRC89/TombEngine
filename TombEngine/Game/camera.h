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

// TODO: Prepared for later refactor.
/*struct CameraSphereData
{
	Vector3 Position		 = Vector3::Zero;
	int		RoomNumber		 = 0;
	Vector3 PositionTarget	 = Vector3::Zero;
	int		RoomNumberTarget = 0;

	Vector3 LookAt				   = Vector3::Zero;
	int		LookAtRoomNumber	   = 0;
	Vector3 LookAtTarget		   = Vector3::Zero;
	int		LookAtRoomNumberTarget = 0;

	short AzimuthAngle		  = 0;
	short AzimuthAngleTarget  = 0;
	short AltitudeAngle		  = 0;
	short AltitudeAngleTarget = 0;
	float Distance		   = 0.0f;
	float DistanceTarget   = 0.0f;

	float SpeedAlpha = 0.0f;
};

class CameraObject
{
private:

public:
	CameraSphereData Sphere = {};

private:
};*/

struct CAMERA_INFO
{
	// Camera sphere
	Vector3 Position		 = Vector3::Zero;
	int		RoomNumber		 = 0;
	Vector3 LookAt			 = Vector3::Zero;
	int		LookAtRoomNumber = 0;
	short	actualAngle		 = 0; // AzimuthAngle
	short	targetAngle		 = 0;
	short	actualElevation	 = 0; // AltitudeAngle
	short	targetElevation	 = 0;
	float	targetDistance	 = 0.0f;
	float	speed			 = 0.0f;
	float	targetspeed		 = 0.0f;

	EulerAngles Rotation = EulerAngles::Identity;

	Vector3 RelShift = Vector3::Zero;

	CameraType type	   = CameraType::Chase;
	CameraType oldType = CameraType::Chase;
	CameraFlag flags   = CameraFlag::None;

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

std::pair<Vector3, int> CameraCollisionBounds(const Vector3& pos, int roomNumber, int push, bool yFirst);

void LookCamera(const ItemInfo& playerItem, const CollisionInfo& coll);

void LookAt(CAMERA_INFO& camera, short roll);
void AlterFOV(short value, bool store = true);
short GetCurrentFOV();
void InitializeCamera();
void MoveCamera(const ItemInfo& playerItem, Vector3 ideal, int idealRoomNumber, float speed);
void ChaseCamera(const ItemInfo& playerItem);
void CombatCamera(const ItemInfo& playerItem);
void UpdateCameraSphere(const ItemInfo& playerItem);
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
