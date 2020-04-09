#include "Camera.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include "draw.h"
#include "lara.h"

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
	LfAspectCorrection = 1; // 1.3333334f / (float)(PhdWidth / PhdHeight);
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
		Camera.mikePos.x += PhdPerspective * SIN(angle) >> W2V_SHIFT;
		Camera.mikePos.y = Camera.pos.y;
		Camera.mikePos.z += PhdPerspective * COS(angle) >> W2V_SHIFT;
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
}