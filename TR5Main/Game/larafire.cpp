#include "larafire.h"

#include "items.h"
#include "lara.h"
#include "laraflar.h"
#include "lara1gun.h"
#include "lara2gun.h"

#include "..\Scripting\GameFlowScript.h"

#include <stdio.h>

WEAPON_INFO Weapons[NUM_WEAPONS] =
{
	/* No weapons */
	{ { ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) }, 0x0000, 0x0000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x0000 },
	/* Pistols */
	{ { 54616, ANGLE(60), 54616, ANGLE(60) },{ 34596, ANGLE(60), 50976, ANGLE(80) },{ 54616, ANGLE(170), 50976, ANGLE(80) }, 0x071C, 0x05B0, 0x028A, 0x2000, 100, 0x09, 0x03, 0x00, 0x0008 },
	/* Revolver */
	{ { 54616, ANGLE(60), 54616, ANGLE(60) },{ 63716, ANGLE(10), 50976, ANGLE(80) },{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) }, 0x071C, 0x02D8, 0x028A, 0x2000, 0x15, 0x10, 0x03, 0x00, 0x0079 },
	/* Uzis */
	{ { 54616, ANGLE(60), 54616, ANGLE(60) },{ 34596, ANGLE(60), 50976, ANGLE(80) },{ 54616, ANGLE(170), 50976, ANGLE(80) }, 0x071C, 0x05B0, 0x028A, 0x2000, 0x01, 0x03, 0x03, 0x00, 0x002B },
	/* Shotgun */
	{ { 54616, ANGLE(60), 55526, ANGLE(55) },{ 50976, ANGLE(80), 53706, ANGLE(65) },{ 50976, ANGLE(80), 53706, ANGLE(65) }, 0x071C, 0x0000, 0x01F4, 0x2000, 0x03, 0x09, 0x03, 0x0A, 0x002D },
	/* HK */
	{ { 54616, ANGLE(60), 55526, ANGLE(55) },{ 50976, ANGLE(80), 53706, ANGLE(65) },{ 50976, ANGLE(80), 53706, ANGLE(65) }, 0x071C, 0x02D8, 0x01F4, 0x3000, 0x04, 0x00, 0x03, 0x10, 0x0000 },
	/* Crossbow */
	{ { 54616, ANGLE(60), 55526, ANGLE(55) },{ 50976, ANGLE(80), 53706, ANGLE(65) },{ 50976, ANGLE(80), 53706, ANGLE(65) }, 0x071C, 0x05B0, 0x01F4, 0x2000, 0x05, 0x00, 0x02, 0x0A, 0x0000 },
	/* Flare */
	{ { ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) }, 0x0000, 0x0000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x0000 },
	/* Flare 2 */
	{ { 60076, ANGLE(30), 55526, ANGLE(55) },{ 60076, ANGLE(30), 55526, ANGLE(55) },{ 60076, ANGLE(30), 55526, ANGLE(55) }, 0x071C, 0x05B0, 0x0190, 0x2000, 0x03, 0x00, 0x02, 0x00, 0x002B },
	/* Grenade launcher */
	{ { -ANGLE(60), ANGLE(60), -ANGLE(55), ANGLE(55) },{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) }, ANGLE(10), ANGLE(8), 500, 8 * WALL_SIZE, 20, 0, 2, 10, 0 }, 
	/* Harpoon gun */
	{ { -ANGLE(60), ANGLE(60), -ANGLE(65), ANGLE(65) },{ -ANGLE(20), ANGLE(20), -ANGLE(75), ANGLE(75) },{ -ANGLE(80), ANGLE(80), -ANGLE(75), ANGLE(75) }, ANGLE(10), ANGLE(8), 500, 8 * WALL_SIZE, 6, 0, 2, 10, 0 },
	/* Rocket launcher */
	{ { -ANGLE(60), ANGLE(60), -ANGLE(55), ANGLE(55) },{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) }, ANGLE(10), ANGLE(8), 500, 8 * WALL_SIZE, 30, 0, 2, 12, 77 }
};

extern GameFlow* g_GameFlow;

__int32 __cdecl WeaponObject(__int32 weaponType)
{
	switch (weaponType)
	{
	case WEAPON_UZI:
		return ID_UZI_ANIM;
	case WEAPON_SHOTGUN:
		return ID_SHOTGUN_ANIM;
	case WEAPON_REVOLVER:
		return ID_REVOLVER_ANIM;
	case WEAPON_CROSSBOW:
		return ID_CROSSBOW_ANIM;
	case WEAPON_HK:
		return ID_HK_ANIM;
	case WEAPON_FLARE:
		return ID_FLARE_ANIM;
	case WEAPON_GRENADE:
		return ID_GRENADE_ANIM;
	case WEAPON_ROCKET:
		return ID_ROCKET_ANIM;
	case WEAPON_HARPOON:
		return ID_HARPOON_ANIM;
	default:
		return ID_PISTOLS_ANIM;
	}
}

