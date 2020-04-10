#include "Camera.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include "draw.h"
#include "lara.h"
#include "effects.h"
#include "effect2.h"
#include "debris.h"

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

#define LfAspectCorrection VAR_U_(0x0055DA30, float)
#define LastTarget			VAR_U_(0x00EEFA30, GAME_VECTOR)

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

void ActivateCamera()
{
	KeyTriggerActive = 2;
}

void LookAt(int posX, int posY, int posZ, int targetX, int targetY, int targetZ, short roll)
{
	Vector3 position = Vector3(posX, posY, posZ);
	Vector3 target = Vector3(targetX, targetY, targetZ);
	Vector3 up = Vector3(0.0f, -1.0f, 0.0f);
	float fov = TR_ANGLE_TO_RAD(CurrentFOV / 1.333333f);
	float r = TR_ANGLE_TO_RAD(roll);

	// This should not be needed but it seems to solve our issues
	if (posX == targetX && posY == targetY && posZ == targetZ)
		return;

	short angles[2];
	phd_GetVectorAngles(targetX - posX, targetY - posY, targetZ - posZ, angles);

	PHD_3DPOS pos;

	pos.xPos = posX;
	pos.yPos = posY;
	pos.zPos = posZ;
	pos.xRot = angles[1];
	pos.yRot = angles[0];
	pos.zRot = roll;

	CurrentCameraPosition.x = posX;
	CurrentCameraPosition.y = posY;
	CurrentCameraPosition.z = posZ;

	/*CurrentCameraRotation.vx = mGetAngle(0, 0, sqrt(roll), posY - targetY) >> 4;
	CurrentCameraRotation.vy = mGetAngle(posZ, posX, targetZ, targetX) >> 4;
	CurrentCameraRotation.vz = 0;*/

	phd_GenerateW2V(&pos);

	g_Renderer->UpdateCameraMatrices(posX, posY, posZ, targetX, targetY, targetZ, r, fov);
}

