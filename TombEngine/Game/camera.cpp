#include "framework.h"
#include "Game/camera.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
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

using namespace TEN::Effects::Environment;
using namespace TEN::Entities::Generic;
using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Utils;
using TEN::Renderer::g_Renderer;

constexpr auto PARTICLE_FADE_THRESHOLD = BLOCK(14);
constexpr auto COLL_CHECK_THRESHOLD    = BLOCK(4);
constexpr auto COLL_DISCARD_THRESHOLD  = CLICK(0.5f);

constexpr auto SWIVEL_STEP_COUNT = 16;

struct OLD_CAMERA
{
	short ActiveState;
	short TargetState;
	int targetDistance;
	short actualElevation;
	short targetElevation;
	short actualAngle;
	Pose pos;
	Pose pos2;
	Vector3i target;
};

GameVector LastTarget;

GameVector LastIdeal;
GameVector Ideals[SWIVEL_STEP_COUNT];
OLD_CAMERA OldCam;
int CameraSnaps = 0;
int TargetSnaps = 0;
GameVector LookCamPosition;
GameVector LookCamTarget;
Vector3i CamOldPos;
CAMERA_INFO Camera;
ObjectCameraInfo ItemCamera;
GameVector ForcedFixedCamera;
int UseForcedFixedCamera;

CameraType BinocularOldCamera;

short CurrentFOV;
short LastFOV;

int RumbleTimer = 0;
int RumbleCounter = 0;

bool  ScreenFadedOut = false;
bool  ScreenFading = false;
float ScreenFadeSpeed = 0;
float ScreenFadeStart = 0;
float ScreenFadeEnd = 0;
float ScreenFadeCurrent = 0;

float CinematicBarsHeight = 0;
float CinematicBarsDestinationHeight = 0;
float CinematicBarsSpeed = 0;

void DoThumbstickCamera()
{
	constexpr auto X_ANGLE_CONSTRAINT = ANGLE(80.0f);
	constexpr auto Y_ANGLE_CONSTRAINT = ANGLE(120.0f);

	if (!g_Configuration.EnableThumbstickCamera)
		return;

	if (Camera.laraNode == -1 && Camera.target.ToVector3i() == OldCam.target)
	{
		const auto& cameraAxis = GetCameraAxis();

		if (abs(cameraAxis.x) > EPSILON && abs(Camera.targetAngle) == 0)
			Camera.targetAngle = ANGLE(Y_ANGLE_CONSTRAINT * cameraAxis.x);

		if (abs(cameraAxis.y) > EPSILON)
			Camera.targetElevation = ANGLE((X_ANGLE_CONSTRAINT * cameraAxis.y) - ANGLE(10.0f));
	}
}

static int GetLookCameraVerticalOffset(const ItemInfo& item, const CollisionInfo& coll)
{
	constexpr auto VERTICAL_OFFSET_DEFAULT		  = -BLOCK(1 / 16.0f);
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

	// Return appropriate vertical offset.
	return -((verticalOffset < floorToCeilHeight) ? verticalOffset : floorToCeilHeight);
}

void LookCamera(ItemInfo& item, const CollisionInfo& coll)
{
	constexpr auto POS_LERP_ALPHA	 = 0.25f;
	constexpr auto COLL_PUSH		 = BLOCK(0.25f) - BLOCK(1 / 16.0f);
	constexpr auto LOOK_AT_DIST		 = BLOCK(0.5f);
	constexpr auto CAMERA_DIST_COEFF = 0.7f;
	constexpr auto CAMERA_DIST_MAX	 = BLOCK(0.75f);

	const auto& player = GetLaraInfo(item);

	int verticalOffset = GetLookCameraVerticalOffset(item, coll);
	auto pivotOffset = Vector3i(0, verticalOffset, 0);

	float idealDist = -std::max(Camera.targetDistance * CAMERA_DIST_COEFF, CAMERA_DIST_MAX);

	// Define absolute camera orientation.
	auto orient = player.Control.Look.Orientation +
		EulerAngles(item.Pose.Orientation.x, item.Pose.Orientation.y, 0) +
		EulerAngles(0, Camera.targetAngle, 0);
	orient.x = std::clamp(orient.x, LOOKCAM_ORIENT_CONSTRAINT.first.x, LOOKCAM_ORIENT_CONSTRAINT.second.x);

	// Determine base position.
	bool isInSwamp = TestEnvironment(ENV_FLAG_SWAMP, item.RoomNumber);
	auto basePos = Vector3i(
		item.Pose.Position.x,
		isInSwamp ? g_Level.Rooms[item.RoomNumber].maxceiling : item.Pose.Position.y,
		item.Pose.Position.z);

	// Define landmarks.
	auto pivotPos = Geometry::TranslatePoint(basePos, item.Pose.Orientation.y, pivotOffset);
	auto idealPos = Geometry::TranslatePoint(pivotPos, orient, idealDist);
	auto lookAtPos = Geometry::TranslatePoint(pivotPos, orient, LOOK_AT_DIST);

	// Determine best position.
	auto origin = GameVector(pivotPos, GetCollision(&item, item.Pose.Orientation.y, pivotOffset.z, pivotOffset.y).RoomNumber);
	auto target = GameVector(idealPos, GetCollision(origin.ToVector3i(), origin.RoomNumber, orient, idealDist).RoomNumber);

	// Handle room and object collisions.
	LOSAndReturnTarget(&origin, &target, 0);
	CameraCollisionBounds(&target, COLL_PUSH, true);
	ItemsCollideCamera();

	// Smoothly update camera position.
	MoveCamera(&target, Camera.speed);
	Camera.target = GameVector(Camera.target.ToVector3i() + (lookAtPos - Camera.target.ToVector3i()) * POS_LERP_ALPHA, item.RoomNumber);

	LookAt(&Camera, 0);
	UpdateMikePos(item);
	Camera.oldType = Camera.type;
}

void LookAt(CAMERA_INFO* cam, short roll)
{
	auto pos = cam->pos.ToVector3();
	auto target = cam->target.ToVector3();
	auto up = Vector3::Down;
	float fov = TO_RAD(CurrentFOV / 1.333333f);
	float r = TO_RAD(roll);

	float levelFarView = g_GameFlow->GetLevel(CurrentLevel)->GetFarView() * float(BLOCK(1));

	g_Renderer.UpdateCameraMatrices(cam, r, fov, levelFarView);
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
	Rumble(std::clamp((float)abs(Camera.bounce) / 70.0f, 0.0f, 0.8f), 0.2f);
}


void InitializeCamera()
{
	Camera.shift = LaraItem->Pose.Position.y - BLOCK(1);

	LastTarget = GameVector(
		LaraItem->Pose.Position.x,
		Camera.shift,
		LaraItem->Pose.Position.z,
		LaraItem->RoomNumber);

	Camera.target = GameVector(
		LastTarget.x,
		Camera.shift,
		LastTarget.z,
		LaraItem->RoomNumber);

	Camera.pos = GameVector(
		LastTarget.x,
		Camera.shift,
		LastTarget.z - 100,
		LaraItem->RoomNumber);

	Camera.targetDistance = BLOCK(1.5f);
	Camera.item = nullptr;
	Camera.numberFrames = 1;
	Camera.type = CameraType::Chase;
	Camera.speed = 1;
	Camera.flags = CF_NONE;
	Camera.bounce = 0;
	Camera.number = -1;
	Camera.fixedCamera = false;

	AlterFOV(ANGLE(DEFAULT_FOV));

	UseForcedFixedCamera = 0;
	CalculateCamera(LaraCollision);

	// Fade in screen.
	SetScreenFadeIn(FADE_SCREEN_SPEED);
}

