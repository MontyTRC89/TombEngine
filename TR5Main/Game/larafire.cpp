#include "larafire.h"

#include "items.h"
#include "Lara.h"
#include "laraflar.h"
#include "lara1gun.h"
#include "lara2gun.h"

#include "..\Scripting\GameFlowScript.h"

#include <stdio.h>

WEAPON_INFO Weapons[NUM_WEAPONS] =
{
	/* No weapons */
	{
		{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },
		{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },
		{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},
	/* Pistols */
	{
		{ -ANGLE(60),  +ANGLE(60),  -ANGLE(60), +ANGLE(60) },
		{ -ANGLE(170), +ANGLE(60),  -ANGLE(80), +ANGLE(80) },
		{ -ANGLE(60),  +ANGLE(170), -ANGLE(80), +ANGLE(80) },
		ANGLE(10),
		ANGLE(8),
		650,
		8 * WALL_SIZE,
		1,
		9,
		3,
		0,
		8
	},
	/* Revolver */
	{
		{ -ANGLE(60), +ANGLE(60), -ANGLE(60), +ANGLE(60) },
		{ -ANGLE(10), +ANGLE(10), -ANGLE(80), +ANGLE(80) },
		{  ANGLE(0),   ANGLE(0),   ANGLE(0),   ANGLE(0) },
		ANGLE(10),
		ANGLE(4),
		650,
		8 * WALL_SIZE,
		21,
		16,
		3,
		0,
		121
	},
	/* Uzis */
	{
		{ -ANGLE(60),  +ANGLE(60),  -ANGLE(60), +ANGLE(60) },
		{ -ANGLE(170), +ANGLE(60),  -ANGLE(80), +ANGLE(80) },
		{ -ANGLE(60),  +ANGLE(170), -ANGLE(80), +ANGLE(80) },
		ANGLE(10),
		ANGLE(8),
		650,
		8 * WALL_SIZE,
		1,
		3,
		3,
		0,
		43
	},
	/* Shotgun */
	{
		{ -ANGLE(60), +ANGLE(60), -ANGLE(55), +ANGLE(55) },
		{ -ANGLE(80), +ANGLE(80), -ANGLE(65), +ANGLE(65) },
		{ -ANGLE(80), +ANGLE(80), -ANGLE(65), +ANGLE(65) },
		ANGLE(10),
		0,
		500,
		8 * WALL_SIZE,
		3,
		9,
		3,
		10,
		45
	},
	/* HK */
	{
		{ -ANGLE(60), +ANGLE(60), -ANGLE(55), +ANGLE(55) },
		{ -ANGLE(80), +ANGLE(80), -ANGLE(65), +ANGLE(65) },
		{ -ANGLE(80), +ANGLE(80), -ANGLE(65), +ANGLE(65) },
		ANGLE(10),
		ANGLE(4),
		500,
		12 * WALL_SIZE,
		4,
		0,
		3,
		10,
		0
	},
	/* Crossbow */
	{
		{ -ANGLE(60), +ANGLE(60), -ANGLE(55), +ANGLE(55) },
		{ -ANGLE(80), +ANGLE(80), -ANGLE(65), +ANGLE(65) },
		{ -ANGLE(80), +ANGLE(80), -ANGLE(65), +ANGLE(65) },
		ANGLE(10),
		ANGLE(8),
		500,
		8 * WALL_SIZE,
		5,
		0,
		2,
		10,
		0
	},
	/* Flare */
	{
		{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },
		{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },
		{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},
	/* Flare 2 */
	{
		{ -ANGLE(30), +ANGLE(30), -ANGLE(55), ANGLE(55) },
		{ -ANGLE(30), +ANGLE(30), -ANGLE(55), ANGLE(55) },
		{ -ANGLE(30), +ANGLE(30), -ANGLE(55), ANGLE(55) },
		ANGLE(10),
		ANGLE(8),
		400,
		8 * WALL_SIZE,
		3,
		0,
		2,
		0,
		43
	},
	/* Grenade launcher */
	{
		{ -ANGLE(60), +ANGLE(60), -ANGLE(55), +ANGLE(55) },
		{ -ANGLE(80), +ANGLE(80), -ANGLE(65), +ANGLE(65) },
		{ -ANGLE(80), +ANGLE(80), -ANGLE(65), +ANGLE(65) },
		ANGLE(10),
		ANGLE(8),
		500,
		8 * WALL_SIZE,
		20,
		0, 
		2,
		10,
		0
	}, 
	/* Harpoon gun */
	{
		{ -ANGLE(60), +ANGLE(60), -ANGLE(65), +ANGLE(65) },
		{ -ANGLE(20), +ANGLE(20), -ANGLE(75), +ANGLE(75) },
		{ -ANGLE(80), +ANGLE(80), -ANGLE(75), +ANGLE(75) },
		ANGLE(10),
		ANGLE(8),
		500,
		8 * WALL_SIZE,
		6,
		0,
		2,
		10,
		0
	},
	/* Rocket launcher */
	{
		{ -ANGLE(60), +ANGLE(60), -ANGLE(55), +ANGLE(55) },
		{ -ANGLE(80), +ANGLE(80), -ANGLE(65), +ANGLE(65) },
		{ -ANGLE(80), +ANGLE(80), -ANGLE(65), +ANGLE(65) },
		ANGLE(10),
		ANGLE(8),
		500,
		8 * WALL_SIZE,
		30,
		0,
		2,
		12,
		77
	}
};

extern GameFlow* g_GameFlow;
extern LaraExtraInfo g_LaraExtra;

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
	case WEAPON_GRENADE_LAUNCHER:
		return ID_GRENADE_ANIM;
	case WEAPON_ROCKET_LAUNCHER:
		return ID_ROCKET_ANIM;
	case WEAPON_HARPOON_GUN:
		return ID_HARPOON_ANIM;
	default:
		return ID_PISTOLS_ANIM;
	}
}

