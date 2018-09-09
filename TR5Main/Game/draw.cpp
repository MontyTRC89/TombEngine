#include "draw.h"
#include "lara.h"
#include "..\Renderer\Renderer.h"

Renderer* g_Renderer;

__int32 __cdecl DrawPhaseGame()
{
	// Control routines uses joints calculated here for getting Lara joint positions
	CalcLaraMatrices(0);
	phd_PushUnitMatrix();
	CalcLaraMatrices(1);

	// Calls my new rock & roll renderer :)
	g_Renderer->Draw();
	nFrames = g_Renderer->SyncRenderer();

	// We need to pop the matrix stack or the game will crash
	MatrixPtr -= 12;
	DxMatrixPtr -= 48;

	return nFrames;
}

__int32 DrawGame()
{
	// Control routines uses joints calculated here for getting Lara joint positions
	CalcLaraMatrices(0);
	phd_PushUnitMatrix();
	CalcLaraMatrices(1);

	// Calls my new rock & roll renderer :)
	g_Renderer->Draw();
	nFrames = g_Renderer->SyncRenderer();

	// We need to pop the matrix stack or the game will crash
	MatrixPtr -= 12;
	DxMatrixPtr -= 48;

	return nFrames;
}

__int32 DrawInventory()
{
	return g_Renderer->DrawInventory();
}

void __cdecl DoBootScreen(__int32 language)
{
	//printf("DoBootScreen\n");
	//DrawFullScreenImage((char*)"load.bmp");
}

void __cdecl DrawPistolsMeshes(__int32 weapon)
{
	Lara.holster = 13;
	g_LaraExtra.overrideMeshes[HAND_R] = true;
	if (weapon != WEAPON_REVOLVER)
		g_LaraExtra.overrideMeshes[HAND_L] = true;
}

void __cdecl DrawShotgunMeshes(__int32 weapon)
{
	Lara.backGun = NULL;
	g_LaraExtra.drawWeapon = true;
}

void __cdecl UndrawPistolsMeshLeft(__int32 weapon)
{
	if (weapon != WEAPON_REVOLVER)
	{
		WeaponObject(weapon);
		g_LaraExtra.overrideMeshes[HAND_L] = false;

		if (weapon == WEAPON_PISTOLS)
		{
			Lara.holster = ID_LARA_HOLSTERS_PISTOLS;
		}
		else if (weapon == WEAPON_UZI)
		{
			Lara.holster = ID_LARA_HOLSTERS_UZIS;
		}
	}
}

void __cdecl UndrawPistolsMeshRight(__int32 weapon)
{
	if (weapon != WEAPON_REVOLVER)
	{
		WeaponObject(weapon);
		g_LaraExtra.drawWeapon = false;

		if (weapon == WEAPON_PISTOLS)
		{
			Lara.holster = ID_LARA_HOLSTERS_PISTOLS;
		}
		else if (weapon == WEAPON_UZI)
		{
			Lara.holster = ID_LARA_HOLSTERS_UZIS;
		}
		else if (weapon == WEAPON_REVOLVER)
		{
			Lara.holster = ID_LARA_HOLSTERS_REVOLVER;
		}
	}
}

void __cdecl UndrawShotgunMeshes(__int32 weapon)
{
	Lara.backGun = WeaponObject(weapon);
	g_LaraExtra.drawWeapon = false;
}

void Inject_Draw()
{
	/*INJECT(0x0044DBF0, UndrawShotgunMeshes);
	INJECT(0x0044FED0, UndrawPistolsMeshLeft);
	INJECT(0x0044FF40, UndrawPistolsMeshRight);
	INJECT(0x0044DBB0, DrawShotgunMeshes);
	INJECT(0x0044FE60, DrawPistolsMeshes);*/
	//INJECT(0x0042A400, DrawPhaseGame);
	INJECT(0x004B8A80, DoBootScreen);
}