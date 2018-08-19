#include "camera.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include "draw.h"
 
void __cdecl phd_LookAt(__int32 posX, __int32 posY, __int32 posZ,
						__int32 targetX, __int32 targetY, __int32 targetZ,
						__int16 roll)
{
	D3DXVECTOR3 position = D3DXVECTOR3(posX, posY, posZ);
	D3DXVECTOR3 target = D3DXVECTOR3(targetX, targetY, targetZ);
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	float fov = TR_ANGLE_TO_RAD(CurrentFOV);

	D3DXMatrixLookAtRH(&g_Renderer->ViewMatrix, &position, &target, &up);
	D3DXMatrixPerspectiveFovRH(&g_Renderer->ProjectionMatrix, fov, g_Renderer->ScreenWidth / (float)g_Renderer->ScreenHeight, 32.0f, 200000.0f);

	//printf("%d %d %d\n", posX, posY, posZ);
}

void __cdecl phd_AlterFOV(__int32 value)
{
	CurrentFOV = value;
}

void __cdecl j_CalculateCamera()
{
	//printf("Item: %d\n", Camera.target);
	CalculateCamera();
}

void Inject_Camera()
{
	INJECT(0x0048EDC0, phd_AlterFOV);
	INJECT(0x0048F760, phd_LookAt);
	INJECT(0x00401D5C, j_CalculateCamera);
}