void __cdecl AimWeapon(WEAPON_INFO* winfo, LARA_ARM* arm)
{
	__int16 rotY, rotX, speed = 0, x = 0, y = 0;

	speed = winfo->aimSpeed;

	if (arm->lock)
	{
		y = Lara.targetAngles[0];
		x = Lara.targetAngles[1];
	}
	else
	{
		y = 0;
		x = 0;
	}

	/* move y axis */
	rotY = arm->yRot;
	if ((rotY >= (y - speed)) && (rotY <= (y + speed)))
		rotY = y;
	else if (rotY < speed)
		rotY += speed;
	else
		rotY -= speed;
	arm->yRot = rotY;

	/* move x axis */
	rotX = arm->xRot;
	if ((rotX >= (x - speed)) && (rotX <= (x + speed)))
		rotX = x;
	else if (rotX < x)
		rotX += speed;
	else
		rotX -= speed;
	arm->xRot = rotX;

	/* move z axis */
	arm->zRot = 0;
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
		else if ((TrInput & IN_FLARE) && (g_GameFlow->GetLevel(CurrentLevel)->LaraType != LARA_YOUNG))
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
		else if (Lara.gunType != WEAPON_HARPOON_GUN && Lara.waterStatus != LW_ABOVE_WATER &&
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
			case WEAPON_GRENADE_LAUNCHER:
			case WEAPON_ROCKET_LAUNCHER:
			case WEAPON_HARPOON_GUN:

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
		Lara.meshPtrs[HEAD] = Meshes[Objects[ID_LARA].meshIndex + 14 * 2];

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
		case WEAPON_GRENADE_LAUNCHER:
		case WEAPON_ROCKET_LAUNCHER:
		case WEAPON_HARPOON_GUN:
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
		if (!(TrInput & IN_ACTION))
			meshIndex = Objects[ID_LARA].meshIndex;
		else
			meshIndex = Objects[ID_LARA_SCREAM].meshIndex;
		Lara.meshPtrs[HEAD] = Meshes[meshIndex + 14 * 2];
		
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
		case WEAPON_GRENADE_LAUNCHER:
		case WEAPON_ROCKET_LAUNCHER:
		case WEAPON_HARPOON_GUN:
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
			if (g_LaraExtra.Vehicle != NO_ITEM || CheckForHoldingState(LaraItem->currentAnimState))
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
				Lara.flareControlLeft = (g_LaraExtra.Vehicle != NO_ITEM || CheckForHoldingState(LaraItem->currentAnimState));
				DoFlareInHand(Lara.flareAge);
				SetFlareArm(Lara.leftArm.frameNumber);
			}
		}
		break;

	}
}

__int16* __cdecl GetAmmo(__int32 weaponType)
{
	return &g_LaraExtra.Weapons[weaponType].Ammo[g_LaraExtra.Weapons[weaponType].SelectedAmmo];
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
	case WEAPON_GRENADE_LAUNCHER:
	case WEAPON_HARPOON_GUN:
	case WEAPON_ROCKET_LAUNCHER:
		Lara.rightArm.frameBase = Objects[WeaponObject(Lara.gunType)].frameBase;
		Lara.leftArm.frameBase = Lara.rightArm.frameBase;
		if (Lara.gunStatus)
			DrawShotgunMeshes(Lara.gunType);
		break;

	case WEAPON_FLARE:
		Lara.rightArm.frameBase = Objects[ID_FLARE_ANIM].frameBase;
		Lara.leftArm.frameBase = Objects[ID_FLARE_ANIM].frameBase;
		if (Lara.gunStatus)
			DrawFlareMeshes();
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
		return ((g_LaraExtra.Weapons[WEAPON_REVOLVER].HasLasersight) != 0 ? ID_LARA_REVOLVER_LASER : ID_REVOLVER_ANIM);

	case WEAPON_UZI:
		return ID_UZI_ANIM;

	case WEAPON_SHOTGUN:
		return ID_SHOTGUN_ANIM;

	case WEAPON_HK:
		return ID_HK_ANIM;

	case WEAPON_CROSSBOW:
		return ((g_LaraExtra.Weapons[WEAPON_CROSSBOW].HasLasersight) != 0 ? ID_LARA_CROSSBOW_LASER : ID_CROSSBOW_ANIM);
		
	case WEAPON_GRENADE_LAUNCHER:
		return ID_GRENADE_ANIM;

	case WEAPON_HARPOON_GUN:
		return ID_HARPOON_ANIM;

	case WEAPON_ROCKET_LAUNCHER:
		return ID_ROCKET_ANIM;

	default:
		return ID_PISTOLS_ANIM;

	}
}

void Inject_LaraFire()
{
	INJECT(0x00453490, AimWeapon);
	INJECT(0x00453AE0, WeaponObject);
	INJECT(0x00452430, LaraGun);
	INJECT(0x004546C0, GetAmmo);
	INJECT(0x00452B30, InitialiseNewWeapon);
	INJECT(0x00453B50, WeaponObjectMesh);
	//INJECT(0x00453A90, SmashItem);
}