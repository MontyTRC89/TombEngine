#include "framework.h"
#include "Game/camera.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/effects/effects.h"
#include "Game/effects/debris.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/room.h"
#include "Game/savegame.h"
#include "Game/spotcam.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using TEN::Renderer::g_Renderer;

using namespace TEN::Effects::Environment;
using namespace TEN::Entities::Generic;
using namespace TEN::Input;

constexpr auto PARTICLE_FADE_THRESHOLD = SECTOR(14);
constexpr auto COLL_CHECK_THRESHOLD    = SECTOR(4);
constexpr auto COLL_CANCEL_THRESHOLD   = SECTOR(2);
constexpr auto COLL_DISCARD_THRESHOLD  = CLICK(0.5f);
constexpr auto CAMERA_RADIUS           = CLICK(1);

constexpr auto THUMBCAM_VERTICAL_CONSTRAINT_ANGLE   = 120.0f;
constexpr auto THUMBCAM_HORIZONTAL_CONSTRAINT_ANGLE = 80.0f;

struct OLD_CAMERA
{
	short ActiveState;
	short TargetState;
	float targetDistance;
	short actualElevation;
	short targetElevation;
	short actualAngle;
	Pose pos;
	Pose pos2;

	Vector3 target = Vector3::Zero;
};

Vector3 LastTarget			 = Vector3::Zero;
int		LastTargetRoomNumber = 0;

Vector3 LastIdeal;
int		LastIdealRoomNumber;
Vector3 Ideals[5];
int		IdealRoomNumbers[5];

OLD_CAMERA OldCam;
int CameraSnaps = 0;
int TargetSnaps = 0;
GameVector LookCamPosition;
GameVector LookCamTarget;
Vector3 CamOldPos;
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

