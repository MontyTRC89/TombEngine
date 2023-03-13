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
#include "Game/room.h"
#include "Game/savegame.h"
#include "Game/spotcam.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using TEN::Renderer::g_Renderer;

using namespace TEN::Effects::Environment;
using namespace TEN::Entities::Generic;
using namespace TEN::Input;
using namespace TEN::Math;

constexpr auto PARTICLE_FADE_THRESHOLD = BLOCK(14);
constexpr auto COLL_CHECK_THRESHOLD    = BLOCK(4);
constexpr auto COLL_CANCEL_THRESHOLD   = BLOCK(2);
constexpr auto COLL_DISCARD_THRESHOLD  = CLICK(0.5f);
constexpr auto CAMERA_RADIUS           = CLICK(1);

constexpr auto LOOKCAM_TURN_RATE_ACCEL = ANGLE(0.75f);
constexpr auto LOOKCAM_TURN_RATE_MAX   = ANGLE(4.0f);

constexpr auto LOOKCAM_ORIENT_CONSTRAINT = std::pair<EulerAngles, EulerAngles>(
	EulerAngles(ANGLE(-70.0f), ANGLE(-90.0f), 0),
	EulerAngles(ANGLE(60.0f), ANGLE(90.0f), 0));

constexpr auto THUMBCAM_VERTICAL_CONSTRAINT_ANGLE	= ANGLE(120.0f);
constexpr auto THUMBCAM_HORIZONTAL_CONSTRAINT_ANGLE = ANGLE(80.0f);

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
GameVector Ideals[5];
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

int BinocularRange;
bool BinocularOn;
CameraType BinocularOldCamera;
bool LaserSight;

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

void DoLookAround(ItemInfo* item, bool invertVerticalAxis)
{
	auto& player = *GetLaraInfo(item);

	Camera.type = CameraType::Look;

	// Determine vertical axis coefficient.
	float vAxisCoeff = 0.0f;
	if ((IsHeld(In::Forward) || IsHeld(In::Back)) &&
		(player.Control.Look.Mode == LookMode::Vertical || player.Control.Look.Mode == LookMode::Free))
	{
		vAxisCoeff = AxisMap[InputAxis::MoveVertical];
	}

	// Determine horizontal axis coefficient.
	float hAxisCoeff = 0.0f;
	if ((IsHeld(In::Left) || IsHeld(In::Right)) &&
		(player.Control.Look.Mode == LookMode::Horizontal || player.Control.Look.Mode == LookMode::Free))
	{
		hAxisCoeff = AxisMap[InputAxis::MoveHorizontal];
	}

	short turnRateMax = LOOKCAM_TURN_RATE_MAX;
	if (BinocularRange)
		turnRateMax *= (BinocularRange - ANGLE(10.0f)) / ANGLE(17.0f);

	// Modulate and apply turn rates.
	player.Control.Look.TurnRate.x = ModulateLaraTurnRate(player.Control.Look.TurnRate.x, LOOKCAM_TURN_RATE_ACCEL, 0, turnRateMax, vAxisCoeff, invertVerticalAxis);
	player.Control.Look.TurnRate.y = ModulateLaraTurnRate(player.Control.Look.TurnRate.y, LOOKCAM_TURN_RATE_ACCEL, 0, turnRateMax, hAxisCoeff, false);
	player.Control.Look.Orientation += player.Control.Look.TurnRate;
	player.Control.Look.Orientation.x = std::clamp(player.Control.Look.Orientation.x, LOOKCAM_ORIENT_CONSTRAINT.first.x, LOOKCAM_ORIENT_CONSTRAINT.second.x);
	player.Control.Look.Orientation.y = std::clamp(player.Control.Look.Orientation.y, LOOKCAM_ORIENT_CONSTRAINT.first.y, LOOKCAM_ORIENT_CONSTRAINT.second.y);

	// Visually adapt head and torso orientations.
	// TODO: Rubber band effect.
	player.ExtraHeadRot = player.Control.Look.Orientation / 2;
	if (player.Control.HandStatus != HandStatus::Busy &&
		!player.LeftArm.Locked && !player.RightArm.Locked &&
		player.Vehicle == NO_ITEM)
	{
		player.ExtraTorsoRot = player.ExtraHeadRot;
	}

	ClearLookAroundActions(item);
}

void ClearLookAroundActions(ItemInfo* item)
{
	const auto& player = *GetLaraInfo(item);

	switch (player.Control.Look.Mode)
	{
	default:
	case LookMode::None:
		break;

	case LookMode::Vertical:
		ClearAction(In::Forward);
		ClearAction(In::Back);
		break;

	case LookMode::Horizontal:
		ClearAction(In::Left);
		ClearAction(In::Right);
		break;

	case LookMode::Free:
		ClearAction(In::Forward);
		ClearAction(In::Back);
		ClearAction(In::Left);
		ClearAction(In::Right);
		break;
	}
}

