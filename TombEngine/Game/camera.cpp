#include "framework.h"
#include "Game/camera.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Los.h"
#include "Game/control/los.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Game/room.h"
#include "Game/savegame.h"
#include "Game/Setup.h"
#include "Game/spotcam.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/trutils.h"

using namespace TEN::Collision::Los;
using namespace TEN::Effects::Environment;
using namespace TEN::Entities::Generic;
using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Utils;
using TEN::Renderer::g_Renderer;

constexpr auto CAMERA_OBJECT_COLL_DIST_THRESHOLD   = BLOCK(4);
constexpr auto CAMERA_OBJECT_COLL_EXTENT_THRESHOLD = CLICK(0.5f);

struct OLD_CAMERA
{
	Pose pos;
	Pose pos2;
	Vector3 target; // LookAt

	// Camera sphere
	short actualAngle	  = 0.0f;
	short actualElevation = 0;
	short targetElevation = 0;
	float targetDistance  = 0.0f;

	// Player anim state
	int ActiveState = 0;
	int TargetState = 0;
};

struct ObjectCameraInfo
{
	GameVector LastAngle;
	bool ItemCameraOn;
};

CAMERA_INFO		 Camera;
ScreenEffectData g_ScreenEffect;

OLD_CAMERA OldCam;
GameVector LastTarget;

Vector3 LastIdeal;
int LastIdealRoomNumber;

int TargetSnaps = 0;
ObjectCameraInfo ItemCamera;
GameVector ForcedFixedCamera;
bool UseForcedFixedCamera;

CameraType BinocularOldCamera;

short CurrentFOV;
short LastFOV;

int RumbleTimer = 0;
int RumbleCounter = 0;

// NOTE: Function label comments will be a temporary reference until a camera class
// is created to keep them neatly organised as private methods. -- Sezz 2024.03.13

// ----------------
// HELPER FUNCTIONS
// ----------------

// GetCameraLos()
// GetCameraRelativeShift()
// GetCameraPlayerOffset()

static bool TestCameraCollidableBox(const BoundingOrientedBox& box)
{
	// Test if any 2 box extents are smaller than threshold.
	if ((abs(box.Extents.x) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD && abs(box.Extents.y) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD) ||
		(abs(box.Extents.x) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD && abs(box.Extents.z) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD) ||
		(abs(box.Extents.y) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD && abs(box.Extents.z) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD))
	{
		return false;
	}

	return true;
}

static bool TestCameraCollidableItem(const ItemInfo& item)
{
	// 1) Check if item is player or bridge.
	if (item.IsLara() || item.IsBridge())
		return false;

	// 2) Test distance.
	float distSqr = Vector3i::DistanceSquared(item.Pose.Position, Camera.Position);
	if (distSqr >= SQUARE(CAMERA_OBJECT_COLL_DIST_THRESHOLD))
		return false;

	// 3) Check object collidability.
	const auto& object = Objects[item.ObjectNumber];
	if (!item.Collidable || !object.usingDrawAnimatingItem)
		return false;

	// 4) Check object attributes.
	if (object.intelligent || object.isPickup || object.isPuzzleHole || object.collision == nullptr)
		return false;

	// 5) Test if box is collidable.
	auto box = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);
	if (!TestCameraCollidableBox(box))
		return false;

	return true;
}

static bool TestCameraCollidableStatic(const MESH_INFO& staticObject)
{
	// 1) Test distance.
	float distSqr = Vector3i::DistanceSquared(Camera.Position, staticObject.pos.Position);
	if (distSqr >= SQUARE(CAMERA_OBJECT_COLL_DIST_THRESHOLD))
		return false;

	// 2) Check if static is visible.
	if (!(staticObject.flags & StaticMeshFlags::SM_VISIBLE))
		return false;

	// 3) Test if box is collidable.
	auto box = GetBoundsAccurate(staticObject, false).ToBoundingOrientedBox(staticObject.pos);
	if (!TestCameraCollidableBox(box))
		return false;

	return true;
}

static std::optional<std::pair<Vector3, int>> GetCameraLosIntersect(const Vector3& origin, int originRoomNumber, const Vector3& target)
{
	constexpr auto BUFFER = BLOCK(0.1f);

	auto dir = target - origin;
	dir.Normalize();
	float dist = Vector3::Distance(origin, target);

	// Run through LOS instances.
	auto losInstances = GetLosInstances(origin, originRoomNumber, dir, dist);
	for (const auto& losInstance : losInstances)
	{
		// Test object collidability (if applicable).
		if (losInstance.ObjectPtr.has_value())
		{
			// FAILSAFE: Ignore sphere.
			if (losInstance.SphereID != NO_VALUE)
				continue;

			if (std::holds_alternative<ItemInfo*>(*losInstance.ObjectPtr))
			{
				const auto& item = *std::get<ItemInfo*>(*losInstance.ObjectPtr);
				if (!TestCameraCollidableItem(item))
					continue;

			}
			else if (std::holds_alternative<MESH_INFO*>(*losInstance.ObjectPtr))
			{
				const auto& staticObject = *std::get<MESH_INFO*>(*losInstance.ObjectPtr);
				if (!TestCameraCollidableStatic(staticObject))
					continue;
			}
		}

		// TODO: Redo buffer.
		return std::pair(losInstance.Position, losInstance.RoomNumber);
	}

	// No intersection; return nullopt.
	return std::nullopt;
}

std::pair<Vector3, int> GetCameraWallShift(const Vector3& pos, int roomNumber, int push, bool yFirst)
{
	auto collidedPos = std::pair(pos, roomNumber);
	auto pointColl = CollisionResult{};

	if (yFirst)
	{
		pointColl = GetCollision(pos, roomNumber);

		int buffer = CLICK(1) - 1;
		if ((collidedPos.first.y - buffer) < pointColl.Position.Ceiling &&
			(collidedPos.first.y + buffer) > pointColl.Position.Floor &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			collidedPos.first.y = (pointColl.Position.Floor + pointColl.Position.Ceiling) / 2;
		}
		else if ((collidedPos.first.y + buffer) > pointColl.Position.Floor &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			collidedPos.first.y = pointColl.Position.Floor - buffer;
		}
		else if ((collidedPos.first.y - buffer) < pointColl.Position.Ceiling &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			collidedPos.first.y = pointColl.Position.Ceiling + buffer;
		}
	}

	pointColl = GetCollision(collidedPos.first.x - push, collidedPos.first.y, collidedPos.first.z, roomNumber);
	if (collidedPos.first.y > pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		collidedPos.first.y < pointColl.Position.Ceiling)
	{
		collidedPos.first.x = ((int)collidedPos.first.x & (~WALL_MASK)) + push;
	}

	pointColl = GetCollision(collidedPos.first.x, collidedPos.first.y, collidedPos.first.z - push, roomNumber);
	if (collidedPos.first.y > pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		collidedPos.first.y < pointColl.Position.Ceiling)
	{
		collidedPos.first.z = ((int)collidedPos.first.z & (~WALL_MASK)) + push;
	}

	pointColl = GetCollision(collidedPos.first.x + push, collidedPos.first.y, collidedPos.first.z, roomNumber);
	if (collidedPos.first.y > pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		collidedPos.first.y < pointColl.Position.Ceiling)
	{
		collidedPos.first.x = ((int)collidedPos.first.x | WALL_MASK) - push;
	}

	pointColl = GetCollision(collidedPos.first.x, collidedPos.first.y, collidedPos.first.z + push, roomNumber);
	if (collidedPos.first.y > pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		collidedPos.first.y < pointColl.Position.Ceiling)
	{
		collidedPos.first.z = ((int)collidedPos.first.z | WALL_MASK) - push;
	}

	if (!yFirst)
	{
		pointColl = GetCollision(collidedPos.first.x, collidedPos.first.y, collidedPos.first.z, roomNumber);

		int buffer = CLICK(1) - 1;
		if ((collidedPos.first.y - buffer) < pointColl.Position.Ceiling &&
			(collidedPos.first.y + buffer) > pointColl.Position.Floor &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			collidedPos.first.y = (pointColl.Position.Floor + pointColl.Position.Ceiling) / 2;
		}
		else if ((collidedPos.first.y + buffer) > pointColl.Position.Floor &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			collidedPos.first.y = pointColl.Position.Floor - buffer;
		}
		else if ((collidedPos.first.y - buffer) < pointColl.Position.Ceiling &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			collidedPos.first.y = pointColl.Position.Ceiling + buffer;
		}
	}

	pointColl = GetCollision(collidedPos.first.x, collidedPos.first.y, collidedPos.first.z, roomNumber);
	if (collidedPos.first.y > pointColl.Position.Floor ||
		collidedPos.first.y < pointColl.Position.Ceiling ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor)
	{
		return collidedPos;
	}

	collidedPos.second = pointColl.RoomNumber;
	return collidedPos;
}

