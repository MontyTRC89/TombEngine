#include "camera.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include "draw.h"

extern int KeyTriggerActive;

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

	// Maybe we should add this code, let's test before
	/*short angles[2];
	phd_GetVectorAngles(targetX - posX, targetY - posY, targetZ - posZ, angles);
	v13 = HIWORD(targetZ);
	v15 = roll;
	v14 = targetZ;
	v10 = posX;
	v11 = posY;
	v12 = posZ;
	roll = (v10 - targetX) * (v10 - targetX) + (v12 - v7) * (v12 - v7);
	CamRotX = j_mGetAngle(0, 0, sqrt(roll), posY - targetY) >> 4;
	v8 = j_mGetAngle(posZ, posX, v7, targetX);
	CamRotZ = 0;
	CamRotY = v8 >> 4;
	CameraPosX = posX;
	CameraPosY = posY;
	Camera.pos.x = posX;
	Camera.pos.y = posY;
	Camera.pos.z = posZ;
	Camera.*/

	g_Renderer->UpdateCameraMatrices(posX, posY, posZ, targetX, targetY, targetZ, r, fov);
}

void AlterFOV(int value)
{ 
	if (SIN(value / 2) == 0) return; /* @FIXME Integer division by zero */

	CurrentFOV = value;
	PhdPerspective = PhdWidth / 2 * COS(CurrentFOV / 2) / SIN(CurrentFOV / 2);
}

void Inject_Camera()
{
	INJECT(0x0048EDC0, AlterFOV);
	INJECT(0x0048F760, LookAt);
}