void DoThumbstickCamera()
{
	if (!g_Configuration.EnableThumbstickCameraControl)
		return;

	if (Camera.laraNode == -1 && (Camera.target.x == OldCam.target.x &&
		Camera.target.y == OldCam.target.y &&
		Camera.target.z == OldCam.target.z))
	{
		float hAxisCoeff = AxisMap[InputAxis::CameraHorizontal];
		float vAxisCoeff = AxisMap[InputAxis::CameraVertical];

		Camera.targetAngle = THUMBCAM_VERTICAL_CONSTRAINT_ANGLE * hAxisCoeff;
		Camera.targetElevation = ANGLE(-10.0f) + (THUMBCAM_HORIZONTAL_CONSTRAINT_ANGLE * hAxisCoeff);
	}
}

void LookCamera(ItemInfo* item)
{
	const auto& player = *GetLaraInfo(item);

	// TODO:
	// - Set modes in remaining states.

	constexpr auto VERTICAL_OFFSET_DEFAULT		  = -BLOCK(1.0f / 8);
	constexpr auto VERTICAL_OFFSET_SWAMP		  = BLOCK(0.5f); // TODO
	constexpr auto VERTICAL_OFFSET_MONKEY_SWING	  = BLOCK(0.25f);
	constexpr auto VERTICAL_OFFSET_TREADING_WATER = BLOCK(0.5f);
	constexpr auto POS_LERP_ALPHA				  = 0.25f;
	constexpr auto COLL_PUSH					  = BLOCK(0.25f) - BLOCK(1.0f / 16);

	// Determine vertical offset.
	float verticalOffset = -LaraCollision.Setup.Height;
	if (player.Control.IsMonkeySwinging)
	{
		verticalOffset += VERTICAL_OFFSET_MONKEY_SWING;
	}
	else if (player.Control.WaterStatus == WaterStatus::TreadWater)
	{
		verticalOffset += VERTICAL_OFFSET_TREADING_WATER;
	}
	else
	{
		verticalOffset += VERTICAL_OFFSET_DEFAULT;
	}

	// Calculate key offsets.
	auto pivotPosOffset = Vector3(0.0f, verticalOffset, 0.0f);
	float idealPosDist = -std::max(Camera.targetDistance * 0.5f, BLOCK(3.0f / 4));
	float lookAtPosDist = BLOCK(0.5f);

	// Define absolute camera orientation.
	auto orient = player.Control.Look.Orientation +
		EulerAngles(item->Pose.Orientation.x, item->Pose.Orientation.y, 0) +
		EulerAngles(0, Camera.targetAngle, 0);
	orient.x = std::clamp(orient.x, LOOKCAM_ORIENT_CONSTRAINT.first.x, LOOKCAM_ORIENT_CONSTRAINT.second.x);

	// Define landmarks.
	auto pivotPos = Geometry::TranslatePoint(item->Pose.Position, item->Pose.Orientation.y, pivotPosOffset.z, pivotPosOffset.y, pivotPosOffset.x); // TODO: Use overload from ladder branch.
	auto idealPos = Geometry::TranslatePoint(pivotPos, orient, idealPosDist);
	auto lookAtPos = Geometry::TranslatePoint(pivotPos, orient, lookAtPosDist);

	// Determine best position.
	auto origin = GameVector(pivotPos, GetCollision(item, item->Pose.Orientation.y, pivotPosOffset.z, pivotPosOffset.y).RoomNumber);
	auto target = GameVector(idealPos, GetCollision(origin.ToVector3i(), origin.RoomNumber, orient, idealPosDist).RoomNumber);

	// Handle room and object collisions.
	LOSAndReturnTarget(&origin, &target, 0);
	CameraCollisionBounds(&target, COLL_PUSH, true);
	ItemsCollideCamera();

	// Doesn't seem necessary? Can use player's room number.
	//auto lookAt = GameVector(lookAtPos, GetCollision(origin.ToVector3i(), origin.RoomNumber, orient, BLOCK(3, 8)).RoomNumber);

	// Smoothly update camera position.
	MoveCamera(&target, Camera.speed);
	Camera.target = GameVector(Camera.target.ToVector3i() + (lookAtPos - Camera.target.ToVector3i()) * POS_LERP_ALPHA, item->RoomNumber);

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

void InitialiseCamera()
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
	CalculateCamera();

	// Fade in screen.
	SetScreenFadeIn(FADE_SCREEN_SPEED);
}

