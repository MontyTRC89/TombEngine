#include "framework.h"
#include "lara_fire.h"
#include "items.h"
#include "lara_flare.h"
#include "lara_one_gun.h"
#include "lara_two_guns.h"
#include "camera.h"
#include "objects.h"
#include "effects/effects.h"
#include "sphere.h"
#include "animation.h"
#include "level.h"
#include "control/lot.h"
#include "setup.h"
#include "input.h"
#include "Sound/sound.h"
#include "control/los.h"
#include "savegame.h"
#include "GameFlowScript.h"
#include "lara_struct.h"
#include "itemdata/creature_info.h"
#include "Objects/Generic/Object/burning_torch.h"

using namespace TEN::Entities::Generic;

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
		SFX_TR4_LARA_FIRE,
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
		SFX_TR4_DESSERT_EAGLE_FIRE,
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
		SFX_TR4_LARA_UZI_FIRE,
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
		SFX_TR4_LARA_SHOTGUN,
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
		SFX_TR4_LARA_CROSSBOW,
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
		SFX_TR4_LARA_UZI_FIRE,
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
		SFX_TR4_LARA_UZI_FIRE,
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

bool MonksAttackLara;
ITEM_INFO* LastTargets[MAX_TARGETS];
ITEM_INFO* TargetList[MAX_TARGETS];

GAME_OBJECT_ID WeaponObject(int weaponType)
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

void AimWeapon(ITEM_INFO* lara, WEAPON_INFO* weaponInfo, LARA_ARM* arm)
{
	LaraInfo*& laraInfo = lara->data;
	short rotY, rotX, speed, x, y;

	speed = weaponInfo->aimSpeed;

	// Have target lock, so get XY angles for arms.
	if (arm->lock)
	{
		y = laraInfo->targetAngles[0];
		x = laraInfo->targetAngles[1];
	}
	// No target lock, so aim straight.
	else
	{
		y = 0;
		x = 0;
	}

	// Rotate arms on y axis toward target.
	rotY = arm->yRot;
	if (rotY >= (y - speed) && rotY <= (y + speed))
		rotY = y;
	else if (rotY < y)
		rotY += speed;
	else
		rotY -= speed;
	arm->yRot = rotY;

	// Rotate arms on x axis toward target.
	rotX = arm->xRot;
	if (rotX >= (x - speed) && rotX <= (x + speed))
		rotX = x;
	else if (rotX < x)
		rotX += speed;
	else
		rotX -= speed;
	arm->xRot = rotX;

	// TODO: Set arms to inherit rotations of parent bones.
	arm->zRot = 0;
}

void SmashItem(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	if (item->objectNumber >= ID_SMASH_OBJECT1 && item->objectNumber <= ID_SMASH_OBJECT8)
		SmashObject(itemNum);
}