void LookAt(CAMERA_INFO* cam, short roll)
{
	Vector3 position = Vector3(cam->pos.x, cam->pos.y, cam->pos.z);
	Vector3 target = Vector3(cam->target.x, cam->target.y, cam->target.z);
	Vector3 up = Vector3(0.0f, -1.0f, 0.0f);
	float fov = TO_RAD(CurrentFOV / 1.333333f);
	float r = TO_RAD(roll);

	float levelFarView = g_GameFlow->GetLevel(CurrentLevel)->GetFarView() * float(SECTOR(1));

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
	Camera.shift = LaraItem->Pose.Position.y - SECTOR(1);

	LastTarget.x = LaraItem->Pose.Position.x;
	LastTarget.y = Camera.shift;
	LastTarget.z = LaraItem->Pose.Position.z;
	LastTargetRoomNumber = LaraItem->RoomNumber;

	Camera.target.x = LastTarget.x;
	Camera.target.y = Camera.shift;
	Camera.target.z = LastTarget.z;
	Camera.TargetRoomNumber = LaraItem->RoomNumber;

	Camera.pos.x = LastTarget.x;
	Camera.pos.y = Camera.shift;
	Camera.pos.z = LastTarget.z - 100.0f;
	Camera.RoomNumber = LaraItem->RoomNumber;

	Camera.targetDistance = BLOCK(1.5f);
	Camera.item = NULL;
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
	GameVector from, to;

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
		LastIdealRoomNumber = ideal->RoomNumber;
	}
	else
	{
		ideal->x = LastIdeal.x;
		ideal->y = LastIdeal.y;
		ideal->z = LastIdeal.z;
		ideal->RoomNumber = LastIdealRoomNumber;
	}

	if (IsHeld(In::Action))
		LaraItem->Pose.Orientation.y += 1;

	Camera.pos.x += (ideal->x - Camera.pos.x) / speed;
	Camera.pos.y += (ideal->y - Camera.pos.y) / speed;
	Camera.pos.z += (ideal->z - Camera.pos.z) / speed;
	Camera.RoomNumber = ideal->RoomNumber;

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
	if (TestEnvironment(ENV_FLAG_SWAMP, Camera.RoomNumber))
		y = g_Level.Rooms[Camera.RoomNumber].y - CLICK(1);

	auto probe = GetCollision(Camera.pos.x, y, Camera.pos.z, Camera.RoomNumber);
	if (y < probe.Position.Ceiling ||
		y > probe.Position.Floor)
	{
		auto origin = GameVector(Camera.target, Camera.TargetRoomNumber);
		auto target = GameVector(Camera.pos, Camera.RoomNumber);
		LOSAndReturnTarget(&origin, &target, 0);
		Camera.pos = target.ToVector3();
		Camera.RoomNumber = target.RoomNumber;

		if (abs(Camera.pos.x - ideal->x) < SECTOR(0.5f) &&
			abs(Camera.pos.y - ideal->y) < SECTOR(0.5f) &&
			abs(Camera.pos.z - ideal->z) < SECTOR(0.5f))
		{
			to.x = Camera.pos.x;
			to.y = Camera.pos.y;
			to.z = Camera.pos.z;
			to.RoomNumber = Camera.RoomNumber;

			from.x = ideal->x;
			from.y = ideal->y;
			from.z = ideal->z;
			from.RoomNumber = ideal->RoomNumber;

			if (!LOSAndReturnTarget(&from, &to, 0) &&
				++CameraSnaps >= 8)
			{
				Camera.pos.x = ideal->x;
				Camera.pos.y = ideal->y;
				Camera.pos.z = ideal->z;
				Camera.RoomNumber = ideal->RoomNumber;
				CameraSnaps = 0;
			}
		}
	}

	probe = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.RoomNumber);

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
		Camera.pos.x = ideal->x;
		Camera.pos.y = ideal->y;
		Camera.pos.z = ideal->z;
		Camera.RoomNumber = ideal->RoomNumber;
	}

	ItemsCollideCamera();

	Camera.RoomNumber = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.RoomNumber).RoomNumber;
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
	auto pos = GetJointPosition(camSlotId, camMeshId).ToVector3();
	//Get mesh2 to attach target to
	//Vector3i pos2 = Vector3i::Zero;
	auto pos2 = GetJointPosition(targetItem, targetMeshId).ToVector3();

	if (OldCam.pos.Position != pos ||
		OldCam.targetDistance  != Camera.targetDistance  ||
		OldCam.targetElevation != Camera.targetElevation ||
		OldCam.actualElevation != Camera.actualElevation ||
		OldCam.actualAngle != Camera.actualAngle ||
		OldCam.target != Camera.target ||
		Camera.oldType != Camera.type ||
		BinocularOn)
	{
		OldCam.pos.Position = pos;
		OldCam.targetDistance = Camera.targetDistance;
		OldCam.targetElevation = Camera.targetElevation;
		OldCam.actualElevation = Camera.actualElevation;
		OldCam.actualAngle = Camera.actualAngle;
		OldCam.target = Camera.target;
		LastIdeal = pos;
		LastIdealRoomNumber = ideal->RoomNumber;
		LastTarget = pos2;
	}
	else
	{
		pos  = LastIdeal;
		pos2 = LastTarget;
		ideal->RoomNumber = LastIdealRoomNumber;
	}

	Camera.pos += (ideal->ToVector3() - Camera.pos) / speed;
	Camera.RoomNumber = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.RoomNumber).RoomNumber;
	LookAt(&Camera, 0);

	auto angle = Camera.target - Camera.pos;
	auto position = Vector3i(Camera.target - Camera.pos);

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
		ItemCamera.LastAngle = Vector3i(
			ItemCamera.LastAngle.x = angle.x, 
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
	if (!Camera.targetElevation)
		Camera.targetElevation = -ANGLE(10.0f);

	Camera.targetElevation += item->Pose.Orientation.x;
	UpdateCameraElevation();

	// Clamp x rotation.
	if (Camera.actualElevation > ANGLE(85.0f))
		Camera.actualElevation = ANGLE(85.0f);
	else if (Camera.actualElevation < -ANGLE(85.0f))
		Camera.actualElevation = -ANGLE(85.0f);

	float distance = Camera.targetDistance * phd_cos(Camera.actualElevation);

	auto probe = GetCollision(Camera.target.x, Camera.target.y + CLICK(1), Camera.target.z, Camera.TargetRoomNumber);

	if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		Camera.target.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);

	int y = Camera.target.y;
	probe = GetCollision(Camera.target.x, y, Camera.target.z, Camera.TargetRoomNumber);
	if (((y < probe.Position.Ceiling || probe.Position.Floor < y) || probe.Position.Floor <= probe.Position.Ceiling) ||
		(probe.Position.Floor == NO_HEIGHT || probe.Position.Ceiling == NO_HEIGHT))
	{
		TargetSnaps++;
		Camera.target.x = LastTarget.x;
		Camera.target.y = LastTarget.y;
		Camera.target.z = LastTarget.z;
		Camera.TargetRoomNumber = LastTargetRoomNumber;
	}
	else
		TargetSnaps = 0;

	for (int i = 0; i < 5; i++)
		Ideals[i].y = Camera.target.y + Camera.targetDistance * phd_sin(Camera.actualElevation);

	float farthest = INFINITY;
	int farthestnum = 0;
	GameVector temp[2];

	for (int i = 0; i < 5; i++)
	{
		short angle;

		if (i == 0)
			angle = Camera.actualAngle;
		else
			angle = (i - 1) * ANGLE(90.0f);

		Ideals[i].x = Camera.target.x - distance * phd_sin(angle);
		Ideals[i].z = Camera.target.z - distance * phd_cos(angle);
		IdealRoomNumbers[i] = Camera.TargetRoomNumber;


		auto origin = GameVector(Camera.target, Camera.TargetRoomNumber);
		auto target = GameVector(Ideals[i], IdealRoomNumbers[i]);
		bool hasLos = LOSAndReturnTarget(&origin, &target, 200);
		Ideals[i] = target.ToVector3();
		if (hasLos)
		{
			temp[0].x = Ideals[i].x;
			temp[0].y = Ideals[i].y;
			temp[0].z = Ideals[i].z;
			temp[0].RoomNumber = IdealRoomNumbers[i];

			temp[1].x = Camera.pos.x;
			temp[1].y = Camera.pos.y;
			temp[1].z = Camera.pos.z;
			temp[1].RoomNumber = Camera.RoomNumber;

			if (i == 0 || LOSAndReturnTarget(&temp[0], &temp[1], 0))
			{
				if (i == 0)
				{
					farthestnum = 0;
					break;
				}

				float dx = (Camera.pos.x - Ideals[i].x) * (Camera.pos.x - Ideals[i].x);
				dx += (Camera.pos.z - Ideals[i].z) * (Camera.pos.z - Ideals[i].z);
				if (dx < farthest)
				{
					farthest = dx;
					farthestnum = i;
				}
			}
		}
		else if (i == 0)
		{
			temp[0].x = Ideals[i].x;
			temp[0].y = Ideals[i].y;
			temp[0].z = Ideals[i].z;
			temp[0].RoomNumber = IdealRoomNumbers[i];

			temp[1].x = Camera.pos.x;
			temp[1].y = Camera.pos.y;
			temp[1].z = Camera.pos.z;
			temp[1].RoomNumber = Camera.RoomNumber;

			if (i == 0 || LOSAndReturnTarget(&temp[0], &temp[1], 0))
			{
				int dx = (Camera.target.x - Ideals[i].x) * (Camera.target.x - Ideals[i].x);
				int dz = (Camera.target.z - Ideals[i].z) * (Camera.target.z - Ideals[i].z);

				if ((dx + dz) > 0x90000)
				{
					farthestnum = 0;
					break;
				}
			}
		}
	}

	auto ideal = GameVector(Ideals[farthestnum].x , Ideals[farthestnum].y, Ideals[farthestnum].z, IdealRoomNumbers[farthestnum]);
	CameraCollisionBounds(&ideal, CLICK(1.5f), 1);
	MoveCamera(&ideal, Camera.speed);
}