void MoveCamera(GameVector* ideal, float speed)
{
	if (Lara.Control.Look.IsUsingBinoculars)
		speed = 1.0f;

	if (OldCam.pos.Orientation != LaraItem->Pose.Orientation ||
		OldCam.pos2.Orientation.x != Lara.ExtraHeadRot.x ||
		OldCam.pos2.Orientation.y != Lara.ExtraHeadRot.y ||
		OldCam.pos2.Position.x != Lara.ExtraTorsoRot.x ||
		OldCam.pos2.Position.y != Lara.ExtraTorsoRot.y ||
		OldCam.pos.Position != LaraItem->Pose.Position ||
		OldCam.ActiveState != LaraItem->Animation.ActiveState ||
		OldCam.TargetState != LaraItem->Animation.TargetState ||
		OldCam.targetDistance != Camera.targetDistance ||
		OldCam.targetElevation != Camera.targetElevation ||
		OldCam.actualElevation != Camera.actualElevation ||
		OldCam.actualAngle != Camera.actualAngle ||
		OldCam.target.x != Camera.target.x ||
		OldCam.target.y != Camera.target.y ||
		OldCam.target.z != Camera.target.z ||
		Camera.oldType != Camera.type ||
		Lara.Control.Look.IsUsingBinoculars)
	{
		OldCam.pos.Orientation = LaraItem->Pose.Orientation;
		OldCam.pos2.Orientation.x = Lara.ExtraHeadRot.x;
		OldCam.pos2.Orientation.y = Lara.ExtraHeadRot.y;
		OldCam.pos2.Position.x = Lara.ExtraTorsoRot.x;
		OldCam.pos2.Position.y = Lara.ExtraTorsoRot.y;
		OldCam.pos.Position = LaraItem->Pose.Position;
		OldCam.ActiveState = LaraItem->Animation.ActiveState;
		OldCam.TargetState = LaraItem->Animation.TargetState;
		OldCam.targetDistance = Camera.targetDistance;
		OldCam.targetElevation = Camera.targetElevation;
		OldCam.actualElevation = Camera.actualElevation;
		OldCam.actualAngle = Camera.actualAngle;
		OldCam.target.x = Camera.target.x;
		OldCam.target.y = Camera.target.y;
		OldCam.target.z = Camera.target.z;
		LastIdeal.x = ideal->x;
		LastIdeal.y = ideal->y;
		LastIdeal.z = ideal->z;
		LastIdeal.RoomNumber = ideal->RoomNumber;
	}
	else
	{
		ideal->x = LastIdeal.x;
		ideal->y = LastIdeal.y;
		ideal->z = LastIdeal.z;
		ideal->RoomNumber = LastIdeal.RoomNumber;
	}

	Camera.pos.x += (ideal->x - Camera.pos.x) / speed;
	Camera.pos.y += (ideal->y - Camera.pos.y) / speed;
	Camera.pos.z += (ideal->z - Camera.pos.z) / speed;
	Camera.pos.RoomNumber = ideal->RoomNumber;

	if (Camera.bounce)
	{
		if (Camera.bounce <= 0)
		{
			int bounce = -Camera.bounce;
			int bounce2 = bounce / 2;
			Camera.target.x += GetRandomControl() % bounce - bounce2;
			Camera.target.y += GetRandomControl() % bounce - bounce2;
			Camera.target.z += GetRandomControl() % bounce - bounce2;
			Camera.bounce += 5;
			RumbleFromBounce();
		}
		else
		{
			Camera.pos.y += Camera.bounce;
			Camera.target.y += Camera.bounce;
			Camera.bounce = 0;
		}
	}

	// Avoid entering swamp rooms.
	int y = Camera.pos.y;
	if (TestEnvironment(ENV_FLAG_SWAMP, Camera.pos.RoomNumber))
		y = g_Level.Rooms[Camera.pos.RoomNumber].y - CLICK(1);

	// Beyond floor or ceiling.
	auto pointColl = GetCollision(Camera.pos.x, y, Camera.pos.z, Camera.pos.RoomNumber);
	if (y < pointColl.Position.Ceiling ||
		y > pointColl.Position.Floor)
	{
		LOSAndReturnTarget(&Camera.target, &Camera.pos, 0);

		if (abs(Camera.pos.x - ideal->x) < BLOCK(0.5f) &&
			abs(Camera.pos.y - ideal->y) < BLOCK(0.5f) &&
			abs(Camera.pos.z - ideal->z) < BLOCK(0.5f))
		{
			auto origin = *ideal;
			auto target = Camera.pos;
			if (!LOSAndReturnTarget(&origin, &target, 0) &&
				++CameraSnaps >= 8)
			{
				Camera.pos = *ideal;
				CameraSnaps = 0;
			}
		}
	}

	pointColl = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber);

	int buffer = CLICK(1) - 1;
	if ((Camera.pos.y - buffer) < pointColl.Position.Ceiling &&
		(Camera.pos.y + buffer) > pointColl.Position.Floor &&
		pointColl.Position.Ceiling < pointColl.Position.Floor &&
		pointColl.Position.Ceiling != NO_HEIGHT &&
		pointColl.Position.Floor != NO_HEIGHT)
	{
		Camera.pos.y = (pointColl.Position.Floor + pointColl.Position.Ceiling) / 2;
	}
	else if ((Camera.pos.y + buffer) > pointColl.Position.Floor &&
		pointColl.Position.Ceiling < pointColl.Position.Floor &&
		pointColl.Position.Ceiling != NO_HEIGHT &&
		pointColl.Position.Floor != NO_HEIGHT)
	{
		Camera.pos.y = pointColl.Position.Floor - buffer;
	}
	else if ((Camera.pos.y - buffer) < pointColl.Position.Ceiling &&
		pointColl.Position.Ceiling < pointColl.Position.Floor &&
		pointColl.Position.Ceiling != NO_HEIGHT &&
		pointColl.Position.Floor != NO_HEIGHT)
	{
		Camera.pos.y = pointColl.Position.Ceiling + buffer;
	}
	else if (pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT)
	{
		Camera.pos = *ideal;
	}

	// TODO: Use item collision intersection to recalculate ideal earlier in this function.
	ItemsCollideCamera();

	Camera.pos.RoomNumber = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber).RoomNumber;
	LookAt(&Camera, 0);
	UpdateMikePos(*LaraItem);
	Camera.oldType = Camera.type;
}

void ObjCamera(ItemInfo* camSlotId, int camMeshId, ItemInfo* targetItem, int targetMeshId, bool cond)
{
	//camSlotId and targetItem stay the same object until I know how to expand targetItem to another object.
	//activates code below ->  void CalculateCamera().
	ItemCamera.ItemCameraOn = cond;

	UpdateCameraElevation();

	//get mesh 0 coordinates.	
	auto pos = GetJointPosition(camSlotId, 0, Vector3i::Zero);
	auto dest = Vector3(pos.x, pos.y, pos.z);

	GameVector from = GameVector(dest, camSlotId->RoomNumber);
	Camera.fixedCamera = true;

	MoveObjCamera(&from, camSlotId, camMeshId, targetItem, targetMeshId);
	Camera.timer = -1;
}

void ClearObjCamera()
{
	ItemCamera.ItemCameraOn = false;
}