static Vector3 GetCameraPlayerOffset(const ItemInfo& item, const CollisionInfo& coll)
{
	constexpr auto VERTICAL_OFFSET_DEFAULT		  = -BLOCK(0.05f);
	constexpr auto VERTICAL_OFFSET_SWAMP		  = BLOCK(0.4f);
	constexpr auto VERTICAL_OFFSET_MONKEY_SWING	  = BLOCK(0.25f);
	constexpr auto VERTICAL_OFFSET_TREADING_WATER = BLOCK(0.5f);

	const auto& player = GetLaraInfo(item);

	bool isInSwamp = TestEnvironment(ENV_FLAG_SWAMP, item.RoomNumber);

	// Determine contextual vertical offset.
	float verticalOffset = coll.Setup.Height;
	if (player.Control.IsMonkeySwinging)
	{
		verticalOffset -= VERTICAL_OFFSET_MONKEY_SWING;
	}
	else if (player.Control.WaterStatus == WaterStatus::TreadWater)
	{
		verticalOffset -= VERTICAL_OFFSET_TREADING_WATER;
	}
	else if (isInSwamp)
	{
		verticalOffset = VERTICAL_OFFSET_SWAMP;
	}
	else
	{
		verticalOffset -= VERTICAL_OFFSET_DEFAULT;
	}

	// Get floor-to-ceiling height.
	auto pointColl = GetCollision(item);
	int floorToCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);

	// Return offset.
	return Vector3(
		0.0f,
		-((verticalOffset < floorToCeilHeight) ? verticalOffset : floorToCeilHeight),
		0.0f);
}

// ----------------
// CAMERA FUNCTIONS
// ----------------

void UpdatePlayerRefCameraOrient(ItemInfo& item)
{
	auto& player = GetLaraInfo(item);

	float vel = Vector2(item.Animation.Velocity.x, item.Animation.Velocity.z).Length();

	bool isSpotCameraSwitch = (UseSpotCam != PrevUseSpotCam);
	bool isMoving = (GetMoveAxis() != Vector2::Zero || IsHeld(In::StepLeft) || IsHeld(In::StepRight) || vel != 0.0f);

	if (isSpotCameraSwitch && isMoving)
	{
		player.Control.LockRefCameraOrient = true;
	}
	else if (!isMoving)
	{
		player.Control.LockRefCameraOrient = false;
	}

	if (!player.Control.LockRefCameraOrient)
		player.Control.RefCameraOrient = EulerAngles(Camera.actualElevation, Camera.actualAngle, 0);

	g_Renderer.PrintDebugMessage("%d", (int)UseSpotCam);
	g_Renderer.PrintDebugMessage("%d", (int)Camera.type);
	g_Renderer.PrintDebugMessage("%d", (int)player.Control.LockRefCameraOrient);
	g_Renderer.PrintDebugMessage("%d", Camera.actualElevation);
	g_Renderer.PrintDebugMessage("%d", Camera.actualAngle);
}

void LookCamera(const ItemInfo& playerItem, const CollisionInfo& coll)
{
	constexpr auto DIST_COEFF = 0.5f;

	const auto& player = GetLaraInfo(playerItem);

	// Determine base position.
	bool isInSwamp = TestEnvironment(ENV_FLAG_SWAMP, playerItem.RoomNumber);
	auto basePos = Vector3(
		playerItem.Pose.Position.x,
		isInSwamp ? g_Level.Rooms[playerItem.RoomNumber].maxceiling : playerItem.Pose.Position.y,
		playerItem.Pose.Position.z);

	// Calculate direction.
	auto orient = player.Control.Look.Orientation + EulerAngles(playerItem.Pose.Orientation.x, playerItem.Pose.Orientation.y + Camera.targetAngle, 0);
	orient.x = std::clamp(orient.x, LOOKCAM_ORIENT_CONSTRAINT.first.x, LOOKCAM_ORIENT_CONSTRAINT.second.x);
	auto dir = -orient.ToDirection();

	float dist = Camera.targetDistance * DIST_COEFF;

	// Define landmarks.
	auto lookAt = basePos + GetCameraPlayerOffset(playerItem, coll);
	auto idealPos = std::pair(Geometry::TranslatePoint(lookAt, dir, dist), Camera.LookAtRoomNumber);

	// Calculate LOS intersection.
	auto intersect = GetCameraLosIntersect(Camera.LookAt, Camera.LookAtRoomNumber, idealPos.first);
	if (intersect.has_value())
		idealPos = *intersect;

	// Update camera.
	Camera.LookAt += (lookAt - Camera.LookAt) * (1.0f / Camera.speed);
	MoveCamera(playerItem, idealPos.first, idealPos.second, Camera.speed);
}

void LookAt(CAMERA_INFO& camera, short roll)
{
	float fov = TO_RAD(CurrentFOV / 1.333333f);
	float farView = BLOCK(g_GameFlow->GetLevel(CurrentLevel)->GetFarView());

	g_Renderer.UpdateCameraMatrices(camera, TO_RAD(roll), fov, farView);
}

void AlterFOV(short value, bool store)
{
	if (store)
		LastFOV = value;

	CurrentFOV = value;
}

short GetCurrentFOV()
{
	return CurrentFOV;
}

inline void RumbleFromBounce()
{
	Rumble(std::clamp(abs(Camera.bounce) / 70.0f, 0.0f, 0.8f), 0.2f);
}

void InitializeCamera()
{
	Camera.shift = LaraItem->Pose.Position.y - BLOCK(1);

	LastTarget = GameVector(
		LaraItem->Pose.Position.x,
		Camera.shift,
		LaraItem->Pose.Position.z,
		LaraItem->RoomNumber);

	Camera.LookAt = Vector3(LastTarget.x, Camera.shift, LastTarget.z);
	Camera.LookAtRoomNumber = LaraItem->RoomNumber;

	Camera.Position = Vector3(LastTarget.x, Camera.shift, LastTarget.z - 100);
	Camera.RoomNumber = LaraItem->RoomNumber;

	Camera.targetDistance = BLOCK(1.5f);
	Camera.item = nullptr;
	Camera.numberFrames = 1;
	Camera.type = CameraType::Chase;
	Camera.speed = 1.0f;
	Camera.flags = CameraFlag::None;
	Camera.bounce = 0;
	Camera.number = NO_VALUE;
	Camera.fixedCamera = false;

	AlterFOV(ANGLE(DEFAULT_FOV));

	UseForcedFixedCamera = false;
	CalculateCamera(*LaraItem, LaraCollision);

	// Fade in screen.
	SetScreenFadeIn(FADE_SCREEN_SPEED);
}

