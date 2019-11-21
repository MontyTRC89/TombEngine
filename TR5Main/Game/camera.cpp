#include "Camera.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include "draw.h"
 
void __cdecl LookAt(__int32 posX, __int32 posY, __int32 posZ,
	__int32 targetX, __int32 targetY, __int32 targetZ,
	__int16 roll)
{
	Vector3 position = Vector3(posX, posY, posZ);
	Vector3 target = Vector3(targetX, targetY, targetZ);
	Vector3 up = Vector3(0.0f, -1.0f, 0.0f);
	float fov = TR_ANGLE_TO_RAD(CurrentFOV);
	float r = TR_ANGLE_TO_RAD(roll);

	g_Renderer->UpdateCameraMatrices(posX, posY, posZ, targetX, targetY, targetZ, r, fov);
}

void __cdecl AlterFOV(__int32 value)
{ 
	CurrentFOV = value;
	PhdPerspective = PhdWidth / 2 * COS(CurrentFOV / 2) / SIN(CurrentFOV / 2);
}

void __cdecl j_CalculateCamera()
{
	CalculateCamera();
}

void Inject_Camera()
{
	INJECT(0x0048EDC0, AlterFOV);
	INJECT(0x0048F760, LookAt);
	INJECT(0x00401D5C, j_CalculateCamera);
}