void __cdecl SmashItem(__int16 itemNum)
{
	/*ITEM_INFO* item = &Items[itemNum];
	__int16 objectNumber = item->objectNumber;
	printf("SmashItem\n");
	if (objectNumber >= ID_SMASH_OBJECT1 && objectNumber <= ID_SMASH_OBJECT8)
		SmashObject(itemNum);
	else if (objectNumber == ID_BELL_SWITCH)
	{
		if (item->status != ITEM_ACTIVE)
		{
			item->status = ITEM_ACTIVE;
			AddActiveItem(itemNum);
		}
	}*/
}

void __cdecl LaraGun()
{
	__int32 meshIndex;

	if (Lara.leftArm.flash_gun > 0)
		--Lara.leftArm.flash_gun;
	if (Lara.rightArm.flash_gun > 0)
		--Lara.rightArm.flash_gun;

	if (Lara.gunType == WEAPON_TORCH)
	{
		DoFlameTorch();
		return;
	}

	__int32 gunStatus = Lara.gunStatus;

	if (LaraItem->hitPoints <= 0)
	{
		gunStatus = LG_NO_ARMS;
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (!Lara.gunStatus)
	{
		if (TrInput & IN_DRAW)
			Lara.requestGunType = Lara.lastGunType;
		else if ((TrInput & IN_FLARE) && (g_GameFlow->GetLevel(CurrentLevel)->LaraType != LARA_DRAW_TYPE::LARA_YOUNG))
		{
			if (LaraItem->currentAnimState == 71 && LaraItem->animNumber != 222)
				return;

			if (Lara.gunType == WEAPON_FLARE && !Lara.leftArm.frameNumber)
				Lara.gunStatus = LG_UNDRAW_GUNS;
			else if (Lara.numFlares)
			{
				if (Lara.numFlares != -1)
					Lara.numFlares--;
				Lara.requestGunType = WEAPON_FLARE;
			}
		}

		if ((Lara.requestGunType != Lara.gunType) || (TrInput & IN_DRAW))
		{
			if ((LaraItem->currentAnimState == 71 || LaraItem->currentAnimState == 105 || LaraItem->currentAnimState == 106) &&
				(Lara.requestGunType == WEAPON_HK || Lara.requestGunType == WEAPON_CROSSBOW || Lara.requestGunType == WEAPON_FLARE))
			{
				if (Lara.gunType != WEAPON_FLARE)
					Lara.requestGunType = WEAPON_FLARE;
			}
			else if (Lara.requestGunType != WEAPON_FLARE)
			{
				if (Lara.waterStatus)
				{
					if (Lara.waterStatus != 4 || Lara.waterSurfaceDist <= -Weapons[Lara.gunType].gunHeight)
					{
						Lara.lastGunType = Lara.requestGunType;
						if (Lara.gunType != WEAPON_FLARE)
							Lara.gunType = Lara.requestGunType;
						else
							Lara.requestGunType = WEAPON_FLARE;
					}
				}
			}
			else if (Lara.gunType == WEAPON_FLARE)
			{
				CreateFlare(ID_FLARE_ITEM, 0);
				UndrawFlaresMeshes();
				Lara.flareControlLeft;
				Lara.flareAge = 0;
			}

			Lara.gunType = Lara.requestGunType;
			InitialiseNewWeapon();
			Lara.rightArm.frameNumber = 0;
			Lara.leftArm.frameNumber = 0;
			Lara.gunStatus = LG_DRAW_GUNS;
		}
	}
	else if (Lara.gunStatus == LG_READY)
	{
		if ((TrInput & IN_DRAW) || Lara.requestGunType != Lara.gunType)  
			Lara.gunStatus = LG_UNDRAW_GUNS;
		else if (Lara.gunType != WEAPON_HARPOON && Lara.waterStatus != LW_ABOVE_WATER &&
			(Lara.waterStatus != LW_WADE || Lara.waterSurfaceDist < -Weapons[Lara.gunType].gunHeight)) 
			Lara.gunStatus = LG_UNDRAW_GUNS;
	}
	else if (Lara.gunStatus == LG_HANDS_BUSY
		&& TrInput & IN_FLARE
		&& LaraItem->currentAnimState == 80
		&& LaraItem->animNumber == 263)
		Lara.requestGunType = WEAPON_FLARE;

	switch (Lara.gunStatus)
	{
	case LG_DRAW_GUNS:
		if (Lara.gunType != WEAPON_FLARE && Lara.gunType)
			Lara.lastGunType = Lara.gunType;
		switch (Lara.gunType)
		{
		case WEAPON_PISTOLS:
		case WEAPON_REVOLVER:
		case WEAPON_UZI:
			if (Camera.type != CAMERA_TYPE::CINEMATIC_CAMERA && Camera.type != CAMERA_TYPE::LOOK_CAMERA && 
				Camera.type != CAMERA_TYPE::HEAVY_CAMERA)
				Camera.type = CAMERA_TYPE::COMBAT_CAMERA;
			DrawPistols(Lara.gunType);
			break;

			case WEAPON_SHOTGUN:
			case WEAPON_CROSSBOW:
			case WEAPON_HK:
			case WEAPON_GRENADE:
			case WEAPON_ROCKET:
			case WEAPON_HARPOON:

			if (Camera.type != CAMERA_TYPE::CINEMATIC_CAMERA && Camera.type != CAMERA_TYPE::LOOK_CAMERA &&
				Camera.type != CAMERA_TYPE::HEAVY_CAMERA)
				Camera.type = CAMERA_TYPE::COMBAT_CAMERA;
			DrawShotgun(Lara.gunType);
			break;

		case WEAPON_FLARE:
			DrawFlare();
			break;

		default:
			Lara.gunStatus = LG_NO_ARMS;
			break;
		}

		break;

	case LG_SPECIAL:
		DrawFlare();
		break;

	case LG_UNDRAW_GUNS:
		Lara.meshPtrs[HEAD] = Meshes[Objects[ID_LARA].meshIndex + 28];

		switch (Lara.gunType)
		{
		case WEAPON_PISTOLS:
		case WEAPON_REVOLVER:
		case WEAPON_UZI:
			UndrawPistols(Lara.gunType);
			break;

		case WEAPON_SHOTGUN:
		case WEAPON_CROSSBOW:
		case WEAPON_HK:
		case WEAPON_GRENADE:
		case WEAPON_ROCKET:
		case WEAPON_HARPOON:
			UndrawShotgun(Lara.gunType);
			break;

		case WEAPON_FLARE:
			UndrawFlare();
			break;

		default:
			return;
		}
		break;

	case LG_READY:
		meshIndex = Objects[ID_LARA_SCREAM].meshIndex;
		if (!(TrInput & IN_ACTION))
			meshIndex = Objects[ID_LARA].meshIndex;
		Lara.meshPtrs[HEAD] = Meshes[meshIndex + 28];
		
		if (Camera.type != CAMERA_TYPE::CINEMATIC_CAMERA && Camera.type != CAMERA_TYPE::LOOK_CAMERA &&
			Camera.type != CAMERA_TYPE::HEAVY_CAMERA)
			Camera.type = CAMERA_TYPE::COMBAT_CAMERA;

		if (TrInput & IN_ACTION)
		{
			if (!GetAmmo(Lara.gunType))
			{
				Lara.requestGunType = Objects[ID_PISTOLS_ITEM].loaded;
				return;
			}
		}

		switch (Lara.gunType)
		{
		case WEAPON_PISTOLS:
		case WEAPON_UZI:
			PistolsHandler(Lara.gunType);
			break;

		case WEAPON_SHOTGUN:
		case WEAPON_CROSSBOW:
		case WEAPON_HK:
		case WEAPON_GRENADE:
		case WEAPON_ROCKET:
		case WEAPON_HARPOON:
		case WEAPON_REVOLVER:
			RifleHandler(Lara.gunType);
			break;

		default:
			return;
		}
		break;

	case LG_NO_ARMS:
		if (Lara.gunType == WEAPON_FLARE)
		{
			if (g_LaraExtra.vehicle != NO_ITEM || CheckForHoldingState(LaraItem->currentAnimState))
			{
				if (Lara.flareControlLeft)
				{
					if (Lara.leftArm.frameNumber)
					{
						if (++Lara.leftArm.frameNumber == 110)
							Lara.leftArm.frameNumber = 0;
					}
				}
				else
				{
					Lara.leftArm.frameNumber = 95;
					Lara.flareControlLeft = true;
				}
			}
			else
			{
				Lara.flareControlLeft = false;
			}

			DoFlareInHand(Lara.flareAge);
			SetFlareArm(Lara.leftArm.frameNumber);
		}
		break;

	case LG_HANDS_BUSY:
		if (Lara.gunType == WEAPON_FLARE)
		{
			if (Lara.meshPtrs[HAND_L] == Meshes[Objects[ID_FLARE_ANIM].meshIndex + 26])
			{
				Lara.flareControlLeft = (g_LaraExtra.vehicle != NO_ITEM || CheckForHoldingState(LaraItem->currentAnimState));
				DoFlareInHand(Lara.flareAge);
				SetFlareArm(Lara.leftArm.frameNumber);
			}
		}
		break;

	}
}

__int16* __cdecl GetAmmo(__int32 weaponType)
{
	switch (weaponType)
	{
	case WEAPON_SHOTGUN:
		if (Lara.shotgunTypeCarried & 8)
			return &Lara.numShotgunAmmo1;
		else
			return &Lara.numShotgunAmmo2;

	case WEAPON_REVOLVER:
		return &Lara.numRevolverAmmo;

	case WEAPON_UZI:
		return &Lara.numUziAmmo;

	case WEAPON_HK:
		return &Lara.numHKammo1;

	case WEAPON_CROSSBOW:
		if (Lara.crossbowTypeCarried & 8)
			return &Lara.numCrossbowAmmo2;
		else
			return &Lara.numCrossbowAmmo1;

	case WEAPON_GRENADE:
		return &g_LaraExtra.numGrenadeAmmos;

	case WEAPON_HARPOON:
		return &g_LaraExtra.numHarpoonAmmos;

	case WEAPON_ROCKET:
		return &g_LaraExtra.numRocketAmmos;

	default:
		return &Lara.numPistolsAmmo;

	}
}

void __cdecl InitialiseNewWeapon()
{
	Lara.rightArm.frameNumber = 0;
	Lara.leftArm.frameNumber = 0;
	Lara.leftArm.zRot = 0;
	Lara.leftArm.yRot = 0;
	Lara.leftArm.xRot = 0;
	Lara.rightArm.zRot = 0;
	Lara.rightArm.yRot = 0;
	Lara.rightArm.xRot = 0;
	Lara.target = 0;
	Lara.rightArm.lock = 0;
	Lara.leftArm.lock = 0;
	Lara.rightArm.flash_gun = 0;
	Lara.leftArm.flash_gun = 0;

	switch (Lara.gunType)
	{
	case WEAPON_PISTOLS:
	case WEAPON_UZI:
		Lara.rightArm.frameBase = Objects[ID_PISTOLS_ANIM].frameBase;
		Lara.leftArm.frameBase = Objects[ID_PISTOLS_ANIM].frameBase;
		if (Lara.gunStatus)
			DrawPistolMeshes(Lara.gunType);
		break;

	case WEAPON_SHOTGUN:
	case WEAPON_REVOLVER:
	case WEAPON_HK:
	case WEAPON_GRENADE:
	case WEAPON_HARPOON:
	case WEAPON_ROCKET:
		Lara.rightArm.frameBase = Objects[WeaponObject(Lara.gunType)].frameBase;
		Lara.leftArm.frameBase = Lara.rightArm.frameBase;
		if (Lara.gunStatus)
			DrawShotgunMeshes(Lara.gunType);
		break;

	case WEAPON_FLARE:
		Lara.rightArm.frameBase = Objects[ID_FLARE_ANIM].frameBase;
		Lara.leftArm.frameBase = Objects[ID_FLARE_ANIM].frameBase;
		if (Lara.gunStatus)
			DrawFlaresMeshes();
		break;

	default:
		Lara.rightArm.frameBase = Anims[LaraItem->animNumber].framePtr;
		Lara.leftArm.frameBase = Anims[LaraItem->animNumber].framePtr;
		break;
	}
}

__int32 __cdecl WeaponObjectMesh(__int32 weaponType)
{
	switch (weaponType)
	{
	case WEAPON_REVOLVER:
		return ((Lara.sixshooterTypeCarried & 4) != 0 ? ID_LARA_REVOLVER_LASER : ID_REVOLVER_ANIM);

	case WEAPON_UZI:
		return ID_UZI_ANIM;

	case WEAPON_SHOTGUN:
		return ID_SHOTGUN_ANIM;

	case WEAPON_HK:
		return ID_HK_ANIM;

	case WEAPON_CROSSBOW:
		return ((Lara.crossbowTypeCarried & 4) != 0 ? ID_LARA_CROSSBOW_LASER : ID_CROSSBOW_ANIM);
		
	case WEAPON_GRENADE:
		return ID_GRENADE_ANIM;

	case WEAPON_HARPOON:
		return ID_HARPOON_ANIM;

	case WEAPON_ROCKET:
		return ID_ROCKET_ANIM;

	default:
		return ID_PISTOLS_ANIM;

	}
}

void Inject_LaraFire()
{
	INJECT(0x00453AE0, WeaponObject);
	INJECT(0x00452430, LaraGun);
	INJECT(0x004546C0, GetAmmo);
	INJECT(0x00452B30, InitialiseNewWeapon);
	INJECT(0x00453B50, WeaponObjectMesh);
	//INJECT(0x00453A90, SmashItem);
}