void DoThumbstickCamera()
{
	if (!g_Configuration.EnableThumbstickCameraControl)
		return;

	if (Camera.laraNode == -1 && (Camera.target.x == OldCam.target.x &&
		Camera.target.y == OldCam.target.y &&
		Camera.target.z == OldCam.target.z))
	{
		Camera.targetAngle = ANGLE(THUMBCAM_VERTICAL_CONSTRAINT_ANGLE * AxisMap[InputAxis::CameraHorizontal]);
		Camera.targetElevation = ANGLE(-10.0f + (THUMBCAM_HORIZONTAL_CONSTRAINT_ANGLE * AxisMap[InputAxis::CameraVertical]));
	}
}

void UpdateCameraElevation()
{
	DoThumbstickCamera();

	if (Camera.laraNode != -1)
	{
		auto pos = GetJointPosition(LaraItem, Camera.laraNode, Vector3i::Zero);
		auto pos1 = GetJointPosition(LaraItem, Camera.laraNode, Vector3i(0, -CLICK(1), SECTOR(2)));
		pos = pos1 - pos;
		Camera.actualAngle = Camera.targetAngle + phd_atan(pos.z, pos.x);
	}
	else
		Camera.actualAngle = LaraItem->Pose.Orientation.y + Camera.targetAngle;

	Camera.actualElevation += (Camera.targetElevation - Camera.actualElevation) / 8;
}

void CombatCamera(ItemInfo* item)
{
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

	auto probe = GetCollision(Camera.target.x, Camera.target.y + CLICK(1), Camera.target.z, Camera.TargetRoomNumber);
	if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		Camera.target.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);

	probe = GetCollision(Camera.target.x, Camera.target.y, Camera.target.z, Camera.TargetRoomNumber);
	Camera.TargetRoomNumber = probe.RoomNumber;

	int buffer = CLICK(0.25f);
	if ((probe.Position.Ceiling + buffer) > (probe.Position.Floor - buffer) &&
		probe.Position.Floor != NO_HEIGHT &&
		probe.Position.Ceiling != NO_HEIGHT)
	{
		Camera.target.y = (probe.Position.Ceiling + probe.Position.Floor) >> 1;
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
	probe = GetCollision(Camera.target.x, y, Camera.target.z, Camera.TargetRoomNumber);
	Camera.TargetRoomNumber = probe.RoomNumber;

	if (y < probe.Position.Ceiling ||
		y > probe.Position.Floor ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		TargetSnaps++;
		Camera.target.x = LastTarget.x;
		Camera.target.y = LastTarget.y;
		Camera.target.z = LastTarget.z;
		Camera.TargetRoomNumber = LastTargetRoomNumber;
	}
	else
		TargetSnaps = 0;

	UpdateCameraElevation();

	Camera.targetDistance = BLOCK(1.5f);
	float distance = Camera.targetDistance * phd_cos(Camera.actualElevation);

	for (int i = 0; i < 5; i++)
		Ideals[i].y = Camera.target.y + Camera.targetDistance * phd_sin(Camera.actualElevation);

	float farthest = INFINITY;
	int farthestnum = 0;
	GameVector temp[2];

	for (int i = 0; i < 5; i++)
	{
		short angle;

		if (i == 0)
			angle = Camera.actualAngle;
		else
			angle = (i - 1) * ANGLE(90.0f);

		Ideals[i].x = Camera.target.x - distance * phd_sin(angle);
		Ideals[i].z = Camera.target.z - distance * phd_cos(angle);
		IdealRoomNumbers[i] = Camera.TargetRoomNumber;

		auto origin = GameVector(Camera.target, Camera.TargetRoomNumber);
		auto target = GameVector(Ideals[i], IdealRoomNumbers[i]);
		bool hasLos = LOSAndReturnTarget(&origin, &target, 200);
		Ideals[i] = target.ToVector3();
		if (hasLos)
		{
			temp[0].x = Ideals[i].x;
			temp[0].y = Ideals[i].y;
			temp[0].z = Ideals[i].z;
			temp[0].RoomNumber = IdealRoomNumbers[i];

			temp[1].x = Camera.pos.x;
			temp[1].y = Camera.pos.y;
			temp[1].z = Camera.pos.z;
			temp[1].RoomNumber = Camera.RoomNumber;

			if (i == 0 ||
				LOSAndReturnTarget(&temp[0], &temp[1], 0))
			{
				if (i == 0)
				{
					farthestnum = 0;
					break;
				}

				float dx = (Camera.pos.x - Ideals[i].x) * (Camera.pos.x - Ideals[i].x);
				dx += (Camera.pos.z - Ideals[i].z) * (Camera.pos.z - Ideals[i].z);
				if (dx < farthest)
				{
					farthest = dx;
					farthestnum = i;
				}
			}
		}
		else if (i == 0)
		{
			temp[0].x = Ideals[i].x;
			temp[0].y = Ideals[i].y;
			temp[0].z = Ideals[i].z;
			temp[0].RoomNumber = IdealRoomNumbers[i];

			temp[1].x = Camera.pos.x;
			temp[1].y = Camera.pos.y;
			temp[1].z = Camera.pos.z;
			temp[1].RoomNumber = Camera.RoomNumber;

			if (i == 0 ||
				LOSAndReturnTarget(&temp[0], &temp[1], 0))
			{
				int dx = (Camera.target.x - Ideals[i].x) * (Camera.target.x - Ideals[i].x);
				int dz = (Camera.target.z - Ideals[i].z) * (Camera.target.z - Ideals[i].z);

				if ((dx + dz) > 0x90000)
				{
					farthestnum = 0;
					break;
				}
			}
		}
	}

	auto ideal = GameVector(Ideals[farthestnum].x, Ideals[farthestnum].y, Ideals[farthestnum].z, IdealRoomNumbers[farthestnum]);

	CameraCollisionBounds(&ideal, CLICK(1.5f), 1);

	if (Camera.oldType == CameraType::Fixed)
		Camera.speed = 1;

	MoveCamera(&ideal, Camera.speed);
}