void LaraGun(ITEM_INFO* laraItem)
{
	LaraInfo*& laraInfo = laraItem->data;

	if (laraInfo->leftArm.flash_gun > 0)
		--laraInfo->leftArm.flash_gun;
	if (laraInfo->rightArm.flash_gun > 0)
		--laraInfo->rightArm.flash_gun;

	if (laraInfo->gunType == WEAPON_TORCH)
	{
		DoFlameTorch();

		return;
	}

	if (laraItem->hitPoints <= 0)
		laraInfo->gunStatus = LG_NO_ARMS;
	else if (laraInfo->gunStatus == LG_NO_ARMS)
	{
		// Draw weapon.
		if (TrInput & IN_DRAW)
			laraInfo->requestGunType = laraInfo->lastGunType;
		// Draw flare.
		else if (TrInput & IN_FLARE &&
			(g_GameFlow->GetLevel(CurrentLevel)->LaraType != LaraType::Young))
		{
			if (laraItem->currentAnimState == LS_CROUCH_IDLE &&
				laraItem->animNumber != LA_CROUCH_IDLE)
			{
				return;
			}

			if (laraInfo->gunType == WEAPON_FLARE)
			{
			//	if (!laraInfo->leftArm.frameNumber)	//NO
				{
					laraInfo->gunStatus = LG_UNDRAW_GUNS;
				}
			}
			else if (laraInfo->NumFlares)
			{
				if (laraInfo->NumFlares != -1)
					laraInfo->NumFlares--;

				laraInfo->requestGunType = WEAPON_FLARE;
			}
		}

		if (TrInput & IN_DRAW ||
			laraInfo->requestGunType != laraInfo->gunType)
		{
			if ((laraItem->currentAnimState == LS_CROUCH_IDLE ||
				laraItem->currentAnimState == LS_CROUCH_TURN_LEFT ||
				laraItem->currentAnimState == LS_CROUCH_TURN_RIGHT) &&
				(laraInfo->requestGunType == WEAPON_HK ||
					laraInfo->requestGunType == WEAPON_CROSSBOW ||
					laraInfo->requestGunType == WEAPON_SHOTGUN ||
					laraInfo->requestGunType == WEAPON_HARPOON_GUN))
			{
				if (laraInfo->gunType == WEAPON_FLARE)
					laraInfo->requestGunType = WEAPON_FLARE;
			}
			else if (laraInfo->requestGunType == WEAPON_FLARE ||
				(laraInfo->Vehicle == NO_ITEM &&
					(laraInfo->requestGunType == WEAPON_HARPOON_GUN ||
						laraInfo->waterStatus == LW_ABOVE_WATER ||
						(laraInfo->waterStatus == LW_WADE &&
							laraInfo->waterSurfaceDist > -Weapons[laraInfo->gunType].gunHeight))))
			{
				if (laraInfo->gunType == WEAPON_FLARE)
				{
					CreateFlare(laraItem, ID_FLARE_ITEM, 0);
					UndrawFlareMeshes(laraItem);
					laraInfo->flareControlLeft = false;
					laraInfo->flareAge = 0;
				}

				laraInfo->gunType = laraInfo->requestGunType;
				InitialiseNewWeapon(laraItem);
				laraInfo->rightArm.frameNumber = 0;
				laraInfo->leftArm.frameNumber = 0;
				laraInfo->gunStatus = LG_DRAW_GUNS;
			}
			else
			{
				laraInfo->lastGunType = laraInfo->requestGunType;

				if (laraInfo->gunType != WEAPON_FLARE)
					laraInfo->gunType = laraInfo->requestGunType;
				else
					laraInfo->requestGunType = WEAPON_FLARE;
			}
		}
	}
	else if (laraInfo->gunStatus == LG_READY)
	{
		if (TrInput & IN_DRAW ||
			laraInfo->requestGunType != laraInfo->gunType)
		{
			laraInfo->gunStatus = LG_UNDRAW_GUNS;
		}
		else if (laraInfo->gunType != WEAPON_HARPOON_GUN &&
			laraInfo->waterStatus != LW_ABOVE_WATER &&
			(laraInfo->waterStatus != LW_WADE ||
				laraInfo->waterSurfaceDist < -Weapons[laraInfo->gunType].gunHeight))
		{
			laraInfo->gunStatus = LG_UNDRAW_GUNS;
		}
	}
	else if (TrInput & IN_FLARE &&
		laraInfo->gunStatus == LG_HANDS_BUSY &&
		laraItem->currentAnimState == LS_CRAWL_IDLE &&
		laraItem->animNumber == LA_CRAWL_IDLE)
	{
		laraInfo->requestGunType = WEAPON_FLARE;
	}

	switch (laraInfo->gunStatus)
	{
	case LG_DRAW_GUNS:
		if (laraInfo->gunType != WEAPON_FLARE &&
			laraInfo->gunType != WEAPON_NONE)
		{
			laraInfo->lastGunType = laraInfo->gunType;
		}

		switch (laraInfo->gunType)
		{
		case WEAPON_PISTOLS:
		case WEAPON_REVOLVER:
		case WEAPON_UZI:
			if (Camera.type != CAMERA_TYPE::LOOK_CAMERA && Camera.type != CAMERA_TYPE::HEAVY_CAMERA)
				Camera.type = CAMERA_TYPE::COMBAT_CAMERA;

			draw_pistols(laraInfo->gunType);

			break;

		case WEAPON_SHOTGUN:
		case WEAPON_CROSSBOW:
		case WEAPON_HK:
		case WEAPON_GRENADE_LAUNCHER:
		case WEAPON_ROCKET_LAUNCHER:
		case WEAPON_HARPOON_GUN:
			if (Camera.type != CAMERA_TYPE::LOOK_CAMERA && Camera.type != CAMERA_TYPE::HEAVY_CAMERA)
				Camera.type = CAMERA_TYPE::COMBAT_CAMERA;

			draw_shotgun(laraInfo->gunType);

			break;

		case WEAPON_FLARE:
			DrawFlare(laraItem);

			break;

		default:
			laraInfo->gunStatus = LG_NO_ARMS;

			break;
		}

		break;

	case LG_SPECIAL:
		DrawFlare(laraItem);

		break;

	case LG_UNDRAW_GUNS:
		laraInfo->meshPtrs[LM_HEAD] = Objects[ID_LARA_SKIN].meshIndex + LM_HEAD;

		switch (laraInfo->gunType)
		{
		case WEAPON_PISTOLS:
		case WEAPON_REVOLVER:
		case WEAPON_UZI:
			undraw_pistols(laraInfo->gunType);

			break;

		case WEAPON_SHOTGUN:
		case WEAPON_CROSSBOW:
		case WEAPON_HK:
		case WEAPON_GRENADE_LAUNCHER:
		case WEAPON_ROCKET_LAUNCHER:
		case WEAPON_HARPOON_GUN:
			undraw_shotgun(laraInfo->gunType);

			break;

		case WEAPON_FLARE:
			UndrawFlare(laraItem);

			break;

		default:
			return;
		}

		break;

	case LG_READY:
		if (!(TrInput & IN_ACTION))
			laraInfo->meshPtrs[LM_HEAD] = Objects[ID_LARA_SKIN].meshIndex + LM_HEAD;
		else
			laraInfo->meshPtrs[LM_HEAD] = Objects[ID_LARA_SCREAM].meshIndex + LM_HEAD;

		if (Camera.type != CAMERA_TYPE::LOOK_CAMERA &&
			Camera.type != CAMERA_TYPE::HEAVY_CAMERA)
		{
			Camera.type = CAMERA_TYPE::COMBAT_CAMERA;
		}

		if (TrInput & IN_ACTION)
		{
			if (!GetAmmo(laraItem, laraInfo->gunType))
			{
				laraInfo->requestGunType = Objects[ID_PISTOLS_ITEM].loaded ? WEAPON_PISTOLS : WEAPON_NONE;
				return;
			}
		}

		switch (laraInfo->gunType)
		{
		case WEAPON_PISTOLS:
		case WEAPON_UZI:
			PistolHandler(laraInfo->gunType);

			break;

		case WEAPON_SHOTGUN:
		case WEAPON_CROSSBOW:
		case WEAPON_HK:
		case WEAPON_GRENADE_LAUNCHER:
		case WEAPON_ROCKET_LAUNCHER:
		case WEAPON_HARPOON_GUN:
		case WEAPON_REVOLVER:
			RifleHandler(laraInfo->gunType);

			break;

		default:
			return;
		}

		break;

	case LG_NO_ARMS:
		if (laraInfo->gunType == WEAPON_FLARE)
		{
			if (laraInfo->Vehicle != NO_ITEM ||
				CheckForHoldingState(laraItem->currentAnimState))
			{
				if (laraInfo->flareControlLeft)
				{
					if (laraInfo->leftArm.frameNumber)
					{
						if (++laraInfo->leftArm.frameNumber == 110)
							laraInfo->leftArm.frameNumber = 0;
					}
				}
				else
				{
					laraInfo->leftArm.frameNumber = 95;
					laraInfo->flareControlLeft = true;
				}
			}
			else
				laraInfo->flareControlLeft = false;

			DoFlareInHand(laraItem, laraInfo->flareAge);
			SetFlareArm(laraItem, laraInfo->leftArm.frameNumber);
		}

		break;

	case LG_HANDS_BUSY:
		if (laraInfo->gunType == WEAPON_FLARE)
		{
			if (laraInfo->meshPtrs[LM_LHAND] == Objects[ID_LARA_FLARE_ANIM].meshIndex + LM_LHAND)
			{
				laraInfo->flareControlLeft = (laraInfo->Vehicle != NO_ITEM || CheckForHoldingState(laraItem->currentAnimState));
				DoFlareInHand(laraItem, laraInfo->flareAge);
				SetFlareArm(laraItem, laraInfo->leftArm.frameNumber);
			}
		}

		break;
	}
}