static void UpdateAzimuthAngle(const ItemInfo& item)
{
	constexpr auto BASE_VEL					= BLOCK(0.05f);
	constexpr auto BASE_ANGLE				= ANGLE(90.0f);
	constexpr auto AUTO_ROT_DELTA_ANGLE_MAX = BASE_ANGLE * 1.5f;
	constexpr auto AZIMUTH_ANGLE_LERP_ALPHA = 0.01f;

	if (!IsUsingModernControls() || IsPlayerStrafing(item))
		return;

	float vel = Vector2(item.Animation.Velocity.x, item.Animation.Velocity.z).Length();
	if (GetMoveAxis() == Vector2::Zero || vel == 0.0f)
		return;

	float alpha = std::clamp(vel / BASE_VEL, 0.0f, 1.0f);

	short deltaAngle = Geometry::GetShortestAngle(Camera.actualAngle, GetPlayerHeadingAngleY(item));
	if (abs(deltaAngle) <= BASE_ANGLE)
	{
		Camera.actualAngle += deltaAngle * (AZIMUTH_ANGLE_LERP_ALPHA * alpha);
	}
	else if (abs(deltaAngle) <= AUTO_ROT_DELTA_ANGLE_MAX)
	{
		int sign = std::copysign(1, deltaAngle);
		Camera.actualAngle += (BASE_ANGLE * (AZIMUTH_ANGLE_LERP_ALPHA * alpha)) * sign;
	}
}

void MoveCamera(const ItemInfo& playerItem, Vector3 idealPos, int idealRoomNumber, float speed)
{
	constexpr auto BUFFER = BLOCK(0.2f);

	const auto& player = GetLaraInfo(playerItem);

	if (player.Control.Look.IsUsingBinoculars)
		speed = 1.0f;

	UpdateAzimuthAngle(playerItem);
	UpdateListenerPosition(playerItem);

	OldCam.pos.Orientation = playerItem.Pose.Orientation;
	OldCam.pos2.Orientation.x = player.ExtraHeadRot.x;
	OldCam.pos2.Orientation.y = player.ExtraHeadRot.y;
	OldCam.pos2.Position.x = player.ExtraTorsoRot.x;
	OldCam.pos2.Position.y = player.ExtraTorsoRot.y;
	OldCam.pos.Position = playerItem.Pose.Position;
	OldCam.ActiveState = playerItem.Animation.ActiveState;
	OldCam.TargetState = playerItem.Animation.TargetState;
	OldCam.targetDistance = Camera.targetDistance;
	OldCam.targetElevation = Camera.targetElevation;
	OldCam.actualElevation = Camera.actualElevation;
	OldCam.actualAngle = Camera.actualAngle;
	OldCam.target = Camera.LookAt;
	LastIdeal = idealPos;
	LastIdealRoomNumber = idealRoomNumber;

	// Translate camera.
	Camera.Position = Vector3::Lerp(Camera.Position, idealPos, 1.0f / speed);
	Camera.RoomNumber = idealRoomNumber;

	// Assess LOS.
	auto intersect = GetCameraLosIntersect(Camera.LookAt, Camera.LookAtRoomNumber, Camera.Position);
	if (intersect.has_value())
	{
		Camera.Position = intersect->first;
		Camera.RoomNumber = intersect->second;
	}

	// Bounce.
	if (Camera.bounce != 0)
	{
		if (Camera.bounce <= 0)
		{
			int bounce = -Camera.bounce;
			int bounce2 = bounce / 2;

			Camera.LookAt.x += Random::GenerateInt() % bounce - bounce2;
			Camera.LookAt.y += Random::GenerateInt() % bounce - bounce2;
			Camera.LookAt.z += Random::GenerateInt() % bounce - bounce2;
			Camera.bounce += 5;
			RumbleFromBounce();
		}
		else
		{
			Camera.Position.y += Camera.bounce;
			Camera.LookAt.y += Camera.bounce;
			Camera.bounce = 0;
		}
	}

	// Avoid entering swamp rooms.
	if (TestEnvironment(ENV_FLAG_SWAMP, Camera.RoomNumber))
		Camera.Position.y = g_Level.Rooms[Camera.RoomNumber].y - BUFFER;

	Camera.RoomNumber = GetCollision(Camera.Position.x, Camera.Position.y, Camera.Position.z, Camera.RoomNumber).RoomNumber;
	LookAt(Camera, 0);
	Camera.oldType = Camera.type;
}

void ObjCamera(ItemInfo* item, int boneID, ItemInfo* targetItem, int targetBoneID, bool cond)
{
	// item and targetItem remain same object until it becomes possible to extend targetItem to another object.
	// Activates code below -> void CalculateCamera().
	ItemCamera.ItemCameraOn = cond;

	UpdateCameraSphere(*LaraItem);

	// Get position of bone 0.	
	auto pos = GetJointPosition(item, 0, Vector3i::Zero);
	auto target = GameVector(pos, item->RoomNumber);

	Camera.fixedCamera = true;

	MoveObjCamera(&target, item, boneID, targetItem, targetBoneID);
	Camera.timer = NO_VALUE;
}

void ClearObjCamera()
{
	ItemCamera.ItemCameraOn = false;
}

void MoveObjCamera(GameVector* ideal, ItemInfo* item, int boneID, ItemInfo* targetItem, int targetBoneID)
{
	constexpr auto ANGLE_THRESHOLD_DEG = 100.0f;

	auto idealPos = GetJointPosition(item, boneID, Vector3i::Zero).ToVector3();
	auto lookAt = GetJointPosition(targetItem, targetBoneID, Vector3i::Zero).ToVector3();

	if (OldCam.pos.Position != idealPos ||
		OldCam.targetDistance != Camera.targetDistance  ||
		OldCam.targetElevation != Camera.targetElevation ||
		OldCam.actualElevation != Camera.actualElevation ||
		OldCam.actualAngle != Camera.actualAngle ||
		OldCam.target != Camera.LookAt ||
		Camera.oldType != Camera.type ||
		Lara.Control.Look.IsUsingBinoculars)
	{
		OldCam.pos.Position = idealPos;
		OldCam.targetDistance = Camera.targetDistance;
		OldCam.targetElevation = Camera.targetElevation;
		OldCam.actualElevation = Camera.actualElevation;
		OldCam.actualAngle = Camera.actualAngle;
		OldCam.target = Camera.LookAt;
		LastIdeal = idealPos;
		LastIdealRoomNumber = ideal->RoomNumber;
		LastTarget = Vector3i(lookAt);
	}
	else
	{
		idealPos  = LastIdeal;
		lookAt = LastTarget.ToVector3();
		ideal->RoomNumber = LastIdealRoomNumber;
	}

	float speedAlpha = 1.0f;

	Camera.Position = Vector3::Lerp(Camera.Position, ideal->ToVector3(), speedAlpha);
	Camera.RoomNumber = GetCollision(Camera.Position, Camera.RoomNumber).RoomNumber;
	LookAt(Camera, 0);

	auto angle = Camera.LookAt - Camera.Position;
	auto position = Vector3i(Camera.LookAt - Camera.Position);

	// Store previous frame's camera angle to LastAngle to compare if next frame camera angle has bigger step than 100.
	// To move camera smoothely, speed alpha 0.5 is set.
	// To cut immediately, speed alpha 1 is set.

	if (LastTarget.x - Camera.LookAt.x > ANGLE_THRESHOLD_DEG ||
		LastTarget.y - Camera.LookAt.y > ANGLE_THRESHOLD_DEG ||
		LastTarget.z - Camera.LookAt.z > ANGLE_THRESHOLD_DEG)
	{
		speedAlpha = 1.0f;
	}
	else
	{
		speedAlpha = 0.5f;
	}

	// Move lookAt.
	Camera.LookAt = Vector3::Lerp(Camera.LookAt, lookAt, speedAlpha);

	if (ItemCamera.LastAngle != position)
	{
		ItemCamera.LastAngle = Vector3i(ItemCamera.LastAngle.x = angle.x, 
										ItemCamera.LastAngle.y = angle.y, 
										ItemCamera.LastAngle.z = angle.z);
	}
}