bool CameraCollisionBounds(GameVector* ideal, int push, int yFirst)
{
	int x = ideal->x;
	int y = ideal->y;
	int z = ideal->z;

	CollisionResult probe;

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
	GameVector from, to;

	// Fixed cameras before TR3 had optional "movement" effect. 
	// Later for some reason it was forced to always be 1, and actual speed value
	// from camera trigger was ignored. In TEN, we move speed value out of legacy
	// floordata trigger to camera itself and make use of it again. Still, by default,
	// value is 1 for UseForcedFixedCamera hack.

	int moveSpeed = 1;

	if (UseForcedFixedCamera)
	{
		from.x = ForcedFixedCamera.x;
		from.y = ForcedFixedCamera.y;
		from.z = ForcedFixedCamera.z;
		from.RoomNumber = ForcedFixedCamera.RoomNumber;
	}
	else
	{
		auto* camera = &g_Level.Cameras[Camera.number];

		from = GameVector(camera->Position, camera->RoomNumber);

		// Multiply original speed by 8 to comply with original bitshifted speed from TR1-2
		moveSpeed = camera->Speed * 8 + 1;
	}

	Camera.fixedCamera = true;

	MoveCamera(&from, moveSpeed);

	if (Camera.timer)
	{
		if (!--Camera.timer)
			Camera.timer = -1;
	}
}