void MoveCamera(GameVector* ideal, int speed)
{
	if (BinocularOn)
		speed = 1;

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
		BinocularOn)
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
			int bounce2 = -Camera.bounce >> 2;
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

	int y = Camera.pos.y;
	if (TestEnvironment(ENV_FLAG_SWAMP, Camera.pos.RoomNumber))
		y = g_Level.Rooms[Camera.pos.RoomNumber].y - CLICK(1);

	auto probe = GetCollision(Camera.pos.x, y, Camera.pos.z, Camera.pos.RoomNumber);
	if (y < probe.Position.Ceiling ||
		y > probe.Position.Floor)
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

	probe = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber);

	int buffer = CLICK(1) - 1;
	if ((Camera.pos.y - buffer) < probe.Position.Ceiling &&
		(Camera.pos.y + buffer) > probe.Position.Floor &&
		probe.Position.Ceiling < probe.Position.Floor &&
		probe.Position.Ceiling != NO_HEIGHT &&
		probe.Position.Floor != NO_HEIGHT)
	{
		Camera.pos.y = (probe.Position.Floor + probe.Position.Ceiling) / 2;
	}
	else if ((Camera.pos.y + buffer) > probe.Position.Floor &&
		probe.Position.Ceiling < probe.Position.Floor &&
		probe.Position.Ceiling != NO_HEIGHT &&
		probe.Position.Floor != NO_HEIGHT)
	{
		Camera.pos.y = probe.Position.Floor - buffer;
	}
	else if ((Camera.pos.y - buffer) < probe.Position.Ceiling &&
		probe.Position.Ceiling < probe.Position.Floor &&
		probe.Position.Ceiling != NO_HEIGHT &&
		probe.Position.Floor != NO_HEIGHT)
	{
		Camera.pos.y = probe.Position.Ceiling + buffer;
	}
	else if (probe.Position.Ceiling >= probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		Camera.pos = *ideal;
	}

	ItemsCollideCamera();

	Camera.pos.RoomNumber = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber).RoomNumber;
	LookAt(&Camera, 0);
	UpdateMikePos(LaraItem);
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
		BinocularOn)
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
	static const unsigned int maxSwivelSteps = 5;

	if (!Camera.targetElevation)
		Camera.targetElevation = -ANGLE(10.0f);

	Camera.targetElevation += item->Pose.Orientation.x;
	UpdateCameraElevation();

	// Clamp X orientation.
	if (Camera.actualElevation > ANGLE(85.0f))
		Camera.actualElevation = ANGLE(85.0f);
	else if (Camera.actualElevation < ANGLE(-85.0f))
		Camera.actualElevation = ANGLE(-85.0f);

	int distance = Camera.targetDistance * phd_cos(Camera.actualElevation);

	auto probe = GetCollision(Camera.target.x, Camera.target.y + CLICK(1), Camera.target.z, Camera.target.RoomNumber);

	if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		Camera.target.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);

	int y = Camera.target.y;
	probe = GetCollision(Camera.target.x, y, Camera.target.z, Camera.target.RoomNumber);
	if (((y < probe.Position.Ceiling || probe.Position.Floor < y) || probe.Position.Floor <= probe.Position.Ceiling) ||
		(probe.Position.Floor == NO_HEIGHT || probe.Position.Ceiling == NO_HEIGHT))
	{
		TargetSnaps++;
		Camera.target = LastTarget;
	}
	else
		TargetSnaps = 0;

	for (int i = 0; i < maxSwivelSteps; i++)
		Ideals[i].y = Camera.target.y + (Camera.targetDistance * phd_sin(Camera.actualElevation));

	// Determine best player viewing angle.
	float farthestDist = FLT_MAX;
	int indexOfFarthestIdeal = 0;
	GameVector temp[2];
	for (int i = 0; i < maxSwivelSteps; i++)
	{
		// Incrementally swivel camera position.
		short angle = (i == 0) ? Camera.actualAngle : ((i - 1) * ANGLE(90.0f));

		// Record ideal position at default distance for given swivel.
		Ideals[i].x = Camera.target.x - (distance * phd_sin(angle));
		Ideals[i].z = Camera.target.z - (distance * phd_cos(angle));
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
	CameraCollisionBounds(&ideal, CLICK(1.5f), 1);
	MoveCamera(&ideal, Camera.speed);
}

void UpdateCameraElevation()
{
	DoThumbstickCamera();

	if (Camera.laraNode != -1)
	{
		auto pos = GetJointPosition(LaraItem, Camera.laraNode, Vector3i::Zero);
		auto pos1 = GetJointPosition(LaraItem, Camera.laraNode, Vector3i(0, -CLICK(1), BLOCK(2)));
		pos = pos1 - pos;
		Camera.actualAngle = Camera.targetAngle + phd_atan(pos.z, pos.x);
	}
	else
		Camera.actualAngle = LaraItem->Pose.Orientation.y + Camera.targetAngle;

	Camera.actualElevation += (Camera.targetElevation - Camera.actualElevation) / 8;
}