void MoveObjCamera(GameVector* ideal, ItemInfo* camSlotId, int camMeshId, ItemInfo* targetItem, int targetMeshId)
{
	int	speed = 1;
	//Get mesh1 to attach camera to
	//Vector3i pos = Vector3i::Zero;
	auto pos = GetJointPosition(camSlotId, camMeshId, Vector3i::Zero);
	//Get mesh2 to attach target to
	//Vector3i pos2 = Vector3i::Zero;
	auto pos2 = GetJointPosition(targetItem, targetMeshId, Vector3i::Zero);

	if (OldCam.pos.Position != pos ||
		OldCam.targetDistance  != Camera.targetDistance  ||
		OldCam.targetElevation != Camera.targetElevation ||
		OldCam.actualElevation != Camera.actualElevation ||
		OldCam.actualAngle != Camera.actualAngle ||
		OldCam.target != Camera.target.ToVector3i() ||
		Camera.oldType != Camera.type ||
		Lara.Control.Look.IsUsingBinoculars)
	{
		OldCam.pos.Position = pos;
		OldCam.targetDistance = Camera.targetDistance;
		OldCam.targetElevation = Camera.targetElevation;
		OldCam.actualElevation = Camera.actualElevation;
		OldCam.actualAngle = Camera.actualAngle;
		OldCam.target = Camera.target.ToVector3i();
		LastIdeal = pos;
		LastIdeal.RoomNumber = ideal->RoomNumber;
		LastTarget = pos2;
	}
	else
	{
		pos  = LastIdeal.ToVector3i();
		pos2 = LastTarget.ToVector3i();
		ideal->RoomNumber = LastIdeal.RoomNumber;
	}

	Camera.pos += (ideal->ToVector3i() - Camera.pos.ToVector3i()) / speed;
	Camera.pos.RoomNumber = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber).RoomNumber;
	LookAt(&Camera, 0);

	auto angle = Camera.target.ToVector3i() - Camera.pos.ToVector3i();
	auto position = Vector3i(Camera.target.ToVector3i() - Camera.pos.ToVector3i());

	// write last frame camera angle to LastAngle to compare if next frame camera angle has a bigger step than 100.
	// To make camera movement smoother a speed of 2 is used.
	// While for big camera angle steps (cuts) -
	// the speed is set to 1 to make the cut immediatelly.
	constexpr int angleThresholdDegrees = 100;

	if (LastTarget.x - Camera.target.x > angleThresholdDegrees ||
		LastTarget.y - Camera.target.y > angleThresholdDegrees ||
		LastTarget.z - Camera.target.z > angleThresholdDegrees)
	{
		speed = 1;
	}
	else
	{
		speed = 2;
	}

	//actual movement of the target.
	Camera.target.x += (pos2.x - Camera.target.x) / speed;
	Camera.target.y += (pos2.y - Camera.target.y) / speed;
	Camera.target.z += (pos2.z - Camera.target.z) / speed;

	if (ItemCamera.LastAngle != position)
	{
		ItemCamera.LastAngle = Vector3i(ItemCamera.LastAngle.x = angle.x, 
										ItemCamera.LastAngle.y = angle.y, 
										ItemCamera.LastAngle.z = angle.z);
	}
}

void RefreshFixedCamera(short camNumber)
{
	auto& camera = g_Level.Cameras[camNumber];

	auto origin = GameVector(camera.Position, camera.RoomNumber);
	int moveSpeed = camera.Speed * 8 + 1;

	MoveCamera(&origin, moveSpeed);
}

void ChaseCamera(ItemInfo* item)
{
	constexpr auto MODERN_CAMERA_ABOVE_WATER_X_ANGLE_CONSTRAINT = std::pair<short, short>(-ANGLE(85.0f), ANGLE(70.0f));
	constexpr auto MODERN_CAMERA_UNDERWATER_X_ANGLE_CONSTRAINT	= std::pair<short, short>(-ANGLE(85.0f), ANGLE(80.0f));
	constexpr auto TANK_CAMERA_X_ANGLE_CONSTRAINT				= ANGLE(85.0f);
	constexpr auto BUFFER										= 100;

	const auto& player = GetLaraInfo(*item);

	if (Camera.targetElevation == 0)
		Camera.targetElevation = ANGLE(-10.0f);

	Camera.targetElevation += item->Pose.Orientation.x;
	UpdateCameraElevation();

	// Clamp X orientation.
	if (IsUsingModernControls())
	{
		const auto& xAngleConstraint = (player.Control.WaterStatus == WaterStatus::Underwater) ?
			MODERN_CAMERA_UNDERWATER_X_ANGLE_CONSTRAINT : MODERN_CAMERA_ABOVE_WATER_X_ANGLE_CONSTRAINT;
		if (Camera.actualElevation > xAngleConstraint.second)
		{
			Camera.actualElevation = xAngleConstraint.second;
		}
		else if (Camera.actualElevation < xAngleConstraint.first)
		{
			Camera.actualElevation = xAngleConstraint.first;
		}
	}
	else
	{
		if (Camera.actualElevation > TANK_CAMERA_X_ANGLE_CONSTRAINT)
		{
			Camera.actualElevation = TANK_CAMERA_X_ANGLE_CONSTRAINT;
		}
		else if (Camera.actualElevation < -TANK_CAMERA_X_ANGLE_CONSTRAINT)
		{
			Camera.actualElevation = -TANK_CAMERA_X_ANGLE_CONSTRAINT;
		}
	}

	auto pointColl = GetCollision(Camera.target.x, Camera.target.y + CLICK(1), Camera.target.z, Camera.target.RoomNumber);

	if (TestEnvironment(ENV_FLAG_SWAMP, pointColl.RoomNumber))
		Camera.target.y = g_Level.Rooms[pointColl.RoomNumber].maxceiling - CLICK(1);

	int y = Camera.target.y;
	pointColl = GetCollision(Camera.target.x, y, Camera.target.z, Camera.target.RoomNumber);
	if (((y < pointColl.Position.Ceiling || pointColl.Position.Floor < y) || pointColl.Position.Floor <= pointColl.Position.Ceiling) ||
		(pointColl.Position.Floor == NO_HEIGHT || pointColl.Position.Ceiling == NO_HEIGHT))
	{
		TargetSnaps++;
		Camera.target = LastTarget;
	}
	else
	{
		TargetSnaps = 0;
	}

	// Move camera.
	if (IsUsingModernControls())
	{
		auto dir = -EulerAngles(Camera.actualElevation, Camera.actualAngle, 0).ToDirection();
		auto idealPos = Geometry::TranslatePoint(Camera.target.ToVector3(), dir, Camera.targetDistance);

		// Determine best position.
		auto origin = Camera.target;
		auto target = GameVector(idealPos, GetCollision(Camera.target.ToVector3i(), Camera.target.RoomNumber, dir, Camera.targetDistance).RoomNumber);
		LOSAndReturnTarget(&origin, &target, 0);

		// Apply buffer if origin and target are too close.
		if (Vector3::Distance(origin.ToVector3(), target.ToVector3()) <= BUFFER)
			target = GameVector(Geometry::TranslatePoint(target.ToVector3i(), dir, BUFFER), target.RoomNumber);

		// Update camera position.
		MoveCamera(&target, Camera.speed * ((Camera.type != CameraType::Look) ? 0.25f : 1.0f));
	}
	else
	{
		for (int i = 0; i < SWIVEL_STEP_COUNT; i++)
			Ideals[i].y = Camera.target.y + (Camera.targetDistance * phd_sin(Camera.actualElevation));

		float dist = Camera.targetDistance * phd_cos(Camera.actualElevation);

		// Determine best player viewing angle.
		float farthestDist = FLT_MAX;
		int indexOfFarthestIdeal = 0;
		GameVector temp[2];
		for (int i = 0; i < SWIVEL_STEP_COUNT; i++)
		{
			// Incrementally swivel camera position.
			short angle = (i == 0) ? Camera.actualAngle : ((i - 1) * ANGLE(90.0f));

			// Record ideal position at default distance for given swivel.
			Ideals[i].x = Camera.target.x - (dist * phd_sin(angle));
			Ideals[i].z = Camera.target.z - (dist * phd_cos(angle));
			Ideals[i].RoomNumber = Camera.target.RoomNumber;

			// Assess LOS.
			if (LOSAndReturnTarget(&Camera.target, &Ideals[i], 200))
			{
				temp[0] = Ideals[i];
				temp[1] = Camera.pos;

				if (i == 0 || LOSAndReturnTarget(&temp[0], &temp[1], 0))
				{
					if (i == 0)
					{
						indexOfFarthestIdeal = 0;
						break;
					}

					float dx = (Camera.pos.x - Ideals[i].x) * (Camera.pos.x - Ideals[i].x);
					dx += (Camera.pos.z - Ideals[i].z) * (Camera.pos.z - Ideals[i].z);
					if (dx < farthestDist)
					{
						farthestDist = dx;
						indexOfFarthestIdeal = i;
					}
				}
			}
			else if (i == 0)
			{
				temp[0] = Ideals[i];
				temp[1] = Camera.pos;

				if (i == 0 || LOSAndReturnTarget(&temp[0], &temp[1], 0))
				{
					float dx = (Camera.target.x - Ideals[i].x) * (Camera.target.x - Ideals[i].x);
					float dz = (Camera.target.z - Ideals[i].z) * (Camera.target.z - Ideals[i].z);

					if ((dx + dz) > SQUARE(BLOCK(0.75f)))
					{
						indexOfFarthestIdeal = 0;
						break;
					}
				}
			}
		}

		auto ideal = Ideals[indexOfFarthestIdeal];
		CameraCollisionBounds(&ideal, CLICK(1.5f), true);
		MoveCamera(&ideal, Camera.speed);
	}
}

