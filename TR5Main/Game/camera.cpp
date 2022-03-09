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
#include "Objects/Generic/Object/burning_torch.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using TEN::Renderer::g_Renderer;
using namespace TEN::Entities::Generic;
using namespace TEN::Effects::Environment;

constexpr auto COLL_CHECK_THRESHOLD   = SECTOR(4);
constexpr auto COLL_CANCEL_THRESHOLD  = SECTOR(2);
constexpr auto COLL_DISCARD_THRESHOLD = CLICK(0.5f);
constexpr auto CAMERA_RADIUS          = CLICK(1);

struct OLD_CAMERA
{
	short ActiveState;
	short TargetState;
	int targetDistance;
	short actualElevation;
	short targetElevation;
	short actualAngle;
	PHD_3DPOS pos;
	PHD_3DPOS pos2;
	PHD_VECTOR target;
};

float LfAspectCorrection;
GAME_VECTOR LastTarget;
byte SniperCamActive;

extern int KeyTriggerActive;

PHD_VECTOR CurrentCameraPosition;
SVECTOR CurrentCameraRotation;
GAME_VECTOR LastIdeal;
GAME_VECTOR	Ideals[5];
OLD_CAMERA OldCam;
int CameraSnaps = 0;
int TargetSnaps = 0;
GAME_VECTOR LookCamPosition;
GAME_VECTOR LookCamTarget;
int LSHKTimer = 0;
int LSHKShotsFired = 0;
PHD_VECTOR CamOldPos;
CAMERA_INFO Camera;
GAME_VECTOR ForcedFixedCamera;
int UseForcedFixedCamera;
int NumberCameras;
int BinocularRange;
int BinocularOn;
CameraType BinocularOldCamera;
bool LaserSight;
int PhdPerspective;
short CurrentFOV;

int RumbleTimer = 0;
int RumbleCounter = 0;

bool ScreenFadedOut = false;
bool ScreenFading = false;
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
	float r = 0; TO_RAD(roll);

	g_Renderer.UpdateCameraMatrices(cam, r, fov);
}

void AlterFOV(int value)
{
	CurrentFOV = value;
	PhdPerspective = g_Renderer.ScreenWidth / 2 * phd_cos(CurrentFOV / 2) / phd_sin(CurrentFOV / 2);
}


void InitialiseCamera()
{
	Camera.shift = LaraItem->Position.yPos - SECTOR(1);

	LastTarget.x = LaraItem->Position.xPos;
	LastTarget.y = Camera.shift;
	LastTarget.z = LaraItem->Position.zPos;
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

	AlterFOV(14560);

	UseForcedFixedCamera = 0;
	CalculateCamera();
}