void CombatCamera(ItemInfo* item)
{
	static const unsigned int maxSwivelSteps = 5;

	auto* lara = GetLaraInfo(item);

	Camera.target.x = item->Pose.Position.x;
	Camera.target.z = item->Pose.Position.z;

	if (lara->TargetEntity)
	{
		Camera.targetAngle = lara->TargetArmOrient.y;
		Camera.targetElevation = lara->TargetArmOrient.x + item->Pose.Orientation.x;
	}
	else
	{
		Camera.targetAngle = lara->ExtraHeadRot.y + lara->ExtraTorsoRot.y;
		Camera.targetElevation = lara->ExtraHeadRot.x + lara->ExtraTorsoRot.x + item->Pose.Orientation.x - ANGLE(15.0f);
	}

	auto probe = GetCollision(Camera.target.x, Camera.target.y + CLICK(1), Camera.target.z, Camera.target.RoomNumber);
	if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		Camera.target.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);

	probe = GetCollision(Camera.target.x, Camera.target.y, Camera.target.z, Camera.target.RoomNumber);
	Camera.target.RoomNumber = probe.RoomNumber;

	int buffer = CLICK(0.25f);
	if ((probe.Position.Ceiling + buffer) > (probe.Position.Floor - buffer) &&
		probe.Position.Floor != NO_HEIGHT &&
		probe.Position.Ceiling != NO_HEIGHT)
	{
		Camera.target.y = (probe.Position.Ceiling + probe.Position.Floor) / 2;
		Camera.targetElevation = 0;
	}
	else if (Camera.target.y > (probe.Position.Floor - buffer) &&
		probe.Position.Floor != NO_HEIGHT)
	{
		Camera.target.y = probe.Position.Floor - buffer;
		Camera.targetElevation = 0;
	}
	else if (Camera.target.y < (probe.Position.Ceiling + buffer) &&
		probe.Position.Ceiling != NO_HEIGHT)
	{
		Camera.target.y = probe.Position.Ceiling + buffer;
		Camera.targetElevation = 0;
	}

	int y = Camera.target.y;
	probe = GetCollision(Camera.target.x, y, Camera.target.z, Camera.target.RoomNumber);
	Camera.target.RoomNumber = probe.RoomNumber;

	if (y < probe.Position.Ceiling ||
		y > probe.Position.Floor ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		TargetSnaps++;
		Camera.target = LastTarget;
	}
	else
		TargetSnaps = 0;

	UpdateCameraElevation();

	Camera.targetDistance = BLOCK(1.5f);
	int distance = Camera.targetDistance * phd_cos(Camera.actualElevation);

	for (int i = 0; i < maxSwivelSteps; i++)
		Ideals[i].y = Camera.target.y + (Camera.targetDistance * phd_sin(Camera.actualElevation));

	// Determine best player viewing angle.
	float farthestDist = FLT_MAX;
	int indexOfFarthestIdeal = 0;
	GameVector temp[2];
	for (int i = 0; i < maxSwivelSteps; i++)
	{
		// Incrementally swivel camera position.
		short angle = (i == 0) ? Camera.actualAngle : ((i - 1) * ANGLE(90.0f));

		// Record ideal position at default distance for given swivel.
		Ideals[i].x = Camera.target.x - (distance * phd_sin(angle));
		Ideals[i].z = Camera.target.z - (distance * phd_cos(angle));
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

bool CameraCollisionBounds(GameVector* ideal, int push, bool yFirst)
{
	int x = ideal->x;
	int y = ideal->y;
	int z = ideal->z;

	CollisionResult probe = {};
	if (yFirst)
	{
		probe = GetCollision(x, y, z, ideal->RoomNumber);

		int buffer = CLICK(1) - 1;
		if ((y - buffer) < probe.Position.Ceiling &&
			(y + buffer) > probe.Position.Floor &&
			probe.Position.Ceiling < probe.Position.Floor &&
			probe.Position.Ceiling != NO_HEIGHT &&
			probe.Position.Floor != NO_HEIGHT)
		{
			y = (probe.Position.Floor + probe.Position.Ceiling) / 2;
		}
		else if ((y + buffer) > probe.Position.Floor &&
			probe.Position.Ceiling < probe.Position.Floor &&
			probe.Position.Ceiling != NO_HEIGHT &&
			probe.Position.Floor != NO_HEIGHT)
		{
			y = probe.Position.Floor - buffer;
		}
		else if ((y - buffer) < probe.Position.Ceiling &&
			probe.Position.Ceiling < probe.Position.Floor &&
			probe.Position.Ceiling != NO_HEIGHT &&
			probe.Position.Floor != NO_HEIGHT)
		{
			y = probe.Position.Ceiling + buffer;
		}
	}

	probe = GetCollision(x - push, y, z, ideal->RoomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		x = (x & (~1023)) + push;
	}

	probe = GetCollision(x, y, z - push, ideal->RoomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		z = (z & (~1023)) + push;
	}

	probe = GetCollision(x + push, y, z, ideal->RoomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		x = (x | 1023) - push;
	}

	probe = GetCollision(x, y, z + push, ideal->RoomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		z = (z | 1023) - push;
	}

	if (!yFirst)
	{
		probe = GetCollision(x, y, z, ideal->RoomNumber);

		int buffer = CLICK(1) - 1;
		if ((y - buffer) < probe.Position.Ceiling &&
			(y + buffer) > probe.Position.Floor &&
			probe.Position.Ceiling < probe.Position.Floor &&
			probe.Position.Ceiling != NO_HEIGHT &&
			probe.Position.Floor != NO_HEIGHT)
		{
			y = (probe.Position.Floor + probe.Position.Ceiling) / 2;
		}
		else if ((y + buffer) > probe.Position.Floor &&
			probe.Position.Ceiling < probe.Position.Floor &&
			probe.Position.Ceiling != NO_HEIGHT &&
			probe.Position.Floor != NO_HEIGHT)
		{
			y = probe.Position.Floor - buffer;
		}
		else if ((y - buffer) < probe.Position.Ceiling &&
			probe.Position.Ceiling < probe.Position.Floor &&
			probe.Position.Ceiling != NO_HEIGHT &&
			probe.Position.Floor != NO_HEIGHT)
		{
			y = probe.Position.Ceiling + buffer;
		}
	}

	probe = GetCollision(x, y, z, ideal->RoomNumber);
	if (y > probe.Position.Floor ||
		y < probe.Position.Ceiling ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor)
	{
		return true;
	}

	ideal->RoomNumber = probe.RoomNumber;
	ideal->x = x;
	ideal->y = y;
	ideal->z = z;

	return false;
}

void FixedCamera(ItemInfo* item)
{
	// Fixed cameras before TR3 had optional "movement" effect. 
	// Later for some reason it was forced to always be 1, and actual speed value
	// from camera trigger was ignored. In TEN, we move speed value out of legacy
	// floordata trigger to camera itself and make use of it again. Still, by default,
	// value is 1 for UseForcedFixedCamera hack.

	int moveSpeed = 1;

	GameVector origin, target;
	if (UseForcedFixedCamera)
		origin = ForcedFixedCamera;
	else
	{
		auto* camera = &g_Level.Cameras[Camera.number];

		origin = GameVector(camera->Position, camera->RoomNumber);

		// Multiply original speed by 8 to comply with original bitshifted speed from TR1-2.
		moveSpeed = camera->Speed * 8 + 1;
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
	auto* lara = GetLaraInfo(item);

	if (!LaserSight)
	{
		if (IsClicked(In::Deselect) ||
			IsClicked(In::DrawWeapon) ||
			IsClicked(In::Look) ||
			IsHeld(In::Flare))
		{
			lara->Inventory.IsBusy = false;
			lara->ExtraHeadRot = EulerAngles::Zero;
			lara->ExtraTorsoRot = EulerAngles::Zero;
			Camera.type = BinocularOldCamera;
			BinocularOn = false;
			BinocularRange = 0;
			AlterFOV(LastFOV);
			return;
		}
	}

	AlterFOV(7 * (ANGLE(11.5f) - BinocularRange), false);

	short headXRot = lara->ExtraHeadRot.x * 2;
	short headYRot = lara->ExtraHeadRot.y;

	if (headXRot > ANGLE(75.0f))
		headXRot = ANGLE(75.0f);
	else if (headXRot < -ANGLE(75.0f))
		headXRot = -ANGLE(75.0f);

	if (headYRot > ANGLE(80.0f))
		headYRot = ANGLE(80.0f);
	else if (headYRot < -ANGLE(80.0f))
		headYRot = -ANGLE(80.0f);

	int x = item->Pose.Position.x;
	int y = item->Pose.Position.y - CLICK(2);
	int z = item->Pose.Position.z;

	auto probe = GetCollision(x, y, z, item->RoomNumber);
	if (probe.Position.Ceiling <= (y - CLICK(1)))
		y -= CLICK(1);
	else
		y = probe.Position.Ceiling + CLICK(0.25f);

	Camera.pos.x = x;
	Camera.pos.y = y;
	Camera.pos.z = z;
	Camera.pos.RoomNumber = probe.RoomNumber;

	int l = BLOCK(20.25f) * phd_cos(headXRot);

	int tx = x + l * phd_sin(item->Pose.Orientation.y + headYRot);
	int ty = y - BLOCK(20.25f) * phd_sin(headXRot);
	int tz = z + l * phd_cos(item->Pose.Orientation.y + headYRot);

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
	UpdateMikePos(item);
	Camera.oldType = Camera.type;

	int range = IsHeld(In::Walk) ? ANGLE(0.18f) : ANGLE(0.35f);
	if (IsHeld(In::Sprint) && !IsHeld(In::Crouch))
	{
		BinocularRange -= range;
		if (BinocularRange < ANGLE(0.7f))
			BinocularRange = ANGLE(0.7f);
		else
			SoundEffect(SFX_TR4_BINOCULARS_ZOOM, nullptr, SoundEnvironment::Land, 0.9f);
	}
	else if (IsHeld(In::Crouch) && !IsHeld(In::Sprint))
	{
		BinocularRange += range;
		if (BinocularRange > ANGLE(8.5f))
			BinocularRange = ANGLE(8.5f);
		else
			SoundEffect(SFX_TR4_BINOCULARS_ZOOM, nullptr, SoundEnvironment::Land, 1.0f);
	}

	auto origin = Camera.pos.ToVector3i();
	auto target = Camera.target.ToVector3i();

	GetTargetOnLOS(&Camera.pos, &Camera.target, false, false);

	if (IsHeld(In::Action))
		LaraTorch(&origin, &target, lara->ExtraHeadRot.y, 192);
}

void ConfirmCameraTargetPos()
{
	auto pos = GetJointPosition(LaraItem, LM_TORSO);

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

void CalculateCamera()
{
	CamOldPos.x = Camera.pos.x;
	CamOldPos.y = Camera.pos.y;
	CamOldPos.z = Camera.pos.z;

	if (BinocularOn)
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
			auto dx = Camera.item->Pose.Position.x - item->Pose.Position.x;
			auto dz = Camera.item->Pose.Position.z - item->Pose.Position.z;
			int shift = sqrt(pow(dx, 2) + pow(dz, 2));
			short angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
			short tilt = phd_atan(shift, y - (bounds.Y1 + bounds.Y2) / 2 - Camera.item->Pose.Position.y);
			bounds = GameBoundingBox(Camera.item);
			angle /= 2;
			tilt /= 2;

			if (angle > -ANGLE(50.0f) && angle < ANGLE(50.0f) && tilt > -ANGLE(85.0f) && tilt < ANGLE(85.0f))
			{
				short change = angle - Lara.ExtraHeadRot.y;
				if (change > ANGLE(4.0f))
					Lara.ExtraHeadRot.y += ANGLE(4.0f);
				else if (change < -ANGLE(4.0f))
					Lara.ExtraHeadRot.y -= ANGLE(4.0f);
				else
					Lara.ExtraHeadRot.y += change;
				Lara.ExtraTorsoRot.y = Lara.ExtraHeadRot.y;

				change = tilt - Lara.ExtraHeadRot.x;
				if (change > ANGLE(4.0f))
					Lara.ExtraHeadRot.x += ANGLE(4.0f);
				else if (change < -ANGLE(4.0f))
					Lara.ExtraHeadRot.x -= ANGLE(4.0f);
				else
					Lara.ExtraHeadRot.x += change;
				Lara.ExtraTorsoRot.x = Lara.ExtraHeadRot.x;

				Camera.type = CameraType::Look;
				Camera.item->LookedAt = 1;
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
		}

		Camera.target.RoomNumber = item->RoomNumber;

		if (Camera.fixedCamera || BinocularOn)
		{
			Camera.target.y = y;
			Camera.speed = 1;
		}
		else
		{
			Camera.target.y += (y - Camera.target.y) >> 2;
			Camera.speed = Camera.type != CameraType::Look ? 8 : 4;
		}

		Camera.fixedCamera = false;
		if (Camera.type == CameraType::Look)
			LookCamera(item);
		else
			CombatCamera(item);
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
				Camera.oldType != CameraType::Look &&
				!BinocularOn)
			{
				if (TargetSnaps <= 8)
				{
					x = LastTarget.x + ((x - LastTarget.x) >> 2);
					Camera.target.x = x;
					y = LastTarget.y + ((y - LastTarget.y) >> 2);
					Camera.target.y = y;
					z = LastTarget.z + ((z - LastTarget.z) >> 2);
					Camera.target.z = z;
				}
				else
					TargetSnaps = 0;
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
			FixedCamera(item);
		else
			ChaseCamera(item);
	}

	Camera.fixedCamera = isFixedCamera;
	Camera.last = Camera.number;

	if (Camera.type != CameraType::Heavy ||
		Camera.timer == -1)
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
}

void ResetLook(ItemInfo* item, float alpha)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.Look.Orientation = EulerAngles::Zero;

	if (Camera.type != CameraType::Look)
	{
		if (abs(lara->ExtraHeadRot.x) > ANGLE(0.1f))
			lara->ExtraHeadRot.x *= alpha;
		else
			lara->ExtraHeadRot.x = 0;

		if (abs(lara->ExtraHeadRot.y) > ANGLE(0.1f))
			lara->ExtraHeadRot.y *= alpha;
		else
			lara->ExtraHeadRot.y = 0;

		if (abs(lara->ExtraHeadRot.z) > ANGLE(0.1f))
			lara->ExtraHeadRot.z *= alpha;
		else
			lara->ExtraHeadRot.z = 0;

		if (lara->Control.HandStatus != HandStatus::Busy &&
			!lara->LeftArm.Locked && !lara->RightArm.Locked &&
			lara->Vehicle == NO_ITEM)
		{
			lara->ExtraTorsoRot = lara->ExtraHeadRot;
		}
		else
		{
			if (!lara->ExtraHeadRot.x)
				lara->ExtraTorsoRot.x = 0;
			if (!lara->ExtraHeadRot.y)
				lara->ExtraTorsoRot.y = 0;
			if (!lara->ExtraHeadRot.z)
				lara->ExtraTorsoRot.z = 0;
		}
	}
}

bool TestBoundsCollideCamera(const GameBoundingBox& bounds, const Pose& pose, short radius)
{
	auto sphere = BoundingSphere(Camera.pos.ToVector3(), radius);
	return sphere.Intersects(bounds.ToBoundingOrientedBox(pose));
}

float GetParticleDistanceFade(Vector3i position)
{
	float distance = Vector3::Distance(Camera.pos.ToVector3(), position.ToVector3());

	if (distance <= PARTICLE_FADE_THRESHOLD)
		return 1.0f;

	return std::clamp(1.0f - ((distance - PARTICLE_FADE_THRESHOLD) / COLL_CHECK_THRESHOLD), 0.0f, 1.0f);
}

void ItemPushCamera(GameBoundingBox* bounds, Pose* pos, short radius)
{
	int dx = Camera.pos.x - pos->Position.x;
	int dz = Camera.pos.z - pos->Position.z;
	auto sinY = phd_sin(pos->Orientation.y);
	auto cosY = phd_cos(pos->Orientation.y);
	auto x = (dx * cosY) - (dz * sinY);
	auto z = (dx * sinY) + (dz * cosY);

	int xMin = bounds->X1 - radius;
	int xMax = bounds->X2 + radius;
	int zMin = bounds->Z1 - radius;
	int zMax = bounds->Z2 + radius;

	if (x <= xMin || x >= xMax || z <= zMin || z >= zMax)
		return;

	auto left = x - xMin;
	auto right = xMax - x;
	auto top = zMax - z;
	auto bottom = z - zMin;

	// Left is closest.
	if (left <= right && left <= top && left <= bottom)
		x -= left;
	// Right is closest.
	else if (right <= left && right <= top && right <= bottom)
		x += right;
	// Top is closest.
	else if (top <= left && top <= right && top <= bottom)
		z += top;
	// Bottom is closest.
	else
		z -= bottom;

	Camera.pos.x = pos->Position.x + ((x * cosY) + (z * sinY));
	Camera.pos.z = pos->Position.z + ((z * cosY) - (x * sinY));

	auto pointColl = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber);
	if (pointColl.Position.Floor == NO_HEIGHT || Camera.pos.y > pointColl.Position.Floor || Camera.pos.y < pointColl.Position.Ceiling)
		Camera.pos = GameVector(CamOldPos, pointColl.RoomNumber);
}