void RefreshFixedCamera(int cameraID)
{
	const auto& camera = g_Level.Cameras[cameraID];

	int speed = (camera.Speed * 8) + 1.0f;
	MoveCamera(*LaraItem, camera.Position.ToVector3(), camera.RoomNumber, speed);
}

static void ClampCameraAltitudeAngle(bool isUnderwater)
{
	constexpr auto MODERN_CAMERA_ABOVE_WATER_ANGLE_CONSTRAINT = std::pair<short, short>(ANGLE(-80.0f), ANGLE(70.0f));
	constexpr auto MODERN_CAMERA_UNDERWATER_ANGLE_CONSTRAINT  = std::pair<short, short>(ANGLE(-80.0f), ANGLE(80.0f));
	constexpr auto TANK_CAMERA_ANGLE_CONSTRAINT				  = std::pair<short, short>(ANGLE(-80.0f), ANGLE(70.0f));

	const auto& angleConstraint = IsUsingModernControls() ?
		(isUnderwater ? MODERN_CAMERA_UNDERWATER_ANGLE_CONSTRAINT : MODERN_CAMERA_ABOVE_WATER_ANGLE_CONSTRAINT) :
		TANK_CAMERA_ANGLE_CONSTRAINT;

	if (Camera.actualElevation > angleConstraint.second)
	{
		Camera.actualElevation = angleConstraint.second;
	}
	else if (Camera.actualElevation < angleConstraint.first)
	{
		Camera.actualElevation = angleConstraint.first;
	}
}

static bool TestCameraStrafeZoom(const ItemInfo& playerItem)
{
	const auto& player = GetLaraInfo(playerItem);

	if (!IsUsingModernControls())
		return false;

	if (player.Control.HandStatus == HandStatus::WeaponDraw ||
		player.Control.HandStatus == HandStatus::WeaponReady)
	{
		return false;
	}

	if (IsHeld(In::Look))
		return true;

	return false;
}

static void HandleCameraFollow(const ItemInfo& playerItem, bool isCombatCamera)
{
	constexpr auto STRAFE_CAMERA_FOV			   = ANGLE(86.0f);
	constexpr auto STRAFE_CAMERA_FOV_LERP_ALPHA	   = 0.5f;
	constexpr auto STRAFE_CAMERA_DIST_OFFSET_COEFF = 0.5f;
	constexpr auto STRAFE_CAMERA_ZOOM_BUFFER	   = BLOCK(0.1f);
	constexpr auto TANK_CAMERA_SWIVEL_STEP_COUNT   = 4;
	constexpr auto TANK_CAMERA_CLOSE_DIST_MIN	   = BLOCK(0.75f);

	const auto& player = GetLaraInfo(playerItem);

	ClampCameraAltitudeAngle(player.Control.WaterStatus == WaterStatus::Underwater);

	// Move camera.
	if (IsUsingModernControls() || Camera.IsControllingTankCamera)
	{
		// Calcuate direction.
		auto dir = -EulerAngles(Camera.actualElevation, Camera.actualAngle, 0).ToDirection();

		// Calcuate ideal position.
		auto idealPos = std::pair(
			Geometry::TranslatePoint(Camera.LookAt, dir, Camera.targetDistance), Camera.LookAtRoomNumber);
		
		// Assess LOS.
		auto intersect = GetCameraLosIntersect(Camera.LookAt, Camera.LookAtRoomNumber, idealPos.first);
		if (intersect.has_value())
			idealPos = *intersect;

		// Apply strafe camera effects.
		if (IsPlayerStrafing(playerItem))
		{
			AlterFOV((short)Lerp(CurrentFOV, STRAFE_CAMERA_FOV, STRAFE_CAMERA_FOV_LERP_ALPHA));

			// Apply zoom if using Look action to strafe.
			if (TestCameraStrafeZoom(playerItem))
			{
				float dist = std::max(Vector3::Distance(Camera.LookAt, idealPos.first) * STRAFE_CAMERA_DIST_OFFSET_COEFF, STRAFE_CAMERA_ZOOM_BUFFER);
				idealPos = std::pair(
					Geometry::TranslatePoint(Camera.LookAt, dir, dist),
					GetCollision(Camera.LookAt, Camera.LookAtRoomNumber, dir, dist).RoomNumber);
			}
		}
		else
		{
			AlterFOV((short)Lerp(CurrentFOV, ANGLE(DEFAULT_FOV), STRAFE_CAMERA_FOV_LERP_ALPHA / 2));
		}

		// Update camera.
		float speedCoeff = (Camera.type != CameraType::Look) ? 0.2f : 1.0f;
		MoveCamera(playerItem, idealPos.first, idealPos.second, Camera.speed * speedCoeff);
	}
	else
	{
		auto farthestIdealPos = std::pair<Vector3, int>(Camera.Position, Camera.RoomNumber);
		short farthestIdealAzimuthAngle = Camera.actualAngle;
		float farthestDistSqr = INFINITY;

		// Determine ideal position around player.
		for (int i = 0; i < (TANK_CAMERA_SWIVEL_STEP_COUNT + 1); i++)
		{
			// Calcuate azimuth angle and direction of swivel step.
			short azimuthAngle = (i == 0) ? Camera.actualAngle : (ANGLE(360.0f / TANK_CAMERA_SWIVEL_STEP_COUNT) * (i - 1));
			auto dir = -EulerAngles(Camera.actualElevation, azimuthAngle, 0).ToDirection();

			// Calcuate ideal position.
			auto idealPos = std::pair(
				Geometry::TranslatePoint(Camera.LookAt, dir, Camera.targetDistance), Camera.LookAtRoomNumber);

			// Assess LOS.
			auto intersect = GetCameraLosIntersect(Camera.LookAt, Camera.LookAtRoomNumber, idealPos.first);
			if (intersect.has_value())
				idealPos = *intersect;

			// Has no LOS intersection.
			if (!intersect.has_value())
			{
				// Initial swivel is clear; set ideal.
				if (i == 0)
				{
					farthestIdealPos = idealPos;
					farthestIdealAzimuthAngle = azimuthAngle;
					break;
				}

				// Track closest ideal.
				float distSqr = Vector3::DistanceSquared(Camera.Position, idealPos.first);
				if (distSqr < farthestDistSqr)
				{
					farthestIdealPos = idealPos;
					farthestIdealAzimuthAngle = azimuthAngle;
					farthestDistSqr = distSqr;
				}
			}
			// Has LOS intersection and is initial swivel step.
			else if (i == 0)
			{
				float distSqr = Vector3::DistanceSquared(Camera.LookAt, idealPos.first);
				if (distSqr > SQUARE(TANK_CAMERA_CLOSE_DIST_MIN))
				{
					farthestIdealPos = idealPos;
					farthestIdealAzimuthAngle = azimuthAngle;
					break;
				}
			}
		}

		Camera.actualAngle = farthestIdealAzimuthAngle;
		farthestIdealPos = GetCameraWallShift(farthestIdealPos.first, farthestIdealPos.second, CLICK(1.5f), true);

		if (isCombatCamera)
		{
			// Snap position of fixed camera type.
			if (Camera.oldType == CameraType::Fixed)
				Camera.speed = 1.0f;
		}

		// Update camera.
		MoveCamera(playerItem, farthestIdealPos.first, farthestIdealPos.second, Camera.speed);
	}
}

