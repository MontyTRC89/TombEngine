#include "framework.h"
#include "lara_fire.h"
#include "items.h"
#include "lara_flare.h"
#include "lara_one_gun.h"
#include "lara_two_guns.h"
#include "camera.h"
#include "objects.h"
#include "effect.h"
#include "sphere.h"
#include "draw.h"
#include "effect2.h"
#include "flmtorch.h"
#include "level.h"
#include "lot.h"
#include "setup.h"
#include "input.h"
#include "sound.h"
#include "savegame.h"
#include "GameFlowScript.h"
#include "lara_struct.h"

WEAPON_INFO Weapons[NUM_WEAPONS] =
{
	/* No weapons */
	{
		{ ANGLE(0.0f), ANGLE(0.0f), ANGLE(0.0f), ANGLE(0.0f) },
		{ ANGLE(0.0f), ANGLE(0.0f), ANGLE(0.0f), ANGLE(0.0f) },
		{ ANGLE(0.0f), ANGLE(0.0f), ANGLE(0.0f), ANGLE(0.0f) },
		0,
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
		{ -ANGLE(60.0f),  ANGLE(60.0f),  -ANGLE(60.0f), ANGLE(60.0f) },
		{ -ANGLE(170.0f), ANGLE(60.0f),  -ANGLE(80.0f), ANGLE(80.0f) },
		{ -ANGLE(60.0f),  ANGLE(170.0f), -ANGLE(80.0f), ANGLE(80.0f) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		650,
		SECTOR(8),
		1,
		9,
		3,
		0,
		SFX_LARA_FIRE,
		0
	},
	/* Revolver */
	{
		{ -ANGLE(60.0f), ANGLE(60.0f), -ANGLE(60.0f), ANGLE(60.0f) },
		{ -ANGLE(10.0f), ANGLE(10.0f), -ANGLE(80.0f), ANGLE(80.0f) },
		{  ANGLE(0.0f),   ANGLE(0.0f),   ANGLE(0.0f),  ANGLE(0.0f) },
		ANGLE(10.0f),
		ANGLE(4.0f),
		650,
		SECTOR(8),
		21,
		16,
		3,
		0,
		SFX_REVOLVER,
		0
	},
	/* Uzis */
	{
		{ -ANGLE(60.0f), ANGLE(60.0f), -ANGLE(60.0f), ANGLE(60.0f) },
		{ -ANGLE(170.0f), ANGLE(60.0f),  -ANGLE(80.0f), ANGLE(80.0f) },
		{ -ANGLE(60.0f),  ANGLE(170.0f), -ANGLE(80.0f), ANGLE(80.0f) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		650,
		SECTOR(8),
		1,
		3,
		3,
		0,
		SFX_LARA_UZI_FIRE,
		0
	},
	/* Shotgun */
	{
		{ -ANGLE(60.0f), ANGLE(60.0f), -ANGLE(55.0f), ANGLE(55.0f) },
		{ -ANGLE(80.0f), ANGLE(80.0f), -ANGLE(65.0f), ANGLE(65.0f) },
		{ -ANGLE(80.0f), ANGLE(80.0f), -ANGLE(65.0f), ANGLE(65.0f) },
		ANGLE(10.0f),
		0,
		500,
		SECTOR(8),
		3,
		9,
		3,
		10,
		SFX_LARA_SHOTGUN,
		0
	},
	/* HK */
	{
		{ -ANGLE(60.0f), ANGLE(60.0f), -ANGLE(55.0f), ANGLE(55.0f) },
		{ -ANGLE(80.0f), ANGLE(80.0f), -ANGLE(65.0f), ANGLE(65.0f) },
		{ -ANGLE(80.0f), ANGLE(80.0f), -ANGLE(65.0f), ANGLE(65.0f) },
		ANGLE(10.0f),
		ANGLE(4.0f),
		500,
		SECTOR(12),
		4,
		0,
		3,
		10,
		0,     // FIRE/SILENCER_FIRE
		0
	},
	/* Crossbow */
	{
		{ -ANGLE(60.0f), ANGLE(60.0f), -ANGLE(55.0f), ANGLE(55.0f) },
		{ -ANGLE(80.0f), ANGLE(80.0f), -ANGLE(65.0f), ANGLE(65.0f) },
		{ -ANGLE(80.0f), ANGLE(80.0f), -ANGLE(65.0f), ANGLE(65.0f) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		500,
		SECTOR(8),
		5,
		0,
		2,
		10,
		SFX_LARA_CROSSBOW,
		20
	},
	/* Flare */
	{
		{ ANGLE(0.0f), ANGLE(0.0f), ANGLE(0.0f), ANGLE(0.0f) },
		{ ANGLE(0.0f), ANGLE(0.0f), ANGLE(0.0f), ANGLE(0.0f) },
		{ ANGLE(0.0f), ANGLE(0.0f), ANGLE(0.0f), ANGLE(0.0f) },
		0,
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
		{ -ANGLE(30.0f), ANGLE(30.0f), -ANGLE(55.0f), ANGLE(55.0f) },
		{ -ANGLE(30.0f), ANGLE(30.0f), -ANGLE(55.0f), ANGLE(55.0f) },
		{ -ANGLE(30.0f), ANGLE(30.0f), -ANGLE(55.0f), ANGLE(55.0f) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		400,
		SECTOR(8),
		3,
		0,
		2,
		0,
		SFX_LARA_UZI_FIRE,
		0
	},
	/* Grenade launcher */
	{
		{ -ANGLE(60.0f), ANGLE(60.0f), -ANGLE(55.0f), ANGLE(55.0f) },
		{ -ANGLE(80.0f), ANGLE(80.0f), -ANGLE(65.0f), ANGLE(65.0f) },
		{ -ANGLE(80.0f), ANGLE(80.0f), -ANGLE(65.0f), ANGLE(65.0f) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		500,
		SECTOR(8),
		20,
		0,
		2,
		10,
		0,
		30
	},
	/* Harpoon gun */
	{
		{ -ANGLE(60.0f), ANGLE(60.0f), -ANGLE(65.0f), ANGLE(65.0f) },
		{ -ANGLE(20.0f), ANGLE(20.0f), -ANGLE(75.0f), ANGLE(75.0f) },
		{ -ANGLE(80.0f), ANGLE(80.0f), -ANGLE(75.0f), ANGLE(75.0f) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		500,
		SECTOR(8),
		6,
		0,
		2,
		10,
		0,
		0
	},
	/* Rocket launcher */
	{
		{ -ANGLE(60.0f), ANGLE(60.0f), -ANGLE(55.0f), ANGLE(55.0f) },
		{ -ANGLE(80.0f), ANGLE(80.0f), -ANGLE(65.0f), ANGLE(65.0f) },
		{ -ANGLE(80.0f), ANGLE(80.0f), -ANGLE(65.0f), ANGLE(65.0f) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		500,
		SECTOR(8),
		30,
		0,
		2,
		12,
		77,
		30
	},
	/* Snowmobile */
	{
		{ -ANGLE(30.0f), ANGLE(30.0f), -ANGLE(55.0f), ANGLE(55.0f) },
		{ -ANGLE(30.0f), ANGLE(30.0f), -ANGLE(55.0f), ANGLE(55.0f) },
		{ -ANGLE(30.0f), ANGLE(30.0f), -ANGLE(55.0f), ANGLE(55.0f) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		400,
		SECTOR(8),
		3,
		0,
		0,
		0,
		SFX_LARA_UZI_FIRE,
		0
	}
};

short HoldStates[] = {
	LS_WALK_FORWARD,
	LS_RUN_FORWARD,
	LS_STOP,
	LS_POSE,
	LS_TURN_RIGHT_SLOW,
	LS_TURN_LEFT_SLOW,
	LS_WALK_BACK,
	LS_TURN_FAST,
	LS_STEP_RIGHT,
	LS_STEP_LEFT,
	LS_PICKUP,
	LS_SWITCH_DOWN,
	LS_SWITCH_UP,
	LS_WADE_FORWARD,
	LS_CROUCH_IDLE,
	LS_CROUCH_TURN_LEFT,
	LS_CROUCH_TURN_RIGHT,
	-1
};

extern GameFlow* g_GameFlow;
bool MonksAttackLara;
ITEM_INFO* LastTargets[8];
ITEM_INFO* TargetList[8];

int WeaponObject(int weaponType) // (F) (D)
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
		return ID_LARA_FLARE_ANIM;
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

void AimWeapon(WEAPON_INFO* winfo, LARA_ARM* arm) // (F) (D)
{
	short rotY, rotX, speed, x, y;

	speed = winfo->aimSpeed;

	// Have target lock, so get XY angles for arms.
	if (arm->lock)
	{
		y = Lara.targetAngles[0];
		x = Lara.targetAngles[1];
	}
	// No target lock, so aim straight.
	else
	{
		y = 0;
		x = 0;
	}

	// Rotate arms on y axis toward target.
	rotY = arm->yRot;
	if ((rotY >= y - speed) && (rotY <= y + speed))
		rotY = y;
	else if (rotY < y)
		rotY += speed;
	else
		rotY -= speed;
	arm->yRot = rotY;

	// Rotate arms on x axis toward target.
	rotX = arm->xRot;
	if ((rotX >= x - speed) && (rotX <= x + speed))
		rotX = x;
	else if (rotX < x)
		rotX += speed;
	else
		rotX -= speed;
	arm->xRot = rotX;

	// TODO: set arm rotations to inherit rotations of parent Bones. -Sezz
	arm->zRot = 0;
}

void SmashItem(short itemNum) // (F) (D)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	if (item->objectNumber >= ID_SMASH_OBJECT1 && item->objectNumber <= ID_SMASH_OBJECT8)
		SmashObject(itemNum);
}

void LaraGun() // (F) (D)
{
	if (Lara.leftArm.flash_gun > 0)
		--Lara.leftArm.flash_gun;
	if (Lara.rightArm.flash_gun > 0)
		--Lara.rightArm.flash_gun;

	if (Lara.gunType == WEAPON_TORCH)
	{
		DoFlameTorch();
		return;
	}

	if (LaraItem->hitPoints <= 0)
	{
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (Lara.gunStatus == LG_NO_ARMS)
	{
		if (TrInput & IN_DRAW)
		{
			Lara.requestGunType = Lara.lastGunType;
		}
		else if (TrInput & IN_FLARE && (g_GameFlow->GetLevel(CurrentLevel)->LaraType != LARA_YOUNG))
		{
			if (LaraItem->currentAnimState == LS_CROUCH_IDLE && LaraItem->animNumber != LA_CROUCH_IDLE)
				return;

			if (Lara.gunType == WEAPON_FLARE)
			{
				if (!Lara.leftArm.frameNumber)
				{
					Lara.gunStatus = LG_UNDRAW_GUNS;
				}
			}
			else if (Lara.NumFlares)
			{
				if (Lara.NumFlares != -1)
					Lara.NumFlares--;
				Lara.requestGunType = WEAPON_FLARE;
			}
		}

		if ((Lara.requestGunType != Lara.gunType) || (TrInput & IN_DRAW))
		{
			if ((LaraItem->currentAnimState == LS_CROUCH_IDLE
				|| LaraItem->currentAnimState == LS_CROUCH_TURN_LEFT
				|| LaraItem->currentAnimState == LS_CROUCH_TURN_RIGHT)
				&& (Lara.requestGunType == WEAPON_HK
				|| Lara.requestGunType == WEAPON_CROSSBOW
				|| Lara.requestGunType == WEAPON_SHOTGUN
				|| Lara.requestGunType == WEAPON_HARPOON_GUN))
			{
				if (Lara.gunType == WEAPON_FLARE)
					Lara.requestGunType = WEAPON_FLARE;
			}
			else if (Lara.requestGunType == WEAPON_FLARE
				|| (Lara.Vehicle == NO_ITEM
				&& (Lara.requestGunType == WEAPON_HARPOON_GUN
				|| Lara.waterStatus == LW_ABOVE_WATER
				|| (Lara.waterStatus == LW_WADE
				&& Lara.waterSurfaceDist > -Weapons[Lara.gunType].gunHeight))))
			{
				if (Lara.gunType == WEAPON_FLARE)
				{
					CreateFlare(ID_FLARE_ITEM, 0);
					undraw_flare_meshes();
					Lara.flareControlLeft = false;
					Lara.flareAge = 0;
				}

				Lara.gunType = Lara.requestGunType;
				InitialiseNewWeapon();
				Lara.rightArm.frameNumber = 0;
				Lara.leftArm.frameNumber = 0;
				Lara.gunStatus = LG_DRAW_GUNS;
			}
			else
			{
				Lara.lastGunType = Lara.requestGunType;
				if (Lara.gunType != WEAPON_FLARE)
					Lara.gunType = Lara.requestGunType;
				else
					Lara.requestGunType = WEAPON_FLARE;
			}
		}
	}
	else if (Lara.gunStatus == LG_READY)
	{
		if ((TrInput & IN_DRAW) 
			|| Lara.requestGunType != Lara.gunType)
			Lara.gunStatus = LG_UNDRAW_GUNS;
		else if (Lara.gunType != WEAPON_HARPOON_GUN 
			&& Lara.waterStatus != LW_ABOVE_WATER 
			&& (Lara.waterStatus != LW_WADE 
				|| Lara.waterSurfaceDist < -Weapons[Lara.gunType].gunHeight)) 
			Lara.gunStatus = LG_UNDRAW_GUNS;
	}
	else if (Lara.gunStatus == LG_HANDS_BUSY 
		&& (TrInput & IN_FLARE) 
		&& LaraItem->currentAnimState == LS_CRAWL_IDLE 
		&& LaraItem->animNumber == LA_CRAWL_IDLE)
	{
		Lara.requestGunType = WEAPON_FLARE;
	}

	switch (Lara.gunStatus)
	{
		case LG_DRAW_GUNS:
			if (Lara.gunType != WEAPON_FLARE && Lara.gunType != WEAPON_NONE)
				Lara.lastGunType = Lara.gunType;

		switch (Lara.gunType)
		{
			case WEAPON_PISTOLS:
			case WEAPON_REVOLVER:
			case WEAPON_UZI:
				if (Camera.type != CINEMATIC_CAMERA && Camera.type != LOOK_CAMERA && Camera.type != HEAVY_CAMERA)
					Camera.type = COMBAT_CAMERA;
				draw_pistols(Lara.gunType);
				break;

			case WEAPON_SHOTGUN:
			case WEAPON_CROSSBOW:
			case WEAPON_HK:
			case WEAPON_GRENADE_LAUNCHER:
			case WEAPON_ROCKET_LAUNCHER:
			case WEAPON_HARPOON_GUN:
				if (Camera.type != CINEMATIC_CAMERA && Camera.type != LOOK_CAMERA && Camera.type != HEAVY_CAMERA)
					Camera.type = COMBAT_CAMERA;
				draw_shotgun(Lara.gunType);
				break;

			case WEAPON_FLARE:
				draw_flare();
				break;

			default:
				Lara.gunStatus = LG_NO_ARMS;
				break;
		}
		break;

		case LG_SPECIAL:
			draw_flare();
			break;

		case LG_UNDRAW_GUNS:
			Lara.meshPtrs[LM_HEAD] = Objects[ID_LARA_SKIN].meshIndex + LM_HEAD;

			switch (Lara.gunType)
			{
				case WEAPON_PISTOLS:
				case WEAPON_REVOLVER:
				case WEAPON_UZI:
					undraw_pistols(Lara.gunType);
					break;

				case WEAPON_SHOTGUN:
				case WEAPON_CROSSBOW:
				case WEAPON_HK:
				case WEAPON_GRENADE_LAUNCHER:
				case WEAPON_ROCKET_LAUNCHER:
				case WEAPON_HARPOON_GUN:
					undraw_shotgun(Lara.gunType);
					break;

				case WEAPON_FLARE:
					undraw_flare();
					break;

				default:
					return;
			}
			break;

		case LG_READY:
			if (!(TrInput & IN_ACTION))
				Lara.meshPtrs[LM_HEAD] = Objects[ID_LARA_SKIN].meshIndex + LM_HEAD;
			else
				Lara.meshPtrs[LM_HEAD] = Objects[ID_LARA_SCREAM].meshIndex + LM_HEAD;
		
			if (Camera.type != CINEMATIC_CAMERA && Camera.type != LOOK_CAMERA && Camera.type != HEAVY_CAMERA)
				Camera.type = COMBAT_CAMERA;

			if (TrInput & IN_ACTION)
			{
				if (!*GetAmmo(Lara.gunType))
				{
					Lara.requestGunType = Objects[ID_PISTOLS_ITEM].loaded ? WEAPON_PISTOLS : WEAPON_NONE;
					return;
				}
			}

			switch (Lara.gunType)
			{
				case WEAPON_PISTOLS:
				case WEAPON_UZI:
					PistolHandler(Lara.gunType);
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
				if (Lara.Vehicle != NO_ITEM || CheckForHoldingState(LaraItem->currentAnimState))
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
				set_flare_arm(Lara.leftArm.frameNumber);
			}
			break;

		case LG_HANDS_BUSY:
			if (Lara.gunType == WEAPON_FLARE)
			{
				if (Lara.meshPtrs[LM_LHAND] == Objects[ID_LARA_FLARE_ANIM].meshIndex + LM_LHAND)
				{
					Lara.flareControlLeft = (Lara.Vehicle != NO_ITEM || CheckForHoldingState(LaraItem->currentAnimState));
					DoFlareInHand(Lara.flareAge);
					set_flare_arm(Lara.leftArm.frameNumber);
				}
			}
			break;
	}
}

short* GetAmmo(int weaponType)
{
	return &Lara.Weapons[weaponType].Ammo[Lara.Weapons[weaponType].SelectedAmmo];
}

void InitialiseNewWeapon()
{
	Lara.rightArm.frameNumber = 0;
	Lara.leftArm.frameNumber = 0;
	Lara.leftArm.zRot = 0;
	Lara.leftArm.yRot = 0;
	Lara.leftArm.xRot = 0;
	Lara.rightArm.zRot = 0;
	Lara.rightArm.yRot = 0;
	Lara.rightArm.xRot = 0;
	Lara.target = nullptr;
	Lara.rightArm.lock = false;
	Lara.leftArm.lock = false;
	Lara.rightArm.flash_gun = 0;
	Lara.leftArm.flash_gun = 0;

	switch (Lara.gunType)
	{
	case WEAPON_PISTOLS:
	case WEAPON_UZI:
		Lara.rightArm.frameBase = Objects[ID_PISTOLS_ANIM].frameBase;
		Lara.leftArm.frameBase = Objects[ID_PISTOLS_ANIM].frameBase;
		if (Lara.gunStatus != LG_NO_ARMS)
			draw_pistol_meshes(Lara.gunType);
		break;

	case WEAPON_SHOTGUN:
	case WEAPON_REVOLVER:
	case WEAPON_HK:
	case WEAPON_GRENADE_LAUNCHER:
	case WEAPON_HARPOON_GUN:
	case WEAPON_ROCKET_LAUNCHER:
		Lara.rightArm.frameBase = Objects[WeaponObject(Lara.gunType)].frameBase;
		Lara.leftArm.frameBase = Objects[WeaponObject(Lara.gunType)].frameBase;
		if (Lara.gunStatus != LG_NO_ARMS)
			draw_shotgun_meshes(Lara.gunType);
		break;

	case WEAPON_FLARE:
		Lara.rightArm.frameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;
		Lara.leftArm.frameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;
		if (Lara.gunStatus != LG_NO_ARMS)
			draw_flare_meshes();
		break;

	default:
		Lara.rightArm.frameBase = g_Level.Anims[LaraItem->animNumber].framePtr;
		Lara.leftArm.frameBase = g_Level.Anims[LaraItem->animNumber].framePtr;
		break;
	}
}

int WeaponObjectMesh(int weaponType)
{
	switch (weaponType)
	{
	case WEAPON_REVOLVER:
		return (Lara.Weapons[WEAPON_REVOLVER].HasLasersight == true ? ID_LARA_REVOLVER_LASER : ID_REVOLVER_ANIM);

	case WEAPON_UZI:
		return ID_UZI_ANIM;

	case WEAPON_SHOTGUN:
		return ID_SHOTGUN_ANIM;

	case WEAPON_HK:
		return ID_HK_ANIM;

	case WEAPON_CROSSBOW:
		return (Lara.Weapons[WEAPON_CROSSBOW].HasLasersight == true ? ID_LARA_CROSSBOW_LASER : ID_CROSSBOW_ANIM);
		
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

void HitTarget(ITEM_INFO* item, GAME_VECTOR* hitPos, int damage, int flag)
{
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];
	
	item->hitStatus = true;
	if (creature != nullptr && item != LaraItem)
		creature->hurtByLara = true;

	if (hitPos != nullptr)
	{
		if (obj->hitEffect != HIT_NONE)
		{
			switch (obj->hitEffect)
			{
				case 1:
					DoBloodSplat(hitPos->x, hitPos->y, hitPos->z, (GetRandomControl() & 3) + 3, item->pos.yRot, item->roomNumber);
					break;
				case 3:
					TriggerRicochetSpark(hitPos, LaraItem->pos.yRot, 3, 0);
					break;
				case 2:
					TriggerRicochetSpark(hitPos, LaraItem->pos.yRot, 3, -5);
					SoundEffect(SFX_SWORD_GOD_HITMET, &item->pos, 0);
					break;
			}
		}
	}

	if ((!obj->undead || flag) && item->hitPoints != NOT_TARGETABLE)
	{
		if (item->hitPoints > 0 && item->hitPoints <= damage)
			++Savegame.Level.Kills;
		item->hitPoints -= damage;
	}
}

FireWeaponType FireWeapon(int weaponType, ITEM_INFO* target, ITEM_INFO* src, short* angles) // (F) (D)
{
	short* ammo = GetAmmo(weaponType);
	if (*ammo == 0)
		return FW_NOAMMO;
	if (*ammo != -1)
		*ammo--;

	WEAPON_INFO* weapon = &Weapons[weaponType];
	int r;

	PHD_VECTOR pos, muzzleOffset;
	GetLaraJointPosition(&muzzleOffset, LM_RHAND);
	pos.x = src->pos.xPos;
	pos.y = muzzleOffset.y;
	pos.z = src->pos.zPos;
	PHD_3DPOS rotation;
	rotation.xRot = angles[1] + (GetRandomControl() - 0x4000) * weapon->shotAccuracy / 0x10000;
	rotation.yRot = angles[0] + (GetRandomControl() - 0x4000) * weapon->shotAccuracy / 0x10000;
	rotation.zRot = 0;

	// Calculate ray from rotation angles
	float x =  sin(TO_RAD(rotation.yRot)) * cos(TO_RAD(rotation.xRot));
	float y = -sin(TO_RAD(rotation.xRot));
	float z =  cos(TO_RAD(rotation.yRot)) * cos(TO_RAD(rotation.xRot));
	Vector3 direction = Vector3(x, y, z);
	direction.Normalize();
	Vector3 source = Vector3(pos.x, pos.y, pos.z);
	Vector3 destination = source + direction * weapon->targetDist;
	Ray ray = Ray(source, direction);

	int num = GetSpheres(target, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
	int best = NO_ITEM;
	float bestDistance = FLT_MAX;

	for (int i = 0; i < num; i++)
	{
		BoundingSphere sphere = BoundingSphere(Vector3(CreatureSpheres[i].x, CreatureSpheres[i].y, CreatureSpheres[i].z), CreatureSpheres[i].r);
		float distance;
		if (ray.Intersects(sphere, distance))
		{
			if (distance < bestDistance)
			{
				bestDistance = distance;
				best = i;
			}
		}
	}

	Lara.hasFired = true;
	Lara.fired = true;
	
	GAME_VECTOR vSrc;
	vSrc.x = pos.x;
	vSrc.y = pos.y;
	vSrc.z = pos.z;
	short roomNumber = src->roomNumber;
	GetFloor(pos.x, pos.y, pos.z, &roomNumber);
	vSrc.roomNumber = roomNumber;

	if (best < 0)
	{
		GAME_VECTOR vDest;
		vDest.x = destination.x;
		vDest.y = destination.y;
		vDest.z = destination.z;
		GetTargetOnLOS(&vSrc, &vDest, FALSE, TRUE);
		return FW_MISS;
	}
	else
	{
		Savegame.Game.AmmoHits++;
		destination = source + direction * bestDistance;

		GAME_VECTOR vDest;
		vDest.x = destination.x;
		vDest.y = destination.y;
		vDest.z = destination.z;

		// TODO: enable it when the slot is created !
		/*
		if (target->objectNumber == ID_TRIBEBOSS)
		{
			long dx, dy, dz;

			dx = (vDest.x - vSrc.x) / 32;
			dy = (vDest.y - vSrc.y) / 32;
			dz = (vDest.z - vSrc.z) / 32;
			FindClosestShieldPoint(vDest.x - dx, vDest.y - dy, vDest.z - dz, target);
		}
		else if (target->objectNumber == ID_ARMY_WINSTON || target->objectNumber == ID_LONDONBOSS) //Don't want blood on Winston - never get the stains out
		{
			short ricochet_angle;
			target->hitStatus = true; //need to do this to maintain defence state
			target->hitPoints--;
			ricochet_angle = (mGetAngle(LaraItem->pos.zPos, LaraItem->pos.xPos, target->pos.zPos, target->pos.xPos) / 16) & 4095;
			TriggerRicochetSparks(&vDest, ricochet_angle, 16, 0);
			SoundEffect(SFX_LARA_RICOCHET, &target->pos, 0);		// play RICOCHET Sample
		}
		else if (target->objectNumber == ID_SHIVA) //So must be Shiva
		{
			z = target->pos.zPos - lara_item->pos.zPos;
			x = target->pos.xPos - lara_item->pos.xPos;
			angle = 0x8000 + phd_atan(z, x) - target->pos.yRot;

			if ((target->currentAnimState > 1 && target->currentAnimState < 5) && angle < 0x4000 && angle > -0x4000)
			{
				target->hitStatus = true; //need to do this to maintain defence state
				ricochet_angle = (mGetAngle(LaraItem->pos.zPos, LaraItem->pos.xPos, target->pos.zPos, target->pos.xPos) / 16) & 4095;
				TriggerRicochetSparks(&vDest, ricochet_angle, 16, 0);
				SoundEffect(SFX_LARA_RICOCHET, &target->pos, 0); // play RICOCHET Sample
			}
			else //Shiva's not in defence mode or has its back to Lara
				HitTarget(target, &vDest, weapon->damage, 0);
		}
		else
		{*/
			if (!GetTargetOnLOS(&vSrc, &vDest, FALSE, TRUE))
				HitTarget(target, &vDest, weapon->damage, NULL);
		//}
		
		return FW_MAYBEHIT;
	}
}

void find_target_point(ITEM_INFO* item, GAME_VECTOR* target) // (F) (D)
{
	BOUNDING_BOX* bounds;
	int x, y, z;
	float c, s;

	bounds = (BOUNDING_BOX*)GetBestFrame(item);
	x = (int)(bounds->X1 + bounds->X2) / 2;
	y = (int) bounds->Y1 + (bounds->Y2 - bounds->Y1) / 3;
	z = (int)(bounds->Z1 + bounds->Z2) / 2;
	c = phd_cos(item->pos.yRot);
	s = phd_sin(item->pos.yRot);

	target->x = item->pos.xPos + c * x + s * z;
	target->y = item->pos.yPos + y;
	target->z = item->pos.zPos + c * z - s * x;
	target->roomNumber = item->roomNumber;
}

void LaraTargetInfo(WEAPON_INFO* weapon) // (F) (D)
{
	if (Lara.target == nullptr)
	{
		Lara.rightArm.lock = false;
		Lara.leftArm.lock = false;
		Lara.targetAngles[1] = 0;
		Lara.targetAngles[0] = 0;
		return;
	}

	GAME_VECTOR src, targetPoint;
	PHD_VECTOR muzzleOffset;
	short angles[2];

	GetLaraJointPosition(&muzzleOffset, LM_RHAND);
	src.x = LaraItem->pos.xPos;
	src.y = muzzleOffset.y;
	src.z = LaraItem->pos.zPos;
	src.roomNumber = LaraItem->roomNumber;
	find_target_point(Lara.target, &targetPoint);
	phd_GetVectorAngles(targetPoint.x - src.x, targetPoint.y - src.y, targetPoint.z - src.z, angles);

	angles[0] -= LaraItem->pos.yRot;
	angles[1] -= LaraItem->pos.xRot;

	if (LOS(&src, &targetPoint))
	{
		if (angles[0] >= weapon->lockAngles[0]
		&&  angles[0] <= weapon->lockAngles[1]
		&&  angles[1] >= weapon->lockAngles[2]
		&&  angles[1] <= weapon->lockAngles[3])
		{
			Lara.rightArm.lock = true;
			Lara.leftArm.lock = true;
		}
		else
		{
			if (Lara.leftArm.lock)
			{
				if ((angles[0] < weapon->leftAngles[0] ||
					 angles[0] > weapon->leftAngles[1] ||
					 angles[1] < weapon->leftAngles[2] ||
					 angles[1] > weapon->leftAngles[3]))
					Lara.leftArm.lock = false;
			}

			if (Lara.rightArm.lock)
			{
				if ((angles[0] < weapon->rightAngles[0] ||
					 angles[0] > weapon->rightAngles[1] ||
					 angles[1] < weapon->rightAngles[2] ||
					 angles[1] > weapon->rightAngles[3]))
					Lara.rightArm.lock = false;
			}
		}
	}
	else
	{
		Lara.rightArm.lock = false;
		Lara.leftArm.lock = false;
	}

	Lara.targetAngles[0] = angles[0];
	Lara.targetAngles[1] = angles[1];
}

bool CheckForHoldingState(int state) // (F) (D)
{
#if 0
	if (Lara.ExtraAnim != NO_ITEM)
		return false;
#endif

	short* holdState = HoldStates;
	while (*holdState >= 0)
	{
		if (state == *holdState)
			return true;
		holdState++;
	}
	
	return false;
}

void LaraGetNewTarget(WEAPON_INFO* weapon) // (F) (D)
{
	GAME_VECTOR src, target;
	PHD_VECTOR muzzleOffset;
	int bestDistance, maxDistance, targets, slot, x, y, z, distance;
	ITEM_INFO* bestItem, *item;
	short bestYrot, angle[2], match;
	bool flag, loop;

	if (BinocularRange)
	{
		Lara.target = nullptr;
		return;
	}

	GetLaraJointPosition(&muzzleOffset, LM_RHAND);
	src.x = LaraItem->pos.xPos;
	src.y = muzzleOffset.y;
	src.z = LaraItem->pos.zPos;
	src.roomNumber = LaraItem->roomNumber;

	bestItem = NULL;
	bestYrot = MAXSHORT;
	bestDistance = MAXINT;
	maxDistance = weapon->targetDist;
	targets = 0;

	for (slot = 0; slot < NUM_SLOTS; ++slot)
	{
		if (BaddieSlots[slot].itemNum != NO_ITEM)
		{
			item = &g_Level.Items[BaddieSlots[slot].itemNum];
			if (item->hitPoints > 0)
			{
				x = item->pos.xPos - src.x;
				y = item->pos.yPos - src.y;
				z = item->pos.zPos - src.z;
				if (abs(x) <= maxDistance && abs(y) <= maxDistance && abs(z) <= maxDistance)
				{
					distance = SQUARE(x) + SQUARE(y) + SQUARE(z);
					if (distance < SQUARE(maxDistance))
					{
						find_target_point(item, &target);
						if (LOS(&src, &target))
						{
							phd_GetVectorAngles(target.x - src.x, target.y - src.y, target.z - src.z, angle);
							angle[0] -= LaraItem->pos.yRot + Lara.torsoYrot;
							angle[1] -= LaraItem->pos.xRot + Lara.torsoXrot;
							if (angle[0] >= weapon->lockAngles[0] && angle[0] <= weapon->lockAngles[1] && angle[1] >= weapon->lockAngles[2] && angle[1] <= weapon->lockAngles[3])
							{
								TargetList[targets] = item;
								++targets;
								if (abs(angle[0]) < bestYrot + ANGLE(15.0f) && distance < bestDistance)
								{
									bestDistance = distance;
									bestYrot = abs(angle[0]);
									bestItem = item;
								}
							}
						}
					}
				}
			}
		}
	}

	TargetList[targets] = NULL;
	if (!TargetList[0])
	{
		Lara.target = NULL;
	}
	else
	{
		for (slot = 0; slot < 8; ++slot)
		{
			if (!TargetList[slot])
				Lara.target = NULL;
			if (TargetList[slot] == Lara.target)
				break;
		}
		if (Lara.gunStatus != LG_NO_ARMS || TrInput & IN_LOOKSWITCH)
		{
			if (!Lara.target)
			{
				Lara.target = bestItem;
				LastTargets[0] = NULL;
			}
			else if (TrInput & IN_LOOKSWITCH)
			{
				Lara.target = NULL;
				flag = true;
				for (match = 0; match < 8 && TargetList[match]; ++match)
				{
					loop = false;
					for (slot = 0; slot < 8 && LastTargets[slot]; ++slot)
					{
						if (LastTargets[slot] == TargetList[match])
						{
							loop = true;
							break;
						}
					}
					if (!loop)
					{
						Lara.target = TargetList[match];
						if (Lara.target)
							flag = false;
						break;
					}
				}
				if (flag)
				{
					Lara.target = bestItem;
					LastTargets[0] = NULL;
				}
			}
		}
	}

	if (Lara.target != LastTargets[0])
	{
		for (slot = 7; slot > 0; --slot)
			LastTargets[slot] = LastTargets[slot - 1];
		LastTargets[0] = Lara.target;
	}

	LaraTargetInfo(weapon);
}

void DoProperDetection(short itemNumber, int x, int y, int z, int xv, int yv, int zv)
{
	HEIGHT_TYPES oldtype;
	int ceiling, height, oldonobj, oldheight;
	int bs, yang;

	ITEM_INFO* item = &g_Level.Items[itemNumber];

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	oldheight = GetFloorHeight(floor, x, y, z);
	oldonobj = OnObject;
	oldtype = HeightType;

	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	if (item->pos.yPos >= height)
	{
		bs = 0;

		if ((HeightType == BIG_SLOPE || HeightType == DIAGONAL) && oldheight < height)
		{
			yang = (long)((unsigned short)item->pos.yRot);
			if (TiltYOffset < 0)
			{
				if (yang >= 0x8000)
					bs = 1;
			}
			else if (TiltYOffset > 0)
			{
				if (yang <= 0x8000)
					bs = 1;
			}

			if (TiltXOffset < 0)
			{
				if (yang >= 0x4000 && yang <= 0xc000)
					bs = 1;
			}
			else if (TiltXOffset > 0)
			{
				if (yang <= 0x4000 || yang >= 0xc000)
					bs = 1;
			}
		}

		/* If last position of item was also below this floor height, we've hit a wall, else we've hit a floor */

		if (y > (height + 32) && bs == 0 &&
			(((x / SECTOR(1)) != (item->pos.xPos / SECTOR(1))) ||
			((z / SECTOR(1)) != (item->pos.zPos / SECTOR(1)))))
		{
			// Need to know which direction the wall is.

			long	xs;

			if ((x & (~(WALL_SIZE - 1))) != (item->pos.xPos & (~(WALL_SIZE - 1))) &&	// X crossed boundary?
				(z & (~(WALL_SIZE - 1))) != (item->pos.zPos & (~(WALL_SIZE - 1))))	// Z crossed boundary as well?
			{
				if (abs(x - item->pos.xPos) < abs(z - item->pos.zPos))
					xs = 1;	// X has travelled the shortest, so (maybe) hit first. (Seems to work ok).
				else
					xs = 0;
			}
			else
				xs = 1;

			if ((x & (~(WALL_SIZE - 1))) != (item->pos.xPos & (~(WALL_SIZE - 1))) && xs)	// X crossed boundary?
			{
				if (xv <= 0)	// Hit angle = 0xc000.
					item->pos.yRot = 0x4000 + (0xc000 - item->pos.yRot);
				else			// Hit angle = 0x4000.
					item->pos.yRot = 0xc000 + (0x4000 - item->pos.yRot);
			}
			else		// Z crossed boundary.
				item->pos.yRot = 0x8000 - item->pos.yRot;

			item->speed /= 2;

			/* Put item back in its last position */
			item->pos.xPos = x;
			item->pos.yPos = y;
			item->pos.zPos = z;
		}
		else if (HeightType == BIG_SLOPE || HeightType == DIAGONAL) 	// Hit a steep slope?
		{
			// Need to know which direction the slope is.

			item->speed -= (item->speed / 4);

			if (TiltYOffset < 0 && ((abs(TiltYOffset)) - (abs(TiltXOffset)) >= 2))	// Hit angle = 0x4000
			{
				if (((unsigned short)item->pos.yRot) > 0x8000)
				{
					item->pos.yRot = 0x4000 + (0xc000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed -= TiltYOffset * 2;
						if ((unsigned short)item->pos.yRot > 0x4000 && (unsigned short)item->pos.yRot < 0xc000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0x4000)
								item->pos.yRot = 0x4000;
						}
						else if ((unsigned short)item->pos.yRot < 0x4000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0x4000)
								item->pos.yRot = 0x4000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (TiltYOffset > 0 && ((abs(TiltYOffset)) - (abs(TiltXOffset)) >= 2))	// Hit angle = 0xc000
			{
				if (((unsigned short)item->pos.yRot) < 0x8000)
				{
					item->pos.yRot = 0xc000 + (0x4000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += TiltYOffset * 2;
						if ((unsigned short)item->pos.yRot > 0xc000 || (unsigned short)item->pos.yRot < 0x4000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0xc000)
								item->pos.yRot = 0xc000;
						}
						else if ((unsigned short)item->pos.yRot < 0xc000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0xc000)
								item->pos.yRot = 0xc000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (TiltXOffset < 0 && ((abs(TiltXOffset)) - (abs(TiltYOffset)) >= 2))	// Hit angle = 0
			{
				if (((unsigned short)item->pos.yRot) > 0x4000 && ((unsigned short)item->pos.yRot) < 0xc000)
				{
					item->pos.yRot = (0x8000 - item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed -= TiltXOffset * 2;

						if ((unsigned short)item->pos.yRot < 0x8000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot > 0xf000)
								item->pos.yRot = 0;
						}
						else if ((unsigned short)item->pos.yRot >= 0x8000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot < 0x1000)
								item->pos.yRot = 0;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (TiltXOffset > 0 && ((abs(TiltXOffset)) - (abs(TiltYOffset)) >= 2))	// Hit angle = 0x8000
			{
				if (((unsigned short)item->pos.yRot) > 0xc000 || ((unsigned short)item->pos.yRot) < 0x4000)
				{
					item->pos.yRot = (0x8000 - item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += TiltXOffset * 2;

						if ((unsigned short)item->pos.yRot > 0x8000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0x8000)
								item->pos.yRot = 0x8000;
						}
						else if ((unsigned short)item->pos.yRot < 0x8000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0x8000)
								item->pos.yRot = 0x8000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (TiltYOffset < 0 && TiltXOffset < 0)	// Hit angle = 0x2000
			{
				if (((unsigned short)item->pos.yRot) > 0x6000 && ((unsigned short)item->pos.yRot) < 0xe000)
				{
					item->pos.yRot = 0x2000 + (0xa000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += (-TiltYOffset) + (-TiltXOffset);
						if ((unsigned short)item->pos.yRot > 0x2000 && (unsigned short)item->pos.yRot < 0xa000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0x2000)
								item->pos.yRot = 0x2000;
						}
						else if ((unsigned short)item->pos.yRot != 0x2000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0x2000)
								item->pos.yRot = 0x2000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (TiltYOffset < 0 && TiltXOffset > 0)	// Hit angle = 0x6000
			{
				if (((unsigned short)item->pos.yRot) > 0xa000 || ((unsigned short)item->pos.yRot) < 0x2000)
				{
					item->pos.yRot = 0x6000 + (0xe000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += (-TiltYOffset) + TiltXOffset;
						if ((unsigned short)item->pos.yRot < 0xe000 && (unsigned short)item->pos.yRot > 0x6000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0x6000)
								item->pos.yRot = 0x6000;
						}
						else if ((unsigned short)item->pos.yRot != 0x6000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0x6000)
								item->pos.yRot = 0x6000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (TiltYOffset > 0 && TiltXOffset > 0)	// Hit angle = 0xa000
			{
				if (((unsigned short)item->pos.yRot) > 0xe000 || ((unsigned short)item->pos.yRot) < 0x6000)
				{
					item->pos.yRot = 0xa000 + (0x2000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += TiltYOffset + TiltXOffset;
						if ((unsigned short)item->pos.yRot < 0x2000 || (unsigned short)item->pos.yRot > 0xa000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0xa000)
								item->pos.yRot = 0xa000;
						}
						else if ((unsigned short)item->pos.yRot != 0xa000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0xa000)
								item->pos.yRot = 0xa000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (TiltYOffset > 0 && TiltXOffset < 0)	// Hit angle = 0xe000
			{
				if (((unsigned short)item->pos.yRot) > 0x2000 && ((unsigned short)item->pos.yRot) < 0xa000)
				{
					item->pos.yRot = 0xe000 + (0x6000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += TiltYOffset + (-TiltXOffset);
						if ((unsigned short)item->pos.yRot < 0x6000 || (unsigned short)item->pos.yRot > 0xe000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0xe000)
								item->pos.yRot = 0xe000;
						}
						else if ((unsigned short)item->pos.yRot != 0xe000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0xe000)
								item->pos.yRot = 0xe000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}

			/* Put item back in its last position */
			item->pos.xPos = x;
			item->pos.yPos = y;
			item->pos.zPos = z;
		}
		else
		{
			/* Hit the floor; bounce and slow down */
			if (item->fallspeed > 0)
			{
				if (item->fallspeed > 16)
				{
					if (item->objectNumber == ID_GRENADE)
						item->fallspeed = -(item->fallspeed - (item->fallspeed / 2));
					else
					{
						item->fallspeed = -(item->fallspeed / 2);
						if (item->fallspeed < -100)
							item->fallspeed = -100;
					}
				}
				else
				{
					/* Roll on floor */
					item->fallspeed = 0;
					if (item->objectNumber == ID_GRENADE)
					{
						item->requiredAnimState = 1;
						item->pos.xRot = 0;
						item->speed--;
					}
					else
						item->speed -= 3;

					if (item->speed < 0)
						item->speed = 0;
				}
			}
			item->pos.yPos = height;
		}
	}
	else	// Check for on top of object.
	{
		if (yv >= 0)
		{
			roomNumber = item->roomNumber;
			floor = GetFloor(item->pos.xPos, y, item->pos.zPos, &roomNumber);
			oldheight = GetFloorHeight(floor, item->pos.xPos, y, item->pos.zPos);
			oldonobj = OnObject;
			roomNumber = item->roomNumber;
			floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
			GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

			/* Bounce off floor */
			if (item->pos.yPos >= oldheight && oldonobj)	// If old and new pos above object then detect.
			{
				/* Hit the floor; bounce and slow down */
				if (item->fallspeed > 0)
				{
					if (item->fallspeed > 16)
					{
						if (item->objectNumber == ID_GRENADE)
							item->fallspeed = -(item->fallspeed - (item->fallspeed / 2));
						else
						{
							item->fallspeed = -(item->fallspeed / 4);
							if (item->fallspeed < -100)
								item->fallspeed = -100;
						}
					}
					else
					{
						/* Roll on floor */
						item->fallspeed = 0;
						if (item->objectNumber == ID_GRENADE)
						{
							item->requiredAnimState = 1;
							item->pos.xRot = 0;
							item->speed--;
						}
						else
							item->speed -= 3;

						if (item->speed < 0)
							item->speed = 0;
					}
				}
				item->pos.yPos = oldheight;
			}
		}
		//		else
		{
			/* Bounce off ceiling */
			roomNumber = item->roomNumber;
			floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
			ceiling = GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
			if (item->pos.yPos < ceiling)
			{
				if (y < ceiling &&
					(((x / SECTOR(1)) != (item->pos.xPos / SECTOR(1))) ||
					((z / SECTOR(1)) != (item->pos.zPos / SECTOR(1)))))
				{
					// Need to know which direction the wall is.

					if ((x & (~(WALL_SIZE - 1))) != (item->pos.xPos & (~(WALL_SIZE - 1))))	// X crossed boundary?
					{
						if (xv <= 0)	// Hit angle = 0xc000.
							item->pos.yRot = 0x4000 + (0xc000 - item->pos.yRot);
						else			// Hit angle = 0x4000.
							item->pos.yRot = 0xc000 + (0x4000 - item->pos.yRot);
					}
					else		// Z crossed boundary.
					{
						item->pos.yRot = 0x8000 - item->pos.yRot;
					}

					if (item->objectNumber == ID_GRENADE)
						item->speed -= item->speed / 8;
					else
						item->speed /= 2;

					/* Put item back in its last position */
					item->pos.xPos = x;
					item->pos.yPos = y;
					item->pos.zPos = z;
				}
				else
					item->pos.yPos = ceiling;

				if (item->fallspeed < 0)
					item->fallspeed = -item->fallspeed;
			}
		}
	}

	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	if (roomNumber != item->roomNumber)
		ItemNewRoom(itemNumber, roomNumber);
}

HOLSTER_SLOT HolsterSlotForWeapon(LARA_WEAPON_TYPE weapon)
{
	switch(weapon){
		case WEAPON_PISTOLS:
			return HOLSTER_SLOT::Pistols;
		case WEAPON_UZI:
			return HOLSTER_SLOT::Uzis;
		case WEAPON_REVOLVER:
			return HOLSTER_SLOT::Revolver;
		case WEAPON_SHOTGUN:
			return HOLSTER_SLOT::Shotgun;
		case WEAPON_HK:
			return HOLSTER_SLOT::HK;
		case WEAPON_HARPOON_GUN:
			return HOLSTER_SLOT::Harpoon;
		case WEAPON_CROSSBOW:
			return HOLSTER_SLOT::Crowssbow;
		case WEAPON_GRENADE_LAUNCHER:
			return HOLSTER_SLOT::GrenadeLauncher;
		case WEAPON_ROCKET_LAUNCHER:
			return HOLSTER_SLOT::RocketLauncher;
		default:
			return HOLSTER_SLOT::Empty;
	}
}