void UpdateCameraElevation()
{
	constexpr auto CAMERA_AXIS_COEFF = 20.0f;
	constexpr auto MOUSE_AXIS_COEFF	 = 250.0f;

	DoThumbstickCamera();

	if (Camera.laraNode != -1)
	{
		auto pos = GetJointPosition(LaraItem, Camera.laraNode, Vector3i::Zero);
		auto pos1 = GetJointPosition(LaraItem, Camera.laraNode, Vector3i(0, -CLICK(1), BLOCK(2)));
		pos = pos1 - pos;
		Camera.actualAngle = Camera.targetAngle + phd_atan(pos.z, pos.x);
	}
	else
	{
		if (IsUsingModernControls())
		{
			const auto& cameraAxis = GetCameraAxis();
			const auto& mouseAxis = GetMouseAxis();
			const auto& activeAxis = (cameraAxis != Vector2::Zero) ? (cameraAxis * CAMERA_AXIS_COEFF) : (mouseAxis * MOUSE_AXIS_COEFF);

			Camera.actualAngle += ANGLE(activeAxis.x);
			Camera.actualElevation -= ANGLE(activeAxis.y);
		}
		else
		{
			Camera.actualAngle = LaraItem->Pose.Orientation.y + Camera.targetAngle;
		}
	}

	if (!IsUsingModernControls())
		Camera.actualElevation += (Camera.targetElevation - Camera.actualElevation) / 8;
}