void AlterFOV(int value)
{ 
	CurrentFOV = value;
	PhdPerspective = PhdWidth / 2 * COS(CurrentFOV / 2) / SIN(CurrentFOV / 2);
	LfAspectCorrection = 1.3333334f / (float)(PhdWidth / PhdHeight);
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
	LookAt(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.target.x, Camera.target.y, Camera.target.z, 0);
	
	if (Camera.mikeAtLara)
	{
		Camera.mikePos.x = LaraItem->pos.xPos;
		Camera.mikePos.y = LaraItem->pos.yPos;
		Camera.mikePos.z = LaraItem->pos.zPos;
		Camera.oldType = Camera.type;
	}
	else
	{
		short angle = ATAN(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.mikePos.x = Camera.pos.x + (PhdPerspective * SIN(angle) >> W2V_SHIFT);
		Camera.mikePos.y = Camera.pos.y;
		Camera.mikePos.z = Camera.pos.z + (PhdPerspective * COS(angle) >> W2V_SHIFT);
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

	int distance = Camera.targetDistance * COS(Camera.actualElevation) >> W2V_SHIFT;

	GetFloor(Camera.target.x, Camera.target.y, Camera.target.z, &Camera.target.roomNumber);
	if (Rooms[Camera.target.roomNumber].flags & ENV_FLAG_SWAMP)
		Camera.target.y = Rooms[Camera.target.roomNumber].y - 256;

	int wx = Camera.target.x;
	int wy = Camera.target.y;
	int wz = Camera.target.z;

	short roomNumber = Camera.target.roomNumber;
	FLOOR_INFO* floor = GetFloor(wx, wy, wz, &roomNumber);
	int h = GetFloorHeight(floor, wx, wy, wz);
	int c = GetCeiling(floor, wx, wy, wz);

	if ((((wy < c) || (h < wy)) || (h <= c)) || ((h == NO_HEIGHT || (c == NO_HEIGHT))))
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
		Ideals[i].y = Camera.target.y + (Camera.targetDistance * SIN(Camera.actualElevation) >> W2V_SHIFT);
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

		Ideals[i].x = Camera.target.x - ((distance * SIN(angle)) >> W2V_SHIFT);
		Ideals[i].z = Camera.target.z - ((distance * COS(angle)) >> W2V_SHIFT);
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
		Camera.actualAngle = Camera.targetAngle + ATAN(pos.z, pos.x);
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
	
	int wx = Camera.target.x;
	int wy = Camera.target.y;
	int wz = Camera.target.z;

	short roomNumber = Camera.target.roomNumber;
	floor = GetFloor(Camera.target.x, Camera.target.y, Camera.target.z, &roomNumber);
	h = GetFloorHeight(floor, wx, wy, wz);
	c = GetCeiling(floor, wx, wy, wz);
	
	if (wy < c || wy > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT)
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
	int distance = Camera.targetDistance * COS(Camera.actualElevation) >> W2V_SHIFT;

	for (int i = 0; i < 5; i++)
	{
		Ideals[i].y = Camera.target.y + (Camera.targetDistance * SIN(Camera.actualElevation) >> W2V_SHIFT);
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

		Ideals[i].x = Camera.target.x - ((distance * SIN(angle)) >> W2V_SHIFT);
		Ideals[i].z = Camera.target.z - ((distance * COS(angle)) >> W2V_SHIFT);
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
	int wx = ideal->x;
	int wy = ideal->y;
	int wz = ideal->z;

	short roomNumber;
	FLOOR_INFO* floor;
	int h;
	int c;
	
	if (yFirst)
	{
		roomNumber = ideal->roomNumber;
		floor = GetFloor(wx, wy, wz, &roomNumber);
		h = GetFloorHeight(floor, wx, wy, wz);
		c = GetCeiling(floor, wx, wy, wz);

		if (wy - 255 < c && wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
			wy = (h + c) >> 1;
		else if (wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
			wy = h - 255;
		else if (wy - 255 < c && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
			wy = c + 255;
	}

	roomNumber = ideal->roomNumber;
	floor = GetFloor(wx - push, wy, wz, &roomNumber);
	h = GetFloorHeight(floor, wx - push, wy, wz);
	c = GetCeiling(floor, wx - push, wy, wz);
	if (wy > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy < c)
		wx = (wx & (~1023)) + push;

	roomNumber = ideal->roomNumber;
	floor = GetFloor(wx, wy, wz - push, &roomNumber);
	h = GetFloorHeight(floor, wx, wy, wz - push);
	c = GetCeiling(floor, wx, wy, wz - push);
	if (wy > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy < c)
		wz = (wz & (~1023)) + push;

	roomNumber = ideal->roomNumber;
	floor = GetFloor(wx + push, wy, wz, &roomNumber);
	h = GetFloorHeight(floor, wx + push, wy, wz);
	c = GetCeiling(floor, wx + push, wy, wz);
	if (wy > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy < c)
		wx = (wx | 1023) - push;

	roomNumber = ideal->roomNumber;
	floor = GetFloor(wx, wy, wz + push, &roomNumber);
	h = GetFloorHeight(floor, wx, wy, wz + push);
	c = GetCeiling(floor, wx, wy, wz + push);
	if (wy > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy < c)
		wz = (wz | 1023) - push;

	if (!yFirst)
	{
		roomNumber = ideal->roomNumber;
		floor = GetFloor(wx, wy, wz, &roomNumber);
		h = GetFloorHeight(floor, wx, wy, wz);
		c = GetCeiling(floor, wx, wy, wz);

		if (wy - 255 < c && wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
			wy = (h + c) >> 1;
		else if (wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
			wy = h - 255;
		else if (wy - 255 < c && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
			wy = c + 255;
	}

	roomNumber = ideal->roomNumber;
	floor = GetFloor(wx, wy, wz, &roomNumber);
	h = GetFloorHeight(floor, wx, wy, wz);
	c = GetCeiling(floor, wx, wy, wz);
	if (wy > h || wy < c || h == NO_HEIGHT || c == NO_HEIGHT || c >= h)
		return 1;

	floor = GetFloor(wx, wy, wz, &ideal->roomNumber);
	ideal->x = wx;
	ideal->y = wy;
	ideal->z = wz;

	return 0;
}

void FixedCamera()
{
	GAME_VECTOR from, to;

	if (UseForcedFixedCamera)
	{
		from.x = ForcedFixedCamera.x;
		from.y = ForcedFixedCamera.y;
		from.z = ForcedFixedCamera.z;
		from.roomNumber = ForcedFixedCamera.roomNumber;
	}
	else
	{
		OBJECT_VECTOR* camera = &Cameras[Camera.number];
		
		from.x = camera->x;
		from.y = camera->y;
		from.z = camera->z;
		from.roomNumber = camera->data;

		if (camera->flags & 2)
		{
			if (FlashFader > 2)
			{
				FlashFader = (FlashFader >> 1) & 0xFE;
			}

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
				objLos = (objLos != 999 && objLos >= 0 && Items[objLos].objectNumber != ID_LARA);

				if (!(GetRandomControl() & 0x3F)
					|| !(GlobalCounter & 0x3F)
					|| objLos && !(GlobalCounter & 0xF) && GetRandomControl() & 1)
				{
					SoundEffect(SFX_EXPLOSION1, 0, 83886084);
					SoundEffect(SFX_HK_FIRE, 0, 0);
					
					FlashFadeR = 192;
					FlashFadeB = 0;
					FlashFader = 24;
					FlashFadeG = (GetRandomControl() & 0x1F) + 160;
					
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
						if (mesh->staticNumber >= 50 && mesh->staticNumber < 58)
						{
							ShatterObject(0, mesh, 128, to.roomNumber, 0);
							mesh->Flags &= ~1;
							SoundEffect(ShatterSounds[CurrentLevel - 5][mesh->staticNumber], (PHD_3DPOS*)mesh, 0);
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

	MoveCamera(&from, 1);

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
	GetLaraJointPosition(&pos, LJ_HEAD);
	
	short roomNumber = LaraItem->roomNumber;
	FLOOR_INFO* floor = GetFloor(pos.x, pos.y, pos.z, &roomNumber);
	int h = GetFloorHeight(floor, pos.x, pos.y, pos.z);
	int c = GetCeiling(floor, pos.x, pos.y, pos.z);
	if (h == NO_HEIGHT || c == NO_HEIGHT || c >= h || pos.y > h || pos.y < c)
	{
		pos.x = 0;
		pos.y = 16;
		pos.z = 0;
		GetLaraJointPosition(&pos, LJ_HEAD);
		
		roomNumber = LaraItem->roomNumber;
		floor = GetFloor(pos.x, pos.y + 256, pos.z, &roomNumber);
		if (Rooms[roomNumber].flags & ENV_FLAG_SWAMP)
		{
			pos.y = Rooms[roomNumber].y - 256;
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
			GetLaraJointPosition(&pos, LJ_HEAD);
		}
	}

	PHD_VECTOR pos2;
	pos2.x = 0;
	pos2.y = 0;
	pos2.z = -1024;
	GetLaraJointPosition(&pos2, LJ_HEAD);

	PHD_VECTOR pos3;
	pos3.x = 0;
	pos3.y = 0;
	pos3.z = 2048;
	GetLaraJointPosition(&pos3, LJ_HEAD);

	int dx = (pos2.x - pos.x) >> 3;
	int dy = (pos2.y - pos.y) >> 3;
	int dz = (pos2.z - pos.z) >> 3;
	int wx = pos.x;
	int wy = pos.y;
	int wz = pos.z;

	short roomNumber2 = LaraItem->roomNumber;
	int i = 0;
	for (i = 0; i < 8; i++)
	{
		roomNumber = roomNumber2;
		floor = GetFloor(wx, wy + 256, wz, &roomNumber2);
		if (Rooms[roomNumber2].flags & ENV_FLAG_SWAMP)
		{
			wy = Rooms[roomNumber2].y - 256;
			break;
		}
		else
			floor = GetFloor(wx, wy, wz, &roomNumber2);
		h = GetFloorHeight(floor, wx, wy, wz);
		c = GetCeiling(floor, wx, wy, wz);
		if (h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy > h || wy < c)
			break;
		wx += dx;
		wy += dy;
		wz += dz;
	}

	if (i)
	{
		wx -= dx;
		wy -= dy;
		wz -= dz;
	}

	GAME_VECTOR ideal;
	ideal.x = wx;
	ideal.y = wy;
	ideal.z = wz;
	ideal.roomNumber = roomNumber;

	if (OldCam.pos.xRot == Lara.headXrot &&
		OldCam.pos.yRot == Lara.headYrot &&
		OldCam.pos.xPos == LaraItem->pos.xPos &&
		OldCam.pos.yPos == LaraItem->pos.yPos &&
		OldCam.pos.zPos == LaraItem->pos.zPos &&
		OldCam.currentAnimState == LaraItem->currentAnimState&&
		OldCam.goalAnimState == LaraItem->goalAnimState&&
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

	wx = Camera.pos.x;
	wy = Camera.pos.y;
	wz = Camera.pos.z;
	roomNumber = Camera.pos.roomNumber;
	floor = GetFloor(wx, wy, wz, &roomNumber);
	h = GetFloorHeight(floor, wx, wy, wz);
	c = GetCeiling(floor, wx, wy, wz);

	if (wy - 255 < c && wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
		Camera.pos.y = (h + c) >> 1;
	else if (wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
		Camera.pos.y = h - 255;
	else if (wy - 255 < c && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
		Camera.pos.y = c + 255;

	wx = Camera.pos.x;
	wy = Camera.pos.y;
	wz = Camera.pos.z;
	roomNumber = Camera.pos.roomNumber;
	floor = GetFloor(wx, wy, wz, &roomNumber);
	h = GetFloorHeight(floor, wx, wy, wz);
	c = GetCeiling(floor, wx, wy, wz);
	if ((Rooms[roomNumber].flags & ENV_FLAG_SWAMP))
		Camera.pos.y = Rooms[roomNumber].y - 256;
	else	if (wy < c || wy > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT)
		mgLOS(&Camera.target, &Camera.pos, 0);

	wx = Camera.pos.x;
	wy = Camera.pos.y;
	wz = Camera.pos.z;
	roomNumber = Camera.pos.roomNumber;
	floor = GetFloor(wx, wy, wz, &roomNumber);
	h = GetFloorHeight(floor, wx, wy, wz);
	c = GetCeiling(floor, wx, wy, wz);
	if (wy < c || wy > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT || (Rooms[roomNumber].flags & ENV_FLAG_SWAMP))
	{
		Camera.pos.x = pos.x;
		Camera.pos.y = pos.y;
		Camera.pos.z = pos.z;
		Camera.pos.roomNumber = LaraItem->roomNumber;
	}

	GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &Camera.pos.roomNumber);
	LookAt(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.target.x, Camera.target.y, Camera.target.z, 0);

	if (Camera.mikeAtLara)
	{
		Camera.actualAngle = LaraItem->pos.yRot + Lara.headYrot + Lara.torsoYrot;
		Camera.mikePos.x = LaraItem->pos.xPos;
		Camera.mikePos.y = LaraItem->pos.yPos;
		Camera.mikePos.z = LaraItem->pos.zPos;
	}
	else
	{
		Camera.actualAngle = ATAN(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.mikePos.x = Camera.pos.x + ((PhdPerspective * SIN(Camera.actualAngle)) >> W2V_SHIFT);
		Camera.mikePos.z = Camera.pos.z + ((PhdPerspective * COS(Camera.actualAngle)) >> W2V_SHIFT);
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

	distance = SQRT_ASM(SQUARE(item->pos.xPos - Camera.pos.x) + SQUARE(item->pos.yPos - Camera.pos.y) + SQUARE(item->pos.zPos - Camera.pos.z));
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

void Inject_Camera()
{
	INJECT(0x0048EDC0, AlterFOV);
	INJECT(0x0048F760, LookAt);
	INJECT(0x0040FA70, mgLOS);
	INJECT(0x0040C7A0, MoveCamera);
	INJECT(0x0040C690, InitialiseCamera);
	INJECT(0x004107C0, UpdateCameraElevation);
	INJECT(0x0040D150, ChaseCamera);
	INJECT(0x0040D640, CombatCamera);
	INJECT(0x0040F5C0, CameraCollisionBounds);
	INJECT(0x0040E890, FixedCamera);
	INJECT(0x0040DC10, LookCamera);
}