void LookCamera(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	short headXRot = lara->ExtraHeadRot.x;
	short headYRot = lara->ExtraHeadRot.y;
	short torsoXRot = lara->ExtraTorsoRot.x;
	short torsoYRot = lara->ExtraTorsoRot.y;

	lara->ExtraTorsoRot.x = 0;
	lara->ExtraTorsoRot.y = 0;
	lara->ExtraHeadRot.x *= 2;
	lara->ExtraHeadRot.y *= 2;

	// Clamp head rotation.
	if (lara->ExtraHeadRot.x > ANGLE(55.0f))
		lara->ExtraHeadRot.x = ANGLE(55.0f);
	else if (lara->ExtraHeadRot.x < -ANGLE(75.0f))
		lara->ExtraHeadRot.x = -ANGLE(75.0f);
	if (lara->ExtraHeadRot.y < -ANGLE(80.0f))
		lara->ExtraHeadRot.y = -ANGLE(80.0f);
	else if (lara->ExtraHeadRot.y > ANGLE(80.0f))
		lara->ExtraHeadRot.y = ANGLE(80.0f);

	if (abs(lara->ExtraHeadRot.x - OldCam.pos.Orientation.x) >= 16)
		OldCam.pos.Orientation.x = (lara->ExtraHeadRot.x + OldCam.pos.Orientation.x) / 2;
	else
		OldCam.pos.Orientation.x = lara->ExtraHeadRot.x;
	if (abs(lara->ExtraHeadRot.y - OldCam.pos.Orientation.y) >= 16)
		OldCam.pos.Orientation.y = (lara->ExtraHeadRot.y + OldCam.pos.Orientation.y) / 2;
	else
		OldCam.pos.Orientation.y = lara->ExtraHeadRot.y;

	auto pos = GetJointPosition(item, LM_HEAD, Vector3i(0, CLICK(0.25f) / 4, CLICK(0.25f)));

	auto probe = GetCollision(pos.x, pos.y, pos.z, item->RoomNumber);
	if (probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		pos.y > probe.Position.Floor ||
		pos.y < probe.Position.Ceiling)
	{
		pos = GetJointPosition(item, LM_HEAD, Vector3i(0, CLICK(0.25f) / 4, 0));

		probe = GetCollision(pos.x, pos.y + CLICK(1), pos.z, item->RoomNumber);
		if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		{
			pos.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);
			probe = GetCollision(pos.x, pos.y, pos.z, probe.RoomNumber);
		}
		else
			probe = GetCollision(pos.x, pos.y, pos.z, probe.RoomNumber);

		if (probe.Position.Floor == NO_HEIGHT ||
			probe.Position.Ceiling == NO_HEIGHT ||
			probe.Position.Ceiling >= probe.Position.Floor ||
			pos.y > probe.Position.Floor ||
			pos.y < probe.Position.Ceiling)
		{
			pos = GetJointPosition(item, LM_HEAD, Vector3i(0, CLICK(0.25f) / 4, -CLICK(0.25f)));
		}
	}

	auto pos2 = GetJointPosition(item, LM_HEAD, Vector3i(0, 0, -SECTOR(1)));
	auto pos3 = GetJointPosition(item, LM_HEAD, Vector3i(0, 0, CLICK(8)));

	int dx = (pos2.x - pos.x) >> 3;
	int dy = (pos2.y - pos.y) >> 3;
	int dz = (pos2.z - pos.z) >> 3;
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;

	int roomNumber;
	probe.RoomNumber = item->RoomNumber;
	int i = 0;
	for (i = 0; i < 8; i++)
	{
		roomNumber = probe.RoomNumber;
		probe = GetCollision(x, y + CLICK(1), z, probe.RoomNumber);
		if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		{
			y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);
			break;
		}
		else
			probe = GetCollision(x, y, z, probe.RoomNumber);

		if (probe.Position.Floor == NO_HEIGHT ||
			probe.Position.Ceiling == NO_HEIGHT ||
			probe.Position.Ceiling >= probe.Position.Floor ||
			y > probe.Position.Floor ||
			y < probe.Position.Ceiling)
		{
			break;
		}

		x += dx;
		y += dy;
		z += dz;
	}

	if (i)
	{
		x -= dx;
		y -= dy;
		z -= dz;
	}

	auto ideal = GameVector(x, y, z, roomNumber);

	if (OldCam.pos.Orientation.x == lara->ExtraHeadRot.x &&
		OldCam.pos.Orientation.y == lara->ExtraHeadRot.y &&
		OldCam.pos.Position.x == item->Pose.Position.x &&
		OldCam.pos.Position.y == item->Pose.Position.y &&
		OldCam.pos.Position.z == item->Pose.Position.z &&
		OldCam.ActiveState == item->Animation.ActiveState &&
		OldCam.TargetState == item->Animation.TargetState &&
		Camera.oldType == CameraType::Look)
	{
		ideal.x = LookCamPosition.x;
		ideal.y = LookCamPosition.y;
		ideal.z = LookCamPosition.z;
		ideal.RoomNumber = LookCamPosition.RoomNumber;
		pos3.x = LookCamTarget.x;
		pos3.y = LookCamTarget.y;
		pos3.z = LookCamTarget.z;
	}
	else
	{
		OldCam.pos.Orientation.x = lara->ExtraHeadRot.x;
		OldCam.pos.Orientation.y = lara->ExtraHeadRot.y;
		OldCam.pos.Position.x = item->Pose.Position.x;
		OldCam.pos.Position.y = item->Pose.Position.y;
		OldCam.pos.Position.z = item->Pose.Position.z;
		OldCam.ActiveState = item->Animation.ActiveState;
		OldCam.TargetState = item->Animation.TargetState;
		LookCamPosition.x = ideal.x;
		LookCamPosition.y = ideal.y;
		LookCamPosition.z = ideal.z;
		LookCamPosition.RoomNumber = ideal.RoomNumber;
		LookCamTarget.x = pos3.x;
		LookCamTarget.y = pos3.y;
		LookCamTarget.z = pos3.z;
	}

	CameraCollisionBounds(&ideal, CLICK(1) - CLICK(0.25f) / 2, 1);

	if (Camera.oldType == CameraType::Fixed)
	{
		Camera.pos.x = ideal.x;
		Camera.pos.y = ideal.y;
		Camera.pos.z = ideal.z;
		Camera.target.x = pos3.x;
		Camera.target.y = pos3.y;
		Camera.target.z = pos3.z;
		Camera.TargetRoomNumber = item->RoomNumber;
	}
	else
	{
		Camera.pos.x += (ideal.x - Camera.pos.x) / 4;
		Camera.pos.y += (ideal.y - Camera.pos.y) / 4;
		Camera.pos.z += (ideal.z - Camera.pos.z) / 4;
		Camera.target.x += (pos3.x - Camera.target.x) / 4;
		Camera.target.y += (pos3.y - Camera.target.y) / 4;
		Camera.target.z += (pos3.z - Camera.target.z) / 4;
		Camera.TargetRoomNumber = item->RoomNumber;
	}

	if (Camera.bounce && Camera.type == Camera.oldType)
	{
		if (Camera.bounce <= 0)
		{
			Camera.target.x += GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1);
			Camera.target.y += GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1);
			Camera.target.z += GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1);
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

	y = Camera.pos.y;
	probe = GetCollision(Camera.pos.x, y, Camera.pos.z, Camera.RoomNumber);

	int buffer = CLICK(1) - 1;
	if ((y - buffer) < probe.Position.Ceiling &&
		(y + buffer) > probe.Position.Floor &&
		probe.Position.Ceiling < probe.Position.Floor &&
		probe.Position.Ceiling != NO_HEIGHT && probe.Position.Floor != NO_HEIGHT)
	{
		Camera.pos.y = (probe.Position.Floor + probe.Position.Ceiling) / 2;
	}
	else if ((y + buffer) > probe.Position.Floor &&
		probe.Position.Ceiling < probe.Position.Floor &&
		probe.Position.Ceiling != NO_HEIGHT &&
		probe.Position.Floor != NO_HEIGHT)
	{
		Camera.pos.y = probe.Position.Floor - buffer;
	}
	else if ((y - buffer) < probe.Position.Ceiling &&
		probe.Position.Ceiling < probe.Position.Floor &&
		probe.Position.Ceiling != NO_HEIGHT &&
		probe.Position.Floor != NO_HEIGHT)
	{
		Camera.pos.y = probe.Position.Ceiling + buffer;
	}

	y = Camera.pos.y;
	probe = GetCollision(Camera.pos.x, y, Camera.pos.z, Camera.RoomNumber);
	if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		Camera.pos.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);
	else if (y < probe.Position.Ceiling ||
		y > probe.Position.Floor ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		auto origin = GameVector(Camera.target, Camera.TargetRoomNumber);
		auto target = GameVector(Camera.pos, Camera.RoomNumber);
		LOSAndReturnTarget(&origin, &target, 0);
		Camera.pos = target.ToVector3();
		Camera.RoomNumber = target.RoomNumber;
	}

	y = Camera.pos.y;
	probe = GetCollision(Camera.pos.x, y, Camera.pos.z, Camera.RoomNumber);
	if (y < probe.Position.Ceiling ||
		y > probe.Position.Floor ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
	{
		Camera.pos.x = pos.x;
		Camera.pos.y = pos.y;
		Camera.pos.z = pos.z;
		Camera.RoomNumber = item->RoomNumber;
	}

	ItemsCollideCamera();

	GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &Camera.RoomNumber);
	LookAt(&Camera, 0);
	UpdateMikePos(item);
	Camera.oldType = Camera.type;

	lara->ExtraHeadRot.x = headXRot;
	lara->ExtraHeadRot.y = headYRot;
	lara->ExtraTorsoRot.x = torsoXRot;
	lara->ExtraTorsoRot.y = torsoYRot;
}