void CombatCamera(ItemInfo* item)
{
	constexpr auto MODERN_CAMERA_ABOVE_WATER_X_ANGLE_CONSTRAINT = std::pair<short, short>(-ANGLE(85.0f), ANGLE(70.0f));
	constexpr auto MODERN_CAMERA_UNDERWATER_X_ANGLE_CONSTRAINT	= std::pair<short, short>(-ANGLE(85.0f), ANGLE(80.0f));
	constexpr auto TANK_CAMERA_X_ANGLE_CONSTRAINT				= ANGLE(85.0f);
	constexpr auto BUFFER										= 100;

	auto& player = GetLaraInfo(*item);

	Camera.target.x = item->Pose.Position.x;
	Camera.target.z = item->Pose.Position.z;

	if (player.TargetEntity != nullptr)
	{
		Camera.targetAngle = player.TargetArmOrient.y;
		Camera.targetElevation = player.TargetArmOrient.x + item->Pose.Orientation.x;
	}
	else
	{
		Camera.targetAngle = player.ExtraHeadRot.y + player.ExtraTorsoRot.y;
		Camera.targetElevation = player.ExtraHeadRot.x + player.ExtraTorsoRot.x + item->Pose.Orientation.x - ANGLE(15.0f);
	}

	auto pointColl = GetCollision(Camera.target.x, Camera.target.y + CLICK(1), Camera.target.z, Camera.target.RoomNumber);
	if (TestEnvironment(ENV_FLAG_SWAMP, pointColl.RoomNumber))
		Camera.target.y = g_Level.Rooms[pointColl.RoomNumber].y - CLICK(1);

	pointColl = GetCollision(Camera.target.x, Camera.target.y, Camera.target.z, Camera.target.RoomNumber);
	Camera.target.RoomNumber = pointColl.RoomNumber;

	int buffer = CLICK(0.25f);
	if ((pointColl.Position.Ceiling + buffer) > (pointColl.Position.Floor - buffer) &&
		pointColl.Position.Floor != NO_HEIGHT &&
		pointColl.Position.Ceiling != NO_HEIGHT)
	{
		Camera.target.y = (pointColl.Position.Ceiling + pointColl.Position.Floor) / 2;
		Camera.targetElevation = 0;
	}
	else if (Camera.target.y > (pointColl.Position.Floor - buffer) &&
		pointColl.Position.Floor != NO_HEIGHT)
	{
		Camera.target.y = pointColl.Position.Floor - buffer;
		Camera.targetElevation = 0;
	}
	else if (Camera.target.y < (pointColl.Position.Ceiling + buffer) &&
		pointColl.Position.Ceiling != NO_HEIGHT)
	{
		Camera.target.y = pointColl.Position.Ceiling + buffer;
		Camera.targetElevation = 0;
	}

	int y = Camera.target.y;
	pointColl = GetCollision(Camera.target.x, y, Camera.target.z, Camera.target.RoomNumber);
	Camera.target.RoomNumber = pointColl.RoomNumber;

	if (y < pointColl.Position.Ceiling ||
		y > pointColl.Position.Floor ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT)
	{
		TargetSnaps++;
		Camera.target = LastTarget;
	}
	else
	{
		TargetSnaps = 0;
	}

	UpdateCameraElevation();

	// Clamp X orientation.
	if (IsUsingModernControls())
	{
		const auto& xAngleConstraint = (player.Control.WaterStatus == WaterStatus::Underwater) ?
			MODERN_CAMERA_UNDERWATER_X_ANGLE_CONSTRAINT : MODERN_CAMERA_ABOVE_WATER_X_ANGLE_CONSTRAINT;
		if (Camera.actualElevation > xAngleConstraint.second)
		{
			Camera.actualElevation = xAngleConstraint.second;
		}
		else if (Camera.actualElevation < xAngleConstraint.first)
		{
			Camera.actualElevation = xAngleConstraint.first;
		}
	}
	else
	{
		if (Camera.actualElevation > TANK_CAMERA_X_ANGLE_CONSTRAINT)
		{
			Camera.actualElevation = TANK_CAMERA_X_ANGLE_CONSTRAINT;
		}
		else if (Camera.actualElevation < -TANK_CAMERA_X_ANGLE_CONSTRAINT)
		{
			Camera.actualElevation = -TANK_CAMERA_X_ANGLE_CONSTRAINT;
		}
	}

	Camera.targetDistance = BLOCK(1.5f);

	if (IsUsingModernControls())
	{
		auto dir = -EulerAngles(Camera.actualElevation, Camera.actualAngle, 0).ToDirection();
		auto idealPos = Geometry::TranslatePoint(Camera.target.ToVector3(), dir, Camera.targetDistance);

		// Determine best position.
		auto origin = Camera.target;
		auto target = GameVector(idealPos, GetCollision(Camera.target.ToVector3i(), Camera.target.RoomNumber, dir, Camera.targetDistance).RoomNumber);
		LOSAndReturnTarget(&origin, &target, 0);

		// Apply buffer if origin and target are too close.
		if (Vector3::Distance(origin.ToVector3(), target.ToVector3()) <= BUFFER)
			target = GameVector(Geometry::TranslatePoint(target.ToVector3i(), dir, BUFFER), target.RoomNumber);

		// Snap position of fixed camera type.
		if (Camera.oldType == CameraType::Fixed)
			Camera.speed = 1;

		// Update camera position.
		MoveCamera(&target, Camera.speed * ((Camera.type != CameraType::Look) ? 0.25f : 1.0f));
	}
	else
	{
		for (int i = 0; i < SWIVEL_STEP_COUNT; i++)
			Ideals[i].y = Camera.target.y + (Camera.targetDistance * phd_sin(Camera.actualElevation));

		float dist = Camera.targetDistance * phd_cos(Camera.actualElevation);

		// Determine best player viewing angle.
		float farthestDist = FLT_MAX;
		int indexOfFarthestIdeal = 0;
		GameVector temp[2];
		for (int i = 0; i < SWIVEL_STEP_COUNT; i++)
		{
			// Incrementally swivel camera position.
			short angle = (i == 0) ? Camera.actualAngle : ((i - 1) * ANGLE(90.0f));

			// Record ideal position at default distance for given swivel.
			Ideals[i].x = Camera.target.x - (dist * phd_sin(angle));
			Ideals[i].z = Camera.target.z - (dist * phd_cos(angle));
			Ideals[i].RoomNumber = Camera.target.RoomNumber;

			// Assess LOS.
			if (LOSAndReturnTarget(&Camera.target, &Ideals[i], 200))
			{
				temp[0] = Ideals[i];
				temp[1] = Camera.pos;
				if (i == 0 || LOSAndReturnTarget(&temp[0], &temp[1], 0))
				{
					if (i == 0)
					{
						indexOfFarthestIdeal = 0;
						break;
					}

					float dx = (Camera.pos.x - Ideals[i].x) * (Camera.pos.x - Ideals[i].x);
					dx += (Camera.pos.z - Ideals[i].z) * (Camera.pos.z - Ideals[i].z);
					if (dx < farthestDist)
					{
						farthestDist = dx;
						indexOfFarthestIdeal = i;
					}
				}
			}
			else if (i == 0)
			{
				temp[0] = Ideals[i];
				temp[1] = Camera.pos;
				if (i == 0 || LOSAndReturnTarget(&temp[0], &temp[1], 0))
				{
					float dx = (Camera.target.x - Ideals[i].x) * (Camera.target.x - Ideals[i].x);
					float dz = (Camera.target.z - Ideals[i].z) * (Camera.target.z - Ideals[i].z);
					if ((dx + dz) > SQUARE(BLOCK(0.75f)))
					{
						indexOfFarthestIdeal = 0;
						break;
					}
				}
			}
		}

		// Handle room collision.
		auto ideal = Ideals[indexOfFarthestIdeal];
		CameraCollisionBounds(&ideal, CLICK(1.5f), 1);

		// Snap position of fixed camera type.
		if (Camera.oldType == CameraType::Fixed)
			Camera.speed = 1;

		MoveCamera(&ideal, Camera.speed);
	}
}

bool CameraCollisionBounds(GameVector* ideal, int push, bool yFirst)
{
	int x = ideal->x;
	int y = ideal->y;
	int z = ideal->z;

	auto pointColl = CollisionResult{};
	if (yFirst)
	{
		pointColl = GetCollision(x, y, z, ideal->RoomNumber);

		int buffer = CLICK(1) - 1;
		if ((y - buffer) < pointColl.Position.Ceiling &&
			(y + buffer) > pointColl.Position.Floor &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			y = (pointColl.Position.Floor + pointColl.Position.Ceiling) / 2;
		}
		else if ((y + buffer) > pointColl.Position.Floor &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			y = pointColl.Position.Floor - buffer;
		}
		else if ((y - buffer) < pointColl.Position.Ceiling &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			y = pointColl.Position.Ceiling + buffer;
		}
	}

	pointColl = GetCollision(x - push, y, z, ideal->RoomNumber);
	if (y > pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		y < pointColl.Position.Ceiling)
	{
		x = (x & (~WALL_MASK)) + push;
	}

	pointColl = GetCollision(x, y, z - push, ideal->RoomNumber);
	if (y > pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		y < pointColl.Position.Ceiling)
	{
		z = (z & (~WALL_MASK)) + push;
	}

	pointColl = GetCollision(x + push, y, z, ideal->RoomNumber);
	if (y > pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		y < pointColl.Position.Ceiling)
	{
		x = (x | WALL_MASK) - push;
	}

	pointColl = GetCollision(x, y, z + push, ideal->RoomNumber);
	if (y > pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		y < pointColl.Position.Ceiling)
	{
		z = (z | WALL_MASK) - push;
	}

	if (!yFirst)
	{
		pointColl = GetCollision(x, y, z, ideal->RoomNumber);

		int buffer = CLICK(1) - 1;
		if ((y - buffer) < pointColl.Position.Ceiling &&
			(y + buffer) > pointColl.Position.Floor &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			y = (pointColl.Position.Floor + pointColl.Position.Ceiling) / 2;
		}
		else if ((y + buffer) > pointColl.Position.Floor &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			y = pointColl.Position.Floor - buffer;
		}
		else if ((y - buffer) < pointColl.Position.Ceiling &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			y = pointColl.Position.Ceiling + buffer;
		}
	}

	pointColl = GetCollision(x, y, z, ideal->RoomNumber);
	if (y > pointColl.Position.Floor ||
		y < pointColl.Position.Ceiling ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor)
	{
		return true;
	}

	ideal->RoomNumber = pointColl.RoomNumber;
	ideal->x = x;
	ideal->y = y;
	ideal->z = z;

	return false;
}

void FixedCamera(ItemInfo* item)
{
	// Fixed cameras before TR3 had optional "movement" effect. 
	// Later for some reason it was forced to always be 1, and actual speed value
	// from camera trigger was ignored. In TEN, speed value was moved out out of legacy
	// floordata trigger to camera itself to make use of it again. Still, by default,
	// value is 1 for UseForcedFixedCamera hack.

	int moveSpeed = 1;

	GameVector origin, target;
	if (UseForcedFixedCamera)
	{
		origin = ForcedFixedCamera;
	}
	else
	{
		const auto& camera = g_Level.Cameras[Camera.number];

		origin = GameVector(camera.Position, camera.RoomNumber);

		// NOTE: Multiply original speed by 8 to comply with original bitshifted speed from TR1-2.
		moveSpeed = (camera.Speed * 8) + 1;
	}

	Camera.fixedCamera = true;

	MoveCamera(&origin, moveSpeed);

	if (Camera.timer)
	{
		if (!--Camera.timer)
			Camera.timer = -1;
	}
}