Ammo& GetAmmo(ITEM_INFO* lara, int weaponType)
{
	LaraInfo*& laraInfo = lara->data;
	return laraInfo->Weapons[weaponType].Ammo[laraInfo->Weapons[weaponType].SelectedAmmo];
}

void InitialiseNewWeapon(ITEM_INFO* lara)
{
	LaraInfo*& laraInfo = lara->data;

	laraInfo->rightArm.frameNumber = 0;
	laraInfo->leftArm.frameNumber = 0;
	laraInfo->leftArm.zRot = 0;
	laraInfo->leftArm.yRot = 0;
	laraInfo->leftArm.xRot = 0;
	laraInfo->rightArm.zRot = 0;
	laraInfo->rightArm.yRot = 0;
	laraInfo->rightArm.xRot = 0;
	laraInfo->target = nullptr;
	laraInfo->rightArm.lock = false;
	laraInfo->leftArm.lock = false;
	laraInfo->rightArm.flash_gun = 0;
	laraInfo->leftArm.flash_gun = 0;

	switch (laraInfo->gunType)
	{
	case WEAPON_PISTOLS:
	case WEAPON_UZI:
		laraInfo->rightArm.frameBase = Objects[ID_PISTOLS_ANIM].frameBase;
		laraInfo->leftArm.frameBase = Objects[ID_PISTOLS_ANIM].frameBase;
		if (laraInfo->gunStatus != LG_NO_ARMS)
			draw_pistol_meshes(laraInfo->gunType);
		break;

	case WEAPON_SHOTGUN:
	case WEAPON_REVOLVER:
	case WEAPON_HK:
	case WEAPON_GRENADE_LAUNCHER:
	case WEAPON_HARPOON_GUN:
	case WEAPON_ROCKET_LAUNCHER:
		laraInfo->rightArm.frameBase = Objects[WeaponObject(laraInfo->gunType)].frameBase;
		laraInfo->leftArm.frameBase = Objects[WeaponObject(laraInfo->gunType)].frameBase;
		if (laraInfo->gunStatus != LG_NO_ARMS)
			draw_shotgun_meshes(laraInfo->gunType);
		break;

	case WEAPON_FLARE:
		laraInfo->rightArm.frameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;
		laraInfo->leftArm.frameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;
		if (laraInfo->gunStatus != LG_NO_ARMS)
			DrawFlareMeshes(lara);
		break;

	default:
		laraInfo->rightArm.frameBase = g_Level.Anims[lara->animNumber].framePtr;
		laraInfo->leftArm.frameBase = g_Level.Anims[lara->animNumber].framePtr;
		break;
	}
}