bool CheckItemCollideCamera(ItemInfo* item)
{
	bool isCloseEnough = Vector3i::Distance(item->Pose.Position, Camera.pos.ToVector3i()) <= COLL_CHECK_THRESHOLD;

	if (!isCloseEnough || !item->Collidable || !Objects[item->ObjectNumber].usingDrawAnimatingItem)
		return false;

	// TODO: Find a better way to define objects which are collidable with camera.
	auto* object = &Objects[item->ObjectNumber];
	if (object->intelligent || object->isPickup || object->isPuzzleHole || object->collision == nullptr)
		return false;

	// Check extents; if any 2 bounds are smaller than threshold, discard.
	auto extents = GameBoundingBox(item).ToBoundingOrientedBox(item->Pose).Extents;
	if ((abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.y) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.y) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD))
	{
		return false;
	}

	return true;
}

std::vector<short> FillCollideableItemList()
{
	std::vector<short> itemList;
	auto& roomList = g_Level.Rooms[Camera.pos.RoomNumber].neighbors;

	for (short i = 0; i < g_Level.NumItems; i++)
	{
		auto* item = &g_Level.Items[i];

		if (std::find(roomList.begin(), roomList.end(), item->RoomNumber) == roomList.end())
			continue;

		if (!g_Level.Rooms[item->RoomNumber].Active())
			continue;

		if (!CheckItemCollideCamera(&g_Level.Items[i]))
			continue;

		itemList.push_back(i);
	}

	return itemList;
}