void MoveCamera(GAME_VECTOR* ideal, int speed)
{
	GAME_VECTOR from, to;

	if (BinocularOn < 0)
	{
		speed = 1;
		BinocularOn++;
	}

	if (OldCam.pos.xRot != LaraItem->Position.xRot ||
		OldCam.pos.yRot != LaraItem->Position.yRot ||
		OldCam.pos.zRot != LaraItem->Position.zRot ||
		OldCam.pos2.xRot != Lara.ExtraHeadRot.xRot ||
		OldCam.pos2.yRot != Lara.ExtraHeadRot.yRot ||
		OldCam.pos2.xPos != Lara.ExtraTorsoRot.xRot ||
		OldCam.pos2.yPos != Lara.ExtraTorsoRot.yRot ||
		OldCam.pos.xPos != LaraItem->Position.xPos ||
		OldCam.pos.yPos != LaraItem->Position.yPos ||
		OldCam.pos.zPos != LaraItem->Position.zPos ||
		OldCam.ActiveState != LaraItem->ActiveState ||
		OldCam.TargetState != LaraItem->TargetState ||
		OldCam.targetDistance != Camera.targetDistance ||
		OldCam.targetElevation != Camera.targetElevation ||
		OldCam.actualElevation != Camera.actualElevation ||
		OldCam.actualAngle != Camera.actualAngle ||
		OldCam.target.x != Camera.target.x ||
		OldCam.target.y != Camera.target.y ||
		OldCam.target.z != Camera.target.z ||
		Camera.oldType != Camera.type ||
		BinocularOn < 0)
	{
		OldCam.pos.xRot = LaraItem->Position.xRot;
		OldCam.pos.yRot = LaraItem->Position.yRot;
		OldCam.pos.zRot = LaraItem->Position.zRot;
		OldCam.pos2.xRot = Lara.ExtraHeadRot.xRot;
		OldCam.pos2.yRot = Lara.ExtraHeadRot.yRot;
		OldCam.pos2.xPos = Lara.ExtraTorsoRot.xRot;
		OldCam.pos2.yPos = Lara.ExtraTorsoRot.yRot;
		OldCam.pos.xPos = LaraItem->Position.xPos;
		OldCam.pos.yPos = LaraItem->Position.yPos;
		OldCam.pos.zPos = LaraItem->Position.zPos;
		OldCam.ActiveState = LaraItem->ActiveState;
		OldCam.TargetState = LaraItem->TargetState;
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

	auto probe = GetCollisionResult(Camera.pos.x, y, Camera.pos.z, Camera.pos.roomNumber);
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

	probe = GetCollisionResult(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber);

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

	Camera.pos.roomNumber = GetCollisionResult(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber).RoomNumber;
	LookAt(&Camera, 0);

	if (Camera.mikeAtLara)
	{
		Camera.mikePos.x = LaraItem->Position.xPos;
		Camera.mikePos.y = LaraItem->Position.yPos;
		Camera.mikePos.z = LaraItem->Position.zPos;
		Camera.oldType = Camera.type;
	}
	else
	{
		short angle = phd_atan(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.mikePos.x = Camera.pos.x + PhdPerspective * phd_sin(angle);
		Camera.mikePos.y = Camera.pos.y;
		Camera.mikePos.z = Camera.pos.z + PhdPerspective * phd_cos(angle);
		Camera.oldType = Camera.type;
	}
}

void ChaseCamera(ITEM_INFO* item)
{
	if (!Camera.targetElevation)
		Camera.targetElevation = -ANGLE(10.0f);

	Camera.targetElevation += item->Position.xRot;
	UpdateCameraElevation();

	// Clamp x rotation.
	if (Camera.actualElevation > ANGLE(85.0f))
		Camera.actualElevation = ANGLE(85.0f);
	else if (Camera.actualElevation < -ANGLE(85.0f))
		Camera.actualElevation = -ANGLE(85.0f);

	int distance = Camera.targetDistance * phd_cos(Camera.actualElevation);

	auto probe = GetCollisionResult(Camera.target.x, Camera.target.y + CLICK(1), Camera.target.z, Camera.target.roomNumber);

	if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		Camera.target.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);

	int y = Camera.target.y;
	probe = GetCollisionResult(Camera.target.x, y, Camera.target.z, Camera.target.roomNumber);
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
		Ideals[i].y = Camera.target.y + Camera.targetDistance * phd_sin(Camera.actualElevation);

	int farthest = INT_MAX;
	int farthestnum = 0;
	GAME_VECTOR temp[2];

	for (int i = 0; i < 5; i++)
	{
		short angle;

		if (i == 0)
			angle = Camera.actualAngle;
		else
			angle = (i - 1) * ANGLE(90.0f);

		Ideals[i].x = Camera.target.x - distance * phd_sin(angle);
		Ideals[i].z = Camera.target.z - distance * phd_cos(angle);
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

	GAME_VECTOR ideal = { Ideals[farthestnum].x , Ideals[farthestnum].y, Ideals[farthestnum].z };
	ideal.roomNumber = Ideals[farthestnum].roomNumber;

	CameraCollisionBounds(&ideal, CLICK(1.5f), 1);
	MoveCamera(&ideal, Camera.speed);
}

void UpdateCameraElevation()
{
	if (Camera.laraNode != -1)
	{
		PHD_VECTOR pos = { 0, 0, 0 };
		GetLaraJointPosition(&pos, Camera.laraNode);

		PHD_VECTOR pos1 = { 0, -CLICK(1), SECTOR(2) };
		GetLaraJointPosition(&pos1, Camera.laraNode);

		pos.z = pos1.z - pos.z;
		pos.x = pos1.x - pos.x;
		Camera.actualAngle = Camera.targetAngle + phd_atan(pos.z, pos.x);
	}
	else
		Camera.actualAngle = LaraItem->Position.yRot + Camera.targetAngle;

	Camera.actualElevation += (Camera.targetElevation - Camera.actualElevation) / 8;
}

void CombatCamera(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	Camera.target.x = item->Position.xPos;
	Camera.target.z = item->Position.zPos;

	if (lara->TargetEntity)
	{
		Camera.targetAngle = lara->TargetArmAngles[0];
		Camera.targetElevation = lara->TargetArmAngles[1] + item->Position.xRot;
	}
	else
	{
		Camera.targetAngle = lara->ExtraHeadRot.yRot + lara->ExtraTorsoRot.yRot;
		Camera.targetElevation = lara->ExtraHeadRot.xRot + lara->ExtraTorsoRot.xRot + item->Position.xRot - ANGLE(15.0f);
	}

	auto probe = GetCollisionResult(Camera.target.x, Camera.target.y + CLICK(1), Camera.target.z, Camera.target.roomNumber);
	if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		Camera.target.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);

	probe = GetCollisionResult(Camera.target.x, Camera.target.y, Camera.target.z, Camera.target.roomNumber);
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
	probe = GetCollisionResult(Camera.target.x, y, Camera.target.z, Camera.target.roomNumber);
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
	int distance = Camera.targetDistance * phd_cos(Camera.actualElevation);

	for (int i = 0; i < 5; i++)
		Ideals[i].y = Camera.target.y + Camera.targetDistance * phd_sin(Camera.actualElevation);

	int farthest = INT_MAX;
	int farthestnum = 0;
	GAME_VECTOR temp[2];

	for (int i = 0; i < 5; i++)
	{
		short angle;

		if (i == 0)
			angle = Camera.actualAngle;
		else
			angle = (i - 1) * ANGLE(90.0f);

		Ideals[i].x = Camera.target.x - distance * phd_sin(angle);
		Ideals[i].z = Camera.target.z - distance * phd_cos(angle);
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

	GAME_VECTOR ideal = { Ideals[farthestnum].x, Ideals[farthestnum].y, Ideals[farthestnum].z };
	ideal.roomNumber = Ideals[farthestnum].roomNumber;

	CameraCollisionBounds(&ideal, CLICK(1.5f), 1);

	if (Camera.oldType == CameraType::Fixed)
		Camera.speed = 1;

	MoveCamera(&ideal, Camera.speed);
}

bool CameraCollisionBounds(GAME_VECTOR* ideal, int push, int yFirst)
{
	int x = ideal->x;
	int y = ideal->y;
	int z = ideal->z;

	COLL_RESULT probe;

	if (yFirst)
	{
		probe = GetCollisionResult(x, y, z, ideal->roomNumber);

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

	probe = GetCollisionResult(x - push, y, z, ideal->roomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		x = (x & (~1023)) + push;
	}

	probe = GetCollisionResult(x, y, z - push, ideal->roomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		z = (z & (~1023)) + push;
	}

	probe = GetCollisionResult(x + push, y, z, ideal->roomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		x = (x | 1023) - push;
	}

	probe = GetCollisionResult(x, y, z + push, ideal->roomNumber);
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
		probe = GetCollisionResult(x, y, z, ideal->roomNumber);

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

	probe = GetCollisionResult(x, y, z, ideal->roomNumber);
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

void FixedCamera(ITEM_INFO* item)
{
	GAME_VECTOR from, to;

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

void LookCamera(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	short headXRot = lara->ExtraHeadRot.xRot;
	short headYRot = lara->ExtraHeadRot.yRot;
	short torsoXRot = lara->ExtraTorsoRot.xRot;
	short torsoYRot = lara->ExtraTorsoRot.yRot;

	lara->ExtraTorsoRot.xRot = 0;
	lara->ExtraTorsoRot.yRot = 0;
	lara->ExtraHeadRot.xRot *= 2;
	lara->ExtraHeadRot.yRot *= 2;

	// Clamp head rotation.
	if (lara->ExtraHeadRot.xRot > ANGLE(55.0f))
		lara->ExtraHeadRot.xRot = ANGLE(55.0f);
	else if (lara->ExtraHeadRot.xRot < -ANGLE(75.0f))
		lara->ExtraHeadRot.xRot = -ANGLE(75.0f);
	if (lara->ExtraHeadRot.yRot < -ANGLE(80.0f))
		lara->ExtraHeadRot.yRot = -ANGLE(80.0f);
	else if (lara->ExtraHeadRot.yRot > ANGLE(80.0f))
		lara->ExtraHeadRot.yRot = ANGLE(80.0f);

	if (abs(lara->ExtraHeadRot.xRot - OldCam.pos.xRot) >= 16)
		OldCam.pos.xRot = (lara->ExtraHeadRot.xRot + OldCam.pos.xRot) / 2;
	else
		OldCam.pos.xRot = lara->ExtraHeadRot.xRot;
	if (abs(lara->ExtraHeadRot.yRot - OldCam.pos.yRot) >= 16)
		OldCam.pos.yRot = (lara->ExtraHeadRot.yRot + OldCam.pos.yRot) / 2;
	else
		OldCam.pos.yRot = lara->ExtraHeadRot.yRot;

	PHD_VECTOR pos = { 0, (int)CLICK(0.25f) / 4, (int)CLICK(0.25f) };
	GetLaraJointPosition(&pos, LM_HEAD);

	auto probe = GetCollisionResult(pos.x, pos.y, pos.z, item->RoomNumber);
	if (probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		pos.y > probe.Position.Floor ||
		pos.y < probe.Position.Ceiling)
	{
		pos = { 0, (int)CLICK(0.25f) / 4 , 0 };
		GetLaraJointPosition(&pos, LM_HEAD);

		probe = GetCollisionResult(pos.x, pos.y + CLICK(1), pos.z, item->RoomNumber);
		if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		{
			pos.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);
			probe = GetCollisionResult(pos.x, pos.y, pos.z, probe.RoomNumber);
		}
		else
			probe = GetCollisionResult(pos.x, pos.y, pos.z, probe.RoomNumber);

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

	PHD_VECTOR pos2 = { 0, 0, -SECTOR(1) };
	GetLaraJointPosition(&pos2, LM_HEAD);

	PHD_VECTOR pos3 = { 0, 0, CLICK(8) };
	GetLaraJointPosition(&pos3, LM_HEAD);

	int dx = (pos2.x - pos.x) >> 3;
	int dy = (pos2.y - pos.y) >> 3;
	int dz = (pos2.z - pos.z) >> 3;
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;

	int roomNum;
	probe.RoomNumber = item->RoomNumber;
	int i = 0;
	for (i = 0; i < 8; i++)
	{
		roomNum = probe.RoomNumber;
		probe = GetCollisionResult(x, y + CLICK(1), z, probe.RoomNumber);
		if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		{
			y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);
			break;
		}
		else
			probe = GetCollisionResult(x, y, z, probe.RoomNumber);

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

	GAME_VECTOR ideal = { x, y, z };
	ideal.roomNumber = roomNum;

	if (OldCam.pos.xRot == lara->ExtraHeadRot.xRot &&
		OldCam.pos.yRot == lara->ExtraHeadRot.yRot &&
		OldCam.pos.xPos == item->Position.xPos &&
		OldCam.pos.yPos == item->Position.yPos &&
		OldCam.pos.zPos == item->Position.zPos &&
		OldCam.ActiveState == item->ActiveState &&
		OldCam.TargetState == item->TargetState &&
		Camera.oldType == CameraType::Look)
	{
		ideal.x = LookCamPosition.x;
		ideal.y = LookCamPosition.y;
		ideal.z = LookCamPosition.z;
		ideal.roomNumber = LookCamPosition.roomNumber;
		pos3.x = LookCamTarget.x;
		pos3.y = LookCamTarget.y;
		pos3.z = LookCamTarget.z;
	}
	else
	{
		OldCam.pos.xRot = lara->ExtraHeadRot.xRot;
		OldCam.pos.yRot = lara->ExtraHeadRot.yRot;
		OldCam.pos.xPos = item->Position.xPos;
		OldCam.pos.yPos = item->Position.yPos;
		OldCam.pos.zPos = item->Position.zPos;
		OldCam.ActiveState = item->ActiveState;
		OldCam.TargetState = item->TargetState;
		LookCamPosition.x = ideal.x;
		LookCamPosition.y = ideal.y;
		LookCamPosition.z = ideal.z;
		LookCamPosition.roomNumber = ideal.roomNumber;
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
		Camera.pos.x += (ideal.x - Camera.pos.x) >> 2;
		Camera.pos.y += (ideal.y - Camera.pos.y) >> 2;
		Camera.pos.z += (ideal.z - Camera.pos.z) >> 2;
		Camera.target.x += (pos3.x - Camera.target.x) >> 2;
		Camera.target.y += (pos3.y - Camera.target.y) >> 2;
		Camera.target.z += (pos3.z - Camera.target.z) >> 2;
		Camera.target.roomNumber = item->RoomNumber;
	}

	if (Camera.bounce && Camera.type == Camera.oldType)
	{
		if (Camera.bounce <= 0)
		{
			Camera.target.x += GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1);
			Camera.target.y += GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1);
			Camera.target.z += GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1);
			Camera.bounce += 5;
		}
		else
		{
			Camera.pos.y += Camera.bounce;
			Camera.target.y += Camera.bounce;
			Camera.bounce = 0;
		}
	}

	y = Camera.pos.y;
	probe = GetCollisionResult(Camera.pos.x, y, Camera.pos.z, Camera.pos.roomNumber);

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
	probe = GetCollisionResult(Camera.pos.x, y, Camera.pos.z, Camera.pos.roomNumber);
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
	probe = GetCollisionResult(Camera.pos.x, y, Camera.pos.z, Camera.pos.roomNumber);
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
		Camera.actualAngle = item->Position.yRot + lara->ExtraHeadRot.yRot + lara->ExtraTorsoRot.yRot;
		Camera.mikePos.x = item->Position.xPos;
		Camera.mikePos.y = item->Position.yPos;
		Camera.mikePos.z = item->Position.zPos;
	}
	else
	{
		Camera.actualAngle = phd_atan(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.mikePos.x = Camera.pos.x + PhdPerspective * phd_sin(Camera.actualAngle);
		Camera.mikePos.z = Camera.pos.z + PhdPerspective * phd_cos(Camera.actualAngle);
		Camera.mikePos.y = Camera.pos.y;
	}

	Camera.oldType = Camera.type;

	lara->ExtraHeadRot.xRot = headXRot;
	lara->ExtraHeadRot.yRot = headYRot;
	lara->ExtraTorsoRot.xRot = torsoXRot;
	lara->ExtraTorsoRot.yRot = torsoYRot;
}

void BounceCamera(ITEM_INFO* item, short bounce, short maxDistance)
{
	int distance = sqrt(
		pow(item->Position.xPos - Camera.pos.x, 2) +
		pow(item->Position.yPos - Camera.pos.y, 2) +
		pow(item->Position.zPos - Camera.pos.z, 2));

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

void BinocularCamera(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	static bool exitingBinoculars = false;

	if (LSHKTimer)
		--LSHKTimer;

	if (!LaserSight)
	{
		if (InputBusy & IN_DRAW)
			exitingBinoculars = true;
		else if (exitingBinoculars)
		{
			exitingBinoculars = false;
			BinocularRange = 0;
			AlterFOV(14560);
			item->MeshBits = -1;
			lara->IsBusy = false;
			lara->ExtraHeadRot.yRot = 0;
			lara->ExtraHeadRot.xRot = 0;
			lara->ExtraTorsoRot.yRot = 0;
			lara->ExtraTorsoRot.xRot = 0;
			Camera.type = BinocularOldCamera;

			return;
		}
	}

	item->MeshBits = 0;
	AlterFOV(7 * (2080 - BinocularRange));

	short headXRot = lara->ExtraHeadRot.xRot * 2;
	short headYRot = lara->ExtraHeadRot.yRot;

	if (headXRot > ANGLE(75.0f))
		headXRot = ANGLE(75.0f);
	else if (headXRot < -ANGLE(75.0f))
		headXRot = -ANGLE(75.0f);

	if (headYRot > ANGLE(80.0f))
		headYRot = ANGLE(80.0f);
	else if (headYRot < -ANGLE(80.0f))
		headYRot = -ANGLE(80.0f);

	int x = item->Position.xPos;
	int y = item->Position.yPos - CLICK(2);
	int z = item->Position.zPos;

	auto probe = GetCollisionResult(x, y, z, item->RoomNumber);
	if (probe.Position.Ceiling <= (y - CLICK(1)))
		y -= CLICK(1);
	else
		y = probe.Position.Ceiling + CLICK(0.25f);

	Camera.pos.x = x;
	Camera.pos.y = y;
	Camera.pos.z = z;
	Camera.pos.roomNumber = probe.RoomNumber;

	int l = SECTOR(20.25f) * phd_cos(headXRot);

	int tx = x + l * phd_sin(item->Position.yRot + headYRot);
	int ty = y - SECTOR(20.25f) * phd_sin(headXRot);
	int tz = z + l * phd_cos(item->Position.yRot + headYRot);

	if (Camera.oldType == CameraType::Fixed)
	{
		Camera.target.x = tx;
		Camera.target.y = ty;
		Camera.target.z = tz;
		Camera.target.roomNumber = item->RoomNumber;
	}
	else
	{
		Camera.target.x += (tx - Camera.target.x) >> 2;
		Camera.target.y += (ty - Camera.target.y) >> 2;
		Camera.target.z += (tz - Camera.target.z) >> 2;
		Camera.target.roomNumber = item->RoomNumber;
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
		}
		else
		{
			Camera.bounce = 0;
			Camera.target.y += Camera.bounce;
		}
	}

	Camera.target.roomNumber = GetCollisionResult(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.target.roomNumber).RoomNumber;
	LookAt(&Camera, 0);

	if (Camera.mikeAtLara)
	{
		Camera.actualAngle = item->Position.yRot + lara->ExtraHeadRot.yRot + lara->ExtraTorsoRot.yRot;
		Camera.mikePos.x = item->Position.xPos;
		Camera.mikePos.y = item->Position.yPos;
		Camera.mikePos.z = item->Position.zPos;
	}
	else
	{
		Camera.actualAngle = phd_atan(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.mikePos.x = Camera.pos.x + PhdPerspective * phd_sin(Camera.actualAngle);
		Camera.mikePos.z = Camera.pos.z + PhdPerspective * phd_cos(Camera.actualAngle);
		Camera.mikePos.y = Camera.pos.y;
	}

	Camera.oldType = Camera.type;

	int range = 0;
	int flags = 0;

	if (!(InputBusy & IN_WALK))
	{
		range = 64;
		flags = 0x10000;
	}
	else
	{
		range = 32;
		flags = 0x8000;
	}

	if (InputBusy & IN_SPRINT)
	{
		BinocularRange -= range;
		if (BinocularRange < 128)
			BinocularRange = 128;
		else
			SoundEffect(SFX_TR5_ZOOM_VIEW_WHIRR, 0, (flags << 8) | 6);
	}
	else if (InputBusy & IN_CROUCH)
	{
		BinocularRange += range;
		if (BinocularRange > 1536)
			BinocularRange = 1536;
		else
			SoundEffect(SFX_TR5_ZOOM_VIEW_WHIRR, 0, (flags << 8) | 6);
	}

	PHD_VECTOR src = { Camera.pos.x, Camera.pos.y, Camera.pos.z };
	PHD_VECTOR target = { Camera.target.x, Camera.target.y, Camera.target.z };

	if (LaserSight)
	{
		int firing = 0;
		Ammo& ammo = GetAmmo(item, lara->Control.Weapon.GunType);

		if (!(InputBusy & IN_ACTION) ||
			WeaponDelay ||
			!ammo)
		{
			if (!(InputBusy & IN_ACTION))
			{
				if (lara->Control.Weapon.GunType != LaraWeaponType::Crossbow)
					WeaponDelay = 0;

				LSHKShotsFired = 0;
				Camera.bounce = 0;
			}
		}
		else
		{
			if (lara->Control.Weapon.GunType == LaraWeaponType::Revolver)
			{
				firing = 1;
				WeaponDelay = 16;
				Statistics.Game.AmmoUsed++;

				if (!ammo.hasInfinite())
					(ammo)--;

				Camera.bounce = -16 - (GetRandomControl() & 0x1F);
			}
			else if (lara->Control.Weapon.GunType == LaraWeaponType::Crossbow)
			{
				firing = 1;
				WeaponDelay = 32;
			}
			else
			{
				if (lara->Weapons[(int)LaraWeaponType::HK].SelectedAmmo == WEAPON_AMMO1)
				{
					WeaponDelay = 12;
					firing = 1;

					if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
						SoundEffect(SFX_TR5_HK_SILENCED, 0, 0);
					else
					{
						SoundEffect(SFX_TR4_EXPLOSION1, 0, 83888140);
						SoundEffect(SFX_TR5_HK_FIRE, 0, 0);
					}
				}
				else if (lara->Weapons[(int)LaraWeaponType::HK].SelectedAmmo == WEAPON_AMMO2)
				{
					if (!LSHKTimer)
					{
						if (++LSHKShotsFired == 5)
						{
							LSHKShotsFired = 0;
							WeaponDelay = 12;
						}

						LSHKTimer = 4;
						firing = 1;

						if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
							SoundEffect(SFX_TR5_HK_SILENCED, 0, 0);
						else
						{
							SoundEffect(SFX_TR4_EXPLOSION1, 0, 83888140);
							SoundEffect(SFX_TR5_HK_FIRE, 0, 0);
						}
					}
					else
					{
						Camera.bounce = -16 - (GetRandomControl() & 0x1F);

						if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
							SoundEffect(SFX_TR5_HK_SILENCED, 0, 0);
						else
						{
							SoundEffect(SFX_TR4_EXPLOSION1, 0, 83888140);
							SoundEffect(SFX_TR5_HK_FIRE, 0, 0);
						}
					}
				}
				else
				{
					if (LSHKTimer)
					{
						if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
							SoundEffect(SFX_TR5_HK_SILENCED, 0, 0);
						else
						{
							SoundEffect(SFX_TR4_EXPLOSION1, 0, 83888140);
							SoundEffect(SFX_TR5_HK_FIRE, 0, 0);
						}
					}
					else
					{
						LSHKTimer = 4;
						firing = 1;

						if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
							SoundEffect(SFX_TR5_HK_SILENCED, 0, 0);
						else
						{
							SoundEffect(SFX_TR4_EXPLOSION1, 0, 83888140);
							SoundEffect(SFX_TR5_HK_FIRE, 0, 0);
						}
					}

					Camera.bounce = -16 - (GetRandomControl() & 0x1F);
				}

				if (!ammo.hasInfinite())
					(ammo)--;
			}
		}

		GetTargetOnLOS(&Camera.pos, &Camera.target, 1, firing);
	}
	else
	{
		GetTargetOnLOS(&Camera.pos, &Camera.target, 0, 0);

		if (!(InputBusy & IN_ACTION))
		{
			// Reimplement this mode?
		}
		else
			LaraTorch(&src, &target, lara->ExtraHeadRot.yRot, 192);
	}
}

void ConfirmCameraTargetPos()
{
	PHD_VECTOR pos = { 0, 0, 0 };
	GetLaraJointPosition(&pos, LM_TORSO);

	if (Camera.laraNode != -1)
	{
		Camera.target.x = pos.x;
		Camera.target.y = pos.y;
		Camera.target.z = pos.z;
	}
	else
	{
		Camera.target.x = LaraItem->Position.xPos;
		Camera.target.y = (Camera.target.y + pos.y) >> 1;
		Camera.target.z = LaraItem->Position.zPos;
	}

	int y = Camera.target.y;
	auto probe = GetCollisionResult(Camera.target.x, y, Camera.target.z, Camera.target.roomNumber);
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

	if (BinocularRange != 0)
	{
		BinocularOn = 1;
		BinocularCamera(LaraItem);

		if (BinocularRange != 0)
			return;
	}

	if (BinocularOn == 1)
		BinocularOn = -8;

	if (UseForcedFixedCamera != 0)
	{
		Camera.type = CameraType::Fixed;
		if (Camera.oldType != CameraType::Fixed)
			Camera.speed = 1;
	}

	// Camera is in a water room, play water sound effect.
	if (TestEnvironment(ENV_FLAG_WATER, Camera.pos.roomNumber))
	{
		SoundEffect(SFX_TR4_UNDERWATER, NULL, SFX_ALWAYS);
		if (Camera.underwater == false)
			Camera.underwater = true;
	}
	else
	{
		if (Camera.underwater == true)
			Camera.underwater = false;
	}

	ITEM_INFO* item;
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
	int y = ((bounds->Y1 + bounds->Y2) / 2) + item->Position.yPos - CLICK(1);
	int z;

	if (Camera.item)
	{
		if (!fixedCamera)
		{
			auto dx = Camera.item->Position.xPos - item->Position.xPos;
			auto dz = Camera.item->Position.zPos - item->Position.zPos;
			int shift = sqrt(pow(dx, 2) + pow(dz, 2));
			short angle = phd_atan(dz, dx) - item->Position.yRot;
			short tilt = phd_atan(shift, y - (bounds->Y1 + bounds->Y2) / 2 - Camera.item->Position.yPos);
			bounds = GetBoundsAccurate(Camera.item);
			angle /= 2;
			tilt /= 2;

			if (angle > -ANGLE(50.0f) && angle < ANGLE(50.0f) && tilt > -ANGLE(85.0f) && tilt < ANGLE(85.0f))
			{
				short change = angle - Lara.ExtraHeadRot.yRot;
				if (change > ANGLE(4.0f))
					Lara.ExtraHeadRot.yRot += ANGLE(4.0f);
				else if (change < -ANGLE(4.0f))
					Lara.ExtraHeadRot.yRot -= ANGLE(4.0f);
				else
					Lara.ExtraHeadRot.yRot += change;
				Lara.ExtraTorsoRot.yRot = Lara.ExtraHeadRot.yRot;

				change = tilt - Lara.ExtraHeadRot.xRot;
				if (change > ANGLE(4.0f))
					Lara.ExtraHeadRot.xRot += ANGLE(4.0f);
				else if (change < -ANGLE(4.0f))
					Lara.ExtraHeadRot.xRot -= ANGLE(4.0f);
				else
					Lara.ExtraHeadRot.xRot += change;
				Lara.ExtraTorsoRot.xRot = Lara.ExtraHeadRot.xRot;

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
			LastTarget.roomNumber = Camera.target.roomNumber;
		}

		Camera.target.roomNumber = item->RoomNumber;

		if (Camera.fixedCamera || BinocularOn < 0)
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
		LastTarget.roomNumber = Camera.target.roomNumber;

		Camera.target.roomNumber = item->RoomNumber;
		Camera.target.y = y;

		if (Camera.type != CameraType::Chase &&
			Camera.flags != CF_CHASE_OBJECT &&
			(Camera.number != -1 && (SniperCamActive = g_Level.Cameras[Camera.number].flags & 3, g_Level.Cameras[Camera.number].flags & 2)))
		{
			PHD_VECTOR pos = { 0, 0, 0 };
			GetLaraJointPosition(&pos, LM_TORSO);

			x = pos.x;
			y = pos.y;
			z = pos.z;

			Camera.target.x = pos.x;
			Camera.target.y = pos.y;
			Camera.target.z = pos.z;
		}
		else
		{
			auto shift = (bounds->X1 + bounds->X2 + bounds->Z1 + bounds->Z2) / 4;
			x = item->Position.xPos + shift * phd_sin(item->Position.yRot);
			z = item->Position.zPos + shift * phd_cos(item->Position.yRot);

			Camera.target.x = x;
			Camera.target.z = z;

			if (item->ObjectNumber == ID_LARA)
			{
				ConfirmCameraTargetPos();
				x = Camera.target.x;
				y = Camera.target.y;
				z = Camera.target.z;
			}
		}

		if (fixedCamera == Camera.fixedCamera)
		{
			Camera.fixedCamera = false;
			if (Camera.speed != 1 &&
				Camera.oldType != CameraType::Look &&
				BinocularOn >= 0)
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

		Camera.target.roomNumber = GetCollisionResult(x, y, z, Camera.target.roomNumber).RoomNumber;

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
		Camera.targetDistance = 1536;
		Camera.flags = 0;
		Camera.laraNode = -1;
	}
}

void LookLeftRight(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	Camera.type = CameraType::Look;
	if (TrInput & IN_LEFT)
	{
		TrInput &= ~IN_LEFT;
		if (lara->ExtraHeadRot.yRot > -ANGLE(44.0f))
		{
			if (BinocularRange)
				lara->ExtraHeadRot.yRot += ANGLE(2.0f) * (BinocularRange - 1792) / 1536;
			else
				lara->ExtraHeadRot.yRot -= ANGLE(2.0f);
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		TrInput &= ~IN_RIGHT;
		if (lara->ExtraHeadRot.yRot < ANGLE(44.0f))
		{
			if (BinocularRange)
				lara->ExtraHeadRot.yRot += ANGLE(2.0f) * (1792 - BinocularRange) / 1536;
			else
				lara->ExtraHeadRot.yRot += ANGLE(2.0f);
		}
	}
	if (lara->Control.HandStatus != HandStatus::Busy &&
		lara->Vehicle == NO_ITEM &&
		!lara->LeftArm.Locked &&
		!lara->RightArm.Locked)
	{
		lara->ExtraTorsoRot.yRot = lara->ExtraHeadRot.yRot;
	}
}

void LookUpDown(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	Camera.type = CameraType::Look;
	if (TrInput & IN_FORWARD)
	{
		TrInput &= ~IN_FORWARD;
		if (lara->ExtraHeadRot.xRot > -ANGLE(35.0f))
		{
			if (BinocularRange)
				lara->ExtraHeadRot.xRot += ANGLE(2.0f) * (BinocularRange - 1792) / 3072;
			else
				lara->ExtraHeadRot.xRot -= ANGLE(2.0f);
		}
	}
	else if (TrInput & IN_BACK)
	{
		TrInput &= ~IN_BACK;
		if (lara->ExtraHeadRot.xRot < ANGLE(30.0f))
		{
			if (BinocularRange)
				lara->ExtraHeadRot.xRot += ANGLE(2.0f) * (1792 - BinocularRange) / 3072;
			else
				lara->ExtraHeadRot.xRot += ANGLE(2.0f);
		}
	}
	if (lara->Control.HandStatus != HandStatus::Busy &&
		lara->Vehicle == NO_ITEM &&
		!lara->LeftArm.Locked &&
		!lara->RightArm.Locked)
	{
		lara->ExtraTorsoRot.xRot = lara->ExtraHeadRot.xRot;
	}
}

void ResetLook(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	if (Camera.type != CameraType::Look)
	{
		if (abs(lara->ExtraHeadRot.xRot) > ANGLE(0.1f))
			lara->ExtraHeadRot.xRot += lara->ExtraHeadRot.xRot / -8;
		else
			lara->ExtraHeadRot.xRot = 0;

		if (abs(lara->ExtraHeadRot.yRot) > ANGLE(0.1f))
			lara->ExtraHeadRot.yRot += lara->ExtraHeadRot.yRot / -8;
		else
			lara->ExtraHeadRot.yRot = 0;

		if (abs(lara->ExtraHeadRot.zRot) > ANGLE(0.1f))
			lara->ExtraHeadRot.zRot += lara->ExtraHeadRot.zRot / -8;
		else
			lara->ExtraHeadRot.zRot = 0;

		if (lara->Control.HandStatus != HandStatus::Busy &&
			!lara->LeftArm.Locked &&
			!lara->RightArm.Locked &&
			lara->Vehicle == NO_ITEM)
		{
			lara->ExtraTorsoRot.xRot = lara->ExtraHeadRot.xRot;
			lara->ExtraTorsoRot.yRot = lara->ExtraHeadRot.yRot;
			lara->ExtraTorsoRot.zRot = lara->ExtraHeadRot.zRot;
		}
		else
		{
			if (!lara->ExtraHeadRot.xRot)
				lara->ExtraTorsoRot.xRot = 0;
			if (!lara->ExtraHeadRot.yRot)
				lara->ExtraTorsoRot.yRot = 0;
			if (!lara->ExtraHeadRot.zRot)
				lara->ExtraTorsoRot.zRot = 0;
		}
	}
}

bool TestBoundsCollideCamera(BOUNDING_BOX* bounds, PHD_3DPOS* pos, short radius)
{
	auto dxBox = TO_DX_BBOX(*pos, bounds);
	auto sphere = BoundingSphere(Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z), radius);
	return sphere.Intersects(dxBox);
}

void ItemPushCamera(BOUNDING_BOX* bounds, PHD_3DPOS* pos, short radius)
{
	int dx = Camera.pos.x - pos->xPos;
	int dz = Camera.pos.z - pos->zPos;
	auto sin = phd_sin(pos->yRot);
	auto cos = phd_cos(pos->yRot);
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

	Camera.pos.x = pos->xPos + (cos * x + sin * z);
	Camera.pos.z = pos->zPos + (cos * z - sin * x);

	auto coll = GetCollisionResult(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber);
	if (coll.Position.Floor == NO_HEIGHT || Camera.pos.y > coll.Position.Floor || Camera.pos.y < coll.Position.Ceiling)
	{
		Camera.pos.x = CamOldPos.x;
		Camera.pos.y = CamOldPos.y;
		Camera.pos.z = CamOldPos.z;
		Camera.pos.roomNumber = coll.RoomNumber;
	}
}

static bool CheckItemCollideCamera(ITEM_INFO* item)
{
	auto dx = Camera.pos.x - item->Position.xPos;
	auto dy = Camera.pos.y - item->Position.yPos;
	auto dz = Camera.pos.z - item->Position.zPos;

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
	Vector3 extents = TO_DX_BBOX(item->Position, GetBoundsAccurate(item)).Extents;
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
	auto dx = Camera.pos.x - mesh->pos.xPos;
	auto dy = Camera.pos.y - mesh->pos.yPos;
	auto dz = Camera.pos.z - mesh->pos.zPos;

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

		auto dx = abs(LaraItem->Position.xPos - item->Position.xPos);
		auto dy = abs(LaraItem->Position.yPos - item->Position.yPos);
		auto dz = abs(LaraItem->Position.zPos - item->Position.zPos);

		// If camera is stuck behind some item, and Lara runs off somewhere
		if (dx > COLL_CANCEL_THRESHOLD || dz > COLL_CANCEL_THRESHOLD || dy > COLL_CANCEL_THRESHOLD)
			continue;

		auto bounds = GetBoundsAccurate(item);
		if (TestBoundsCollideCamera(bounds, &item->Position, CAMERA_RADIUS))
			ItemPushCamera(bounds, &item->Position, rad);

#ifdef _DEBUG
		TEN::Renderer::g_Renderer.addDebugBox(TO_DX_BBOX(item->Position, bounds),
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

		auto dx = abs(LaraItem->Position.xPos - mesh->pos.xPos);
		auto dy = abs(LaraItem->Position.yPos - mesh->pos.yPos);
		auto dz = abs(LaraItem->Position.zPos - mesh->pos.zPos);

		if (dx > COLL_CANCEL_THRESHOLD || dz > COLL_CANCEL_THRESHOLD || dy > COLL_CANCEL_THRESHOLD)
			continue;

		auto bounds = &stat->collisionBox;
		if (TestBoundsCollideCamera(bounds, &mesh->pos, CAMERA_RADIUS))
			ItemPushCamera(bounds, &mesh->pos, rad);

#ifdef _DEBUG
		TEN::Renderer::g_Renderer.addDebugBox(TO_DX_BBOX(mesh->pos, bounds),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f), RENDERER_DEBUG_PAGE::DIMENSION_STATS);
#endif
	}

	staticList.clear(); // Done
}

void RumbleScreen()
{
	if (!(GlobalCounter & 0x1FF))
		SoundEffect(SFX_TR5_KLAXON, 0, 4104);

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
