#include "camera.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include "draw.h"

#define LfAspectCorrection VAR_U_(0x0055DA30, float)

extern int KeyTriggerActive;

PHD_VECTOR CurrentCameraPosition;
SVECTOR CurrentCameraRotation;

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

void Inject_Camera()
{
	INJECT(0x0048EDC0, AlterFOV);
	INJECT(0x0048F760, LookAt);
	INJECT(0x0040FA70, mgLOS);
}