void ChaseCamera(const ItemInfo& playerItem)
{
	const auto& player = GetLaraInfo(playerItem);

	if (Camera.targetElevation == 0)
		Camera.targetElevation = ANGLE(-10.0f);

	Camera.targetElevation += playerItem.Pose.Orientation.x;
	UpdateCameraSphere(playerItem);

	auto pointColl = GetCollision(Camera.LookAt, Camera.LookAtRoomNumber, 0, 0, CLICK(1));

	if (TestEnvironment(ENV_FLAG_SWAMP, pointColl.RoomNumber))
		Camera.LookAt.y = g_Level.Rooms[pointColl.RoomNumber].maxceiling - CLICK(1);

	int vPos = Camera.LookAt.y;
	pointColl = GetCollision(Camera.LookAt.x, vPos, Camera.LookAt.z, Camera.LookAtRoomNumber);
	if (((vPos < pointColl.Position.Ceiling || pointColl.Position.Floor < vPos) || pointColl.Position.Floor <= pointColl.Position.Ceiling) ||
		(pointColl.Position.Floor == NO_HEIGHT || pointColl.Position.Ceiling == NO_HEIGHT))
	{
		TargetSnaps++;
		Camera.LookAt = LastTarget.ToVector3();
		Camera.LookAtRoomNumber = LastTarget.RoomNumber;
	}
	else
	{
		TargetSnaps = 0;
	}

	HandleCameraFollow(playerItem, false);
}

void CombatCamera(const ItemInfo& playerItem)
{
	constexpr auto BUFFER = CLICK(0.25f);

	const auto& player = GetLaraInfo(playerItem);

	Camera.LookAt.x = playerItem.Pose.Position.x;
	Camera.LookAt.z = playerItem.Pose.Position.z;

	if (player.TargetEntity != nullptr)
	{
		Camera.targetAngle = player.TargetArmOrient.y;
		Camera.targetElevation = player.TargetArmOrient.x + playerItem.Pose.Orientation.x;
	}
	else
	{
		Camera.targetAngle = player.ExtraHeadRot.y + player.ExtraTorsoRot.y;
		Camera.targetElevation = player.ExtraHeadRot.x + player.ExtraTorsoRot.x + playerItem.Pose.Orientation.x - ANGLE(15.0f);
	}

	auto pointColl = GetCollision(Camera.LookAt, Camera.LookAtRoomNumber, 0, 0, CLICK(1));
	if (TestEnvironment(ENV_FLAG_SWAMP, pointColl.RoomNumber))
		Camera.LookAt.y = g_Level.Rooms[pointColl.RoomNumber].y - CLICK(1);

	pointColl = GetCollision(Camera.LookAt.x, Camera.LookAt.y, Camera.LookAt.z, Camera.LookAtRoomNumber);
	Camera.LookAtRoomNumber = pointColl.RoomNumber;

	if ((pointColl.Position.Ceiling + BUFFER) > (pointColl.Position.Floor - BUFFER) &&
		pointColl.Position.Floor != NO_HEIGHT &&
		pointColl.Position.Ceiling != NO_HEIGHT)
	{
		Camera.LookAt.y = (pointColl.Position.Ceiling + pointColl.Position.Floor) / 2;
		Camera.targetElevation = 0;
	}
	else if (Camera.LookAt.y > (pointColl.Position.Floor - BUFFER) &&
		pointColl.Position.Floor != NO_HEIGHT)
	{
		Camera.LookAt.y = pointColl.Position.Floor - BUFFER;
		Camera.targetElevation = 0;
	}
	else if (Camera.LookAt.y < (pointColl.Position.Ceiling + BUFFER) &&
		pointColl.Position.Ceiling != NO_HEIGHT)
	{
		Camera.LookAt.y = pointColl.Position.Ceiling + BUFFER;
		Camera.targetElevation = 0;
	}

	int y = Camera.LookAt.y;
	pointColl = GetCollision(Camera.LookAt.x, y, Camera.LookAt.z, Camera.LookAtRoomNumber);
	Camera.LookAtRoomNumber = pointColl.RoomNumber;

	if (y < pointColl.Position.Ceiling ||
		y > pointColl.Position.Floor ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT)
	{
		TargetSnaps++;
		Camera.LookAt = LastTarget.ToVector3();
		Camera.LookAtRoomNumber = LastTarget.RoomNumber;
	}
	else
	{
		TargetSnaps = 0;
	}

	UpdateCameraSphere(playerItem);

	Camera.targetDistance = BLOCK(1.5f);

	HandleCameraFollow(playerItem, true);
}

static EulerAngles GetCameraControlRotation()
{
	constexpr auto SLOW_ROT_COEFF				 = 0.4f;
	constexpr auto MOUSE_AXIS_SENSITIVITY_COEFF	 = 20.0f;
	constexpr auto CAMERA_AXIS_SENSITIVITY_COEFF = 12.0f;
	constexpr auto SMOOTHING_FACTOR				 = 8.0f;

	bool isUsingMouse = (GetCameraAxis() == Vector2::Zero);
	auto axisSign = Vector2(g_Configuration.InvertCameraXAxis ? -1 : 1, g_Configuration.InvertCameraYAxis ? -1 : 1);

	// Calculate axis.
	auto axis = (isUsingMouse ? GetMouseAxis() : GetCameraAxis()) * axisSign;
	float sensitivityCoeff = isUsingMouse ? MOUSE_AXIS_SENSITIVITY_COEFF : CAMERA_AXIS_SENSITIVITY_COEFF;
	float sensitivity = sensitivityCoeff / (1.0f + (abs(axis.x) + abs(axis.y)));
	axis *= sensitivity * (isUsingMouse ? SMOOTHING_FACTOR : 1.0f);

	// Calculate and return rotation.
	auto rotCoeff = IsHeld(In::Walk) ? SLOW_ROT_COEFF : 1.0f;
	return EulerAngles(ANGLE(axis.x), ANGLE(axis.y), 0) * rotCoeff;
}

static bool CanControlTankCamera()
{
	if (!g_Configuration.EnableTankCameraControl)
		return false;

	bool isUsingMouse = (GetCameraAxis() == Vector2::Zero);
	const auto& axis = isUsingMouse ? GetMouseAxis() : GetCameraAxis();

	// Test if player is stationary.
	if (!IsWakeActionHeld() && (axis != Vector2::Zero || Camera.IsControllingTankCamera))
		return true;

	// Test if player is moving and camera or mouse axis isn't zero.
	if (IsWakeActionHeld() && axis != Vector2::Zero)
		return true;

	return false;
}