bool CheckStaticCollideCamera(MESH_INFO* mesh)
{
	bool isCloseEnough = Vector3i::Distance(mesh->pos.Position, Camera.pos.ToVector3i()) <= COLL_CHECK_THRESHOLD;
	if (!isCloseEnough)
		return false;

	if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
		return false;

	const auto& bounds = GetBoundsAccurate(*mesh, false);
	auto extents = Vector3(
		abs(bounds.X1 - bounds.X2),
		abs(bounds.Y1 - bounds.Y2),
		abs(bounds.Z1 - bounds.Z2)
	);

	// Check extents; if any two bounds are smaller than threshold, discard.
	if ((abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.y) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.y) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD))
	{
		return false;
	}

	return true;
}

std::vector<MESH_INFO*> FillCollideableStaticsList()
{
	std::vector<MESH_INFO*> staticList;
	auto& roomList = g_Level.Rooms[Camera.pos.RoomNumber].neighbors;

	for (int i : roomList)
	{
		auto* room = &g_Level.Rooms[i];

		if (!room->Active())
			continue;

		for (short j = 0; j < room->mesh.size(); j++)
		{
			if (!CheckStaticCollideCamera(&room->mesh[j]))
				continue;

			staticList.push_back(&room->mesh[j]);
		}
	}

	return staticList;
}

