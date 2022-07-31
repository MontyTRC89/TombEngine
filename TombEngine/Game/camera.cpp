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
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using TEN::Renderer::g_Renderer;

using namespace TEN::Input;
using namespace TEN::Entities::Generic;
using namespace TEN::Effects::Environment;

constexpr auto COLL_CHECK_THRESHOLD   = SECTOR(4);
constexpr auto COLL_CANCEL_THRESHOLD  = SECTOR(2);
constexpr auto COLL_DISCARD_THRESHOLD = CLICK(0.5f);
constexpr auto CAMERA_RADIUS          = CLICK(1);

constexpr auto THUMBCAM_VERTICAL_CONSTRAINT_ANGLE   = 120.0f;
constexpr auto THUMBCAM_HORIZONTAL_CONSTRAINT_ANGLE = 80.0f;

struct OLD_CAMERA
{
	int ActiveState;
	int TargetState;
	int targetDistance;
	float actualElevation;
	float targetElevation;
	float actualAngle;
	PHD_3DPOS pos;
	PHD_3DPOS pos2;
	Vector3Int target;
};

GameVector LastTarget;

GameVector LastIdeal;
GameVector Ideals[5];
OLD_CAMERA OldCam;
int CameraSnaps = 0;
int TargetSnaps = 0;
GameVector LookCamPosition;
GameVector LookCamTarget;
Vector3Int CamOldPos;
CAMERA_INFO Camera;
GameVector ForcedFixedCamera;
int UseForcedFixedCamera;

int BinocularRange;
bool BinocularOn;
CameraType BinocularOldCamera;
bool LaserSight;

int LSHKTimer = 0;
int LSHKShotsFired = 0;
int WeaponDelay = 0;

float PhdPerspective;
float CurrentFOV;

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

void LookAt(CAMERA_INFO* cam, float roll)
{
	Vector3 position = Vector3(cam->pos.x, cam->pos.y, cam->pos.z);
	Vector3 target = Vector3(cam->target.x, cam->target.y, cam->target.z);
	Vector3 up = Vector3(0.0f, -1.0f, 0.0f);
	float fov = CurrentFOV / 1.333333f;
	float r = roll;

	float gameFarView  = g_GameFlow->GetGameFarView() * float(SECTOR(1));
	float levelFarView = g_GameFlow->GetLevel(CurrentLevel)->GetFarView() * float(SECTOR(1));

	g_Renderer.UpdateCameraMatrices(cam, r, fov, std::min(gameFarView, levelFarView));
}

void AlterFOV(float value)
{
	CurrentFOV = value;
	PhdPerspective = g_Configuration.Width / 2 * cos(CurrentFOV / 2) / sin(CurrentFOV / 2);
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
	LastTarget.roomNumber = LaraItem->RoomNumber;

	Camera.target.x = LastTarget.x;
	Camera.target.y = Camera.shift;
	Camera.target.z = LastTarget.z;
	Camera.target.roomNumber = LaraItem->RoomNumber;

	Camera.pos.x = LastTarget.x;
	Camera.pos.y = Camera.shift;
	Camera.pos.z = LastTarget.z - 100;
	Camera.pos.roomNumber = LaraItem->RoomNumber;

	Camera.targetDistance = SECTOR(1.5f);
	Camera.item = NULL;
	Camera.numberFrames = 1;
	Camera.type = CameraType::Chase;
	Camera.speed = 1;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.bounce = 0;
	Camera.number = -1;
	Camera.fixedCamera = false;

	AlterFOV(Angle::DegToRad(80.0f));

	UseForcedFixedCamera = 0;
	CalculateCamera();
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
		LastIdeal.roomNumber = ideal->roomNumber;
	}
	else
	{
		ideal->x = LastIdeal.x;
		ideal->y = LastIdeal.y;
		ideal->z = LastIdeal.z;
		ideal->roomNumber = LastIdeal.roomNumber;
	}

	Camera.pos.x += (ideal->x - Camera.pos.x) / speed;
	Camera.pos.y += (ideal->y - Camera.pos.y) / speed;
	Camera.pos.z += (ideal->z - Camera.pos.z) / speed;
	Camera.pos.roomNumber = ideal->roomNumber;

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
	if (TestEnvironment(ENV_FLAG_SWAMP, Camera.pos.roomNumber))
		y = g_Level.Rooms[Camera.pos.roomNumber].y - CLICK(1);

	auto probe = GetCollision(Camera.pos.x, y, Camera.pos.z, Camera.pos.roomNumber);
	if (y < probe.Position.Ceiling ||
		y > probe.Position.Floor)
	{
		LOSAndReturnTarget(&Camera.target, &Camera.pos, 0);

		if (abs(Camera.pos.x - ideal->x) < SECTOR(0.5f) &&
			abs(Camera.pos.y - ideal->y) < SECTOR(0.5f) &&
			abs(Camera.pos.z - ideal->z) < SECTOR(0.5f))
		{
			to.x = Camera.pos.x;
			to.y = Camera.pos.y;
			to.z = Camera.pos.z;
			to.roomNumber = Camera.pos.roomNumber;

			from.x = ideal->x;
			from.y = ideal->y;
			from.z = ideal->z;
			from.roomNumber = ideal->roomNumber;

			if (!LOSAndReturnTarget(&from, &to, 0) &&
				++CameraSnaps >= 8)
			{
				Camera.pos.x = ideal->x;
				Camera.pos.y = ideal->y;
				Camera.pos.z = ideal->z;
				Camera.pos.roomNumber = ideal->roomNumber;
				CameraSnaps = 0;
			}
		}
	}

	probe = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber);

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
		Camera.pos.roomNumber = ideal->roomNumber;
	}

	ItemsCollideCamera();

	Camera.pos.roomNumber = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber).RoomNumber;
	LookAt(&Camera, 0);

	if (Camera.mikeAtLara)
	{
		Camera.mikePos.x = LaraItem->Pose.Position.x;
		Camera.mikePos.y = LaraItem->Pose.Position.y;
		Camera.mikePos.z = LaraItem->Pose.Position.z;
		Camera.oldType = Camera.type;
	}
	else
	{
		float angle = atan2(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.mikePos.x = Camera.pos.x + PhdPerspective * sin(angle);
		Camera.mikePos.y = Camera.pos.y;
		Camera.mikePos.z = Camera.pos.z + PhdPerspective * cos(angle);
		Camera.oldType = Camera.type;
	}
}

