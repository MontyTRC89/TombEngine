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

__int32 __cdecl GetFrame_D2(ITEM_INFO* item, __int16* framePtr[], __int32* rate)
{
	/*__int32 frameNumber = item->frameNumber;
	ANIM_STRUCT* animation = &Anims[item->animNumber];

	framePtr[0] = animation->framePtr;
	framePtr[1] = animation->framePtr;

	__int16 rat = animation->interpolation & 0xFF;
	*rate = rat;

	__int32 frm = frameNumber - animation->frameBase;
	__int32 frameSize = animation->interpolation >> 8;

	framePtr[0] += rat * (frm / rat);
	framePtr[1] = framePtr[0] + rat;

	if (frm % rat)
	{
		__int32 second = rat * (frm / rat + 1);
		if (second > animation->frameEnd)
			*rate = rat + animation->frameEnd - second;
		return (frm % rat);
	}

	return 0;*/

	ANIM_STRUCT *anim;
	int			frm;
	int			first, second;
	int			frame_size;
	int			interp, rat;

	frm = item->frameNumber;
	anim = &Anims[item->animNumber];
	framePtr[0] = framePtr[1] = anim->framePtr;
	rat = *rate = anim->interpolation & 0x00ff;
	frame_size = anim->interpolation >> 8;
	frm -= anim->frameBase;
	first = frm / rat;
	interp = frm % rat;
	framePtr[0] += first * frame_size;				  // Get Frame pointers
	framePtr[1] = framePtr[0] + frame_size;               // and store away
	if (interp == 0)
		return(0);
	second = first * rat + rat;
	if (second>anim->frameEnd)                       // Clamp KeyFrame to End if need be
		*rate = anim->frameEnd - (second - rat);
	return(interp);
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