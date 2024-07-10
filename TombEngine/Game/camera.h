#pragma once
#include "Game/items.h"
#include "Math/Math.h"

struct CollisionInfo;

using namespace TEN::Math;

constexpr auto DEFAULT_FOV				 = 80.0f;
constexpr auto FADE_SCREEN_SPEED		 = 16.0f / 255.0f;
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
	Vector3 LookAt			 = Vector3::Zero;
	int		LookAtRoomNumber = 0;

	EulerAngles Orientation = EulerAngles::Identity;
	float		Distance	= 0.0f;
};

class CameraObject
{
private:
	float _speedAlpha = 0.0f;

public:
	CameraSphereData Sphere		  = {};
	CameraSphereData SphereTarget = {};

private:
	void UpdateSphere()
	{
		auto posDelta = Vector3::Lerp(Sphere.Position, SphereTarget.Position, _speedAlpha) - Sphere.Position;
		auto posDir = SphereTarget.Position - Sphere.Position;
		posDir.Normalize();

		// Update position.
		Sphere.Position += posDelta;
		Sphere.RoomNumber = GetCollision(Sphere.Position, Sphere.RoomNumber, posDir, posDelta.Length()).RoomNumber;

		auto lookAtDelta = Vector3::Lerp(Sphere.LookAt, SphereTarget.LookAt, _speedAlpha) - Sphere.LookAt;
		auto lookAtDir = SphereTarget.LookAt - Sphere.LookAt;
		lookAtDir.Normalize();

		// Update lookAt.
		Sphere.LookAt += posDelta;
		Sphere.LookAtRoomNumber = GetCollision(Sphere.LookAt, Sphere.LookAtRoomNumber, lookAtDir, lookAtDelta.Length()).RoomNumber;

		// Update orientation and distance.
		Sphere.Orientation.Lerp(SphereTarget.Orientation, _speedAlpha);
		Sphere.Distance = Lerp(Sphere.Distance, SphereTarget.Distance, _speedAlpha);
	}
};*/

struct ObjectCameraInfo
{
	GameVector LastAngle	= GameVector::Zero;
	bool	   ItemCameraOn = false;
};

struct PrevCameraData
{
	Pose	pos	   = Pose::Zero;
	Pose	pos2   = Pose::Zero;
	Vector3 target = Vector3::Zero; // LookAt

	// Camera sphere

	short actualAngle	  = 0;
	short actualElevation = 0;
	short targetElevation = 0;
	float targetDistance  = 0.0f;

	// Player anim state

	int ActiveState = 0;
	int TargetState = 0;
};

struct CameraInfo
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
	float	Distance		 = 0.0f;
	float	targetDistance	 = 0.0f;
	float	speed			 = 0.0f;
	float	targetspeed		 = 0.0f;
	short	Fov				 = 0;
	short	PrevFov			 = 0;

	EulerAngles Rotation = EulerAngles::Identity;

	Vector3 RelShift = Vector3::Zero;

	CameraType type	   = CameraType::Chase;
	CameraType oldType = CameraType::Chase;
	CameraFlag flags   = CameraFlag::None;

	int bounce		 = 0;
	int shift		 = 0;
	int numberFrames = 0;
	int laraNode	 = 0;
	int box			 = 0;
	int number		 = 0;
	int last		 = 0;
	int timer		 = 0;

	ObjectCameraInfo ItemCamera = {};
	ItemInfo*		 item		= nullptr;
	ItemInfo*		 lastItem	= nullptr;

	Vector3 ListenerPosition = Vector3::Zero;

	bool fixedCamera			 = false;
	bool underwater				 = false;
	bool IsControllingTankCamera = false;

	int TargetSnaps	  = 0;
	int RumbleTimer	  = 0;
	int RumbleCounter = 0;

	GameVector ForcedFixedCamera	= GameVector::Zero;
	bool	   UseForcedFixedCamera = false;

	PrevCameraData PrevCamera			   = {};
	GameVector	   PrevTarget			   = GameVector::Zero;
	Vector3		   PrevIdeal			   = Vector3::Zero;
	int			   PrevIdealRoomNumber	   = 0;
	CameraType	   PrevBinocularCameraType = CameraType::Chase;
};

struct ScreenEffectData
{
	bool  ScreenFadedOut	= false;
	bool  ScreenFading		= false;
	float ScreenFadeSpeed	= 0.0f;
	float ScreenFadeStart	= 0.0f;
	float ScreenFadeEnd		= 0.0f;
	float ScreenFadeCurrent = 0.0f;

	float CinematicBarsHeight			 = 0.0f;
	float CinematicBarsDestinationHeight = 0.0f;
	float CinematicBarsSpeed			 = 0.0f;
};

extern CameraInfo		g_Camera;
extern ScreenEffectData g_ScreenEffect;

std::pair<Vector3, int> GetCameraWallShift(const Vector3& pos, int roomNumber, int push, bool yFirst);
EulerAngles				GetCameraControlRotation();

void UpdatePlayerRefCameraOrient(ItemInfo& item);
void LookCamera(const ItemInfo& playerItem, const CollisionInfo& coll);

void LookAt(CameraInfo& camera, short roll);
void SetFov(short fov, bool store = true);
short GetCurrentFov();
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
void UpdateListenerPosition(const ItemInfo& item);
void ClearObjCamera();

float GetParticleDistanceFade(const Vector3i& pos);