void UpdateCameraSphere(const ItemInfo& playerItem)
{
	constexpr auto CONTROLLED_CAMERA_ROT_LERP_ALPHA = 0.75f;
	constexpr auto COMBAT_CAMERA_REBOUND_ALPHA		= 0.3f;
	constexpr auto LOCKED_CAMERA_ALTITUDE_ROT_ALPHA = 1 / 8.0f;

	if (Camera.laraNode != NO_VALUE)
	{
		auto origin = GetJointPosition(playerItem, Camera.laraNode, Vector3i::Zero);
		auto target = GetJointPosition(playerItem, Camera.laraNode, Vector3i(0, -CLICK(1), BLOCK(2)));
		auto deltaPos = target - origin;

		Camera.actualAngle = Camera.targetAngle + FROM_RAD(atan2(deltaPos.x, deltaPos.z));
		Camera.actualElevation += (Camera.targetElevation - Camera.actualElevation) * LOCKED_CAMERA_ALTITUDE_ROT_ALPHA;
		Camera.Rotation = EulerAngles::Identity;
	}
	else
	{
		if (IsUsingModernControls())
		{
			Camera.Rotation.Lerp(GetCameraControlRotation(), CONTROLLED_CAMERA_ROT_LERP_ALPHA);

			if (IsPlayerInCombat(playerItem))
			{
				short azimuthRot = Geometry::GetShortestAngle(Camera.actualAngle, (playerItem.Pose.Orientation.y + Camera.targetAngle) + Camera.Rotation.x);
				short altitudeRot = Geometry::GetShortestAngle(Camera.actualElevation, Camera.targetElevation - Camera.Rotation.y);

				Camera.actualAngle += azimuthRot * COMBAT_CAMERA_REBOUND_ALPHA;
				Camera.actualElevation += altitudeRot * COMBAT_CAMERA_REBOUND_ALPHA;
			}
			else
			{
				Camera.actualAngle += Camera.Rotation.x;
				Camera.actualElevation -= Camera.Rotation.y;
			}
		}
		else
		{
			if (CanControlTankCamera())
			{
				Camera.Rotation.Lerp(GetCameraControlRotation(), CONTROLLED_CAMERA_ROT_LERP_ALPHA);

				Camera.actualAngle += Camera.Rotation.x;
				Camera.actualElevation -= Camera.Rotation.y;
				Camera.IsControllingTankCamera = true;
			}
			else
			{
				Camera.actualAngle = playerItem.Pose.Orientation.y + Camera.targetAngle;
				Camera.actualElevation += (Camera.targetElevation - Camera.actualElevation) * LOCKED_CAMERA_ALTITUDE_ROT_ALPHA;
				Camera.Rotation.Lerp(EulerAngles::Identity, CONTROLLED_CAMERA_ROT_LERP_ALPHA);
				Camera.IsControllingTankCamera = false;
			}
		}
	}
}

void FixedCamera()
{
	// Fixed cameras before TR3 had optional "movement" effect. 
	// Later for some reason it was forced to always be 1, and actual speed value
	// from camera trigger was ignored. In TEN, speed value was moved out out of legacy
	// floordata trigger to camera itself to make use of it again. Still, by default,
	// value is 1 for UseForcedFixedCamera hack.

	float speed = 1.0f;

	auto origin = GameVector::Zero;
	if (UseForcedFixedCamera)
	{
		origin = ForcedFixedCamera;
	}
	else
	{
		const auto& camera = g_Level.Cameras[Camera.number];

		origin = GameVector(camera.Position, camera.RoomNumber);
		speed = (camera.Speed * 8) + 1.0f; // Multiply original speed by 8 to comply with original bitshifted speed from TR1-2.
	}

	Camera.fixedCamera = true;

	MoveCamera(*LaraItem, origin.ToVector3(), origin.RoomNumber, speed);

	if (Camera.timer)
	{
		if (!--Camera.timer)
			Camera.timer = NO_VALUE;
	}
}

void BounceCamera(ItemInfo* item, int bounce, float distMax)
{
	float dist = Vector3i::Distance(item->Pose.Position.ToVector3(), Camera.Position);
	if (dist < distMax)
	{
		if (distMax == -1)
		{
			Camera.bounce = bounce;
		}
		else
		{
			Camera.bounce = -(bounce * (distMax - dist) / distMax);
		}
	}
	else if (distMax == -1)
	{
		Camera.bounce = bounce;
	}
}

void BinocularCamera(ItemInfo* item)
{
	auto& player = GetLaraInfo(*item);

	if (!player.Control.Look.IsUsingLasersight)
	{
		if (IsClicked(In::Deselect) ||
			IsClicked(In::Draw) ||
			IsClicked(In::Look) ||
			IsHeld(In::Flare))
		{
			ResetPlayerFlex(item);
			player.Control.Look.OpticRange = 0;
			player.Control.Look.IsUsingBinoculars = false;
			player.Inventory.IsBusy = false;

			Camera.type = BinocularOldCamera;
			AlterFOV(LastFOV);
			return;
		}
	}

	AlterFOV(7 * (ANGLE(11.5f) - player.Control.Look.OpticRange), false);

	int x = item->Pose.Position.x;
	int y = item->Pose.Position.y - CLICK(2);
	int z = item->Pose.Position.z;

	auto probe = GetCollision(x, y, z, item->RoomNumber);
	if (probe.Position.Ceiling <= (y - CLICK(1)))
	{
		y -= CLICK(1);
	}
	else
	{
		y = probe.Position.Ceiling + CLICK(0.25f);
	}

	Camera.Position.x = x;
	Camera.Position.y = y;
	Camera.Position.z = z;
	Camera.RoomNumber = probe.RoomNumber;

	float l = BLOCK(20.25f) * phd_cos(player.Control.Look.Orientation.x);
	float tx = x + l * phd_sin(item->Pose.Orientation.y + player.Control.Look.Orientation.y);
	float ty = y - BLOCK(20.25f) * phd_sin(player.Control.Look.Orientation.x);
	float tz = z + l * phd_cos(item->Pose.Orientation.y + player.Control.Look.Orientation.y);

	if (Camera.oldType == CameraType::Fixed)
	{
		Camera.LookAt.x = tx;
		Camera.LookAt.y = ty;
		Camera.LookAt.z = tz;
		Camera.LookAtRoomNumber = item->RoomNumber;
	}
	else
	{
		Camera.LookAt.x += (tx - Camera.LookAt.x) / 4;
		Camera.LookAt.y += (ty - Camera.LookAt.y) / 4;
		Camera.LookAt.z += (tz - Camera.LookAt.z) / 4;
		Camera.LookAtRoomNumber = item->RoomNumber;
	}

	if (Camera.bounce &&
		Camera.type == Camera.oldType)
	{
		if (Camera.bounce <= 0)
		{
			Camera.LookAt.x += (CLICK(0.25f) / 4) * (Random::GenerateInt() % (-Camera.bounce) - (-Camera.bounce / 2));
			Camera.LookAt.y += (CLICK(0.25f) / 4) * (Random::GenerateInt() % (-Camera.bounce) - (-Camera.bounce / 2));
			Camera.LookAt.z += (CLICK(0.25f) / 4) * (Random::GenerateInt() % (-Camera.bounce) - (-Camera.bounce / 2));
			Camera.bounce += 5;
			RumbleFromBounce();
		}
		else
		{
			Camera.bounce = 0;
			Camera.LookAt.y += Camera.bounce;
		}
	}

	Camera.LookAtRoomNumber = GetCollision(Camera.Position.x, Camera.Position.y, Camera.Position.z, Camera.LookAtRoomNumber).RoomNumber;
	LookAt(Camera, 0);
	UpdateListenerPosition(*item);
	Camera.oldType = Camera.type;

	auto origin0 = GameVector(Camera.Position, Camera.RoomNumber);
	auto target0 = GameVector(Camera.LookAt, Camera.LookAtRoomNumber);
	GetTargetOnLOS(&origin0, &target0, false, false);
	Camera.LookAt = target0.ToVector3();
	Camera.LookAtRoomNumber = target0.RoomNumber;

	if (IsHeld(In::Action))
	{
		auto origin = Vector3i(Camera.Position);
		auto target = Vector3i(Camera.LookAt);
		LaraTorch(&origin, &target, player.ExtraHeadRot.y, 192);
	}
}

