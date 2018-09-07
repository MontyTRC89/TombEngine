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

/*__int16 __cdecl GetFrames2(__int16 animNumber, __int16 frameNumber, __int16* framePtr[], __int16* rate)
{
	int v3; // ebp@1
	ANIM_STRUCT *v4; // edi@1
	__int16 *v5; // eax@1
	signed int v6; // esi@1
	signed int v7; // ebp@1
	signed int v8; // ebx@1
	int result; // eax@2
	int v10; // ecx@3
	int v11; // ebx@3

	ANIM_STRUCT* anim = &Anims[animNumber];
	framePtr[0] = framePtr[1] = anim->framePtr;
	__int16 rate2 = *rate = anim->interpolation & 0x00FF;

	
	frame_size = anim->interpolation >> 8;
	frm -= anim->frame_base;
	first = frm / rat;
	interp = frm % rat;
	frmptr[0] += first * frame_size;				  // Get Frame pointers
	frmptr[1] = frmptr[0] + frame_size;               // and store away
	if (interp == 0)
		return(0);
	second = first * rat + rat;
	if (second>anim->frame_end)                       // Clamp KeyFrame to End if need be
		*rate = anim->frame_end - (second - rat);
	return(interp);

	v7 = v3 - v4->frame_base;
	v8 = v4->interpolation >> 8;
	*(_DWORD *)a2 += 2 * v8 * (v7 / v6);
	*(_DWORD *)(a2 + 4) = *(_DWORD *)a2 + 2 * v8;
	if (v7 % v6)
	{
		v10 = v4->frame_end;
		v11 = v6 * (v7 / v6 + 1);
		if (v11 > v10)
			*a3 = v6 + v10 - v11;
		result = v7 % v6;
	}
	else
	{
		result = 0;
	}
	return result;
}*/

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