void ItemsCollideCamera()
{
	float radius = CLICK(0.5f);
	auto itemList = FillCollideableItemList();

	// Collide with items in the items list.
	for (int i = 0; i < itemList.size(); i++)
	{
		auto* item = &g_Level.Items[itemList[i]];
		if (!item)
			return;

		// Break off if camera is stuck behind an object and the player runs off.
		auto distance = Vector3i::Distance(item->Pose.Position, LaraItem->Pose.Position);
		if (distance > COLL_CANCEL_THRESHOLD)
			continue;

		auto bounds = GameBoundingBox(item);
		if (TestBoundsCollideCamera(bounds, item->Pose, CAMERA_RADIUS))
			ItemPushCamera(&bounds, &item->Pose, radius);

		TEN::Renderer::g_Renderer.AddDebugBox(bounds.ToBoundingOrientedBox(item->Pose),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f), RENDERER_DEBUG_PAGE::LARA_STATS);
	}

	// Done.
	itemList.clear();

	// Collide with static meshes.
	auto staticList = FillCollideableStaticsList();
	for (auto* mesh : staticList)
	{
		if (!mesh)
			return;

		auto distance = Vector3i::Distance(mesh->pos.Position, LaraItem->Pose.Position);
		if (distance > COLL_CANCEL_THRESHOLD)
			continue;

		auto bounds = GetBoundsAccurate(*mesh, false);
		if (TestBoundsCollideCamera(bounds, mesh->pos, CAMERA_RADIUS))
			ItemPushCamera(&bounds, &mesh->pos, radius);

		TEN::Renderer::g_Renderer.AddDebugBox(bounds.ToBoundingOrientedBox(mesh->pos),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f), RENDERER_DEBUG_PAGE::LARA_STATS);
	}

	// Done.
	staticList.clear();
}