void ConfirmCameraTargetPos()
{
	auto pos = Vector3(
		LaraItem->Pose.Position.x,
		LaraItem->Pose.Position.y - (LaraCollision.Setup.Height / 2),
		LaraItem->Pose.Position.z);

	if (Camera.laraNode != NO_VALUE)
	{
		Camera.LookAt = pos;
	}
	else
	{
		Camera.LookAt = Vector3(LaraItem->Pose.Position.x, (Camera.LookAt.y + pos.y) / 2, LaraItem->Pose.Position.z);
	}

	int y = Camera.LookAt.y;
	auto pointColl = GetCollision(Camera.LookAt.x, y, Camera.LookAt.z, Camera.LookAtRoomNumber);
	if (y < pointColl.Position.Ceiling ||
		pointColl.Position.Floor < y ||
		pointColl.Position.Floor <= pointColl.Position.Ceiling ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT)
	{
		Camera.LookAt = pos;
	}
}

void CalculateCamera(ItemInfo& playerItem, const CollisionInfo& coll)
{
	auto& player = GetLaraInfo(playerItem);

	if (player.Control.Look.IsUsingBinoculars)
	{
		BinocularCamera(&playerItem);
		return;
	}

	if (ItemCamera.ItemCameraOn)
		return;

	if (UseForcedFixedCamera)
	{
		Camera.type = CameraType::Fixed;
		if (Camera.oldType != CameraType::Fixed)
			Camera.speed = 1.0f;
	}

	// Play water sound effect if camera is in water room.
	if (TestEnvironment(ENV_FLAG_WATER, Camera.RoomNumber))
	{
		SoundEffect(SFX_TR4_UNDERWATER, nullptr, SoundEnvironment::Always);
		if (Camera.underwater == false)
			Camera.underwater = true;
	}
	else
	{
		if (Camera.underwater == true)
			Camera.underwater = false;
	}

	const ItemInfo* itemPtr = nullptr;
	bool isFixedCamera = false;
	if (Camera.item != nullptr && (Camera.type == CameraType::Fixed || Camera.type == CameraType::Heavy))
	{
		itemPtr = Camera.item;
		isFixedCamera = true;
	}
	else
	{
		itemPtr = &playerItem;
		isFixedCamera = false;
	}

	// TODO: Use DX box.
	auto box = GameBoundingBox(itemPtr).ToBoundingOrientedBox(itemPtr->Pose);
	auto bounds = GameBoundingBox(itemPtr);

	int x = 0;
	int y = itemPtr->Pose.Position.y + bounds.Y2 + ((bounds.Y1 - bounds.Y2) / 2 * 1.5f);
	int z = 0;
	if (itemPtr->IsLara())
	{
		float heightCoeff = IsUsingModernControls() ? 0.9f : 0.75f;
		auto offset = GetCameraPlayerOffset(*itemPtr, coll) * heightCoeff;
		y = itemPtr->Pose.Position.y + offset.y;
	}

	// Make player look toward target item.
	if (Camera.item != nullptr)
	{
		if (!isFixedCamera)
		{
			auto deltaPos = Camera.item->Pose.Position - itemPtr->Pose.Position;
			float dist = Vector3i::Distance(Camera.item->Pose.Position, itemPtr->Pose.Position);

			auto lookOrient = EulerAngles(
				phd_atan(dist, y - (bounds.Y1 + bounds.Y2) / 2 - Camera.item->Pose.Position.y),
				phd_atan(deltaPos.z, deltaPos.x) - itemPtr->Pose.Orientation.y,
				0) / 2;

			if (lookOrient.y > ANGLE(-50.0f) &&	lookOrient.y < ANGLE(50.0f) &&
				lookOrient.z > ANGLE(-85.0f) && lookOrient.z < ANGLE(85.0f))
			{
				short angleDelta = lookOrient.y - player.ExtraHeadRot.y;
				if (angleDelta > ANGLE(4.0f))
				{
					player.ExtraHeadRot.y += ANGLE(4.0f);
				}
				else if (angleDelta < ANGLE(-4.0f))
				{
					player.ExtraHeadRot.y -= ANGLE(4.0f);
				}
				else
				{
					player.ExtraHeadRot.y += angleDelta;
				}
				player.ExtraTorsoRot.y = player.ExtraHeadRot.y;

				angleDelta = lookOrient.z - player.ExtraHeadRot.x;
				if (angleDelta > ANGLE(4.0f))
				{
					player.ExtraHeadRot.x += ANGLE(4.0f);
				}
				else if (angleDelta < ANGLE(-4.0f))
				{
					player.ExtraHeadRot.x -= ANGLE(4.0f);
				}
				else
				{
					player.ExtraHeadRot.x += angleDelta;
				}
				player.ExtraTorsoRot.x = player.ExtraHeadRot.x;

				player.Control.Look.Orientation = lookOrient;

				Camera.type = CameraType::Look;
				Camera.item->LookedAt = true;
			}
		}
	}

	if (Camera.type == CameraType::Look ||
		Camera.type == CameraType::Combat)
	{
		if (Camera.type == CameraType::Combat)
		{
			LastTarget = GameVector(Camera.LookAt, Camera.LookAtRoomNumber);

			if (!IsUsingModernControls())
				y -= CLICK(1);
		}

		Camera.LookAtRoomNumber = itemPtr->RoomNumber;

		if (Camera.fixedCamera || player.Control.Look.IsUsingBinoculars)
		{
			Camera.LookAt.y = y;
			Camera.speed = 1.0f;
		}
		else
		{
			Camera.LookAt.y += (y - Camera.LookAt.y) / 4;
			Camera.speed = (Camera.type != CameraType::Look) ? 8.0f : 4.0f;
		}

		Camera.fixedCamera = false;
		if (Camera.type == CameraType::Look)
		{
			LookCamera(*itemPtr, coll);
		}
		else
		{
			CombatCamera(*itemPtr);
		}
	}
	else
	{
		LastTarget = GameVector(Camera.LookAt, Camera.LookAtRoomNumber);

		Camera.LookAtRoomNumber = itemPtr->RoomNumber;
		Camera.LookAt.y = y;

		x = itemPtr->Pose.Position.x;
		z = itemPtr->Pose.Position.z;

		// -- Troye 2022.8.7
		if (Camera.flags == CameraFlag::FollowCenter)
		{
			int shift = (bounds.Z1 + bounds.Z2) / 2;
			x += shift * phd_sin(itemPtr->Pose.Orientation.y);
			z += shift * phd_cos(itemPtr->Pose.Orientation.y);
		}

		Camera.LookAt.x = x;
		Camera.LookAt.z = z;

		// CameraFlag::FollowCenter sets target on item and
		// ConfirmCameraTargetPos() overrides this target, hence flag check. -- Troye 2022.8.7
		if (itemPtr->IsLara() && Camera.flags != CameraFlag::FollowCenter)
			ConfirmCameraTargetPos();

		if (isFixedCamera == Camera.fixedCamera)
		{
			Camera.fixedCamera = false;
			if (Camera.speed != 1.0f &&
				!player.Control.Look.IsUsingBinoculars)
			{
				if (TargetSnaps <= 8)
				{
					x = LastTarget.x + ((x - LastTarget.x) / 4);
					y = LastTarget.y + ((y - LastTarget.y) / 4);
					z = LastTarget.z + ((z - LastTarget.z) / 4);

					Camera.LookAt.x = x;
					Camera.LookAt.y = y;
					Camera.LookAt.z = z;
				}
				else
				{
					TargetSnaps = 0;
				}
			}
		}
		else
		{
			Camera.fixedCamera = true;
			Camera.speed = 1.0f;
		}

		Camera.LookAtRoomNumber = GetCollision(x, y, z, Camera.LookAtRoomNumber).RoomNumber;

		if (abs(LastTarget.x - Camera.LookAt.x) < 4 &&
			abs(LastTarget.y - Camera.LookAt.y) < 4 &&
			abs(LastTarget.z - Camera.LookAt.z) < 4)
		{
			Camera.LookAt.x = LastTarget.x;
			Camera.LookAt.y = LastTarget.y;
			Camera.LookAt.z = LastTarget.z;
		}

		if (Camera.type != CameraType::Chase && Camera.flags != CameraFlag::ChaseObject)
		{
			FixedCamera();
		}
		else
		{
			ChaseCamera(*itemPtr);
		}
	}

	Camera.fixedCamera = isFixedCamera;
	Camera.last = Camera.number;

	if ((Camera.type != CameraType::Heavy || Camera.timer == -1) &&
		playerItem.HitPoints > 0)
	{
		Camera.type = CameraType::Chase;
		Camera.speed = 10.0f;
		Camera.number = NO_VALUE;
		Camera.lastItem = Camera.item;
		Camera.item = nullptr;
		Camera.targetElevation = 0;
		Camera.targetAngle = 0;
		Camera.targetDistance = BLOCK(1.5f);
		Camera.flags = CameraFlag::None;
		Camera.laraNode = NO_VALUE;
	}
}

