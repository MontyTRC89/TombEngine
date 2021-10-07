#include "framework.h"
#include "camera.h"
#include "animation.h"
#include "lara.h"
#include "effects\effects.h"
#include "effects\debris.h"
#include "lara_fire.h"
#include "lara.h"
#include "effects\weather.h"
#include "sphere.h"
#include "level.h"
#include "setup.h"
#include "collide.h"
#include "Sound\sound.h"
#include "control\los.h"
#include "savegame.h"
#include "input.h"
#include "items.h"
#include "Objects/Generic/Object/burning_torch.h"

using namespace TEN::Entities::Generic;
using TEN::Renderer::g_Renderer;
using namespace TEN::Effects::Environment;

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
int TLFlag = 0;
CAMERA_INFO Camera;
GAME_VECTOR ForcedFixedCamera;
int UseForcedFixedCamera;
int NumberCameras;
int SniperCameraActive;
int BinocularRange;
int BinocularOn;
CAMERA_TYPE BinocularOldCamera;
int LaserSight;
int SniperCount;
int PhdPerspective;
short CurrentFOV;
int GetLaraOnLOS;
int SniperOverlay;

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

int mgLOS(GAME_VECTOR* start, GAME_VECTOR* target, int push)
{
	int x, y, z, dx, dy, dz, i, h, c;
	short room, room2;
	FLOOR_INFO* floor;
	int flag, result;

	x = start->x;
	y = start->y;
	z = start->z;
	room = start->roomNumber;
	dx = target->x - x >> 3;
	dy = target->y - y >> 3;
	dz = target->z - z >> 3;
	flag = 0;
	result = 0;
	for (i = 0; i < 8; ++i)
	{
		room2 = room;
		floor = GetFloor(x, y, z, &room);
		h = GetFloorHeight(floor, x, y, z);
		c = GetCeiling(floor, x, y, z);
		if (h != NO_HEIGHT && c != NO_HEIGHT && c < h)
		{
			if (y > h)
			{
				if (y - h >= push)
				{
					flag = 1;
					break;
				}
				y = h;
			}
			if (y < c) {
				if (c - y >= push)
				{
					flag = 1;
					break;
				}
				y = c;
			}
			result = 1;
		}
		else if (result)
		{
			flag = 1;
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

	GetFloor(x, y, z, &room2);
	target->x = x;
	target->y = y;
	target->z = z;
	target->roomNumber = room2;

	return flag == 0;
}

void InitialiseCamera()
{
	Camera.shift = LaraItem->pos.yPos - 1024;
	
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

	Camera.targetDistance = 1536;
	Camera.item = NULL;
	Camera.numberFrames = 1;
	Camera.type = CHASE_CAMERA;
	Camera.speed = 1;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.bounce = 0;
	Camera.number = -1;
	Camera.fixedCamera = 0;
	
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

	if (OldCam.pos.xRot != LaraItem->pos.xRot
		|| OldCam.pos.yRot != LaraItem->pos.yRot
		|| OldCam.pos.zRot != LaraItem->pos.zRot
		|| OldCam.pos2.xRot != Lara.headXrot
		|| OldCam.pos2.yRot != Lara.headYrot
		|| OldCam.pos2.xPos != Lara.torsoXrot
		|| OldCam.pos2.yPos != Lara.torsoYrot
		|| OldCam.pos.xPos != LaraItem->pos.xPos
		|| OldCam.pos.yPos != LaraItem->pos.yPos
		|| OldCam.pos.zPos != LaraItem->pos.zPos
		|| OldCam.currentAnimState != LaraItem->currentAnimState
		|| OldCam.goalAnimState != LaraItem->goalAnimState
		|| OldCam.targetDistance != Camera.targetDistance
		|| OldCam.targetElevation != Camera.targetElevation
		|| OldCam.actualElevation != Camera.actualElevation
		|| OldCam.actualAngle != Camera.actualAngle
		|| OldCam.target.x != Camera.target.x
		|| OldCam.target.y != Camera.target.y
		|| OldCam.target.z != Camera.target.z
		|| Camera.oldType != Camera.type
		|| SniperOverlay
		|| BinocularOn < 0)
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

	short roomNumber = Camera.pos.roomNumber;
	FLOOR_INFO* floor = GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &roomNumber);
	int height = GetFloorHeight(floor, Camera.pos.x, Camera.pos.y, Camera.pos.z);

	if (Camera.pos.y < GetCeiling(floor, Camera.pos.x, Camera.pos.y, Camera.pos.z) || Camera.pos.y > height)
	{
		mgLOS(&Camera.target, &Camera.pos, 0);
		
		if (abs(Camera.pos.x - ideal->x) < 768
			&& abs(Camera.pos.y - ideal->y) < 768
			&& abs(Camera.pos.z - ideal->z) < 768)
		{
			to.x = Camera.pos.x;
			to.y = Camera.pos.y;
			to.z = Camera.pos.z;
			to.roomNumber = Camera.pos.roomNumber;

			from.x = ideal->x;
			from.y = ideal->y;
			from.z = ideal->z;
			from.roomNumber = ideal->roomNumber;

			if (!mgLOS(&from, &to, 0) && ++CameraSnaps >= 8)
			{
				Camera.pos.x = ideal->x;
				Camera.pos.y = ideal->y;
				Camera.pos.z = ideal->z;
				Camera.pos.roomNumber = ideal->roomNumber;
				CameraSnaps = 0;
			}
		}
	}

	roomNumber = Camera.pos.roomNumber;
	floor = GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &roomNumber);
	height = GetFloorHeight(floor, Camera.pos.x, Camera.pos.y, Camera.pos.z);
	int ceiling = GetCeiling(floor, Camera.pos.x, Camera.pos.y, Camera.pos.z);

	if (Camera.pos.y - 255 < ceiling && Camera.pos.y + 255 > height && ceiling < height && ceiling != NO_HEIGHT && height != NO_HEIGHT)
		Camera.pos.y = (height + ceiling) >> 1;
	else if (Camera.pos.y + 255 > height && ceiling < height && ceiling != NO_HEIGHT && height != NO_HEIGHT)
		Camera.pos.y = height - 255;
	else if (Camera.pos.y - 255 < ceiling && ceiling < height && ceiling != NO_HEIGHT && height != NO_HEIGHT)
		Camera.pos.y = ceiling + 255;
	else if (ceiling >= height || height == NO_HEIGHT || ceiling == NO_HEIGHT)
	{
		Camera.pos.x = ideal->x;
		Camera.pos.y = ideal->y;
		Camera.pos.z = ideal->z;
		Camera.pos.roomNumber = ideal->roomNumber;
	}

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
	{
		Camera.targetElevation = -ANGLE(10);
	}

	Camera.targetElevation += item->pos.xRot;
	UpdateCameraElevation();
	
	if (Camera.actualElevation > ANGLE(85))
		Camera.actualElevation = ANGLE(85);
	else if (Camera.actualElevation < -ANGLE(85))
		Camera.actualElevation = -ANGLE(85);

	int distance = Camera.targetDistance * phd_cos(Camera.actualElevation);

	GetFloor(Camera.target.x, Camera.target.y, Camera.target.z, &Camera.target.roomNumber);
	if (g_Level.Rooms[Camera.target.roomNumber].flags & ENV_FLAG_SWAMP)
		Camera.target.y = g_Level.Rooms[Camera.target.roomNumber].y - 256;

	int x = Camera.target.x;
	int y = Camera.target.y;
	int z = Camera.target.z;

	short roomNumber = Camera.target.roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int h = GetFloorHeight(floor, x, y, z);
	int c = GetCeiling(floor, x, y, z);

	if ((((y < c) || (h < y)) || (h <= c)) || ((h == NO_HEIGHT || (c == NO_HEIGHT))))
	{
		TargetSnaps++;
		Camera.target.x = LastTarget.x;
		Camera.target.y = LastTarget.y;
		Camera.target.z = LastTarget.z;
		Camera.target.roomNumber = LastTarget.roomNumber;
	}
	else
	{
		TargetSnaps = 0;
	}
	
	for (int i = 0; i < 5; i++)
	{
		Ideals[i].y = Camera.target.y + Camera.targetDistance * phd_sin(Camera.actualElevation);
	}

	int farthest = 0x7FFFFFFF;
	int farthestnum = 0;
	GAME_VECTOR temp[2];

	for (int i = 0; i < 5; i++)
	{
		short angle;

		if (i == 0)
		{
			angle = Camera.actualAngle;
		}
		else
		{
			angle = (i - 1) * ANGLE(90);
		}

		Ideals[i].x = Camera.target.x - distance * phd_sin(angle);
		Ideals[i].z = Camera.target.z - distance * phd_cos(angle);
		Ideals[i].roomNumber = Camera.target.roomNumber;

		if (mgLOS(&Camera.target, &Ideals[i], 200))
		{
			temp[0].x = Ideals[i].x;
			temp[0].y = Ideals[i].y;
			temp[0].z = Ideals[i].z;
			temp[0].roomNumber = Ideals[i].roomNumber;

			temp[1].x = Camera.pos.x;
			temp[1].y = Camera.pos.y;
			temp[1].z = Camera.pos.z;
			temp[1].roomNumber = Camera.pos.roomNumber;

			if (i == 0 || mgLOS(&temp[0], &temp[1], 0))
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

			if (i == 0 || mgLOS(&temp[0], &temp[1], 0))
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

	GAME_VECTOR ideal;
	ideal.x = Ideals[farthestnum].x;
	ideal.y = Ideals[farthestnum].y;
	ideal.z = Ideals[farthestnum].z;
	ideal.roomNumber = Ideals[farthestnum].roomNumber;

	CameraCollisionBounds(&ideal, 384, 1);

	MoveCamera(&ideal, Camera.speed);
}

void UpdateCameraElevation()
{
	PHD_VECTOR pos;
	PHD_VECTOR pos1;

	if (Camera.laraNode != -1)
	{
		pos.z = 0;
		pos.y = 0;
		pos.x = 0;
		GetLaraJointPosition(&pos, Camera.laraNode);

		pos1.x = 0;
		pos1.y = -256;
		pos1.z = 2048;
		GetLaraJointPosition(&pos1, Camera.laraNode);

		pos.z = pos1.z - pos.z;
		pos.x = pos1.x - pos.x;
		Camera.actualAngle = Camera.targetAngle + phd_atan(pos.z, pos.x);
	}
	else
	{
		Camera.actualAngle = LaraItem->pos.yRot + Camera.targetAngle;
	}

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
		Camera.targetElevation = Lara.headXrot + Lara.torsoXrot + item->pos.xRot - ANGLE(15);
	}
	
	FLOOR_INFO* floor = GetFloor(Camera.target.x, Camera.target.y, Camera.target.z, &Camera.target.roomNumber);
	int h = GetFloorHeight(floor, Camera.target.x, Camera.target.y, Camera.target.z);
	int c = GetCeiling(floor, Camera.target.x, Camera.target.y, Camera.target.z);
	
	if (c + 64 > h - 64 && h != NO_HEIGHT && c != NO_HEIGHT)
	{
		Camera.target.y = (c + h) >> 1;
		Camera.targetElevation = 0;
	}
	else if (Camera.target.y > h - 64 && h != NO_HEIGHT)
	{
		Camera.target.y = h - 64;
		Camera.targetElevation = 0;
	}
	else if (Camera.target.y < c + 64 && c != NO_HEIGHT)
	{
		Camera.target.y = c + 64;
		Camera.targetElevation = 0;
	}

	GetFloor(Camera.target.x, Camera.target.y, Camera.target.z, &Camera.target.roomNumber);
	
	int x = Camera.target.x;
	int y = Camera.target.y;
	int z = Camera.target.z;

	short roomNumber = Camera.target.roomNumber;
	floor = GetFloor(Camera.target.x, Camera.target.y, Camera.target.z, &roomNumber);
	h = GetFloorHeight(floor, x, y, z);
	c = GetCeiling(floor, x, y, z);
	
	if (y < c || y > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT)
	{
		TargetSnaps++;
		Camera.target.x = LastTarget.x;
		Camera.target.y = LastTarget.y;
		Camera.target.z = LastTarget.z;
		Camera.target.roomNumber = LastTarget.roomNumber;
	}
	else
	{
		TargetSnaps = 0;
	}

	UpdateCameraElevation();

	Camera.targetDistance = 1536;
	int distance = Camera.targetDistance * phd_cos(Camera.actualElevation);

	for (int i = 0; i < 5; i++)
	{
		Ideals[i].y = Camera.target.y + Camera.targetDistance * phd_sin(Camera.actualElevation);
	}

	int farthest = 0x7FFFFFFF;
	int farthestnum = 0;
	GAME_VECTOR temp[2];

	for (int i = 0; i < 5; i++)
	{
		short angle;

		if (i == 0)
		{
			angle = Camera.actualAngle;
		}
		else
		{
			angle = (i - 1) * ANGLE(90);
		}

		Ideals[i].x = Camera.target.x - distance * phd_sin(angle);
		Ideals[i].z = Camera.target.z - distance * phd_cos(angle);
		Ideals[i].roomNumber = Camera.target.roomNumber;

		if (mgLOS(&Camera.target, &Ideals[i], 200))
		{
			temp[0].x = Ideals[i].x;
			temp[0].y = Ideals[i].y;
			temp[0].z = Ideals[i].z;
			temp[0].roomNumber = Ideals[i].roomNumber;

			temp[1].x = Camera.pos.x;
			temp[1].y = Camera.pos.y;
			temp[1].z = Camera.pos.z;
			temp[1].roomNumber = Camera.pos.roomNumber;

			if (i == 0 || mgLOS(&temp[0], &temp[1], 0))
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

			if (i == 0 || mgLOS(&temp[0], &temp[1], 0))
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

	GAME_VECTOR ideal;
	ideal.x = Ideals[farthestnum].x;
	ideal.y = Ideals[farthestnum].y;
	ideal.z = Ideals[farthestnum].z;
	ideal.roomNumber = Ideals[farthestnum].roomNumber;

	CameraCollisionBounds(&ideal, 384, 1);

	if (Camera.oldType == FIXED_CAMERA)
	{
		Camera.speed = 1;
	}

	MoveCamera(&ideal, Camera.speed);
}

int CameraCollisionBounds(GAME_VECTOR* ideal, int push, int yFirst)
{
	int x = ideal->x;
	int y = ideal->y;
	int z = ideal->z;

	short roomNumber;
	FLOOR_INFO* floor;
	int h;
	int c;
	
	if (yFirst)
	{
		roomNumber = ideal->roomNumber;
		floor = GetFloor(x, y, z, &roomNumber);
		h = GetFloorHeight(floor, x, y, z);
		c = GetCeiling(floor, x, y, z);

		if (y - 255 < c && y + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
			y = (h + c) >> 1;
		else if (y + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
			y = h - 255;
		else if (y - 255 < c && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
			y = c + 255;
	}

	roomNumber = ideal->roomNumber;
	floor = GetFloor(x - push, y, z, &roomNumber);
	h = GetFloorHeight(floor, x - push, y, z);
	c = GetCeiling(floor, x - push, y, z);
	if (y > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || y < c)
		x = (x & (~1023)) + push;

	roomNumber = ideal->roomNumber;
	floor = GetFloor(x, y, z - push, &roomNumber);
	h = GetFloorHeight(floor, x, y, z - push);
	c = GetCeiling(floor, x, y, z - push);
	if (y > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || y < c)
		z = (z & (~1023)) + push;

	roomNumber = ideal->roomNumber;
	floor = GetFloor(x + push, y, z, &roomNumber);
	h = GetFloorHeight(floor, x + push, y, z);
	c = GetCeiling(floor, x + push, y, z);
	if (y > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || y < c)
		x = (x | 1023) - push;

	roomNumber = ideal->roomNumber;
	floor = GetFloor(x, y, z + push, &roomNumber);
	h = GetFloorHeight(floor, x, y, z + push);
	c = GetCeiling(floor, x, y, z + push);
	if (y > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || y < c)
		z = (z | 1023) - push;

	if (!yFirst)
	{
		roomNumber = ideal->roomNumber;
		floor = GetFloor(x, y, z, &roomNumber);
		h = GetFloorHeight(floor, x, y, z);
		c = GetCeiling(floor, x, y, z);

		if (y - 255 < c && y + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
			y = (h + c) >> 1;
		else if (y + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
			y = h - 255;
		else if (y - 255 < c && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
			y = c + 255;
	}

	roomNumber = ideal->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	h = GetFloorHeight(floor, x, y, z);
	c = GetCeiling(floor, x, y, z);
	if (y > h || y < c || h == NO_HEIGHT || c == NO_HEIGHT || c >= h)
		return 1;

	floor = GetFloor(x, y, z, &ideal->roomNumber);
	ideal->x = x;
	ideal->y = y;
	ideal->z = z;

	return 0;
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

		if (camera->flags & 2)
		{
			SniperOverlay = 1;
			
			Camera.target.x = (Camera.target.x + 2 * LastTarget.x) / 3;
			Camera.target.y = (Camera.target.y + 2 * LastTarget.y) / 3;
			Camera.target.z = (Camera.target.z + 2 * LastTarget.z) / 3;
			
			if (SniperCount)
			{
				SniperCount--;
			}
			else
			{
				to.x = Camera.target.x + ((Camera.target.x - Camera.pos.x) >> 1);
				to.y = Camera.target.y + ((Camera.target.y - Camera.pos.y) >> 1);
				to.z = Camera.target.z + ((Camera.target.z - Camera.pos.z) >> 1);
				
				int los = LOS(&from, &to);
				GetLaraOnLOS = 1;

				PHD_VECTOR pos;
				int objLos = ObjectOnLOS2(&from, &to, &pos, &CollidedMeshes[0]);
				objLos = (objLos != NO_LOS_ITEM && objLos >= 0 && g_Level.Items[objLos].objectNumber != ID_LARA);

				if (!(GetRandomControl() & 0x3F)
					|| !(GlobalCounter & 0x3F)
					|| objLos && !(GlobalCounter & 0xF) && GetRandomControl() & 1)
				{
					SoundEffect(SFX_TR4_EXPLOSION1, 0, 83886084);
					SoundEffect(SFX_TR5_HK_FIRE, 0, 0);

					auto R = 192;
					auto G = (GetRandomControl() & 0x1F) + 160;
					auto B = 0;
					Weather.Flash(R, G, B, 0.04f);
					
					SniperCount = 15;
					
					if (objLos && GetRandomControl() & 3)
					{
						DoBloodSplat(pos.x, pos.y, pos.z, (GetRandomControl() & 3) + 3, 2 * GetRandomControl(), LaraItem->roomNumber);
						LaraItem->hitPoints -= 100;
						GetLaraOnLOS = 0;
					}
					else if (objLos < 0)
					{
						MESH_INFO* mesh = CollidedMeshes[0];
						if (StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
						{
							ShatterObject(0, mesh, 128, to.roomNumber, 0);
							mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
							SoundEffect(GetShatterSound(mesh->staticNumber), (PHD_3DPOS*)mesh, 0);
						}
						TriggerRicochetSpark(&to, 2 * GetRandomControl(), 3, 0);
						TriggerRicochetSpark(&to, 2 * GetRandomControl(), 3, 0);
						GetLaraOnLOS = 0;
					}
					else if (!los)
					{
						TriggerRicochetSpark(&to, 2 * GetRandomControl(), 3, 0);
					}
				}
				GetLaraOnLOS = 0;
			}
		}
	}

	Camera.fixedCamera = 1;

	MoveCamera(&from, moveSpeed);

	if (Camera.timer)
	{
		if (!--Camera.timer)
			Camera.timer = -1;
	}
}


void LookCamera(ITEM_INFO* item)
{
	short headXrot = Lara.headXrot;
	short headYrot = Lara.headYrot;
	short torsoXrot = Lara.torsoXrot;
	short torsoYrot = Lara.torsoYrot;

	Lara.torsoXrot = 0;
	Lara.torsoYrot = 0;
	Lara.headXrot <<= 1;
	Lara.headYrot <<= 1;

	if (Lara.headXrot > ANGLE(55)) 
		Lara.headXrot = ANGLE(55);
	else if (Lara.headXrot < -ANGLE(75)) 
		Lara.headXrot = -ANGLE(75);
	if (Lara.headYrot < -ANGLE(80))
		Lara.headYrot = -ANGLE(80);
	else if (Lara.headYrot > ANGLE(80))
		Lara.headYrot = ANGLE(80);

	if (abs(Lara.headXrot - OldCam.pos.xRot) >= 16)
		OldCam.pos.xRot = (Lara.headXrot + OldCam.pos.xRot) >> 1;
	else
		OldCam.pos.xRot = Lara.headXrot;
	if (abs(Lara.headYrot - OldCam.pos.yRot) >= 16)
		OldCam.pos.yRot = (Lara.headYrot + OldCam.pos.yRot) >> 1;
	else
		OldCam.pos.yRot = Lara.headYrot;

	PHD_VECTOR pos;
	pos.x = 0;
	pos.y = 16;
	pos.z = 64;
	GetLaraJointPosition(&pos, LM_HEAD);
	
	short roomNumber = LaraItem->roomNumber;
	FLOOR_INFO* floor = GetFloor(pos.x, pos.y, pos.z, &roomNumber);
	int h = GetFloorHeight(floor, pos.x, pos.y, pos.z);
	int c = GetCeiling(floor, pos.x, pos.y, pos.z);
	if (h == NO_HEIGHT || c == NO_HEIGHT || c >= h || pos.y > h || pos.y < c)
	{
		pos.x = 0;
		pos.y = 16;
		pos.z = 0;
		GetLaraJointPosition(&pos, LM_HEAD);
		
		roomNumber = LaraItem->roomNumber;
		floor = GetFloor(pos.x, pos.y + 256, pos.z, &roomNumber);
		if (g_Level.Rooms[roomNumber].flags & ENV_FLAG_SWAMP)
		{
			pos.y = g_Level.Rooms[roomNumber].y - 256;
			floor = GetFloor(pos.x, pos.y, pos.z, &roomNumber);
		}
		else
			floor = GetFloor(pos.x, pos.y, pos.z, &roomNumber);

		h = GetFloorHeight(floor, pos.x, pos.y, pos.z);
		c = GetCeiling(floor, pos.x, pos.y, pos.z);
		if (h == NO_HEIGHT || c == NO_HEIGHT || c >= h || pos.y > h || pos.y < c)
		{
			pos.x = 0;
			pos.y = 16;
			pos.z = -64;
			GetLaraJointPosition(&pos, LM_HEAD);
		}
	}

	PHD_VECTOR pos2;
	pos2.x = 0;
	pos2.y = 0;
	pos2.z = -1024;
	GetLaraJointPosition(&pos2, LM_HEAD);

	PHD_VECTOR pos3;
	pos3.x = 0;
	pos3.y = 0;
	pos3.z = 2048;
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
		roomNumber = roomNumber2;
		floor = GetFloor(x, y + 256, z, &roomNumber2);
		if (g_Level.Rooms[roomNumber2].flags & ENV_FLAG_SWAMP)
		{
			y = g_Level.Rooms[roomNumber2].y - 256;
			break;
		}
		else
			floor = GetFloor(x, y, z, &roomNumber2);
		h = GetFloorHeight(floor, x, y, z);
		c = GetCeiling(floor, x, y, z);
		if (h == NO_HEIGHT || c == NO_HEIGHT || c >= h || y > h || y < c)
			break;
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

	GAME_VECTOR ideal;
	ideal.x = x;
	ideal.y = y;
	ideal.z = z;
	ideal.roomNumber = roomNumber;

	if (OldCam.pos.xRot == Lara.headXrot &&
		OldCam.pos.yRot == Lara.headYrot &&
		OldCam.pos.xPos == LaraItem->pos.xPos &&
		OldCam.pos.yPos == LaraItem->pos.yPos &&
		OldCam.pos.zPos == LaraItem->pos.zPos &&
		OldCam.currentAnimState == LaraItem->currentAnimState &&
		OldCam.goalAnimState == LaraItem->goalAnimState &&
		Camera.oldType == LOOK_CAMERA)
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

	CameraCollisionBounds(&ideal, 224, 1);

	if (Camera.oldType == FIXED_CAMERA)
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
	roomNumber = Camera.pos.roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	h = GetFloorHeight(floor, x, y, z);
	c = GetCeiling(floor, x, y, z);

	if (y - 255 < c && y + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
		Camera.pos.y = (h + c) >> 1;
	else if (y + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
		Camera.pos.y = h - 255;
	else if (y - 255 < c && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
		Camera.pos.y = c + 255;

	x = Camera.pos.x;
	y = Camera.pos.y;
	z = Camera.pos.z;
	roomNumber = Camera.pos.roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	h = GetFloorHeight(floor, x, y, z);
	c = GetCeiling(floor, x, y, z);
	if ((g_Level.Rooms[roomNumber].flags & ENV_FLAG_SWAMP))
		Camera.pos.y = g_Level.Rooms[roomNumber].y - 256;
	else if (y < c || y > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT)
		mgLOS(&Camera.target, &Camera.pos, 0);

	x = Camera.pos.x;
	y = Camera.pos.y;
	z = Camera.pos.z;
	roomNumber = Camera.pos.roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	h = GetFloorHeight(floor, x, y, z);
	c = GetCeiling(floor, x, y, z);
	if (y < c || y > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT || (g_Level.Rooms[roomNumber].flags & ENV_FLAG_SWAMP))
	{
		Camera.pos.x = pos.x;
		Camera.pos.y = pos.y;
		Camera.pos.z = pos.z;
		Camera.pos.roomNumber = LaraItem->roomNumber;
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

	Lara.headXrot = headXrot;
	Lara.headYrot = headYrot;
	Lara.torsoXrot = torsoXrot;
	Lara.torsoYrot = torsoYrot;
}

void BounceCamera(ITEM_INFO* item, short bounce, short maxDistance)
{
	int distance;

	distance = sqrt(SQUARE(item->pos.xPos - Camera.pos.x) + SQUARE(item->pos.yPos - Camera.pos.y) + SQUARE(item->pos.zPos - Camera.pos.z));
	if (distance < maxDistance)
	{
		if (maxDistance == -1)
		{
			Camera.bounce = bounce;
		}
		else
		{
			Camera.bounce = -(bounce * (maxDistance - distance) / maxDistance);
		}
	}
	else if (maxDistance == -1)
	{
		Camera.bounce = bounce;
	}
}

void BinocularCamera(ITEM_INFO* item)
{
	static int exittingBinos = 0;

	if (LSHKTimer)
		--LSHKTimer;

	if (!LaserSight)
	{
		if (InputBusy & IN_DRAW)
		{
			exittingBinos = 1;
		}
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

	if (headXrot > ANGLE(75))
		headXrot = ANGLE(75);
	else if (headXrot < -ANGLE(75))
		headXrot = -ANGLE(75);

	if (headYrot > ANGLE(80))
		headYrot = ANGLE(80);
	else if (headYrot < -ANGLE(80))
		headYrot = -ANGLE(80);

	int x = LaraItem->pos.xPos;
	int y = LaraItem->pos.yPos - 512;
	int z = LaraItem->pos.zPos;

	short roomNumber = LaraItem->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int c = GetCeiling(floor, x, y, z);
	if (c <= y - 256)
		y -= 256;
	else
		y = c + 64;

	Camera.pos.x = x;
	Camera.pos.y = y;
	Camera.pos.z = z;	
	Camera.pos.roomNumber = roomNumber;
	
	int l = 20736 * phd_cos(headXrot);
	
	int tx = x + l * phd_sin(LaraItem->pos.yRot + headYrot);
	int ty = y - 20736 * phd_sin(headXrot);
	int tz = z + l * phd_cos(LaraItem->pos.yRot + headYrot);

	if (Camera.oldType == FIXED_CAMERA)
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
			Camera.target.x += 16 * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1));
			Camera.target.y += 16 * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1));
			Camera.target.z += 16 * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce >> 1));
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

	PHD_VECTOR src;
	src.x = Camera.pos.x;
	src.y = Camera.pos.y;
	src.z = Camera.pos.z;

	PHD_VECTOR target;
	target.x = Camera.target.x;
	target.y = Camera.target.y;
	target.z = Camera.target.z;

	if (LaserSight)
	{
		int firing = 0;
		Ammo& ammo = GetAmmo(Lara.gunType);

		if (!(InputBusy & IN_ACTION) || WeaponDelay || !ammo)
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
				Savegame.Game.AmmoUsed++;
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
					{
						SoundEffect(SFX_TR5_HK_SILENCED, 0, 0);
					}
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
						{
							SoundEffect(SFX_TR5_HK_SILENCED, 0, 0);
						}
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
						{
							SoundEffect(SFX_TR5_HK_SILENCED, 0, 0);
						}
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
						{
							SoundEffect(SFX_TR5_HK_SILENCED, 0, 0);
						}
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
						{
							SoundEffect(SFX_TR5_HK_SILENCED, 0, 0);
						}
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
		{
			LaraTorch(&src, &target, Lara.headYrot, 192);
		}
	}
}

void LaraTorch(PHD_VECTOR* src, PHD_VECTOR* target, int rot, int color)
{
	GAME_VECTOR pos1;
	pos1.x = src->x;
	pos1.y = src->y;
	pos1.z = src->z;
	pos1.roomNumber = LaraItem->roomNumber;

	GAME_VECTOR pos2;
	pos2.x = target->x;
	pos2.y = target->y;
	pos2.z = target->z;

	TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 12, color, color, color >> 1);
	
	if (!LOS(&pos1, &pos2))
	{
		int l = sqrt(SQUARE(pos1.x - pos2.x) + SQUARE(pos1.y - pos2.y) + SQUARE(pos1.z - pos2.z)) * 256;
		
		if (l + 8 > 31)
			l = 31;
		
		if (color - l >= 0)
			TriggerDynamicLight(pos2.x, pos2.y, pos2.z, l + 8, color - l, color - l, (color - l) * 2);
	}
}

void ConfirmCameraTargetPos() 
{
	PHD_VECTOR pos;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;

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

	int x = Camera.target.x;
	int y = Camera.target.y;
	int z = Camera.target.z;

	short roomNumber = Camera.target.roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int h = GetFloorHeight(floor, x, y, z);
	int c = GetCeiling(floor, x, y, z);

	if (y < c || h < y || h <= c || h == NO_HEIGHT || c == NO_HEIGHT)
	{
		Camera.target.x = pos.x;
		Camera.target.y = pos.y;
		Camera.target.z = pos.z;
	}
}

void CalculateCamera()
{
	SniperOverlay = 0;
	// FIXME
	//SniperCamActive = 0;

	CamOldPos.x = Camera.pos.x;
	CamOldPos.y = Camera.pos.y;
	CamOldPos.z = Camera.pos.z;

	if (BinocularRange != 0)
	{
		BinocularOn = 1;
		BinocularCamera(LaraItem);

		if (BinocularRange != 0)
		{
			return;
		}
	}

	if (BinocularOn == 1)
	{
		BinocularOn = -8;
	}

	if (UseForcedFixedCamera != 0)
	{
		Camera.type = FIXED_CAMERA;
		if (Camera.oldType != FIXED_CAMERA)
		{
			Camera.speed = 1;
		}
	}

	if (TLFlag == 1 && Camera.underwater != 0)
	{
		Camera.underwater = 0;
	}
	TLFlag = 0;
		//Camera is in a water room, play water sound effect.
		if ((g_Level.Rooms[Camera.pos.roomNumber].flags & ENV_FLAG_WATER))
		{
			SoundEffect(SFX_TR4_UNDERWATER, NULL, SFX_ALWAYS);
			if (Camera.underwater == 0)
			{
				Camera.underwater = 1;
			}
		}
		else
		{
			if (Camera.underwater != 0)
			{
				Camera.underwater = 0;
			}
		}

	if (Camera.type == CINEMATIC_CAMERA)
	{
		//Legacy_do_new_cutscene_camera();
		return;
	}

	ITEM_INFO* item;
	int fixedCamera = 0;
	if (Camera.item != NULL && (Camera.type == FIXED_CAMERA || Camera.type == HEAVY_CAMERA))
	{
		item = Camera.item;
		fixedCamera = 1;
	}
	else
	{
		item = LaraItem;
		fixedCamera = 0;
	}

	BOUNDING_BOX* bounds = GetBoundsAccurate(item);
	
	int x;
	int y = ((bounds->Y1 + bounds->Y2) / 2) + item->pos.yPos - 256;
	int z;

	if (Camera.item)
	{
		if (!fixedCamera)
		{
			int dx = Camera.item->pos.xPos - item->pos.xPos;
			int dz = Camera.item->pos.zPos - item->pos.zPos;
			int shift = sqrt(SQUARE(dx) + SQUARE(dz));
			short angle = phd_atan(dz, dx) - item->pos.yRot;
			short tilt = phd_atan(shift, y - (bounds->Y1 + bounds->Y2) / 2 - Camera.item->pos.yPos);
			bounds = GetBoundsAccurate(Camera.item);
			angle >>= 1;
			tilt >>= 1;

			if (angle > -ANGLE(50) && angle < ANGLE(50) && tilt > -ANGLE(85) && tilt < ANGLE(85))
			{
				short change = angle - Lara.headYrot;
				if (change > ANGLE(4))
					Lara.headYrot += ANGLE(4);
				else if (change < -ANGLE(4))
					Lara.headYrot -= ANGLE(4);
				else
					Lara.headYrot += change;
				Lara.torsoYrot = Lara.headYrot;

				change = tilt - Lara.headXrot;
				if (change > ANGLE(4))
					Lara.headXrot += ANGLE(4);
				else if (change < -ANGLE(4))
					Lara.headXrot -= ANGLE(4);
				else
					Lara.headXrot += change;
				Lara.torsoXrot = Lara.headXrot;

				Camera.type = LOOK_CAMERA;
				Camera.item->lookedAt = 1;
			}
		}
	}

	if (Camera.type == LOOK_CAMERA || Camera.type == COMBAT_CAMERA)
	{
		if (Camera.type == COMBAT_CAMERA)
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
			Camera.speed = Camera.type != LOOK_CAMERA ? 8 : 4;
		}

		Camera.fixedCamera = 0;
		if (Camera.type == LOOK_CAMERA)
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

		if (Camera.type
			&& Camera.flags != CF_CHASE_OBJECT
			&& (Camera.number != -1 &&(SniperCamActive = g_Level.Cameras[Camera.number].flags & 3, g_Level.Cameras[Camera.number].flags & 2)))
		{
			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = 0;
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
			int shift = (bounds->X1 + bounds->X2 + bounds->Z1 + bounds->Z2) / 4;
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
			Camera.fixedCamera = 0;
			if (Camera.speed != 1 && Camera.oldType != LOOK_CAMERA && BinocularOn >= 0)
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
				{
					TargetSnaps = 0;
				}
			}
		}
		else
		{
			SniperCount = 30;
			Camera.fixedCamera = 1;
			Camera.speed = 1;
		}

		GetFloor(x, y, z, &Camera.target.roomNumber);

		if (abs(LastTarget.x - Camera.target.x) < 4
			&& abs(LastTarget.y - Camera.target.y) < 4
			&& abs(LastTarget.z - Camera.target.z) < 4)
		{
			Camera.target.x = LastTarget.x;
			Camera.target.y = LastTarget.y;
			Camera.target.z = LastTarget.z;
		}

		if (Camera.type && Camera.flags != CF_CHASE_OBJECT)
			FixedCamera(item);
		else
			ChaseCamera(item);
	}

	Camera.fixedCamera = fixedCamera;
	Camera.last = Camera.number;

	if (Camera.type != HEAVY_CAMERA || Camera.timer == -1)
	{
		Camera.type = CHASE_CAMERA;
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
	Camera.type = LOOK_CAMERA;
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
	if (Lara.gunStatus != LG_HANDS_BUSY
		&& Lara.Vehicle == NO_ITEM 
		&& !Lara.leftArm.lock 
		&& !Lara.rightArm.lock)
		Lara.torsoYrot = Lara.headYrot;
}

void LookUpDown()
{
	Camera.type = LOOK_CAMERA;
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
	if (Lara.gunStatus != LG_HANDS_BUSY 
		&& Lara.Vehicle == NO_ITEM 
		&& !Lara.leftArm.lock 
		&& !Lara.rightArm.lock)
		Lara.torsoXrot = Lara.headXrot;
}

void ResetLook()
{
	if (Camera.type != LOOK_CAMERA)
	{
		if (Lara.headXrot <= -ANGLE(2.0f) || Lara.headXrot >= ANGLE(2.0f))
			Lara.headXrot = Lara.headXrot / -8 + Lara.headXrot;
		else
			Lara.headXrot = 0;

		if (Lara.headYrot <= -ANGLE(2.0f) || Lara.headYrot >= ANGLE(2.0f))
			Lara.headYrot = Lara.headYrot / -8 + Lara.headYrot;
		else
			Lara.headYrot = 0;

		if (Lara.gunStatus != LG_HANDS_BUSY 
			&& !Lara.leftArm.lock 
			&& !Lara.rightArm.lock
			&& Lara.Vehicle == NO_ITEM)
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
			InGameCounter = 0;
			RumbleTimer = -32 - (GetRandomControl() & 0x1F);
			return;
		}
	}

	if (RumbleTimer < 0)
	{
		if (InGameCounter >= abs(RumbleTimer))
		{
			Camera.bounce = -(GetRandomControl() % abs(RumbleTimer));
			RumbleTimer++;
		}
		else
		{
			InGameCounter++;
			Camera.bounce = -(GetRandomControl() % InGameCounter);
		}
	}
}