void ChaseCamera(ItemInfo* item)
{
	if (!Camera.targetElevation)
		Camera.targetElevation = Angle::DegToRad(-10.0f);

	Camera.targetElevation += item->Pose.Orientation.x;
	UpdateCameraElevation();

	// Clamp x rotation.
	if (Camera.actualElevation > Angle::DegToRad(85.0f))
		Camera.actualElevation = Angle::DegToRad(85.0f);
	else if (Camera.actualElevation < Angle::DegToRad(-85.0f))
		Camera.actualElevation = Angle::DegToRad(-85.0f);

	int distance = Camera.targetDistance * cos(Camera.actualElevation);

	auto probe = GetCollision(Camera.target.x, Camera.target.y + CLICK(1), Camera.target.z, Camera.target.roomNumber);

	if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		Camera.target.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);

	int y = Camera.target.y;
	probe = GetCollision(Camera.target.x, y, Camera.target.z, Camera.target.roomNumber);
	if (((y < probe.Position.Ceiling || probe.Position.Floor < y) || probe.Position.Floor <= probe.Position.Ceiling) ||
		(probe.Position.Floor == NO_HEIGHT || probe.Position.Ceiling == NO_HEIGHT))
	{
		TargetSnaps++;
		Camera.target.x = LastTarget.x;
		Camera.target.y = LastTarget.y;
		Camera.target.z = LastTarget.z;
		Camera.target.roomNumber = LastTarget.roomNumber;
	}
	else
		TargetSnaps = 0;

	for (int i = 0; i < 5; i++)
		Ideals[i].y = Camera.target.y + Camera.targetDistance * sin(Camera.actualElevation);

	int farthest = INT_MAX;
	int farthestnum = 0;
	GameVector temp[2];

	for (int i = 0; i < 5; i++)
	{
		float angle;

		if (i == 0)
			angle = Camera.actualAngle;
		else
			angle = (i - 1) * Angle::DegToRad(90.0f);

		Ideals[i].x = Camera.target.x - distance * sin(angle);
		Ideals[i].z = Camera.target.z - distance * cos(angle);
		Ideals[i].roomNumber = Camera.target.roomNumber;

		if (LOSAndReturnTarget(&Camera.target, &Ideals[i], 200))
		{
			temp[0].x = Ideals[i].x;
			temp[0].y = Ideals[i].y;
			temp[0].z = Ideals[i].z;
			temp[0].roomNumber = Ideals[i].roomNumber;

			temp[1].x = Camera.pos.x;
			temp[1].y = Camera.pos.y;
			temp[1].z = Camera.pos.z;
			temp[1].roomNumber = Camera.pos.roomNumber;

			if (i == 0 || LOSAndReturnTarget(&temp[0], &temp[1], 0))
			{
				if (i == 0)
				{
					farthestnum = 0;
					break;
				}

				int dx = (Camera.pos.x - Ideals[i].x) * (Camera.pos.x - Ideals[i].x);
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
			temp[0].roomNumber = Ideals[i].roomNumber;

			temp[1].x = Camera.pos.x;
			temp[1].y = Camera.pos.y;
			temp[1].z = Camera.pos.z;
			temp[1].roomNumber = Camera.pos.roomNumber;

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

	GameVector ideal = { Ideals[farthestnum].x , Ideals[farthestnum].y, Ideals[farthestnum].z };
	ideal.roomNumber = Ideals[farthestnum].roomNumber;

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
		Camera.targetAngle = Angle::DegToRad(THUMBCAM_VERTICAL_CONSTRAINT_ANGLE * AxisMap[InputAxis::CameraHorizontal]);
		Camera.targetElevation = Angle::DegToRad(-10.0f + (THUMBCAM_HORIZONTAL_CONSTRAINT_ANGLE * AxisMap[InputAxis::CameraVertical]));
	}
}

void UpdateCameraElevation()
{
	DoThumbstickCamera();

	if (Camera.laraNode != -1)
	{
		auto pos = Vector3Int();
		GetLaraJointPosition(&pos, Camera.laraNode);

		auto pos1 = Vector3Int(0, -CLICK(1), SECTOR(2));
		GetLaraJointPosition(&pos1, Camera.laraNode);

		pos.x = pos1.x - pos.x;
		pos.z = pos1.z - pos.z;
		Camera.actualAngle = Camera.targetAngle + atan2(pos.z, pos.x);
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
		Camera.targetElevation = lara->ExtraHeadRot.x + lara->ExtraTorsoRot.x + item->Pose.Orientation.x - Angle::DegToRad(15.0f);
	}

	auto probe = GetCollision(Camera.target.x, Camera.target.y + CLICK(1), Camera.target.z, Camera.target.roomNumber);
	if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		Camera.target.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);

	probe = GetCollision(Camera.target.x, Camera.target.y, Camera.target.z, Camera.target.roomNumber);
	Camera.target.roomNumber = probe.RoomNumber;

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
	probe = GetCollision(Camera.target.x, y, Camera.target.z, Camera.target.roomNumber);
	Camera.target.roomNumber = probe.RoomNumber;

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
		Camera.target.roomNumber = LastTarget.roomNumber;
	}
	else
		TargetSnaps = 0;

	UpdateCameraElevation();

	Camera.targetDistance = SECTOR(1.5f);
	int distance = Camera.targetDistance * cos(Camera.actualElevation);

	for (int i = 0; i < 5; i++)
		Ideals[i].y = Camera.target.y + Camera.targetDistance * sin(Camera.actualElevation);

	int farthest = INT_MAX;
	int farthestnum = 0;
	GameVector temp[2];

	for (int i = 0; i < 5; i++)
	{
		float angle;

		if (i == 0)
			angle = Camera.actualAngle;
		else
			angle = (i - 1) * Angle::DegToRad(90.0f);

		Ideals[i].x = Camera.target.x - distance * sin(angle);
		Ideals[i].z = Camera.target.z - distance * cos(angle);
		Ideals[i].roomNumber = Camera.target.roomNumber;

		if (LOSAndReturnTarget(&Camera.target, &Ideals[i], 200))
		{
			temp[0].x = Ideals[i].x;
			temp[0].y = Ideals[i].y;
			temp[0].z = Ideals[i].z;
			temp[0].roomNumber = Ideals[i].roomNumber;

			temp[1].x = Camera.pos.x;
			temp[1].y = Camera.pos.y;
			temp[1].z = Camera.pos.z;
			temp[1].roomNumber = Camera.pos.roomNumber;

			if (i == 0 ||
				LOSAndReturnTarget(&temp[0], &temp[1], 0))
			{
				if (i == 0)
				{
					farthestnum = 0;
					break;
				}

				int dx = (Camera.pos.x - Ideals[i].x) * (Camera.pos.x - Ideals[i].x);
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
			temp[0].roomNumber = Ideals[i].roomNumber;

			temp[1].x = Camera.pos.x;
			temp[1].y = Camera.pos.y;
			temp[1].z = Camera.pos.z;
			temp[1].roomNumber = Camera.pos.roomNumber;

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

	GameVector ideal = { Ideals[farthestnum].x, Ideals[farthestnum].y, Ideals[farthestnum].z };
	ideal.roomNumber = Ideals[farthestnum].roomNumber;

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
		probe = GetCollision(x, y, z, ideal->roomNumber);

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

	probe = GetCollision(x - push, y, z, ideal->roomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		x = (x & (~1023)) + push;
	}

	probe = GetCollision(x, y, z - push, ideal->roomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		z = (z & (~1023)) + push;
	}

	probe = GetCollision(x + push, y, z, ideal->roomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		x = (x | 1023) - push;
	}

	probe = GetCollision(x, y, z + push, ideal->roomNumber);
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
		probe = GetCollision(x, y, z, ideal->roomNumber);

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

	probe = GetCollision(x, y, z, ideal->roomNumber);
	if (y > probe.Position.Floor ||
		y < probe.Position.Ceiling ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor)
	{
		return true;
	}

	ideal->roomNumber = probe.RoomNumber;
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
		from.roomNumber = ForcedFixedCamera.roomNumber;
	}
	else
	{
		LEVEL_CAMERA_INFO* camera = &g_Level.Cameras[Camera.number];

		from.x = camera->x;
		from.y = camera->y;
		from.z = camera->z;
		from.roomNumber = camera->roomNumber;

		// Multiply original speed by 8 to comply with original bitshifted speed from TR1-2
		moveSpeed = camera->speed * 8 + 1;
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

	auto oldHeadRot = lara->ExtraHeadRot;
	auto oldTorsoRot = lara->ExtraTorsoRot;

	lara->ExtraTorsoRot = EulerAngles::Zero;
	lara->ExtraHeadRot.x = lara->ExtraHeadRot.x * 2.0f;
	lara->ExtraHeadRot.y = lara->ExtraHeadRot.y * 2.0f;

	// Clamp head rotation.
	if (lara->ExtraHeadRot.x > Angle::DegToRad(55.0f))
		lara->ExtraHeadRot.x = Angle::DegToRad(55.0f);
	else if (lara->ExtraHeadRot.x < Angle::DegToRad(-75.0f))
		lara->ExtraHeadRot.x = Angle::DegToRad(-75.0f);
	if (lara->ExtraHeadRot.y < Angle::DegToRad(-80.0f))
		lara->ExtraHeadRot.y = Angle::DegToRad(-80.0f);
	else if (lara->ExtraHeadRot.y > Angle::DegToRad(80.0f))
		lara->ExtraHeadRot.y = Angle::DegToRad(80.0f);

	// Prevent following of breathing motion.
	if (abs(Angle::Normalize(lara->ExtraHeadRot.x - OldCam.pos.Orientation.x)) >= Angle::DegToRad(0.09f))
		OldCam.pos.Orientation.x = (lara->ExtraHeadRot.x + OldCam.pos.Orientation.x) / 2.0f;
	else
		OldCam.pos.Orientation.x = lara->ExtraHeadRot.x;
	if (abs(Angle::Normalize(lara->ExtraHeadRot.y - OldCam.pos.Orientation.y)) >= Angle::DegToRad(0.09f))
		OldCam.pos.Orientation.y = (lara->ExtraHeadRot.y + OldCam.pos.Orientation.y) / 2.0f;
	else
		OldCam.pos.Orientation.SetY(lara->ExtraHeadRot.y);

	auto pos = Vector3Int(0, CLICK(0.25f) / 4, CLICK(0.25f));
	GetLaraJointPosition(&pos, LM_HEAD);

	auto probe = GetCollision(pos.x, pos.y, pos.z, item->RoomNumber);
	if (probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		pos.y > probe.Position.Floor ||
		pos.y < probe.Position.Ceiling)
	{
		pos = Vector3Int(0, CLICK(0.25f) / 4 , 0);
		GetLaraJointPosition(&pos, LM_HEAD);

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
			pos.x = 0;
			pos.y = CLICK(0.25f) / 4;
			pos.z = -CLICK(0.25f);
			GetLaraJointPosition(&pos, LM_HEAD);
		}
	}

	auto pos2 = Vector3Int(0, 0, -SECTOR(1));
	GetLaraJointPosition(&pos2, LM_HEAD);

	auto pos3 = Vector3Int(0, 0, CLICK(8));
	GetLaraJointPosition(&pos3, LM_HEAD);

	int dx = (pos2.x - pos.x) / 8;
	int dy = (pos2.y - pos.y) / 8;
	int dz = (pos2.z - pos.z) / 8;
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
		OldCam.pos.Position == item->Pose.Position &&
		OldCam.ActiveState == item->Animation.ActiveState &&
		OldCam.TargetState == item->Animation.TargetState &&
		Camera.oldType == CameraType::Look)
	{
		ideal = LookCamPosition;
		pos3.x = LookCamTarget.x;
		pos3.y = LookCamTarget.y;
		pos3.z = LookCamTarget.z;
	}
	else
	{
		OldCam.pos.Orientation.x = lara->ExtraHeadRot.x;
		OldCam.pos.Orientation.SetY(lara->ExtraHeadRot.y);
		OldCam.pos.Position = item->Pose.Position;
		OldCam.ActiveState = item->Animation.ActiveState;
		OldCam.TargetState = item->Animation.TargetState;
		LookCamPosition = ideal;
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
		Camera.target.roomNumber = item->RoomNumber;
	}
	else
	{
		Camera.pos.x += (ideal.x - Camera.pos.x) / 4;
		Camera.pos.y += (ideal.y - Camera.pos.y) / 4;
		Camera.pos.z += (ideal.z - Camera.pos.z) / 4;
		Camera.target.x += (pos3.x - Camera.target.x) / 4;
		Camera.target.y += (pos3.y - Camera.target.y) / 4;
		Camera.target.z += (pos3.z - Camera.target.z) / 4;
		Camera.target.roomNumber = item->RoomNumber;
	}

	if (Camera.bounce && Camera.type == Camera.oldType)
	{
		if (Camera.bounce <= 0)
		{
			Camera.target.x += GetRandomControl() % (-Camera.bounce) - (-Camera.bounce / 2);
			Camera.target.y += GetRandomControl() % (-Camera.bounce) - (-Camera.bounce / 2);
			Camera.target.z += GetRandomControl() % (-Camera.bounce) - (-Camera.bounce / 2);
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
	probe = GetCollision(Camera.pos.x, y, Camera.pos.z, Camera.pos.roomNumber);

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
	probe = GetCollision(Camera.pos.x, y, Camera.pos.z, Camera.pos.roomNumber);
	if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		Camera.pos.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);
	else if (y < probe.Position.Ceiling ||
		y > probe.Position.Floor ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		LOSAndReturnTarget(&Camera.target, &Camera.pos, 0);
	}

	y = Camera.pos.y;
	probe = GetCollision(Camera.pos.x, y, Camera.pos.z, Camera.pos.roomNumber);
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
		Camera.pos.roomNumber = item->RoomNumber;
	}

	ItemsCollideCamera();

	GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &Camera.pos.roomNumber);
	LookAt(&Camera, 0);

	if (Camera.mikeAtLara)
	{
		Camera.actualAngle = Angle::Normalize(item->Pose.Orientation.y + lara->ExtraHeadRot.y + lara->ExtraTorsoRot.y);
		Camera.mikePos.x = item->Pose.Position.x;
		Camera.mikePos.y = item->Pose.Position.y;
		Camera.mikePos.z = item->Pose.Position.z;
	}
	else
	{
		Camera.actualAngle = atan2(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.mikePos.x = Camera.pos.x + PhdPerspective * sin(Camera.actualAngle);
		Camera.mikePos.z = Camera.pos.z + PhdPerspective * cos(Camera.actualAngle);
		Camera.mikePos.y = Camera.pos.y;
	}

	Camera.oldType = Camera.type;

	lara->ExtraHeadRot = oldHeadRot;
	lara->ExtraTorsoRot = oldTorsoRot;
}