void BounceCamera(ItemInfo* item, short bounce, short maxDistance)
{
	float distance = Vector3i::Distance(item->Pose.Position, Camera.pos.ToVector3i());
	if (distance < maxDistance)
	{
		if (maxDistance == -1)
			Camera.bounce = bounce;
		else
			Camera.bounce = -(bounce * (maxDistance - distance) / maxDistance);
	}
	else if (maxDistance == -1)
		Camera.bounce = bounce;
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

	Camera.pos.x = x;
	Camera.pos.y = y;
	Camera.pos.z = z;
	Camera.pos.RoomNumber = probe.RoomNumber;

	float l = BLOCK(20.25f) * phd_cos(player.Control.Look.Orientation.x);
	float tx = x + l * phd_sin(item->Pose.Orientation.y + player.Control.Look.Orientation.y);
	float ty = y - BLOCK(20.25f) * phd_sin(player.Control.Look.Orientation.x);
	float tz = z + l * phd_cos(item->Pose.Orientation.y + player.Control.Look.Orientation.y);

	if (Camera.oldType == CameraType::Fixed)
	{
		Camera.target.x = tx;
		Camera.target.y = ty;
		Camera.target.z = tz;
		Camera.target.RoomNumber = item->RoomNumber;
	}
	else
	{
		Camera.target.x += (tx - Camera.target.x) / 4;
		Camera.target.y += (ty - Camera.target.y) / 4;
		Camera.target.z += (tz - Camera.target.z) / 4;
		Camera.target.RoomNumber = item->RoomNumber;
	}

	if (Camera.bounce &&
		Camera.type == Camera.oldType)
	{
		if (Camera.bounce <= 0)
		{
			Camera.target.x += (CLICK(0.25f) / 4) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce / 2));
			Camera.target.y += (CLICK(0.25f) / 4) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce / 2));
			Camera.target.z += (CLICK(0.25f) / 4) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce / 2));
			Camera.bounce += 5;
			RumbleFromBounce();
		}
		else
		{
			Camera.bounce = 0;
			Camera.target.y += Camera.bounce;
		}
	}

	Camera.target.RoomNumber = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.target.RoomNumber).RoomNumber;
	LookAt(&Camera, 0);
	UpdateMikePos(*item);
	Camera.oldType = Camera.type;

	GetTargetOnLOS(&Camera.pos, &Camera.target, false, false);

	if (IsHeld(In::Action))
	{
		auto origin = Camera.pos.ToVector3i();
		auto target = Camera.target.ToVector3i();
		LaraTorch(&origin, &target, player.ExtraHeadRot.y, 192);
	}
}

void ConfirmCameraTargetPos()
{
	auto pos = Vector3i(
		LaraItem->Pose.Position.x,
		LaraItem->Pose.Position.y - (LaraCollision.Setup.Height / 2),
		LaraItem->Pose.Position.z);

	if (Camera.laraNode != -1)
	{
		Camera.target.x = pos.x;
		Camera.target.y = pos.y;
		Camera.target.z = pos.z;
	}
	else
	{
		Camera.target.x = LaraItem->Pose.Position.x;
		Camera.target.y = (Camera.target.y + pos.y) / 2;
		Camera.target.z = LaraItem->Pose.Position.z;
	}

	int y = Camera.target.y;
	auto probe = GetCollision(Camera.target.x, y, Camera.target.z, Camera.target.RoomNumber);
	if (y < probe.Position.Ceiling ||
		probe.Position.Floor < y ||
		probe.Position.Floor <= probe.Position.Ceiling ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		Camera.target.x = pos.x;
		Camera.target.y = pos.y;
		Camera.target.z = pos.z;
	}
}

static void UpdatePlayerRefCameraOrient(ItemInfo& item)
{
	auto& player = GetLaraInfo(item);

	bool isCameraTransition = (Camera.type == CameraType::Fixed && (Camera.oldType == CameraType::Chase || Camera.oldType == CameraType::Combat)) ||
							  ((Camera.type == CameraType::Chase || Camera.type == CameraType::Combat) && Camera.oldType == CameraType::Fixed);

	if (isCameraTransition && GetMoveAxis() != Vector2::Zero)
	{
		player.Control.LockRefCameraOrient = true;
	}
	else if (GetMoveAxis() == Vector2::Zero)
	{
		player.Control.LockRefCameraOrient = false;
	}

	if (!player.Control.LockRefCameraOrient)
		player.Control.RefCameraOrient = EulerAngles(Camera.actualElevation, Camera.actualAngle, 0);
}