void BounceCamera(ItemInfo* item, short bounce, short maxDistance)
{
	float distance = sqrt(
		pow(item->Pose.Position.x - Camera.pos.x, 2) +
		pow(item->Pose.Position.y - Camera.pos.y, 2) +
		pow(item->Pose.Position.z - Camera.pos.z, 2));

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
	Camera.RoomNumber = probe.RoomNumber;

	int l = SECTOR(20.25f) * phd_cos(headXRot);

	int tx = x + l * phd_sin(item->Pose.Orientation.y + headYRot);
	int ty = y - SECTOR(20.25f) * phd_sin(headXRot);
	int tz = z + l * phd_cos(item->Pose.Orientation.y + headYRot);

	if (Camera.oldType == CameraType::Fixed)
	{
		Camera.target.x = tx;
		Camera.target.y = ty;
		Camera.target.z = tz;
		Camera.TargetRoomNumber = item->RoomNumber;
	}
	else
	{
		Camera.target.x += (tx - Camera.target.x) / 4;
		Camera.target.y += (ty - Camera.target.y) / 4;
		Camera.target.z += (tz - Camera.target.z) / 4;
		Camera.TargetRoomNumber = item->RoomNumber;
	}

	if (Camera.bounce &&
		Camera.type == Camera.oldType)
	{
		if (Camera.bounce <= 0)
		{
			Camera.target.x += (CLICK(0.25f) / 4) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1));
			Camera.target.y += (CLICK(0.25f) / 4) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1));
			Camera.target.z += (CLICK(0.25f) / 4) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1));
			Camera.bounce += 5;
			RumbleFromBounce();
		}
		else
		{
			Camera.bounce = 0;
			Camera.target.y += Camera.bounce;
		}
	}

	Camera.TargetRoomNumber = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.TargetRoomNumber).RoomNumber;
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

	auto origin = Vector3i(Camera.pos.x, Camera.pos.y, Camera.pos.z);
	auto target = Vector3i(Camera.target.x, Camera.target.y, Camera.target.z);

	auto origin2 = GameVector(Camera.pos, Camera.RoomNumber);
	auto target2 = GameVector(Camera.target, Camera.TargetRoomNumber);
	GetTargetOnLOS(&origin2, &target2, false, false);
	Camera.target = target2.ToVector3();
	Camera.TargetRoomNumber = origin2.RoomNumber;

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
	auto probe = GetCollision(Camera.target.x, y, Camera.target.z, Camera.TargetRoomNumber);
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
	CamOldPos = Camera.pos;

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

	ItemInfo* item;
	bool fixedCamera = false;
	if (Camera.item != NULL &&
		(Camera.type == CameraType::Fixed || Camera.type == CameraType::Heavy))
	{
		item = Camera.item;
		fixedCamera = true;
	}
	else
	{
		item = LaraItem;
		fixedCamera = false;
	}

	auto bounds = GameBoundingBox(item);

	int x;
	int y = item->Pose.Position.y + bounds.Y2 + (3 * (bounds.Y1 - bounds.Y2) / 4);
	int z;

	if (Camera.item)
	{
		if (!fixedCamera)
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
			LastTargetRoomNumber = Camera.TargetRoomNumber;
		}

		Camera.TargetRoomNumber = item->RoomNumber;

		if (Camera.fixedCamera || BinocularOn)
		{
			Camera.target.y = y;
			Camera.speed = 1;
		}
		else
		{
			Camera.target.y += (y - Camera.target.y) / 4;
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
		LastTargetRoomNumber = Camera.TargetRoomNumber;

		Camera.TargetRoomNumber = item->RoomNumber;
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

		if (fixedCamera == Camera.fixedCamera)
		{
			Camera.fixedCamera = false;
			if (Camera.speed != 1 &&
				Camera.oldType != CameraType::Look &&
				!BinocularOn)
			{
				if (TargetSnaps <= 8)
				{
					x = LastTarget.x + ((x - LastTarget.x) * 0.25f);
					Camera.target.x = x;
					y = LastTarget.y + ((y - LastTarget.y) * 0.25f);
					Camera.target.y = y;
					z = LastTarget.z + ((z - LastTarget.z) * 0.25f);
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

		Camera.TargetRoomNumber = GetCollision(x, y, z, Camera.TargetRoomNumber).RoomNumber;

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

	Camera.fixedCamera = fixedCamera;
	Camera.last = Camera.number;

	if (Camera.type != CameraType::Heavy ||
		Camera.timer == -1)
	{
		Camera.type = CameraType::Chase;
		Camera.speed = 10;
		Camera.number = -1;
		Camera.lastItem = Camera.item;
		Camera.item = NULL;
		Camera.targetElevation = 0;
		Camera.targetAngle = 0;
		Camera.targetDistance = SECTOR(1.5f);
		Camera.flags = 0;
		Camera.laraNode = -1;
	}
}

void LookLeftRight(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	Camera.type = CameraType::Look;
	if (TrInput & IN_LEFT)
	{
		ClearAction(In::Left);

		if (lara->ExtraHeadRot.y > -ANGLE(44.0f))
		{
			if (BinocularRange)
				lara->ExtraHeadRot.y += ANGLE(2.0f) * (BinocularRange - ANGLE(10.0f)) / ANGLE(8.5f);
			else
				lara->ExtraHeadRot.y -= ANGLE(2.0f);
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		ClearAction(In::Right);

		if (lara->ExtraHeadRot.y < ANGLE(44.0f))
		{
			if (BinocularRange)
				lara->ExtraHeadRot.y += ANGLE(2.0f) * (ANGLE(10.0f) - BinocularRange) / ANGLE(8.5f);
			else
				lara->ExtraHeadRot.y += ANGLE(2.0f);
		}
	}

	if (lara->Control.HandStatus != HandStatus::Busy &&
		lara->Vehicle == NO_ITEM &&
		!lara->LeftArm.Locked &&
		!lara->RightArm.Locked)
	{
		lara->ExtraTorsoRot.y = lara->ExtraHeadRot.y;
	}
}

void LookUpDown(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	Camera.type = CameraType::Look;
	if (TrInput & IN_FORWARD)
	{
		ClearAction(In::Forward);

		if (lara->ExtraHeadRot.x > -ANGLE(35.0f))
		{
			if (BinocularRange)
				lara->ExtraHeadRot.x += ANGLE(2.0f) * (BinocularRange - ANGLE(10.0f)) / ANGLE(17.0f);
			else
				lara->ExtraHeadRot.x -= ANGLE(2.0f);
		}
	}
	else if (TrInput & IN_BACK)
	{
		ClearAction(In::Back);

		if (lara->ExtraHeadRot.x < ANGLE(30.0f))
		{
			if (BinocularRange)
				lara->ExtraHeadRot.x += ANGLE(2.0f) * (ANGLE(10.0f) - BinocularRange) / ANGLE(17.0f);
			else
				lara->ExtraHeadRot.x += ANGLE(2.0f);
		}
	}

	if (lara->Control.HandStatus != HandStatus::Busy &&
		lara->Vehicle == NO_ITEM &&
		!lara->LeftArm.Locked &&
		!lara->RightArm.Locked)
	{
		lara->ExtraTorsoRot.x = lara->ExtraHeadRot.x;
	}
}

void ResetLook(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (Camera.type != CameraType::Look)
	{
		if (abs(lara->ExtraHeadRot.x) > ANGLE(0.1f))
			lara->ExtraHeadRot.x += lara->ExtraHeadRot.x / -8;
		else
			lara->ExtraHeadRot.x = 0;

		if (abs(lara->ExtraHeadRot.y) > ANGLE(0.1f))
			lara->ExtraHeadRot.y += lara->ExtraHeadRot.y / -8;
		else
			lara->ExtraHeadRot.y = 0;

		if (abs(lara->ExtraHeadRot.z) > ANGLE(0.1f))
			lara->ExtraHeadRot.z += lara->ExtraHeadRot.z / -8;
		else
			lara->ExtraHeadRot.z = 0;

		if (lara->Control.HandStatus != HandStatus::Busy &&
			!lara->LeftArm.Locked &&
			!lara->RightArm.Locked &&
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
	auto sphere = BoundingSphere(Camera.pos, radius);
	return sphere.Intersects(bounds.ToBoundingOrientedBox(pose));
}

float GetParticleDistanceFade(Vector3i position)
{
	float distance = Vector3::Distance(Camera.pos, position.ToVector3());

	if (distance <= PARTICLE_FADE_THRESHOLD)
		return 1.0f;

	return std::clamp(1.0f - ((distance - PARTICLE_FADE_THRESHOLD) / COLL_CHECK_THRESHOLD), 0.0f, 1.0f);
}

void ItemPushCamera(GameBoundingBox* bounds, Pose* pos, short radius)
{
	int dx = Camera.pos.x - pos->Position.x;
	int dz = Camera.pos.z - pos->Position.z;
	auto sin = phd_sin(pos->Orientation.y);
	auto cos = phd_cos(pos->Orientation.y);
	auto x = dx * cos - dz * sin;
	auto z = dx * sin + dz * cos;

	int xmin = bounds->X1 - radius;
	int xmax = bounds->X2 + radius;
	int zmin = bounds->Z1 - radius;
	int zmax = bounds->Z2 + radius;

	if (x <= xmin || x >= xmax || z <= zmin || z >= zmax)
		return;

	auto left = x - xmin;
	auto right = xmax - x;
	auto top = zmax - z;
	auto bottom = z - zmin;

	if (left <= right && left <= top && left <= bottom) // Left is closest
		x -= left;
	else if (right <= left && right <= top && right <= bottom) // Right is closest
		x += right;
	else if (top <= left && top <= right && top <= bottom) // Top is closest
		z += top;
	else
		z -= bottom; // Bottom

	Camera.pos.x = pos->Position.x + (cos * x + sin * z);
	Camera.pos.z = pos->Position.z + (cos * z - sin * x);

	auto coll = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.RoomNumber);
	if (coll.Position.Floor == NO_HEIGHT || Camera.pos.y > coll.Position.Floor || Camera.pos.y < coll.Position.Ceiling)
	{
		Camera.pos = CamOldPos;
		Camera.RoomNumber = coll.RoomNumber;
	}
}

static bool CheckItemCollideCamera(ItemInfo* item)
{
	auto dx = Camera.pos.x - item->Pose.Position.x;
	auto dy = Camera.pos.y - item->Pose.Position.y;
	auto dz = Camera.pos.z - item->Pose.Position.z;

	bool closeEnough = dx > -COLL_CHECK_THRESHOLD && dx < COLL_CHECK_THRESHOLD &&
						dz > -COLL_CHECK_THRESHOLD && dz < COLL_CHECK_THRESHOLD &&
						dy > -COLL_CHECK_THRESHOLD && dy < COLL_CHECK_THRESHOLD;

	if (!closeEnough || !item->Collidable || !Objects[item->ObjectNumber].usingDrawAnimatingItem)
		return false;

	// TODO: Find a better way to define objects which are collidable with camera.
	auto obj = &Objects[item->ObjectNumber];
	if (obj->intelligent || obj->isPickup || obj->isPuzzleHole || obj->collision == nullptr)
		return false;

	// Check extents, if any 2 bounds are smaller than threshold, discard.
	auto extents = GameBoundingBox(item).ToBoundingOrientedBox(item->Pose).Extents;
	if ((abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.y) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.y) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD))
		return false;

	return true;
}

std::vector<short> FillCollideableItemList()
{
	std::vector<short> itemList;
	auto& roomList = g_Level.Rooms[Camera.RoomNumber].neighbors;

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

static bool CheckStaticCollideCamera(MESH_INFO* mesh)
{
	auto dx = Camera.pos.x - mesh->pos.Position.x;
	auto dy = Camera.pos.y - mesh->pos.Position.y;
	auto dz = Camera.pos.z - mesh->pos.Position.z;

	bool closeEnough = dx > -COLL_CHECK_THRESHOLD && dx < COLL_CHECK_THRESHOLD &&
						dz > -COLL_CHECK_THRESHOLD && dz < COLL_CHECK_THRESHOLD &&
						dy > -COLL_CHECK_THRESHOLD && dy < COLL_CHECK_THRESHOLD;

	if (!closeEnough)
		return false;

	if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
		return false;

	const auto& bounds = GetBoundsAccurate(*mesh, false);
	auto extents = Vector3(
		abs(bounds.X1 - bounds.X2),
		abs(bounds.Y1 - bounds.Y2),
		abs(bounds.Z1 - bounds.Z2)
	);

	// Check extents, if any 2 bounds are smaller than threshold, discard.
	if ((abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.y) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.y) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD))
		return false;

	return true;
}

std::vector<MESH_INFO*> FillCollideableStaticsList()
{
	std::vector<MESH_INFO*> staticList;
	auto& roomList = g_Level.Rooms[Camera.RoomNumber].neighbors;

	for (auto i : roomList)
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
	auto rad = 128;
	auto itemList = FillCollideableItemList();

	// Collide with the items list

	for (int i = 0; i < itemList.size(); i++)
	{
		auto item = &g_Level.Items[itemList[i]];

		if (!item)
			return;

		auto dx = abs(LaraItem->Pose.Position.x - item->Pose.Position.x);
		auto dy = abs(LaraItem->Pose.Position.y - item->Pose.Position.y);
		auto dz = abs(LaraItem->Pose.Position.z - item->Pose.Position.z);

		// If camera is stuck behind some item, and Lara runs off somewhere
		if (dx > COLL_CANCEL_THRESHOLD || dz > COLL_CANCEL_THRESHOLD || dy > COLL_CANCEL_THRESHOLD)
			continue;

		auto bounds = GameBoundingBox(item);
		if (TestBoundsCollideCamera(bounds, item->Pose, CAMERA_RADIUS))
			ItemPushCamera(&bounds, &item->Pose, rad);

		TEN::Renderer::g_Renderer.AddDebugBox(bounds.ToBoundingOrientedBox(item->Pose),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f), RENDERER_DEBUG_PAGE::LARA_STATS);
	}

	itemList.clear(); // Done

	// Collide with static meshes

	auto staticList = FillCollideableStaticsList();

	for (int i = 0; i < staticList.size(); i++)
	{
		auto* mesh = staticList[i];
		if (!mesh)
			return;

		auto dx = abs(LaraItem->Pose.Position.x - mesh->pos.Position.x);
		auto dy = abs(LaraItem->Pose.Position.y - mesh->pos.Position.y);
		auto dz = abs(LaraItem->Pose.Position.z - mesh->pos.Position.z);

		if (dx > COLL_CANCEL_THRESHOLD || dz > COLL_CANCEL_THRESHOLD || dy > COLL_CANCEL_THRESHOLD)
			continue;

		auto bounds = GetBoundsAccurate(*mesh, false);
		if (TestBoundsCollideCamera(bounds, mesh->pos, CAMERA_RADIUS))
			ItemPushCamera(&bounds, &mesh->pos, rad);

		TEN::Renderer::g_Renderer.AddDebugBox(bounds.ToBoundingOrientedBox(mesh->pos),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f), RENDERER_DEBUG_PAGE::LARA_STATS);
	}

	staticList.clear(); // Done
}