GAME_OBJECT_ID WeaponObjectMesh(ITEM_INFO* lara, int weaponType) {
	LaraInfo*& laraInfo = lara->data;

	switch (weaponType)
	{
	case WEAPON_REVOLVER:
		return (laraInfo->Weapons[WEAPON_REVOLVER].HasLasersight == true ? ID_LARA_REVOLVER_LASER : ID_REVOLVER_ANIM);

	case WEAPON_UZI:
		return ID_UZI_ANIM;

	case WEAPON_SHOTGUN:
		return ID_SHOTGUN_ANIM;

	case WEAPON_HK:
		return ID_HK_ANIM;

	case WEAPON_CROSSBOW:
		return (laraInfo->Weapons[WEAPON_CROSSBOW].HasLasersight == true ? ID_LARA_CROSSBOW_LASER : ID_CROSSBOW_ANIM);
		
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

void HitTarget(ITEM_INFO* lara, ITEM_INFO* target, GAME_VECTOR* hitPos, int damage, int grenade)
{	
	LaraInfo*& laraInfo = lara->data;
	target->hitStatus = true;

	if (target->data.is<CREATURE_INFO>())
		((CREATURE_INFO*)target->data)->hurtByLara = true;

	OBJECT_INFO* obj = &Objects[target->objectNumber];

	if (hitPos != nullptr)
	{
		if (obj->hitEffect != HIT_NONE)
		{
			switch (obj->hitEffect)
			{
			case HIT_BLOOD:
				if (target->objectNumber == ID_BADDY2 &&
					(target->currentAnimState == 8 || GetRandomControl() & 1) &&
					(laraInfo->gunType == WEAPON_PISTOLS ||
						laraInfo->gunType == WEAPON_SHOTGUN ||
						laraInfo->gunType == WEAPON_UZI))
				{
					// Baddy2 gun hitting sword
					SoundEffect(SFX_TR4_BAD_SWORD_RICO, &target->pos, 0);
					TriggerRicochetSpark(hitPos, lara->pos.yRot, 3, 0);

					return;
				}
				else
					DoBloodSplat(hitPos->x, hitPos->y, hitPos->z, (GetRandomControl() & 3) + 3, target->pos.yRot, target->roomNumber);

				break;

			case HIT_RICOCHET:
				TriggerRicochetSpark(hitPos, lara->pos.yRot, 3, 0);

				break;

			case HIT_SMOKE:
				TriggerRicochetSpark(hitPos, lara->pos.yRot, 3, -5);

				if (target->objectNumber == ID_ROMAN_GOD1 ||
					target->objectNumber == ID_ROMAN_GOD2)
				{
					SoundEffect(SFX_TR5_SWORD_GOD_HITMET, &target->pos, 0);
				}

				break;
			}
		}
	}

	if (!obj->undead ||
		grenade ||
		target->hitPoints == NOT_TARGETABLE)
	{
		if (target->hitPoints > 0)
		{
			Statistics.Level.AmmoHits++;

			if (target->hitPoints >= damage)
				target->hitPoints -= damage;
			else
				target->hitPoints = 0;
		}
	}
}

FireWeaponType FireWeapon(LARA_WEAPON_TYPE weaponType, ITEM_INFO* target, ITEM_INFO* src, short* angles)
{
	LaraInfo*& laraInfo = src->data;

	Ammo& ammo = GetAmmo(src, weaponType);
	if (ammo.getCount() == 0 && !ammo.hasInfinite())
		return FW_NOAMMO;
	if (!ammo.hasInfinite())
		ammo--;

	WEAPON_INFO* weapon = &Weapons[weaponType];
	int r;

	PHD_VECTOR pos, muzzleOffset;
	GetLaraJointPosition(&muzzleOffset, LM_RHAND);
	pos.x = src->pos.xPos;
	pos.y = muzzleOffset.y;
	pos.z = src->pos.zPos;
	PHD_3DPOS rotation;
	rotation.xRot = angles[1] + (GetRandomControl() - 16384) * weapon->shotAccuracy / 65536;
	rotation.yRot = angles[0] + (GetRandomControl() - 16384) * weapon->shotAccuracy / 65536;
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

	laraInfo->hasFired = true;
	laraInfo->fired = true;
	
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
		GetTargetOnLOS(&vSrc, &vDest, false, true);
		return FW_MISS;
	}
	else
	{
		Statistics.Game.AmmoHits++;
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

			dx = (vDest.x - vSrc.x) >> 5;
			dy = (vDest.y - vSrc.y) >> 5;
			dz = (vDest.z - vSrc.z) >> 5;
			FindClosestShieldPoint(vDest.x - dx, vDest.y - dy, vDest.z - dz, target);
		}
		else if (target->objectNumber == ID_ARMY_WINSTON || target->objectNumber == ID_LONDONBOSS) //Don't want blood on Winston - never get the stains out
		{
			short ricochet_angle;
			target->hitStatus = true; //need to do this to maintain defence state
			target->hitPoints--;
			ricochet_angle = (mGetAngle(lara->pos.zPos, lara->pos.xPos, target->pos.zPos, target->pos.xPos) >> 4) & 4095;
			TriggerRicochetSparks(&vDest, ricochet_angle, 16, 0);
			SoundEffect(SFX_TR4_LARA_RICOCHET, &target->pos, 0);		// play RICOCHET Sample
		}
		else if (target->objectNumber == ID_SHIVA) //So must be Shiva
		{
			z = target->pos.zPos - lara_item->pos.zPos;
			x = target->pos.xPos - lara_item->pos.xPos;
			angle = 0x8000 + phd_atan(z, x) - target->pos.yRot;

			if ((target->currentAnimState > 1 && target->currentAnimState < 5) && angle < 0x4000 && angle > -0x4000)
			{
				target->hitStatus = true; //need to do this to maintain defence state
				ricochet_angle = (mGetAngle(lara->pos.zPos, lara->pos.xPos, target->pos.zPos, target->pos.xPos) >> 4) & 4095;
				TriggerRicochetSparks(&vDest, ricochet_angle, 16, 0);
				SoundEffect(SFX_TR4_LARA_RICOCHET, &target->pos, 0); // play RICOCHET Sample
			}
			else //Shiva's not in defence mode or has its back to Lara
				HitTarget(target, &vDest, weapon->damage, 0);
		}
		else
		{*/

			// NOTE: it seems that items for being hit by Lara in the normal way must have GetTargetOnLOS returning false
			// it's really weird but we decided to replicate original behaviour until we'll fully understand what is happening
			// with weapons
			if (!GetTargetOnLOS(&vSrc, &vDest, false, true))
				HitTarget(src, target, &vDest, weapon->damage, false);
		//}
		
		return FW_MAYBEHIT;
	}
}

void find_target_point(ITEM_INFO* item, GAME_VECTOR* target)
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

void LaraTargetInfo(ITEM_INFO* lara, WEAPON_INFO* weapon)
{
	LaraInfo*& laraInfo = lara->data;

	if (laraInfo->target == nullptr)
	{
		laraInfo->rightArm.lock = false;
		laraInfo->leftArm.lock = false;
		laraInfo->targetAngles[1] = 0;
		laraInfo->targetAngles[0] = 0;
		return;
	}

	GAME_VECTOR src, targetPoint;
	PHD_VECTOR muzzleOffset;
	short angles[2];

	GetLaraJointPosition(&muzzleOffset, LM_RHAND);
	src.x = lara->pos.xPos;
	src.y = muzzleOffset.y;
	src.z = lara->pos.zPos;
	src.roomNumber = lara->roomNumber;
	find_target_point(laraInfo->target, &targetPoint);
	phd_GetVectorAngles(targetPoint.x - src.x, targetPoint.y - src.y, targetPoint.z - src.z, angles);

	angles[0] -= lara->pos.yRot;
	angles[1] -= lara->pos.xRot;

	if (LOS(&src, &targetPoint))
	{
		if (angles[0] >= weapon->lockAngles[0] &&
			angles[0] <= weapon->lockAngles[1] &&
			angles[1] >= weapon->lockAngles[2] &&
			angles[1] <= weapon->lockAngles[3])
		{
			laraInfo->rightArm.lock = true;
			laraInfo->leftArm.lock = true;
		}
		else
		{
			if (laraInfo->leftArm.lock)
			{
				if (angles[0] < weapon->leftAngles[0] ||
					angles[0] > weapon->leftAngles[1] ||
					angles[1] < weapon->leftAngles[2] ||
					angles[1] > weapon->leftAngles[3])
					laraInfo->leftArm.lock = false;
			}

			if (laraInfo->rightArm.lock)
			{
				if (angles[0] < weapon->rightAngles[0] ||
					angles[0] > weapon->rightAngles[1] ||
					angles[1] < weapon->rightAngles[2] ||
					angles[1] > weapon->rightAngles[3])
					laraInfo->rightArm.lock = false;
			}
		}
	}
	else
	{
		laraInfo->rightArm.lock = false;
		laraInfo->leftArm.lock = false;
	}

	laraInfo->targetAngles[0] = angles[0];
	laraInfo->targetAngles[1] = angles[1];
}

bool CheckForHoldingState(int state)
{
#if 0
	if (laraInfo->ExtraAnim != NO_ITEM)
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

void LaraGetNewTarget(ITEM_INFO* lara, WEAPON_INFO* weaponInfo)
{
	LaraInfo*& laraInfo = lara->data;
	GAME_VECTOR src, target;
	PHD_VECTOR muzzleOffset;
	int bestDistance, maxDistance, targets, slot, x, y, z, distance;
	ITEM_INFO* bestItem, *item;
	short bestYrot, angle[2], match;
	bool flag, loop;

	if (BinocularRange)
	{
		laraInfo->target = nullptr;
		return;
	}

	GetLaraJointPosition(&muzzleOffset, LM_RHAND);
	src.x = lara->pos.xPos;
	src.y = muzzleOffset.y;
	src.z = lara->pos.zPos;
	src.roomNumber = lara->roomNumber;

	bestItem = NULL;
	bestYrot = MAXSHORT;
	bestDistance = MAXINT;
	maxDistance = weaponInfo->targetDist;
	targets = 0;

	for (slot = 0; slot < ActiveCreatures.size(); ++slot)
	{
		if (ActiveCreatures[slot]->itemNum != NO_ITEM)
		{
			item = &g_Level.Items[ActiveCreatures[slot]->itemNum];
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
							angle[0] -= lara->pos.yRot + laraInfo->torsoYrot;
							angle[1] -= lara->pos.xRot + laraInfo->torsoXrot;
							if (angle[0] >= weaponInfo->lockAngles[0] && angle[0] <= weaponInfo->lockAngles[1] && angle[1] >= weaponInfo->lockAngles[2] && angle[1] <= weaponInfo->lockAngles[3])
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
		laraInfo->target = NULL;
	}
	else
	{
		for (slot = 0; slot < MAX_TARGETS; ++slot)
		{
			if (!TargetList[slot])
				laraInfo->target = NULL;
			if (TargetList[slot] == laraInfo->target)
				break;
		}
		if (laraInfo->gunStatus != LG_NO_ARMS || TrInput & IN_LOOKSWITCH)
		{
			if (!laraInfo->target)
			{
				laraInfo->target = bestItem;
				LastTargets[0] = NULL;
			}
			else if (TrInput & IN_LOOKSWITCH)
			{
				laraInfo->target = NULL;
				flag = true;
				for (match = 0; match < MAX_TARGETS && TargetList[match]; ++match)
				{
					loop = false;
					for (slot = 0; slot < MAX_TARGETS && LastTargets[slot]; ++slot)
					{
						if (LastTargets[slot] == TargetList[match])
						{
							loop = true;
							break;
						}
					}
					if (!loop)
					{
						laraInfo->target = TargetList[match];
						if (laraInfo->target)
							flag = false;
						break;
					}
				}
				if (flag)
				{
					laraInfo->target = bestItem;
					LastTargets[0] = NULL;
				}
			}
		}
	}

	if (laraInfo->target != LastTargets[0])
	{
		for (slot = 7; slot > 0; --slot)
			LastTargets[slot] = LastTargets[slot - 1];
		LastTargets[0] = laraInfo->target;
	}

	LaraTargetInfo(lara, weaponInfo);
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