void UpdateMikePos(ItemInfo* item)
{
	if (Camera.mikeAtLara)
	{
		Camera.mikePos = item->Pose.Position;
		Camera.actualAngle = item->Pose.Orientation.y;

		if (item->IsLara())
		{
			auto& player = *GetLaraInfo(item);
			Camera.actualAngle += player.ExtraHeadRot.y + player.ExtraTorsoRot.y;
		}
	}
	else
	{
		int phdPerspective = g_Configuration.Width / 2 * phd_cos(CurrentFOV / 2) / phd_sin(CurrentFOV / 2);

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

void SetScreenFadeOut(float speed)
{
	if (!ScreenFading)
	{
		ScreenFading = true;
		ScreenFadeStart = 1.0f;
		ScreenFadeEnd = 0;
		ScreenFadeSpeed = speed;
		ScreenFadeCurrent = ScreenFadeStart;
	}
}

void SetScreenFadeIn(float speed)
{
	if (!ScreenFading)
	{
		ScreenFading = true;
		ScreenFadeStart = 0;
		ScreenFadeEnd = 1.0f;
		ScreenFadeSpeed = speed;
		ScreenFadeCurrent = ScreenFadeStart;
	}
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

void HandleOptics(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	bool breakOptics = true;

	// Imitate pushing look key in binocular mode.
	if (!LaserSight && BinocularOn)
	{
		TrInput |= IN_LOOK;
		DbInput = 0;
	}

	// Standing; can use optics.
	if (item->Animation.ActiveState == LS_IDLE || item->Animation.AnimNumber == LA_STAND_IDLE)
		breakOptics = false;

	// Crouching; can use optics.
	if ((lara->Control.IsLow || !IsHeld(In::Crouch)) &&
		(item->Animation.TargetState == LS_CROUCH_IDLE || item->Animation.AnimNumber == LA_CROUCH_IDLE))
	{
		breakOptics = false;
	}

	// If lasersight, and Look is not pressed, exit optics.
	if (LaserSight && !IsHeld(In::Look))
		breakOptics = true;

	// If lasersight, and weapon is holstered, exit optics.
	if (LaserSight && IsHeld(In::DrawWeapon))
		breakOptics = true;

	// Engage lasersight if available.
	if (!LaserSight && !breakOptics && (TrInput == IN_LOOK))
	{
		if (lara->Control.HandStatus == HandStatus::WeaponReady &&
			((lara->Control.Weapon.GunType == LaraWeaponType::HK && lara->Weapons[(int)LaraWeaponType::HK].HasLasersight) ||
				(lara->Control.Weapon.GunType == LaraWeaponType::Revolver && lara->Weapons[(int)LaraWeaponType::Revolver].HasLasersight) ||
				(lara->Control.Weapon.GunType == LaraWeaponType::Crossbow && lara->Weapons[(int)LaraWeaponType::Crossbow].HasLasersight)))
		{
			BinocularRange = 128;
			BinocularOldCamera = Camera.oldType;
			BinocularOn = true;
			LaserSight = true;
			lara->Inventory.IsBusy = true;
			return;
		}
	}

	if (!breakOptics)
		return;

	// Nothing to process, exit.
	if (!BinocularOn && !LaserSight)
		return;

	BinocularRange = 0;
	BinocularOn = false;
	LaserSight = false;
	Camera.type = BinocularOldCamera;
	Camera.bounce = 0;
	AlterFOV(LastFOV);

	lara->Inventory.IsBusy = false;
	ResetLaraFlex(item);

	ClearAction(In::Look);
}