bool TestBoundsCollideCamera(const GameBoundingBox& bounds, const Pose& pose, float radius)
{
	auto sphere = BoundingSphere(Camera.Position, radius);
	return sphere.Intersects(bounds.ToBoundingOrientedBox(pose));
}

void UpdateListenerPosition(const ItemInfo& item)
{
	float persp = ((g_Configuration.ScreenWidth / 2) * phd_cos(CurrentFOV / 2)) / phd_sin(CurrentFOV / 2);
	Camera.ListenerPosition = Camera.Position + (persp * Vector3(phd_sin(Camera.actualAngle), 0.0f, phd_cos(Camera.actualAngle)));
}

void RumbleScreen()
{
	if (!(GlobalCounter & 0x1FF))
		SoundEffect(SFX_TR5_KLAXON, nullptr, SoundEnvironment::Land, 0.25f);

	if (RumbleTimer >= 0)
		RumbleTimer++;

	if (RumbleTimer > 450)
	{
		if (!(Random::GenerateInt() & 0x1FF))
		{
			RumbleCounter = 0;
			RumbleTimer = -32 - (Random::GenerateInt() & 0x1F);
			return;
		}
	}

	if (RumbleTimer < 0)
	{
		if (RumbleCounter >= abs(RumbleTimer))
		{
			Camera.bounce = -(Random::GenerateInt() % abs(RumbleTimer));
			RumbleTimer++;
		}
		else
		{
			RumbleCounter++;
			Camera.bounce = -(Random::GenerateInt() % RumbleCounter);
		}
	}
}

void SetScreenFadeOut(float speed, bool force)
{
	if (g_ScreenEffect.ScreenFading && !force)
		return;

	g_ScreenEffect.ScreenFading = true;
	g_ScreenEffect.ScreenFadeStart = 1.0f;
	g_ScreenEffect.ScreenFadeEnd = 0;
	g_ScreenEffect.ScreenFadeSpeed = speed;
	g_ScreenEffect.ScreenFadeCurrent = g_ScreenEffect.ScreenFadeStart;
}

void SetScreenFadeIn(float speed, bool force)
{
	if (g_ScreenEffect.ScreenFading && !force)
		return;

	g_ScreenEffect.ScreenFading = true;
	g_ScreenEffect.ScreenFadeStart = 0.0f;
	g_ScreenEffect.ScreenFadeEnd = 1.0f;
	g_ScreenEffect.ScreenFadeSpeed = speed;
	g_ScreenEffect.ScreenFadeCurrent = g_ScreenEffect.ScreenFadeStart;
}

void SetCinematicBars(float height, float speed)
{
	g_ScreenEffect.CinematicBarsDestinationHeight = height;
	g_ScreenEffect.CinematicBarsSpeed = speed;
}

void ClearCinematicBars()
{
	g_ScreenEffect.CinematicBarsHeight = 0.0f;
	g_ScreenEffect.CinematicBarsDestinationHeight = 0.0f;
	g_ScreenEffect.CinematicBarsSpeed = 0.0f;
}

void UpdateFadeScreenAndCinematicBars()
{
	if (g_ScreenEffect.CinematicBarsDestinationHeight < g_ScreenEffect.CinematicBarsHeight)
	{
		g_ScreenEffect.CinematicBarsHeight -= g_ScreenEffect.CinematicBarsSpeed;
		if (g_ScreenEffect.CinematicBarsDestinationHeight > g_ScreenEffect.CinematicBarsHeight)
			g_ScreenEffect.CinematicBarsHeight = g_ScreenEffect.CinematicBarsDestinationHeight;
	}
	else if (g_ScreenEffect.CinematicBarsDestinationHeight > g_ScreenEffect.CinematicBarsHeight)
	{
		g_ScreenEffect.CinematicBarsHeight += g_ScreenEffect.CinematicBarsSpeed;
		if (g_ScreenEffect.CinematicBarsDestinationHeight < g_ScreenEffect.CinematicBarsHeight)
			g_ScreenEffect.CinematicBarsHeight = g_ScreenEffect.CinematicBarsDestinationHeight;
	}

	int prevScreenFadeCurrent = g_ScreenEffect.ScreenFadeCurrent;

	if (g_ScreenEffect.ScreenFadeEnd != 0 && g_ScreenEffect.ScreenFadeEnd >= g_ScreenEffect.ScreenFadeCurrent)
	{
		g_ScreenEffect.ScreenFadeCurrent += g_ScreenEffect.ScreenFadeSpeed;
		if (g_ScreenEffect.ScreenFadeCurrent > g_ScreenEffect.ScreenFadeEnd)
		{
			g_ScreenEffect.ScreenFadeCurrent = g_ScreenEffect.ScreenFadeEnd;
			if (prevScreenFadeCurrent >= g_ScreenEffect.ScreenFadeCurrent)
			{
				g_ScreenEffect.ScreenFadedOut = true;
				g_ScreenEffect.ScreenFading = false;
			}

		}
	}
	else if (g_ScreenEffect.ScreenFadeEnd < g_ScreenEffect.ScreenFadeCurrent)
	{
		g_ScreenEffect.ScreenFadeCurrent -= g_ScreenEffect.ScreenFadeSpeed;
		if (g_ScreenEffect.ScreenFadeCurrent < g_ScreenEffect.ScreenFadeEnd)
		{
			g_ScreenEffect.ScreenFadeCurrent = g_ScreenEffect.ScreenFadeEnd;
			g_ScreenEffect.ScreenFading = false;
		}
	}
}

float GetParticleDistanceFade(const Vector3i& pos)
{
	constexpr auto PARTICLE_FADE_THRESHOLD = BLOCK(14);

	float dist = Vector3::Distance(Camera.Position, pos.ToVector3());
	if (dist <= PARTICLE_FADE_THRESHOLD)
		return 1.0f;

	return std::clamp(1.0f - ((dist - PARTICLE_FADE_THRESHOLD) / CAMERA_OBJECT_COLL_DIST_THRESHOLD), 0.0f, 1.0f);
}