void CalculateCamera(const CollisionInfo& coll)
{
	CamOldPos.x = Camera.pos.x;
	CamOldPos.y = Camera.pos.y;
	CamOldPos.z = Camera.pos.z;

	if (Lara.Control.Look.IsUsingBinoculars)
	{
		BinocularCamera(LaraItem);
		return;
	}

	if (ItemCamera.ItemCameraOn)
	{
		return;
	}

	if (UseForcedFixedCamera != 0)
	{
		Camera.type = CameraType::Fixed;
		if (Camera.oldType != CameraType::Fixed)
			Camera.speed = 1;
	}

	// Camera is in a water room, play water sound effect.
	if (TestEnvironment(ENV_FLAG_WATER, Camera.pos.RoomNumber))
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

	ItemInfo* item = nullptr;
	bool isFixedCamera = false;
	if (Camera.item != nullptr &&
		(Camera.type == CameraType::Fixed || Camera.type == CameraType::Heavy))
	{
		item = Camera.item;
		isFixedCamera = true;
	}
	else
	{
		item = LaraItem;
		isFixedCamera = false;
	}

	auto bounds = GameBoundingBox(item);

	int x;
	int y = item->Pose.Position.y + bounds.Y2 + (3 * (bounds.Y1 - bounds.Y2) / 4);
	int z;

	if (Camera.item)
	{
		if (!isFixedCamera)
		{
			auto deltaPos = Camera.item->Pose.Position - item->Pose.Position;
			float dist = Vector3i::Distance(Camera.item->Pose.Position, item->Pose.Position);

			auto lookOrient = EulerAngles(
				phd_atan(dist, y - (bounds.Y1 + bounds.Y2) / 2 - Camera.item->Pose.Position.y),
				phd_atan(deltaPos.z, deltaPos.x) - item->Pose.Orientation.y,
				0) / 2;

			if (lookOrient.y > ANGLE(-50.0f) &&	lookOrient.y < ANGLE(50.0f) &&
				lookOrient.z > ANGLE(-85.0f) && lookOrient.z < ANGLE(85.0f))
			{
				short angleDelta = lookOrient.y - Lara.ExtraHeadRot.y;
				if (angleDelta > ANGLE(4.0f))
				{
					Lara.ExtraHeadRot.y += ANGLE(4.0f);
				}
				else if (angleDelta < ANGLE(-4.0f))
				{
					Lara.ExtraHeadRot.y -= ANGLE(4.0f);
				}
				else
				{
					Lara.ExtraHeadRot.y += angleDelta;
				}
				Lara.ExtraTorsoRot.y = Lara.ExtraHeadRot.y;

				angleDelta = lookOrient.z - Lara.ExtraHeadRot.x;
				if (angleDelta > ANGLE(4.0f))
				{
					Lara.ExtraHeadRot.x += ANGLE(4.0f);
				}
				else if (angleDelta < ANGLE(-4.0f))
				{
					Lara.ExtraHeadRot.x -= ANGLE(4.0f);
				}
				else
				{
					Lara.ExtraHeadRot.x += angleDelta;
				}
				Lara.ExtraTorsoRot.x = Lara.ExtraHeadRot.x;

				Lara.Control.Look.Orientation = lookOrient;

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
			LastTarget.x = Camera.target.x;
			LastTarget.y = Camera.target.y;
			LastTarget.z = Camera.target.z;
			LastTarget.RoomNumber = Camera.target.RoomNumber;

			y -= CLICK(1);
		}

		Camera.target.RoomNumber = item->RoomNumber;

		if (Camera.fixedCamera || Lara.Control.Look.IsUsingBinoculars)
		{
			Camera.target.y = y;
			Camera.speed = 1;
		}
		else
		{
			Camera.target.y += (y - Camera.target.y) / 4;
			Camera.speed = (Camera.type != CameraType::Look) ? 8 : 4;
		}

		Camera.fixedCamera = false;
		if (Camera.type == CameraType::Look)
		{
			LookCamera(*item, coll);
		}
		else
		{
			CombatCamera(item);
		}
	}
	else
	{
		LastTarget.x = Camera.target.x;
		LastTarget.y = Camera.target.y;
		LastTarget.z = Camera.target.z;
		LastTarget.RoomNumber = Camera.target.RoomNumber;

		Camera.target.RoomNumber = item->RoomNumber;
		Camera.target.y = y;

		x = item->Pose.Position.x;
		z = item->Pose.Position.z;

		if (Camera.flags == CF_FOLLOW_CENTER)	//Troye Aug. 7th 2022
		{
			auto shift = (bounds.Z1 + bounds.Z2) / 2;
			x += shift * phd_sin(item->Pose.Orientation.y);
			z += shift * phd_cos(item->Pose.Orientation.y);
		}

		Camera.target.x = x;
		Camera.target.z = z;

		// CF_FOLLOW_CENTER sets target on the item, ConfirmCameraTargetPos overrides this target, 
		// hence the flag check. Troye Aug. 7th 2022
		
		if (item->IsLara() && Camera.flags != CF_FOLLOW_CENTER)
			ConfirmCameraTargetPos();

		if (isFixedCamera == Camera.fixedCamera)
		{
			Camera.fixedCamera = false;
			if (Camera.speed != 1 &&
				!Lara.Control.Look.IsUsingBinoculars)
			{
				if (TargetSnaps <= 8)
				{
					x = LastTarget.x + ((x - LastTarget.x) / 4);
					y = LastTarget.y + ((y - LastTarget.y) / 4);
					z = LastTarget.z + ((z - LastTarget.z) / 4);

					Camera.target.x = x;
					Camera.target.y = y;
					Camera.target.z = z;
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
			Camera.speed = 1;
		}

		Camera.target.RoomNumber = GetCollision(x, y, z, Camera.target.RoomNumber).RoomNumber;

		if (abs(LastTarget.x - Camera.target.x) < 4 &&
			abs(LastTarget.y - Camera.target.y) < 4 &&
			abs(LastTarget.z - Camera.target.z) < 4)
		{
			Camera.target.x = LastTarget.x;
			Camera.target.y = LastTarget.y;
			Camera.target.z = LastTarget.z;
		}

		if (Camera.type != CameraType::Chase && Camera.flags != CF_CHASE_OBJECT)
		{
			FixedCamera(item);
		}
		else
		{
			ChaseCamera(item);
		}
	}

	Camera.fixedCamera = isFixedCamera;
	Camera.last = Camera.number;

	if ((Camera.type != CameraType::Heavy || Camera.timer == -1) &&
		LaraItem->HitPoints > 0)
	{
		Camera.type = CameraType::Chase;
		Camera.speed = 10;
		Camera.number = -1;
		Camera.lastItem = Camera.item;
		Camera.item = nullptr;
		Camera.targetElevation = 0;
		Camera.targetAngle = 0;
		Camera.targetDistance = BLOCK(1.5f);
		Camera.flags = 0;
		Camera.laraNode = -1;
	}

	UpdatePlayerRefCameraOrient(*LaraItem);
}

bool TestBoundsCollideCamera(const GameBoundingBox& bounds, const Pose& pose, short radius)
{
	auto sphere = BoundingSphere(Camera.pos.ToVector3(), radius);
	return sphere.Intersects(bounds.ToBoundingOrientedBox(pose));
}

float GetParticleDistanceFade(const Vector3i& pos)
{
	float dist = Vector3::Distance(Camera.pos.ToVector3(), pos.ToVector3());
	if (dist <= PARTICLE_FADE_THRESHOLD)
		return 1.0f;

	return std::clamp(1.0f - ((dist - PARTICLE_FADE_THRESHOLD) / COLL_CHECK_THRESHOLD), 0.0f, 1.0f);
}

static bool IsCameraCollidableItem(const ItemInfo& item)
{
	float dist = Vector3i::Distance(item.Pose.Position, Camera.pos.ToVector3i());
	if (dist >= COLL_CHECK_THRESHOLD)
		return false;

	const auto& object = Objects[item.ObjectNumber];
	if (!item.Collidable || !object.usingDrawAnimatingItem)
		return false;

	// TODO: Find better way to define objects which are collidable with camera.
	if (object.intelligent || object.isPickup || object.isPuzzleHole || object.collision == nullptr)
		return false;

	// Test if any 2 box extents are smaller than threshold.
	auto extents = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose).Extents;
	if ((abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.y) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.y) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD))
	{
		return false;
	}

	return true;
}

static std::vector<const ItemInfo*> GetCameraCollidableItemPtrs()
{
	const auto& neighborRoomNumbers = g_Level.Rooms[Camera.pos.RoomNumber].neighbors;

	auto itemPtrs = std::vector<const ItemInfo*>{};
	for (int itemNumber = 0; itemNumber < g_Level.NumItems; itemNumber++)
	{
		const auto& item = g_Level.Items[itemNumber];

		if (!Contains(neighborRoomNumbers, (int)item.RoomNumber))
			continue;

		const auto& room = g_Level.Rooms[item.RoomNumber];
		if (!room.Active())
			continue;

		if (!IsCameraCollidableItem(item))
			continue;

		itemPtrs.push_back(&item);
	}

	return itemPtrs;
}

static std::optional<Vector3> GetCameraRayBoxIntersect(const BoundingOrientedBox& box)
{
	constexpr auto DIST_MIN = 0;// BLOCK(1 / 8.0f);

	auto origin = Camera.target.ToVector3();
	auto target = Camera.pos.ToVector3();
	float dist = Vector3::Distance(origin, target);

	auto dir = -EulerAngles(Camera.actualElevation, Camera.actualAngle, 0).ToDirection();
	dir.Normalize();

	// Calculate and return intersection.
	float intersectDist = 0.0f;
	if (box.Intersects(origin, dir, intersectDist))
	{
		if (intersectDist < dist && intersectDist >= DIST_MIN)
			return Geometry::TranslatePoint(origin, dir, intersectDist);
	}

	// No intersection; return nullopt.
	return std::nullopt;
}

