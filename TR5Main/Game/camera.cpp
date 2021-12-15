#include "framework.h"
#include "camera.h"
#include "animation.h"
#include "lara.h"
#include "effects/effects.h"
#include "effects/debris.h"
#include "lara_fire.h"
#include "effects/weather.h"
#include "level.h"
#include "setup.h"
#include "collide.h"
#include "room.h"
#include "Sound/sound.h"
#include "control/los.h"
#include "savegame.h"
#include "input.h"
#include "items.h"
#include "Objects/Generic/Object/burning_torch.h"

using TEN::Renderer::g_Renderer;
using namespace TEN::Entities::Generic;
using namespace TEN::Effects::Environment;

constexpr auto COLL_CHECK_THRESHOLD   = SECTOR(4);
constexpr auto COLL_CANCEL_THRESHOLD  = SECTOR(2);
constexpr auto COLL_DISCARD_THRESHOLD = CLICK(0.5f);
constexpr auto CAMERA_RADIUS          = CLICK(1);

struct OLD_CAMERA
{
	short currentAnimState;
	short goalAnimState;
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
CAMERA_TYPE BinocularOldCamera;
bool LaserSight;
int PhdPerspective;
short CurrentFOV;

int RumbleTimer = 0;
int RumbleCounter = 0;

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
	Camera.shift = LaraItem->pos.yPos - WALL_SIZE;
	
	LastTarget.x = LaraItem->pos.xPos;
	LastTarget.y = Camera.shift;
	LastTarget.z = LaraItem->pos.zPos;
	LastTarget.roomNumber = LaraItem->roomNumber;

	Camera.target.x = LastTarget.x;
	Camera.target.y = Camera.shift;
	Camera.target.z = LastTarget.z;
	Camera.target.roomNumber = LaraItem->roomNumber;

	Camera.pos.x = LastTarget.x;
	Camera.pos.y = Camera.shift;
	Camera.pos.z = LastTarget.z - 100;
	Camera.pos.roomNumber = LaraItem->roomNumber;

	Camera.targetDistance = WALL_SIZE + STEP_SIZE * 2;
	Camera.item = NULL;
	Camera.numberFrames = 1;
	Camera.type = CAMERA_TYPE::CHASE_CAMERA;
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

	if (OldCam.pos.xRot != LaraItem->pos.xRot ||
		OldCam.pos.yRot != LaraItem->pos.yRot ||
		OldCam.pos.zRot != LaraItem->pos.zRot ||
		OldCam.pos2.xRot != Lara.headXrot ||
		OldCam.pos2.yRot != Lara.headYrot ||
		OldCam.pos2.xPos != Lara.torsoXrot ||
		OldCam.pos2.yPos != Lara.torsoYrot ||
		OldCam.pos.xPos != LaraItem->pos.xPos ||
		OldCam.pos.yPos != LaraItem->pos.yPos ||
		OldCam.pos.zPos != LaraItem->pos.zPos ||
		OldCam.currentAnimState != LaraItem->currentAnimState ||
		OldCam.goalAnimState != LaraItem->goalAnimState ||
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
		OldCam.pos.xRot = LaraItem->pos.xRot;
		OldCam.pos.yRot = LaraItem->pos.yRot;
		OldCam.pos.zRot = LaraItem->pos.zRot;
		OldCam.pos2.xRot = Lara.headXrot;
		OldCam.pos2.yRot = Lara.headYrot;
		OldCam.pos2.xPos = Lara.torsoXrot;
		OldCam.pos2.yPos = Lara.torsoYrot;
		OldCam.pos.xPos = LaraItem->pos.xPos;
		OldCam.pos.yPos = LaraItem->pos.yPos;
		OldCam.pos.zPos = LaraItem->pos.zPos;
		OldCam.currentAnimState = LaraItem->currentAnimState;
		OldCam.goalAnimState = LaraItem->goalAnimState;
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

	auto roomNum = Camera.pos.roomNumber;
	auto yPos = Camera.pos.y;

	if (g_Level.Rooms[roomNum].flags & ENV_FLAG_SWAMP)
		yPos = g_Level.Rooms[roomNum].y - STEP_SIZE;

	FLOOR_INFO* floor = GetFloor(Camera.pos.x, yPos, Camera.pos.z, &roomNum);
	auto floorHeight = GetFloorHeight(floor, Camera.pos.x, yPos, Camera.pos.z);

	if (yPos < GetCeiling(floor, Camera.pos.x, yPos, Camera.pos.z) ||
		yPos > floorHeight)
	{
		LOSAndReturnTarget(&Camera.target, &Camera.pos, 0);
		
		if (abs(Camera.pos.x - ideal->x) < (WALL_SIZE - STEP_SIZE) &&
			abs(Camera.pos.y - ideal->y) < (WALL_SIZE - STEP_SIZE) &&
			abs(Camera.pos.z - ideal->z) < (WALL_SIZE - STEP_SIZE))
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

	roomNum = Camera.pos.roomNumber;
	floor = GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &roomNum);
	floorHeight = GetFloorHeight(floor, Camera.pos.x, Camera.pos.y, Camera.pos.z);
	auto ceilingHeight = GetCeiling(floor, Camera.pos.x, Camera.pos.y, Camera.pos.z);

	auto buffer = STEP_SIZE - 1;
	if ((Camera.pos.y - buffer) < ceilingHeight &&
		(Camera.pos.y + buffer) > floorHeight &&
		ceilingHeight < floorHeight &&
		ceilingHeight != NO_HEIGHT &&
		floorHeight != NO_HEIGHT)
	{
		Camera.pos.y = (floorHeight + ceilingHeight) >> 1;
	}
	else if ((Camera.pos.y + buffer) > floorHeight &&
		ceilingHeight < floorHeight &&
		ceilingHeight != NO_HEIGHT &&
		floorHeight != NO_HEIGHT)
	{
		Camera.pos.y = floorHeight - buffer;
	}
	else if ((Camera.pos.y - buffer) < ceilingHeight &&
		ceilingHeight < floorHeight &&
		ceilingHeight != NO_HEIGHT &&
		floorHeight != NO_HEIGHT)
	{
		Camera.pos.y = ceilingHeight + buffer;
	}
	else if (ceilingHeight >= floorHeight ||
		floorHeight == NO_HEIGHT ||
		ceilingHeight == NO_HEIGHT)
	{
		Camera.pos.x = ideal->x;
		Camera.pos.y = ideal->y;
		Camera.pos.z = ideal->z;
		Camera.pos.roomNumber = ideal->roomNumber;
	}

	ItemsCollideCamera();

	GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &Camera.pos.roomNumber);
	LookAt(&Camera, 0);
	