void UpdateMikePos(ItemInfo* item)
{
	if (Camera.mikeAtLara)
	{
		Camera.mikePos = item->Pose.Position;
		Camera.actualAngle = item->Pose.Orientation.y;

		if (item->IsLara())
		{
			auto* lara = GetLaraInfo(item);
			Camera.actualAngle += lara->ExtraHeadRot.y + lara->ExtraTorsoRot.y;
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

	int oldScreenFadeCurrent = ScreenFadeCurrent;

	if (ScreenFadeEnd != 0 && ScreenFadeEnd >= ScreenFadeCurrent)
	{
		ScreenFadeCurrent += ScreenFadeSpeed;
		if (ScreenFadeCurrent > ScreenFadeEnd)
		{
			ScreenFadeCurrent = ScreenFadeEnd;
			if (oldScreenFadeCurrent >= ScreenFadeCurrent)
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
	bool breakOptics = true;

	if (!LaserSight && BinocularOn) // Imitate pushing look key in binocular mode
	{
		TrInput |= IN_LOOK;
		DbInput = 0;
	}

	auto lara = GetLaraInfo(item);

	// We are standing, can use optics.
	if (item->Animation.ActiveState == LS_IDLE || item->Animation.AnimNumber == LA_STAND_IDLE)
		breakOptics = false;

	// We are crouching, can use optics.
	if ((lara->Control.IsLow || TrInput & IN_CROUCH) &&
		(item->Animation.TargetState == LS_CROUCH_IDLE || item->Animation.AnimNumber == LA_CROUCH_IDLE))
		breakOptics = false;

	// If lasersight, and no look is pressed, exit optics.
	if (LaserSight && !(TrInput & IN_LOOK))
		breakOptics = true;

	// If lasersight, and weapon is holstered, exit optics.
	if (LaserSight && (TrInput & IN_DRAW))
		breakOptics = true;

	if (!LaserSight && !breakOptics && (TrInput == IN_LOOK)) // Engage lasersight, if available.
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
			Lara.Inventory.IsBusy = true;
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

	Lara.Inventory.IsBusy = false;
	ResetLaraFlex(LaraItem);

	TrInput &= ~IN_LOOK;
}