void BounceCamera(ItemInfo* item, int bounce, int maxDistance)
{
	int distance = sqrt(
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

void LaserSightCamera(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (WeaponDelay)
		WeaponDelay--;

	if (LSHKTimer)
		LSHKTimer--;

	bool firing = false;
	auto& ammo = GetAmmo(item, lara->Control.Weapon.GunType);

	if (!(RawInput & IN_ACTION) || WeaponDelay || !ammo)
	{
		if (!(RawInput & IN_ACTION))
		{
			LSHKShotsFired = 0;
			Camera.bounce = 0;
		}
	}
	else
	{
		if (lara->Control.Weapon.GunType == LaraWeaponType::Revolver)
		{
			firing = true;
			WeaponDelay = 16;
			Statistics.Game.AmmoUsed++;

			if (!ammo.hasInfinite())
				(ammo)--;

			Camera.bounce = -16 - (GetRandomControl() & 0x1F);
		}
		else if (lara->Control.Weapon.GunType == LaraWeaponType::Crossbow)
		{
			firing = true;
			WeaponDelay = 32;
		}
		else
		{
			if (lara->Weapons[(int)LaraWeaponType::HK].SelectedAmmo == WeaponAmmoType::Ammo1)
			{
				WeaponDelay = 12;
				firing = true;

				if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
					SoundEffect(SFX_TR4_HK_SILENCED, nullptr);
				else
				{
					SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
					SoundEffect(SFX_TR4_HK_FIRE, nullptr);
				}

				Camera.bounce = -16 - (GetRandomControl() & 0x1F);
			}
			else if (lara->Weapons[(int)LaraWeaponType::HK].SelectedAmmo == WeaponAmmoType::Ammo2)
			{
				if (!LSHKTimer)
				{
					if (++LSHKShotsFired == 5)
					{
						LSHKShotsFired = 0;
						WeaponDelay = 12;
					}

					LSHKTimer = 4;
					firing = true;

					if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
						SoundEffect(SFX_TR4_HK_SILENCED, nullptr);
					else
					{
						SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
						SoundEffect(SFX_TR4_HK_FIRE, nullptr);
					}

					Camera.bounce = -16 - (GetRandomControl() & 0x1F);
				}
				else
				{
					Camera.bounce = -16 - (GetRandomControl() & 0x1F);

					if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
						SoundEffect(SFX_TR4_HK_SILENCED, nullptr);
					else
					{
						SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
						SoundEffect(SFX_TR4_HK_FIRE, nullptr);
					}

					Camera.bounce = -16 - (GetRandomControl() & 0x1F);
				}
			}
			else
			{
				if (LSHKTimer)
				{
					if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
						SoundEffect(SFX_TR4_HK_SILENCED, nullptr);
					else
					{
						SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
						SoundEffect(SFX_TR4_HK_FIRE, nullptr);
					}
				}
				else
				{
					LSHKTimer = 4;
					firing = true;

					if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
						SoundEffect(SFX_TR4_HK_SILENCED, nullptr);
					else
					{
						SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
						SoundEffect(SFX_TR4_HK_FIRE, nullptr);
					}
				}
			}

			if (!ammo.hasInfinite())
				(ammo)--;
		}
	}

	GetTargetOnLOS(&Camera.pos, &Camera.target, true, firing);
}

void BinocularCamera(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (!LaserSight)
	{
		// TODO: Some of these inputs should ideally be blocked. @Sezz 2022.05.19
		if (RawInput & (IN_DESELECT | IN_OPTION | IN_LOOK | IN_DRAW | IN_FLARE | IN_WALK | IN_JUMP))
		{
			item->MeshBits = ALL_JOINT_BITS;
			lara->Inventory.IsBusy = false;
			lara->ExtraHeadRot = EulerAngles::Zero;
			lara->ExtraTorsoRot = EulerAngles::Zero;
			Camera.type = BinocularOldCamera;
			BinocularOn = false;
			BinocularRange = 0;
			AlterFOV(Angle::DegToRad(80.0f));
			return;
		}
	}

	item->MeshBits = NO_JOINT_BITS;
	AlterFOV(7 * (Angle::DegToRad(11.5f) - BinocularRange));

	float headXRot = lara->ExtraHeadRot.x * 2;
	float headYRot = lara->ExtraHeadRot.y;

	if (headXRot > Angle::DegToRad(75.0f))
		headXRot = Angle::DegToRad(75.0f);
	else if (headXRot < Angle::DegToRad(-75.0f))
		headXRot = Angle::DegToRad(-75.0f);

	if (headYRot > Angle::DegToRad(80.0f))
		headYRot = Angle::DegToRad(80.0f);
	else if (headYRot < Angle::DegToRad(-80.0f))
		headYRot = Angle::DegToRad(-80.0f);

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
	Camera.pos.roomNumber = probe.RoomNumber;

	int l = SECTOR(20.25f) * cos(headXRot);

	int tx = x + l * sin(item->Pose.Orientation.y + headYRot);
	int ty = y - SECTOR(20.25f) * sin(headXRot);
	int tz = z + l * cos(item->Pose.Orientation.y + headYRot);

	if (Camera.oldType == CameraType::Fixed)
	{
		Camera.target.x = tx;
		Camera.target.y = ty;
		Camera.target.z = tz;
		Camera.target.roomNumber = item->RoomNumber;
	}
	else
	{
		Camera.target.x += (tx - Camera.target.x) / 4;
		Camera.target.y += (ty - Camera.target.y) / 4;
		Camera.target.z += (tz - Camera.target.z) / 4;
		Camera.target.roomNumber = item->RoomNumber;
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

	Camera.target.roomNumber = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.target.roomNumber).RoomNumber;
	LookAt(&Camera, 0);

	if (Camera.mikeAtLara)
	{
		Camera.actualAngle = Angle::Normalize(item->Pose.Orientation.y + lara->ExtraHeadRot.y + lara->ExtraTorsoRot.y);
		Camera.mikePos = item->Pose.Position;
	}
	else
	{
		Camera.actualAngle = atan2(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.mikePos.x = Camera.pos.x + PhdPerspective * sin(Camera.actualAngle);
		Camera.mikePos.z = Camera.pos.z + PhdPerspective * cos(Camera.actualAngle);
		Camera.mikePos.y = Camera.pos.y;
	}

	Camera.oldType = Camera.type;

	int range = 0;
	int flags = 0;

	if (!(RawInput & IN_WALK))
	{
		range = 64;
		flags = 0x10000;
	}
	else
	{
		range = 32;
		flags = 0x8000;
	}

	if (RawInput & IN_SPRINT)
	{
		BinocularRange -= range;
		if (BinocularRange < Angle::DegToRad(0.7f))
			BinocularRange = Angle::DegToRad(0.7f);
		else
			SoundEffect(SFX_TR4_BINOCULARS_ZOOM, nullptr, SoundEnvironment::Land, 0.9f);
	}
	else if (RawInput & IN_CROUCH)
	{
		BinocularRange += range;
		if (BinocularRange > Angle::DegToRad(8.5f))
			BinocularRange = Angle::DegToRad(8.5f);
		else
			SoundEffect(SFX_TR4_BINOCULARS_ZOOM, nullptr, SoundEnvironment::Land, 1.0f);
	}

	if (!LaserSight)
	{
		auto src = Vector3Int(Camera.pos.x, Camera.pos.y, Camera.pos.z);
		auto target = Vector3Int(Camera.target.x, Camera.target.y, Camera.target.z);

		GetTargetOnLOS(&Camera.pos, &Camera.target, false, false);

		if (RawInput & IN_ACTION)
			LaraTorch(&src, &target, lara->ExtraHeadRot.y, 192);
	}
	else
		LaserSightCamera(item);
}

void ConfirmCameraTargetPos()
{
	auto pos = Vector3Int();
	GetLaraJointPosition(&pos, LM_TORSO);

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
	auto probe = GetCollision(Camera.target.x, y, Camera.target.z, Camera.target.roomNumber);
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

	if (UseForcedFixedCamera != 0)
	{
		Camera.type = CameraType::Fixed;
		if (Camera.oldType != CameraType::Fixed)
			Camera.speed = 1;
	}

	// Camera is in a water room, play water sound effect.
	if (TestEnvironment(ENV_FLAG_WATER, Camera.pos.roomNumber))
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

	auto* bounds = GetBoundsAccurate(item);

	int x;
	int y = ((bounds->Y1 + bounds->Y2) / 2) + item->Pose.Position.y - CLICK(1);
	int z;

	if (Camera.item)
	{
		if (!fixedCamera)
		{
			auto dx = Camera.item->Pose.Position.x - item->Pose.Position.x;
			auto dz = Camera.item->Pose.Position.z - item->Pose.Position.z;
			int shift = sqrt(pow(dx, 2) + pow(dz, 2));
			float angle = atan2(dz, dx) - item->Pose.Orientation.y;
			float tilt = atan2(shift, y - (bounds->Y1 + bounds->Y2) / 2 - Camera.item->Pose.Position.y);
			bounds = GetBoundsAccurate(Camera.item);
			angle /= 2;
			tilt /= 2;

			if (angle > Angle::DegToRad(-50.0f) && angle < Angle::DegToRad(50.0f) && tilt > Angle::DegToRad(-85.0f) && tilt < Angle::DegToRad(85.0f))
			{
				float change = angle - Lara.ExtraHeadRot.y;
				if (change > Angle::DegToRad(4.0f))
					Lara.ExtraHeadRot.SetY(Lara.ExtraHeadRot.y + Angle::DegToRad(4.0f));
				else if (change < Angle::DegToRad(-4.0f))
					Lara.ExtraHeadRot.SetY(Lara.ExtraHeadRot.y - Angle::DegToRad(4.0f));
				else
					Lara.ExtraHeadRot.SetY(Lara.ExtraHeadRot.y + change);
				Lara.ExtraTorsoRot.SetY(Lara.ExtraHeadRot.y);

				change = tilt - Lara.ExtraHeadRot.x;
				if (change > Angle::DegToRad(4.0f))
					Lara.ExtraHeadRot.x = Lara.ExtraHeadRot.x + Angle::DegToRad(4.0f);
				else if (change < Angle::DegToRad(-4.0f))
					Lara.ExtraHeadRot.x = Lara.ExtraHeadRot.x - Angle::DegToRad(4.0f);
				else
					Lara.ExtraHeadRot.x = Lara.ExtraHeadRot.x + change;
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
			LastTarget = Camera.target;

		Camera.target.roomNumber = item->RoomNumber;

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
		LastTarget = Camera.target;

		Camera.target.roomNumber = item->RoomNumber;
		Camera.target.y = y;

		auto shift = (bounds->X1 + bounds->X2 + bounds->Z1 + bounds->Z2) / 4;
		x = item->Pose.Position.x + shift * sin(item->Pose.Orientation.y);
		z = item->Pose.Position.z + shift * cos(item->Pose.Orientation.y);

		Camera.target.x = x;
		Camera.target.z = z;

		if (item->ObjectNumber == ID_LARA)
		{
			ConfirmCameraTargetPos();
			x = Camera.target.x;
			y = Camera.target.y;
			z = Camera.target.z;
		}

		if (fixedCamera == Camera.fixedCamera)
		{
			Camera.fixedCamera = false;
			if (Camera.speed != 1 &&
				Camera.oldType != CameraType::Look &&
				!BinocularOn)
			{
				if (TargetSnaps <= 8)
				{
					x = LastTarget.x + ((x - LastTarget.x) / 4);
					Camera.target.x = x;
					y = LastTarget.y + ((y - LastTarget.y) / 4);
					Camera.target.y = y;
					z = LastTarget.z + ((z - LastTarget.z) / 4);
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

		Camera.target.roomNumber = GetCollision(x, y, z, Camera.target.roomNumber).RoomNumber;

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
		TrInput &= ~IN_LEFT;
		if (lara->ExtraHeadRot.y > Angle::DegToRad(-44.0f))
		{
			if (BinocularRange)
				lara->ExtraHeadRot.SetY(lara->ExtraHeadRot.y + Angle::DegToRad(2.0f) * (BinocularRange - Angle::DegToRad(10.0f)) / Angle::DegToRad(8.5f));
			else
				lara->ExtraHeadRot.SetY(lara->ExtraHeadRot.y - Angle::DegToRad(2.0f));
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		TrInput &= ~IN_RIGHT;
		if (lara->ExtraHeadRot.y < Angle::DegToRad(44.0f))
		{
			if (BinocularRange)
				lara->ExtraHeadRot.SetY(lara->ExtraHeadRot.y + Angle::DegToRad(2.0f) * (Angle::DegToRad(10.0f) - BinocularRange) / Angle::DegToRad(8.5f));
			else
				lara->ExtraHeadRot.SetY(lara->ExtraHeadRot.y + Angle::DegToRad(2.0f));
		}
	}

	if (lara->Control.HandStatus != HandStatus::Busy &&
		lara->Vehicle == NO_ITEM &&
		!lara->LeftArm.Locked &&
		!lara->RightArm.Locked)
	{
		lara->ExtraTorsoRot.SetY(lara->ExtraHeadRot.y);
	}
}

void LookUpDown(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	Camera.type = CameraType::Look;
	if (TrInput & IN_FORWARD)
	{
		TrInput &= ~IN_FORWARD;
		if (lara->ExtraHeadRot.x > Angle::DegToRad(-35.0f))
		{
			if (BinocularRange)
				lara->ExtraHeadRot.x = lara->ExtraHeadRot.x + Angle::DegToRad(2.0f) * (BinocularRange - Angle::DegToRad(10.0f)) / Angle::DegToRad(17.0f);
			else
				lara->ExtraHeadRot.x = lara->ExtraHeadRot.x - Angle::DegToRad(2.0f);
		}
	}
	else if (TrInput & IN_BACK)
	{
		TrInput &= ~IN_BACK;
		if (lara->ExtraHeadRot.x < Angle::DegToRad(30.0f))
		{
			if (BinocularRange)
				lara->ExtraHeadRot.x = lara->ExtraHeadRot.x + Angle::DegToRad(2.0f) * (Angle::DegToRad(10.0f) - BinocularRange) / Angle::DegToRad(17.0f);
			else
				lara->ExtraHeadRot.x = lara->ExtraHeadRot.x + Angle::DegToRad(2.0f);
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
		lara->ExtraHeadRot.InterpolateLinear(EulerAngles::Zero, 0.1f, Angle::DegToRad(0.1f));

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
				lara->ExtraTorsoRot.x = 0.0f;
			if (!lara->ExtraHeadRot.y)
				lara->ExtraTorsoRot.y = 0.0f;
			if (!lara->ExtraHeadRot.z)
				lara->ExtraTorsoRot.z = 0.0f;
		}
	}
}

bool TestBoundsCollideCamera(BOUNDING_BOX* bounds, PHD_3DPOS* pos, int radius)
{
	auto dxBox = TO_DX_BBOX(*pos, bounds);
	auto sphere = BoundingSphere(Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z), radius);
	return sphere.Intersects(dxBox);
}

void ItemPushCamera(BOUNDING_BOX* bounds, PHD_3DPOS* pos, int radius)
{
	int dx = Camera.pos.x - pos->Position.x;
	int dz = Camera.pos.z - pos->Position.z;
	float sinY = sin(pos->Orientation.y);
	float cosY = cos(pos->Orientation.y);
	auto x = dx * cosY - dz * sinY;
	auto z = dx * sinY + dz * cosY;

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

	Camera.pos.x = pos->Position.x + (cosY * x + sinY * z);
	Camera.pos.z = pos->Position.z + (cosY * z - sinY * x);

	auto coll = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber);
	if (coll.Position.Floor == NO_HEIGHT || Camera.pos.y > coll.Position.Floor || Camera.pos.y < coll.Position.Ceiling)
	{
		Camera.pos.x = CamOldPos.x;
		Camera.pos.y = CamOldPos.y;
		Camera.pos.z = CamOldPos.z;
		Camera.pos.roomNumber = coll.RoomNumber;
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
	Vector3 extents = TO_DX_BBOX(item->Pose, GetBoundsAccurate(item)).Extents;
	if ((abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.y) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.y) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD))
		return false;

	return true;
}

std::vector<short> FillCollideableItemList()
{
	std::vector<short> itemList;
	auto roomList = GetRoomList(Camera.pos.roomNumber);

	for (short i = 0; i < g_Level.NumItems; i++)
	{
		auto item = &g_Level.Items[i];

		if (!roomList.count(item->RoomNumber))
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

	auto stat = &StaticObjects[mesh->staticNumber];
	auto extents = Vector3(abs(stat->collisionBox.X1 - stat->collisionBox.X2),
		abs(stat->collisionBox.Y1 - stat->collisionBox.Y2),
		abs(stat->collisionBox.Z1 - stat->collisionBox.Z2));

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
	auto roomList = GetRoomList(Camera.pos.roomNumber);

	for (auto i : roomList)
	{
		auto room = &g_Level.Rooms[i];
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

		auto bounds = GetBoundsAccurate(item);
		if (TestBoundsCollideCamera(bounds, &item->Pose, CAMERA_RADIUS))
			ItemPushCamera(bounds, &item->Pose, rad);

#ifdef _DEBUG
		TEN::Renderer::g_Renderer.AddDebugBox(TO_DX_BBOX(item->Pose, bounds),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f), RENDERER_DEBUG_PAGE::DIMENSION_STATS);
#endif
	}

	itemList.clear(); // Done

	// Collide with static meshes

	auto staticList = FillCollideableStaticsList();

	for (int i = 0; i < staticList.size(); i++)
	{
		auto mesh = staticList[i];
		auto stat = &StaticObjects[mesh->staticNumber];

		if (!mesh || !stat)
			return;

		auto dx = abs(LaraItem->Pose.Position.x - mesh->pos.Position.x);
		auto dy = abs(LaraItem->Pose.Position.y - mesh->pos.Position.y);
		auto dz = abs(LaraItem->Pose.Position.z - mesh->pos.Position.z);

		if (dx > COLL_CANCEL_THRESHOLD || dz > COLL_CANCEL_THRESHOLD || dy > COLL_CANCEL_THRESHOLD)
			continue;

		auto bounds = &stat->collisionBox;
		if (TestBoundsCollideCamera(bounds, &mesh->pos, CAMERA_RADIUS))
			ItemPushCamera(bounds, &mesh->pos, rad);

#ifdef _DEBUG
		TEN::Renderer::g_Renderer.AddDebugBox(TO_DX_BBOX(mesh->pos, bounds),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f), RENDERER_DEBUG_PAGE::DIMENSION_STATS);
#endif
	}

	staticList.clear(); // Done
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

void HandleOptics()
{
	if (!(TrInput & IN_LOOK) || UseSpotCam || TrackCameraInit ||
		((LaraItem->Animation.ActiveState != LS_IDLE || LaraItem->Animation.AnimNumber != LA_STAND_IDLE) &&
		 (!Lara.Control.IsLow || TrInput & IN_CROUCH || LaraItem->Animation.TargetState != LS_CROUCH_IDLE || LaraItem->Animation.AnimNumber != LA_CROUCH_IDLE)))
	{
		if (BinocularRange == 0)
		{
			if (UseSpotCam || TrackCameraInit)
				TrInput &= ~IN_LOOK;
		}
		else
		{
			// If any input but optic controls (directions + action), immediately exit binoculars mode.
			if (TrInput != IN_NONE && ((TrInput & ~IN_OPTIC_CONTROLS) != IN_NONE))
				BinocularRange = 0;

			if (LaserSight)
			{
				BinocularRange = 0;
				BinocularOn = false;
				LaserSight = false;
				Camera.type = BinocularOldCamera;
				Camera.bounce = 0;
				AlterFOV(Angle::DegToRad(80.0f));

				LaraItem->MeshBits = ALL_JOINT_BITS;
				Lara.Inventory.IsBusy = false;
				ResetLaraFlex(LaraItem);

				TrInput &= ~IN_LOOK;
			}
			else
			{
				TrInput |= IN_LOOK;
				DbInput = 0;
			}
		}
	}
	else if (BinocularRange == 0)
	{
		if (Lara.Control.HandStatus == HandStatus::WeaponReady &&
			((Lara.Control.Weapon.GunType == LaraWeaponType::Revolver && Lara.Weapons[(int)LaraWeaponType::Revolver].HasLasersight) ||
				Lara.Control.Weapon.GunType == LaraWeaponType::HK ||
				(Lara.Control.Weapon.GunType == LaraWeaponType::Crossbow && Lara.Weapons[(int)LaraWeaponType::Crossbow].HasLasersight)))
		{
			BinocularRange = 128;
			BinocularOldCamera = Camera.oldType;
			BinocularOn = true;
			LaserSight = true;
			Lara.Inventory.IsBusy = true;
		}
	}
}