	if (Camera.mikeAtLara)
	{
		Camera.mikePos.x = LaraItem->pos.xPos;
		Camera.mikePos.y = LaraItem->pos.yPos;
		Camera.mikePos.z = LaraItem->pos.zPos;
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

	Camera.targetElevation += item->pos.xRot;
	UpdateCameraElevation();
	
	if (Camera.actualElevation > ANGLE(85.0f))
		Camera.actualElevation = ANGLE(85.0f);
	else if (Camera.actualElevation < -ANGLE(85.0f))
		Camera.actualElevation = -ANGLE(85.0f);

	int distance = Camera.targetDistance * phd_cos(Camera.actualElevation);

	short roomNum = Camera.target.roomNumber;
	GetFloor(Camera.target.x, Camera.target.y + STEP_SIZE, Camera.target.z, &roomNum);
	
	if (g_Level.Rooms[roomNum].flags & ENV_FLAG_SWAMP)
		Camera.target.y = g_Level.Rooms[roomNum].y - STEP_SIZE;

	auto x = Camera.target.x;
	auto y = Camera.target.y;
	auto z = Camera.target.z;

	roomNum = Camera.target.roomNumber;
	auto floor = GetFloor(x, y, z, &roomNum);
	auto floorHeight = GetFloorHeight(floor, x, y, z);
	auto ceilingHeight = GetCeiling(floor, x, y, z);

	if (((y < ceilingHeight || floorHeight < y) || floorHeight <= ceilingHeight) ||
		(floorHeight == NO_HEIGHT || ceilingHeight == NO_HEIGHT))
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

	int farthest = 0x7FFFFFFF;
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

	CameraCollisionBounds(&ideal, (STEP_SIZE + STEP_SIZE / 2), 1);
	MoveCamera(&ideal, Camera.speed);
}

void UpdateCameraElevation()
{

	if (Camera.laraNode != -1)
	{
		PHD_VECTOR pos = { 0, 0, 0 };
		GetLaraJointPosition(&pos, Camera.laraNode);

		PHD_VECTOR pos1 = { 0, -STEP_SIZE, WALL_SIZE * 2 };
		GetLaraJointPosition(&pos1, Camera.laraNode);

		pos.z = pos1.z - pos.z;
		pos.x = pos1.x - pos.x;
		Camera.actualAngle = Camera.targetAngle + phd_atan(pos.z, pos.x);
	}
	else
		Camera.actualAngle = LaraItem->pos.yRot + Camera.targetAngle;

	Camera.actualElevation += (Camera.targetElevation - Camera.actualElevation) >> 3;
}

void CombatCamera(ITEM_INFO* item)
{
	Camera.target.x = item->pos.xPos;
	Camera.target.z = item->pos.zPos;
	
	if (Lara.target)
	{
		Camera.targetAngle = Lara.targetAngles[0];
		Camera.targetElevation = Lara.targetAngles[1] + item->pos.xRot;
	}
	else
	{
		Camera.targetAngle = Lara.headYrot + Lara.torsoYrot;
		Camera.targetElevation = Lara.headXrot + Lara.torsoXrot + item->pos.xRot - ANGLE(15.0f);
	}

	auto roomNum = Camera.target.roomNumber;
	GetFloor(Camera.target.x, Camera.target.y + STEP_SIZE, Camera.target.z, &roomNum);

	if (g_Level.Rooms[roomNum].flags & ENV_FLAG_SWAMP)
		Camera.target.y = g_Level.Rooms[roomNum].y - STEP_SIZE;

	auto floor = GetFloor(Camera.target.x, Camera.target.y, Camera.target.z, &Camera.target.roomNumber);
	auto floorHeight = GetFloorHeight(floor, Camera.target.x, Camera.target.y, Camera.target.z);
	auto ceilingHeight = GetCeiling(floor, Camera.target.x, Camera.target.y, Camera.target.z);
	
	auto buffer = STEP_SIZE / 4;
	if ((ceilingHeight + buffer) > (floorHeight - buffer) &&
		floorHeight != NO_HEIGHT &&
		ceilingHeight != NO_HEIGHT)
	{
		Camera.target.y = (ceilingHeight + floorHeight) >> 1;
		Camera.targetElevation = 0;
	}
	else if (Camera.target.y > (floorHeight - buffer) &&
		floorHeight != NO_HEIGHT)
	{
		Camera.target.y = floorHeight - buffer;
		Camera.targetElevation = 0;
	}
	else if (Camera.target.y < (ceilingHeight + buffer) &&
		ceilingHeight != NO_HEIGHT)
	{
		Camera.target.y = ceilingHeight + buffer;
		Camera.targetElevation = 0;
	}

	GetFloor(Camera.target.x, Camera.target.y, Camera.target.z, &Camera.target.roomNumber);
	
	int x = Camera.target.x;
	int y = Camera.target.y;
	int z = Camera.target.z;

	roomNum = Camera.target.roomNumber;
	floor = GetFloor(Camera.target.x, Camera.target.y, Camera.target.z, &roomNum);
	floorHeight = GetFloorHeight(floor, x, y, z);
	ceilingHeight = GetCeiling(floor, x, y, z);
	
	if (y < ceilingHeight ||
		y > floorHeight ||
		ceilingHeight >= floorHeight ||
		floorHeight == NO_HEIGHT ||
		ceilingHeight == NO_HEIGHT)
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

	Camera.targetDistance = WALL_SIZE + STEP_SIZE * 2;
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

	CameraCollisionBounds(&ideal, (STEP_SIZE + STEP_SIZE / 2), 1);

	if (Camera.oldType == CAMERA_TYPE::FIXED_CAMERA)
		Camera.speed = 1;

	MoveCamera(&ideal, Camera.speed);
}

bool CameraCollisionBounds(GAME_VECTOR* ideal, int push, int yFirst)
{
	auto x = ideal->x;
	auto y = ideal->y;
	auto z = ideal->z;

	FLOOR_INFO* floor;
	short roomNum;
	int floorHeight, ceilingHeight;
	
	if (yFirst)
	{
		roomNum = ideal->roomNumber;
		floor = GetFloor(x, y, z, &roomNum);
		floorHeight = GetFloorHeight(floor, x, y, z);
		ceilingHeight = GetCeiling(floor, x, y, z);

		auto buffer = STEP_SIZE - 1;
		if ((y - buffer) < ceilingHeight &&
			(y + buffer) > floorHeight &&
			ceilingHeight < floorHeight &&
			ceilingHeight != NO_HEIGHT &&
			floorHeight != NO_HEIGHT)
		{
			y = (floorHeight + ceilingHeight) >> 1;
		}
		else if ((y + buffer) > floorHeight &&
			ceilingHeight < floorHeight &&
			ceilingHeight != NO_HEIGHT &&
			floorHeight != NO_HEIGHT)
		{
			y = floorHeight - buffer;
		}
		else if ((y - buffer) < ceilingHeight &&
			ceilingHeight < floorHeight &&
			ceilingHeight != NO_HEIGHT &&
			floorHeight != NO_HEIGHT)
		{
			y = ceilingHeight + buffer;
		}
	}

	roomNum = ideal->roomNumber;
	floor = GetFloor(x - push, y, z, &roomNum);
	floorHeight = GetFloorHeight(floor, x - push, y, z);
	ceilingHeight = GetCeiling(floor, x - push, y, z);
	if (y > floorHeight ||
		floorHeight == NO_HEIGHT ||
		ceilingHeight == NO_HEIGHT ||
		ceilingHeight >= floorHeight ||
		y < ceilingHeight)
	{
		x = (x & (~1023)) + push;
	}

	roomNum = ideal->roomNumber;
	floor = GetFloor(x, y, z - push, &roomNum);
	floorHeight = GetFloorHeight(floor, x, y, z - push);
	ceilingHeight = GetCeiling(floor, x, y, z - push);
	if (y > floorHeight ||
		floorHeight == NO_HEIGHT ||
		ceilingHeight == NO_HEIGHT ||
		ceilingHeight >= floorHeight ||
		y < ceilingHeight)
	{
		z = (z & (~1023)) + push;
	}

	roomNum = ideal->roomNumber;
	floor = GetFloor(x + push, y, z, &roomNum);
	floorHeight = GetFloorHeight(floor, x + push, y, z);
	ceilingHeight = GetCeiling(floor, x + push, y, z);
	if (y > floorHeight ||
		floorHeight == NO_HEIGHT ||
		ceilingHeight == NO_HEIGHT ||
		ceilingHeight >= floorHeight ||
		y < ceilingHeight)
	{
		x = (x | 1023) - push;
	}

	roomNum = ideal->roomNumber;
	floor = GetFloor(x, y, z + push, &roomNum);
	floorHeight = GetFloorHeight(floor, x, y, z + push);
	ceilingHeight = GetCeiling(floor, x, y, z + push);
	if (y > floorHeight ||
		floorHeight == NO_HEIGHT ||
		ceilingHeight == NO_HEIGHT ||
		ceilingHeight >= floorHeight ||
		y < ceilingHeight)
	{
		z = (z | 1023) - push;
	}

	if (!yFirst)
	{
		roomNum = ideal->roomNumber;
		floor = GetFloor(x, y, z, &roomNum);
		floorHeight = GetFloorHeight(floor, x, y, z);
		ceilingHeight = GetCeiling(floor, x, y, z);

		auto buffer = STEP_SIZE - 1;
		if ((y - buffer) < ceilingHeight &&
			(y + buffer) > floorHeight &&
			ceilingHeight < floorHeight &&
			ceilingHeight != NO_HEIGHT &&
			floorHeight != NO_HEIGHT)
		{
			y = (floorHeight + ceilingHeight) >> 1;
		}
		else if ((y + buffer) > floorHeight &&
			ceilingHeight < floorHeight &&
			ceilingHeight != NO_HEIGHT &&
			floorHeight != NO_HEIGHT)
		{
			y = floorHeight - buffer;
		}
		else if ((y - buffer) < ceilingHeight &&
			ceilingHeight < floorHeight &&
			ceilingHeight != NO_HEIGHT &&
			floorHeight != NO_HEIGHT)
		{
			y = ceilingHeight + buffer;
		}
	}

	roomNum = ideal->roomNumber;
	floor = GetFloor(x, y, z, &roomNum);
	floorHeight = GetFloorHeight(floor, x, y, z);
	ceilingHeight = GetCeiling(floor, x, y, z);
	if (y > floorHeight ||
		y < ceilingHeight ||
		floorHeight == NO_HEIGHT ||
		ceilingHeight == NO_HEIGHT ||
		ceilingHeight >= floorHeight)
	{
		return true;
	}

	floor = GetFloor(x, y, z, &ideal->roomNumber);
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
	auto headXrot = Lara.headXrot;
	auto headYrot = Lara.headYrot;
	auto torsoXrot = Lara.torsoXrot;
	auto torsoYrot = Lara.torsoYrot;

	Lara.torsoXrot = 0;
	Lara.torsoYrot = 0;
	Lara.headXrot <<= 1;
	Lara.headYrot <<= 1;

	if (Lara.headXrot > ANGLE(55.0f)) 
		Lara.headXrot = ANGLE(55.0f);
	else if (Lara.headXrot < -ANGLE(75.0f))
		Lara.headXrot = -ANGLE(75.0f);
	if (Lara.headYrot < -ANGLE(80.0f))
		Lara.headYrot = -ANGLE(80.0f);
	else if (Lara.headYrot > ANGLE(80.0f))
		Lara.headYrot = ANGLE(80.0f);

	if (abs(Lara.headXrot - OldCam.pos.xRot) >= 16)
		OldCam.pos.xRot = (Lara.headXrot + OldCam.pos.xRot) >> 1;
	else
		OldCam.pos.xRot = Lara.headXrot;
	if (abs(Lara.headYrot - OldCam.pos.yRot) >= 16)
		OldCam.pos.yRot = (Lara.headYrot + OldCam.pos.yRot) >> 1;
	else
		OldCam.pos.yRot = Lara.headYrot;

	PHD_VECTOR pos = { 0, STEP_SIZE / 16, STEP_SIZE / 4 };
	GetLaraJointPosition(&pos, LM_HEAD);
	
	short roomNum = LaraItem->roomNumber;
	FLOOR_INFO* floor = GetFloor(pos.x, pos.y, pos.z, &roomNum);
	auto floorHeight = GetFloorHeight(floor, pos.x, pos.y, pos.z);
	auto ceilingHeight = GetCeiling(floor, pos.x, pos.y, pos.z);
	if (floorHeight == NO_HEIGHT ||
		ceilingHeight == NO_HEIGHT ||
		ceilingHeight >= floorHeight ||
		pos.y > floorHeight ||
		pos.y < ceilingHeight)
	{
		pos = { 0, STEP_SIZE / 16 , 0 };
		GetLaraJointPosition(&pos, LM_HEAD);
		
		roomNum = LaraItem->roomNumber;
		floor = GetFloor(pos.x, pos.y + STEP_SIZE, pos.z, &roomNum);
		if (g_Level.Rooms[roomNum].flags & ENV_FLAG_SWAMP)
		{
			pos.y = g_Level.Rooms[roomNum].y - STEP_SIZE;
			floor = GetFloor(pos.x, pos.y, pos.z, &roomNum);
		}
		else
			floor = GetFloor(pos.x, pos.y, pos.z, &roomNum);

		floorHeight = GetFloorHeight(floor, pos.x, pos.y, pos.z);
		ceilingHeight = GetCeiling(floor, pos.x, pos.y, pos.z);
		if (floorHeight == NO_HEIGHT ||
			ceilingHeight == NO_HEIGHT ||
			ceilingHeight >= floorHeight ||
			pos.y > floorHeight ||
			pos.y < ceilingHeight)
		{
			pos.x = 0;
			pos.y = STEP_SIZE / 16;
			pos.z = -(STEP_SIZE / 4);
			GetLaraJointPosition(&pos, LM_HEAD);
		}
	}

	PHD_VECTOR pos2 = { 0, 0, -WALL_SIZE };
	GetLaraJointPosition(&pos2, LM_HEAD);

	PHD_VECTOR pos3 = { 0, 0, WALL_SIZE * 2 };
	GetLaraJointPosition(&pos3, LM_HEAD);

	int dx = (pos2.x - pos.x) >> 3;
	int dy = (pos2.y - pos.y) >> 3;
	int dz = (pos2.z - pos.z) >> 3;
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;

	short roomNumber2 = LaraItem->roomNumber;
	int i = 0;
	for (i = 0; i < 8; i++)
	{
		roomNum = roomNumber2;
		floor = GetFloor(x, y + STEP_SIZE, z, &roomNumber2);
		if (g_Level.Rooms[roomNumber2].flags & ENV_FLAG_SWAMP)
		{
			y = g_Level.Rooms[roomNumber2].y - STEP_SIZE;

			break;
		}
		else
			floor = GetFloor(x, y, z, &roomNumber2);

		floorHeight = GetFloorHeight(floor, x, y, z);
		ceilingHeight = GetCeiling(floor, x, y, z);
		if (floorHeight == NO_HEIGHT ||
			ceilingHeight == NO_HEIGHT ||
			ceilingHeight >= floorHeight ||
			y > floorHeight ||
			y < ceilingHeight)
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

	if (OldCam.pos.xRot == Lara.headXrot &&
		OldCam.pos.yRot == Lara.headYrot &&
		OldCam.pos.xPos == LaraItem->pos.xPos &&
		OldCam.pos.yPos == LaraItem->pos.yPos &&
		OldCam.pos.zPos == LaraItem->pos.zPos &&
		OldCam.currentAnimState == LaraItem->currentAnimState &&
		OldCam.goalAnimState == LaraItem->goalAnimState &&
		Camera.oldType == CAMERA_TYPE::LOOK_CAMERA)
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
		OldCam.pos.xRot = Lara.headXrot;
		OldCam.pos.yRot = Lara.headYrot;
		OldCam.pos.xPos = LaraItem->pos.xPos;
		OldCam.pos.yPos = LaraItem->pos.yPos;
		OldCam.pos.zPos = LaraItem->pos.zPos;
		OldCam.currentAnimState = LaraItem->currentAnimState;
		OldCam.goalAnimState = LaraItem->goalAnimState;
		LookCamPosition.x = ideal.x;
		LookCamPosition.y = ideal.y;
		LookCamPosition.z = ideal.z;
		LookCamPosition.roomNumber = ideal.roomNumber;
		LookCamTarget.x = pos3.x;
		LookCamTarget.y = pos3.y;
		LookCamTarget.z = pos3.z;
	}

	CameraCollisionBounds(&ideal, (STEP_SIZE - STEP_SIZE / 8), 1);

	if (Camera.oldType == CAMERA_TYPE::FIXED_CAMERA)
	{
		Camera.pos.x = ideal.x;
		Camera.pos.y = ideal.y;
		Camera.pos.z = ideal.z;
		Camera.target.x = pos3.x;
		Camera.target.y = pos3.y;
		Camera.target.z = pos3.z;
		Camera.target.roomNumber = LaraItem->roomNumber;
	}
	else
	{
		Camera.pos.x += (ideal.x - Camera.pos.x) >> 2;
		Camera.pos.y += (ideal.y - Camera.pos.y) >> 2;
		Camera.pos.z += (ideal.z - Camera.pos.z) >> 2;
		Camera.target.x += (pos3.x - Camera.target.x) >> 2;
		Camera.target.y += (pos3.y - Camera.target.y) >> 2;
		Camera.target.z += (pos3.z - Camera.target.z) >> 2;
		Camera.target.roomNumber = LaraItem->roomNumber;
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

	x = Camera.pos.x;
	y = Camera.pos.y;
	z = Camera.pos.z;
	roomNum = Camera.pos.roomNumber;
	floor = GetFloor(x, y, z, &roomNum);
	floorHeight = GetFloorHeight(floor, x, y, z);
	ceilingHeight = GetCeiling(floor, x, y, z);

	auto buffer = STEP_SIZE - 1;
	if ((y - buffer) < ceilingHeight &&
		(y + buffer) > floorHeight &&
		ceilingHeight < floorHeight &&
		ceilingHeight != NO_HEIGHT && floorHeight != NO_HEIGHT)
	{
		Camera.pos.y = (floorHeight + ceilingHeight) >> 1;
	}
	else if ((y + buffer) > floorHeight &&
		ceilingHeight < floorHeight &&
		ceilingHeight != NO_HEIGHT &&
		floorHeight != NO_HEIGHT)
	{
		Camera.pos.y = floorHeight - buffer;
	}
	else if ((y - buffer) < ceilingHeight &&
		ceilingHeight < floorHeight &&
		ceilingHeight != NO_HEIGHT &&
		floorHeight != NO_HEIGHT)
	{
		Camera.pos.y = ceilingHeight + buffer;
	}

	x = Camera.pos.x;
	y = Camera.pos.y;
	z = Camera.pos.z;
	roomNum = Camera.pos.roomNumber;
	floor = GetFloor(x, y, z, &roomNum);
	floorHeight = GetFloorHeight(floor, x, y, z);
	ceilingHeight = GetCeiling(floor, x, y, z);
	if ((g_Level.Rooms[roomNum].flags & ENV_FLAG_SWAMP))
		Camera.pos.y = g_Level.Rooms[roomNum].y - STEP_SIZE;
	else if (y < ceilingHeight ||
		y > floorHeight ||
		ceilingHeight >= floorHeight ||
		floorHeight == NO_HEIGHT ||
		ceilingHeight == NO_HEIGHT)
	{
		LOSAndReturnTarget(&Camera.target, &Camera.pos, 0);
	}

	x = Camera.pos.x;
	y = Camera.pos.y;
	z = Camera.pos.z;
	roomNum = Camera.pos.roomNumber;
	floor = GetFloor(x, y, z, &roomNum);
	floorHeight = GetFloorHeight(floor, x, y, z);
	ceilingHeight = GetCeiling(floor, x, y, z);
	if (y < ceilingHeight ||
		y > floorHeight ||
		ceilingHeight >= floorHeight ||
		floorHeight == NO_HEIGHT ||
		ceilingHeight == NO_HEIGHT ||
		g_Level.Rooms[roomNum].flags & ENV_FLAG_SWAMP)
	{
		Camera.pos.x = pos.x;
		Camera.pos.y = pos.y;
		Camera.pos.z = pos.z;
		Camera.pos.roomNumber = LaraItem->roomNumber;
	}

	ItemsCollideCamera();

	GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &Camera.pos.roomNumber);
	LookAt(&Camera, 0);

	if (Camera.mikeAtLara)
	{
		Camera.actualAngle = LaraItem->pos.yRot + Lara.headYrot + Lara.torsoYrot;
		Camera.mikePos.x = LaraItem->pos.xPos;
		Camera.mikePos.y = LaraItem->pos.yPos;
		Camera.mikePos.z = LaraItem->pos.zPos;
	}
	else
	{
		Camera.actualAngle = phd_atan(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.mikePos.x = Camera.pos.x + PhdPerspective * phd_sin(Camera.actualAngle);
		Camera.mikePos.z = Camera.pos.z + PhdPerspective * phd_cos(Camera.actualAngle);
		Camera.mikePos.y = Camera.pos.y;
	}

	Camera.oldType = Camera.type;

	Lara.headXrot = headXrot;
	Lara.headYrot = headYrot;
	Lara.torsoXrot = torsoXrot;
	Lara.torsoYrot = torsoYrot;
}

void BounceCamera(ITEM_INFO* item, short bounce, short maxDistance)
{
	int distance = sqrt(
		SQUARE(item->pos.xPos - Camera.pos.x) +
		SQUARE(item->pos.yPos - Camera.pos.y) +
		SQUARE(item->pos.zPos - Camera.pos.z));
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
	static int exittingBinos = 0;

	if (LSHKTimer)
		--LSHKTimer;

	if (!LaserSight)
	{
		if (InputBusy & IN_DRAW)
			exittingBinos = 1;
		else if (exittingBinos)
		{
			exittingBinos = 0;
			BinocularRange = 0;
			AlterFOV(14560);
			LaraItem->meshBits = -1;
			Lara.busy = false;
			Lara.headYrot = 0;
			Lara.headXrot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			Camera.type = BinocularOldCamera;

			return;
		}
	}

	LaraItem->meshBits = 0;
	AlterFOV(7 * (2080 - BinocularRange));

	short headXrot = Lara.headXrot * 2;
	short headYrot = Lara.headYrot;

	if (headXrot > ANGLE(75.0f))
		headXrot = ANGLE(75.0f);
	else if (headXrot < -ANGLE(75.0f))
		headXrot = -ANGLE(75.0f);

	if (headYrot > ANGLE(80.0f))
		headYrot = ANGLE(80.0f);
	else if (headYrot < -ANGLE(80.0f))
		headYrot = -ANGLE(80.0f);

	auto x = LaraItem->pos.xPos;
	auto y = LaraItem->pos.yPos - (WALL_SIZE / 2);
	auto z = LaraItem->pos.zPos;

	short roomNum = LaraItem->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNum);
	auto ceilingHeight = GetCeiling(floor, x, y, z);
	if (ceilingHeight <= (y - STEP_SIZE))
		y -= STEP_SIZE;
	else
		y = ceilingHeight + (STEP_SIZE / 4);

	Camera.pos.x = x;
	Camera.pos.y = y;
	Camera.pos.z = z;	
	Camera.pos.roomNumber = roomNum;
	
	int l = (WALL_SIZE * 20 + STEP_SIZE) * phd_cos(headXrot);
	
	int tx = x + l * phd_sin(LaraItem->pos.yRot + headYrot);
	int ty = y - (WALL_SIZE * 20 + STEP_SIZE) * phd_sin(headXrot);
	int tz = z + l * phd_cos(LaraItem->pos.yRot + headYrot);

	if (Camera.oldType == CAMERA_TYPE::FIXED_CAMERA)
	{
		Camera.target.x = tx;
		Camera.target.y = ty;
		Camera.target.z = tz;
		Camera.target.roomNumber = LaraItem->roomNumber;
	}
	else
	{
		Camera.target.x += (tx - Camera.target.x) >> 2;
		Camera.target.y += (ty - Camera.target.y) >> 2;
		Camera.target.z += (tz - Camera.target.z) >> 2;
		Camera.target.roomNumber = LaraItem->roomNumber;
	}
	
	if (Camera.bounce && Camera.type == Camera.oldType)
	{
		if (Camera.bounce <= 0)
		{
			Camera.target.x += (STEP_SIZE / 16) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1));
			Camera.target.y += (STEP_SIZE / 16) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1));
			Camera.target.z += (STEP_SIZE / 16) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1));
			Camera.bounce += 5;
		}
		else
		{
			Camera.bounce = 0;
			Camera.target.y += Camera.bounce;
		}
	}

	GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &Camera.pos.roomNumber);
	LookAt(&Camera, 0);

	if (Camera.mikeAtLara)
	{
		Camera.actualAngle = LaraItem->pos.yRot + Lara.headYrot + Lara.torsoYrot;
		Camera.mikePos.x = LaraItem->pos.xPos;
		Camera.mikePos.y = LaraItem->pos.yPos;
		Camera.mikePos.z = LaraItem->pos.zPos;
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
	else if (InputBusy & IN_DUCK)
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
		Ammo& ammo = GetAmmo(LaraItem, Lara.gunType);

		if (!(InputBusy & IN_ACTION) ||
			WeaponDelay ||
			!ammo)
		{
			if (!(InputBusy & IN_ACTION))
			{
				if (Lara.gunType != WEAPON_CROSSBOW)
					WeaponDelay = 0;

				LSHKShotsFired = 0;
				Camera.bounce = 0;
			}
		}
		else
		{
			if (Lara.gunType == WEAPON_REVOLVER)
			{
				firing = 1;
				WeaponDelay = 16;
				Statistics.Game.AmmoUsed++;

				if (!ammo.hasInfinite())
					(ammo)--;

				Camera.bounce = -16 - (GetRandomControl() & 0x1F);
			}
			else if (Lara.gunType == WEAPON_CROSSBOW)
			{
				firing = 1;
				WeaponDelay = 32;
			}
			else
			{
				if (Lara.Weapons[WEAPON_HK].SelectedAmmo == WEAPON_AMMO1)
				{
					WeaponDelay = 12;
					firing = 1;

					if (Lara.Weapons[WEAPON_HK].HasSilencer)
						SoundEffect(SFX_TR5_HK_SILENCED, 0, 0);
					else
					{
						SoundEffect(SFX_TR4_EXPLOSION1, 0, 83888140);
						SoundEffect(SFX_TR5_HK_FIRE, 0, 0);
					}
				}
				else if (Lara.Weapons[WEAPON_HK].SelectedAmmo == WEAPON_AMMO2)
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

						if (Lara.Weapons[WEAPON_HK].HasSilencer)
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

						if (Lara.Weapons[WEAPON_HK].HasSilencer)
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
						if (Lara.Weapons[WEAPON_HK].HasSilencer)
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

						if (Lara.Weapons[WEAPON_HK].HasSilencer)
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
			LaraTorch(&src, &target, Lara.headYrot, 192);
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
		Camera.target.x = LaraItem->pos.xPos;
		Camera.target.y = (Camera.target.y + pos.y) >> 1;
		Camera.target.z = LaraItem->pos.zPos;
	}

	auto x = Camera.target.x;
	auto y = Camera.target.y;
	auto z = Camera.target.z;

	short roomNumber = Camera.target.roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	auto floorHeight = GetFloorHeight(floor, x, y, z);
	auto ceilingHeight = GetCeiling(floor, x, y, z);

	if (y < ceilingHeight ||
		floorHeight < y ||
		floorHeight <= ceilingHeight ||
		floorHeight == NO_HEIGHT ||
		ceilingHeight == NO_HEIGHT)
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
		Camera.type = CAMERA_TYPE::FIXED_CAMERA;
		if (Camera.oldType != CAMERA_TYPE::FIXED_CAMERA)
			Camera.speed = 1;
	}

	// Camera is in a water room, play water sound effect.

	if (g_Level.Rooms[Camera.pos.roomNumber].flags & ENV_FLAG_WATER)
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
		(Camera.type == CAMERA_TYPE::FIXED_CAMERA || Camera.type == CAMERA_TYPE::HEAVY_CAMERA))
	{
		item = Camera.item;
		fixedCamera = true;
	}
	else
	{
		item = LaraItem;
		fixedCamera = false;
	}

	BOUNDING_BOX* bounds = GetBoundsAccurate(item);
	
	int x;
	int y = ((bounds->Y1 + bounds->Y2) / 2) + item->pos.yPos - STEP_SIZE;
	int z;

	if (Camera.item)
	{
		if (!fixedCamera)
		{
			auto dx = Camera.item->pos.xPos - item->pos.xPos;
			auto dz = Camera.item->pos.zPos - item->pos.zPos;
			int shift = sqrt(SQUARE(dx) + SQUARE(dz));
			short angle = phd_atan(dz, dx) - item->pos.yRot;
			short tilt = phd_atan(shift, y - (bounds->Y1 + bounds->Y2) / 2 - Camera.item->pos.yPos);
			bounds = GetBoundsAccurate(Camera.item);
			angle >>= 1;
			tilt >>= 1;

			if (angle > -ANGLE(50.0f) && angle < ANGLE(50.0f) && tilt > -ANGLE(85.0f) && tilt < ANGLE(85.0f))
			{
				short change = angle - Lara.headYrot;
				if (change > ANGLE(4.0f))
					Lara.headYrot += ANGLE(4.0f);
				else if (change < -ANGLE(4.0f))
					Lara.headYrot -= ANGLE(4.0f);
				else
					Lara.headYrot += change;
				Lara.torsoYrot = Lara.headYrot;

				change = tilt - Lara.headXrot;
				if (change > ANGLE(4.0f))
					Lara.headXrot += ANGLE(4.0f);
				else if (change < -ANGLE(4.0f))
					Lara.headXrot -= ANGLE(4.0f);
				else
					Lara.headXrot += change;
				Lara.torsoXrot = Lara.headXrot;

				Camera.type = CAMERA_TYPE::LOOK_CAMERA;
				Camera.item->lookedAt = 1;
			}
		}
	}

	if (Camera.type == CAMERA_TYPE::LOOK_CAMERA ||
		Camera.type == CAMERA_TYPE::COMBAT_CAMERA)
	{
		if (Camera.type == CAMERA_TYPE::COMBAT_CAMERA)
		{
			LastTarget.x = Camera.target.x;
			LastTarget.y = Camera.target.y;
			LastTarget.z = Camera.target.z;
			LastTarget.roomNumber = Camera.target.roomNumber;
		}

		Camera.target.roomNumber = item->roomNumber;

		if (Camera.fixedCamera || BinocularOn < 0)
		{
			Camera.target.y = y;
			Camera.speed = 1;
		}
		else
		{
			Camera.target.y += (y - Camera.target.y) >> 2;
			Camera.speed = Camera.type != CAMERA_TYPE::LOOK_CAMERA ? 8 : 4;
		}

		Camera.fixedCamera = false;
		if (Camera.type == CAMERA_TYPE::LOOK_CAMERA)
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

		Camera.target.roomNumber = item->roomNumber;
		Camera.target.y = y;

		if (Camera.type != CAMERA_TYPE::CHASE_CAMERA &&
			Camera.flags != CF_CHASE_OBJECT &&
			(Camera.number != -1 &&(SniperCamActive = g_Level.Cameras[Camera.number].flags & 3, g_Level.Cameras[Camera.number].flags & 2)))
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
			x = item->pos.xPos + shift * phd_sin(item->pos.yRot);
			z = item->pos.zPos + shift * phd_cos(item->pos.yRot);

			Camera.target.x = x;
			Camera.target.z = z;

			if (item->objectNumber == ID_LARA)
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
				Camera.oldType != CAMERA_TYPE::LOOK_CAMERA &&
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

		GetFloor(x, y, z, &Camera.target.roomNumber);

		if (abs(LastTarget.x - Camera.target.x) < 4 &&
			abs(LastTarget.y - Camera.target.y) < 4 &&
			abs(LastTarget.z - Camera.target.z) < 4)
		{
			Camera.target.x = LastTarget.x;
			Camera.target.y = LastTarget.y;
			Camera.target.z = LastTarget.z;
		}

		if (Camera.type != CAMERA_TYPE::CHASE_CAMERA && Camera.flags != CF_CHASE_OBJECT)
			FixedCamera(item);
		else
			ChaseCamera(item);
	}

	Camera.fixedCamera = fixedCamera;
	Camera.last = Camera.number;

	if (Camera.type != CAMERA_TYPE::HEAVY_CAMERA ||
		Camera.timer == -1)
	{
		Camera.type = CAMERA_TYPE::CHASE_CAMERA;
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

void LookLeftRight()
{
	Camera.type = CAMERA_TYPE::LOOK_CAMERA;
	if (TrInput & IN_LEFT)
	{
		TrInput &= ~IN_LEFT;
		if (Lara.headYrot > -ANGLE(44.0f))
		{
			if (BinocularRange)
				Lara.headYrot += ANGLE(2.0f) * (BinocularRange - 1792) / 1536;
			else
				Lara.headYrot -= ANGLE(2.0f);
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		TrInput &= ~IN_RIGHT;
		if (Lara.headYrot < ANGLE(44.0f))
		{
			if (BinocularRange)
				Lara.headYrot += ANGLE(2.0f) * (1792 - BinocularRange) / 1536;
			else
				Lara.headYrot += ANGLE(2.0f);
		}
	}
	if (Lara.gunStatus != LG_HANDS_BUSY &&
		Lara.Vehicle == NO_ITEM &&
		!Lara.leftArm.lock &&
		!Lara.rightArm.lock)
	{
		Lara.torsoYrot = Lara.headYrot;
	}
}

void LookUpDown()
{
	Camera.type = CAMERA_TYPE::LOOK_CAMERA;
	if (TrInput & IN_FORWARD)
	{
		TrInput &= ~IN_FORWARD;
		if (Lara.headXrot > -ANGLE(35.0f))
		{
			if (BinocularRange)
				Lara.headXrot += ANGLE(2.0f) * (BinocularRange - 1792) / 3072;
			else
				Lara.headXrot -= ANGLE(2.0f);
		}
	}
	else if (TrInput & IN_BACK)
	{
		TrInput &= ~IN_BACK;
		if (Lara.headXrot < ANGLE(30.0f))
		{
			if (BinocularRange)
				Lara.headXrot += ANGLE(2.0f) * (1792 - BinocularRange) / 3072;
			else
				Lara.headXrot += ANGLE(2.0f);
		}
	}
	if (Lara.gunStatus != LG_HANDS_BUSY &&
		Lara.Vehicle == NO_ITEM &&
		!Lara.leftArm.lock &&
		!Lara.rightArm.lock)
	{
		Lara.torsoXrot = Lara.headXrot;
	}
}

void ResetLook()
{
	if (Camera.type != CAMERA_TYPE::LOOK_CAMERA)
	{
		if (Lara.headXrot <= -ANGLE(2.0f) || Lara.headXrot >= ANGLE(2.0f))
			Lara.headXrot = Lara.headXrot / -8 + Lara.headXrot;
		else
			Lara.headXrot = 0;

		if (Lara.headYrot <= -ANGLE(2.0f) || Lara.headYrot >= ANGLE(2.0f))
			Lara.headYrot = Lara.headYrot / -8 + Lara.headYrot;
		else
			Lara.headYrot = 0;

		if (Lara.gunStatus != LG_HANDS_BUSY &&
			!Lara.leftArm.lock &&
			!Lara.rightArm.lock &&
			 Lara.Vehicle == NO_ITEM)
		{
			Lara.torsoYrot = Lara.headYrot;
			Lara.torsoXrot = Lara.headXrot;
		}
		else
		{
			if (!Lara.headXrot)
				Lara.torsoXrot = 0;
			if (!Lara.headYrot)
				Lara.torsoYrot = 0;
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
	auto dx = Camera.pos.x - pos->xPos;
	auto dz = Camera.pos.z - pos->zPos;
	auto sin = phd_sin(pos->yRot);
	auto cos = phd_cos(pos->yRot);
	auto x = dx * cos - dz * sin;
	auto z = dx * sin + dz * cos;

	auto xmin = bounds->X1 - radius;
	auto xmax = bounds->X2 + radius;
	auto zmin = bounds->Z1 - radius;
	auto zmax = bounds->Z2 + radius;

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
	auto dx = Camera.pos.x - item->pos.xPos;
	auto dy = Camera.pos.y - item->pos.yPos;
	auto dz = Camera.pos.z - item->pos.zPos;

	bool close_enough = dx > -COLL_CHECK_THRESHOLD && dx < COLL_CHECK_THRESHOLD &&
						dz > -COLL_CHECK_THRESHOLD && dz < COLL_CHECK_THRESHOLD && 
						dy > -COLL_CHECK_THRESHOLD && dy < COLL_CHECK_THRESHOLD;

	if (!close_enough || !item->collidable || !Objects[item->objectNumber].usingDrawAnimatingItem)
		return false;

	// TODO: Find a better way to define objects which are collidable with camera.
	auto obj = &Objects[item->objectNumber];
	if (obj->intelligent || obj->isPickup || obj->isPuzzleHole || obj->collision == nullptr)
		return false;

	// Check extents, if any 2 bounds are smaller than threshold, discard.
	Vector3 extents = TO_DX_BBOX(item->pos, GetBoundsAccurate(item)).Extents;
	if ((abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.y) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.y) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD))
		return false;

	return true;
}

std::vector<short> FillCollideableItemList()
{
	std::vector<short> itemList;
	auto roomList = CollectConnectedRooms(Camera.pos.roomNumber);

	for (short i = 0; i < g_Level.NumItems; i++)
	{
		auto item = &g_Level.Items[i];

		if (!roomList.count(item->roomNumber))
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

	bool close_enough = dx > -COLL_CHECK_THRESHOLD && dx < COLL_CHECK_THRESHOLD &&
						dz > -COLL_CHECK_THRESHOLD && dz < COLL_CHECK_THRESHOLD &&
						dy > -COLL_CHECK_THRESHOLD && dy < COLL_CHECK_THRESHOLD;

	if (!close_enough)
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
	auto roomList = CollectConnectedRooms(Camera.pos.roomNumber);

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

		auto dx = abs(LaraItem->pos.xPos - item->pos.xPos);
		auto dy = abs(LaraItem->pos.yPos - item->pos.yPos);
		auto dz = abs(LaraItem->pos.zPos - item->pos.zPos);

		// If camera is stuck behind some item, and Lara runs off somewhere
		if (dx > COLL_CANCEL_THRESHOLD || dz > COLL_CANCEL_THRESHOLD || dy > COLL_CANCEL_THRESHOLD)
			continue;

		auto bounds = GetBoundsAccurate(item);
		if (TestBoundsCollideCamera(bounds, &item->pos, CAMERA_RADIUS))
			ItemPushCamera(bounds, &item->pos, rad);

#ifdef _DEBUG
		TEN::Renderer::g_Renderer.addDebugBox(TO_DX_BBOX(item->pos, bounds),
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

		auto dx = abs(LaraItem->pos.xPos - mesh->pos.xPos);
		auto dy = abs(LaraItem->pos.yPos - mesh->pos.yPos);
		auto dz = abs(LaraItem->pos.zPos - mesh->pos.zPos);

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