static bool IsCameraCollideableStatic(const MESH_INFO& staticObject)
{
	float dist = Vector3i::Distance(Camera.pos.ToVector3i(), staticObject.pos.Position);
	if (dist >= COLL_CHECK_THRESHOLD)
		return false;

	if (!(staticObject.flags & StaticMeshFlags::SM_VISIBLE))
		return false;

	const auto& bounds = GetBoundsAccurate(staticObject, false);
	auto extents = Vector3(
		abs(bounds.X1 - bounds.X2),
		abs(bounds.Y1 - bounds.Y2),
		abs(bounds.Z1 - bounds.Z2));

	// Test if any 2 box extents are smaller than threshold.
	if ((abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.y) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.y) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD))
	{
		return false;
	}

	return true;
}

static std::vector<const MESH_INFO*> GetCameraCollidableStaticPtrs()
{
	const auto& neighborRoomNumbers = g_Level.Rooms[Camera.pos.RoomNumber].neighbors;

	auto staticPtrs = std::vector<const MESH_INFO*>{};
	for (int roomNumber : neighborRoomNumbers)
	{
		const auto& room = g_Level.Rooms[roomNumber];
		if (!room.Active())
			continue;

		for (const auto& staticObject : room.mesh)
		{
			if (!IsCameraCollideableStatic(staticObject))
				continue;

			staticPtrs.push_back(&staticObject);
		}
	}

	return staticPtrs;
}

void ItemsCollideCamera()
{
	//constexpr auto BOX_EXTENT_EXTENSION = Vector3(BLOCK(1 / 8.0f));

	constexpr auto CAMERA_RADIUS   = BLOCK(1 / 8.0f);
	constexpr auto DEBUG_BOX_COLOR = Color(1.0f, 0.0f, 0.0f);

	// Collide with items.
	auto itemPtrs = GetCameraCollidableItemPtrs();
	for (const auto* itemPtr : itemPtrs)
	{
		auto box = GameBoundingBox(itemPtr).ToBoundingOrientedBox(itemPtr->Pose);
		/*box.Extents.x += BOX_EXTENT_EXTENSION.x;
		box.Extents.y += BOX_EXTENT_EXTENSION.y;
		box.Extents.z += BOX_EXTENT_EXTENSION.z;*/

		auto intersect = GetCameraRayBoxIntersect(box);
		if (intersect != std::nullopt)
			Camera.pos = GameVector(*intersect, Camera.pos.RoomNumber);

		g_Renderer.AddDebugBox(box, DEBUG_BOX_COLOR, RendererDebugPage::CollisionStats);
	}

	// Collide with statics.
	auto staticPtrs = GetCameraCollidableStaticPtrs();
	for (const auto* staticPtr : staticPtrs)
	{
		auto box = GetBoundsAccurate(*staticPtr, false).ToBoundingOrientedBox(staticPtr->pos);
		/*box.Extents.x += BOX_EXTENT_EXTENSION.x;
		box.Extents.y += BOX_EXTENT_EXTENSION.y;
		box.Extents.z += BOX_EXTENT_EXTENSION.z;*/

		auto intersect = GetCameraRayBoxIntersect(box);
		if (intersect != std::nullopt)
			Camera.pos = GameVector(*intersect, Camera.pos.RoomNumber);

		g_Renderer.AddDebugBox(box, DEBUG_BOX_COLOR, RendererDebugPage::CollisionStats);
	}
}

void UpdateMikePos(const ItemInfo& item)
{
	if (Camera.mikeAtLara)
	{
		Camera.mikePos = item.Pose.Position;
		Camera.actualAngle = item.Pose.Orientation.y;

		if (item.IsLara())
		{
			const auto& player = GetLaraInfo(item);
			Camera.actualAngle += player.ExtraHeadRot.y + player.ExtraTorsoRot.y;
		}
	}
	else
	{
		int phdPerspective = g_Configuration.ScreenWidth / 2 * phd_cos(CurrentFOV / 2) / phd_sin(CurrentFOV / 2);

		Camera.actualAngle = phd_atan(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.mikePos.x = Camera.pos.x + phdPerspective * phd_sin(Camera.actualAngle);
		Camera.mikePos.z = Camera.pos.z + phdPerspective * phd_cos(Camera.actualAngle);
		Camera.mikePos.y = Camera.pos.y;
	}
}

void RumbleScreen()
{
	if (!(GlobalCounter & 0x1FF))
		SoundEffect(SFX_TR5_KLAXON, nullptr, SoundEnvironment::Land, 0.25f);

	if (RumbleTimer >= 0)
		RumbleTimer++;

	if (RumbleTimer > 450)
	{
		if (!(GetRandomControl() & 0x1FF))
		{
			RumbleCounter = 0;
			RumbleTimer = -32 - (GetRandomControl() & 0x1F);
			return;
		}
	}

	if (RumbleTimer < 0)
	{
		if (RumbleCounter >= abs(RumbleTimer))
		{
			Camera.bounce = -(GetRandomControl() % abs(RumbleTimer));
			RumbleTimer++;
		}
		else
		{
			RumbleCounter++;
			Camera.bounce = -(GetRandomControl() % RumbleCounter);
		}
	}
}

void SetScreenFadeOut(float speed, bool force)
{
	if (ScreenFading && !force)
		return;

	ScreenFading = true;
	ScreenFadeStart = 1.0f;
	ScreenFadeEnd = 0;
	ScreenFadeSpeed = speed;
	ScreenFadeCurrent = ScreenFadeStart;
}

void SetScreenFadeIn(float speed, bool force)
{
	if (ScreenFading && !force)
		return;

	ScreenFading = true;
	ScreenFadeStart = 0;
	ScreenFadeEnd = 1.0f;
	ScreenFadeSpeed = speed;
	ScreenFadeCurrent = ScreenFadeStart;
}

void SetCinematicBars(float height, float speed)
{
	CinematicBarsDestinationHeight = height;
	CinematicBarsSpeed = speed;
}

void ClearCinematicBars()
{
	CinematicBarsHeight = 0;
	CinematicBarsDestinationHeight = 0;
	CinematicBarsSpeed = 0;
}

void UpdateFadeScreenAndCinematicBars()
{
	if (CinematicBarsDestinationHeight < CinematicBarsHeight)
	{
		CinematicBarsHeight -= CinematicBarsSpeed;
		if (CinematicBarsDestinationHeight > CinematicBarsHeight)
			CinematicBarsHeight = CinematicBarsDestinationHeight;
	}
	else if (CinematicBarsDestinationHeight > CinematicBarsHeight)
	{
		CinematicBarsHeight += CinematicBarsSpeed;
		if (CinematicBarsDestinationHeight < CinematicBarsHeight)
			CinematicBarsHeight = CinematicBarsDestinationHeight;
	}

	int prevScreenFadeCurrent = ScreenFadeCurrent;

	if (ScreenFadeEnd != 0 && ScreenFadeEnd >= ScreenFadeCurrent)
	{
		ScreenFadeCurrent += ScreenFadeSpeed;
		if (ScreenFadeCurrent > ScreenFadeEnd)
		{
			ScreenFadeCurrent = ScreenFadeEnd;
			if (prevScreenFadeCurrent >= ScreenFadeCurrent)
			{
				ScreenFadedOut = true;
				ScreenFading = false;
			}

		}
	}
	else if (ScreenFadeEnd < ScreenFadeCurrent)
	{
		ScreenFadeCurrent -= ScreenFadeSpeed;
		if (ScreenFadeCurrent < ScreenFadeEnd)
		{
			ScreenFadeCurrent = ScreenFadeEnd;
			ScreenFading = false;
		}
	}
}
