#include "framework.h"
#include "Game/Lara/lara_fire.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/control/los.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_two_guns.h"
#include "Game/savegame.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Objects/Generic/Object/objects.h"
#include "Scripting/GameFlowScript.h"
#include "Sound/sound.h"
#include "Specific/setup.h"
#include "Specific/input.h"
#include "Specific/level.h"

using namespace TEN::Entities::Generic;

bool MonksAttackLara;
ITEM_INFO* LastTargets[MAX_TARGETS];
ITEM_INFO* TargetList[MAX_TARGETS];

WeaponInfo Weapons[NUM_WEAPONS] =
{
	// No weapons
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
	// Pistols
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
	// Revolver
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
	// Uzis
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
	// Shotgun
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
	// HK
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
	// Crossbow
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
	// Flare
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
	// Flare 2
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
	// Grenade launcher
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
	// Harpoon gun
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
	// Rocket launcher
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
	// Snowmobile
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

// States in which Lara will hold the flare out in front.
int HoldStates[] =
{
	LS_WALK_FORWARD,
	LS_RUN_FORWARD,
	LS_IDLE,
	LS_POSE,
	LS_TURN_RIGHT_SLOW,
	LS_TURN_LEFT_SLOW,
	LS_WALK_BACK,
	LS_TURN_RIGHT_FAST,
	LS_TURN_LEFT_FAST,
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

GAME_OBJECT_ID WeaponObject(LaraWeaponType weaponType)
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

void AimWeapon(ITEM_INFO* laraItem, WeaponInfo* weaponInfo, ArmInfo* arm)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	short x = 0;
	short y = 0;

	// Have target lock; get XY angles for arms.
	if (arm->Locked)
	{
		y = laraInfo->targetAngles[0];
		x = laraInfo->targetAngles[1];
	}

	int speed = weaponInfo->AimSpeed;

	// Rotate arms on y axis toward target.
	short rotY = arm->Rotation.yRot;
	if (rotY >= (y - speed) && rotY <= (y + speed))
		rotY = y;
	else if (rotY < y)
		rotY += speed;
	else
		rotY -= speed;
	arm->Rotation.yRot = rotY;

	// Rotate arms on x axis toward target.
	short rotX = arm->Rotation.xRot;
	if (rotX >= (x - speed) && rotX <= (x + speed))
		rotX = x;
	else if (rotX < x)
		rotX += speed;
	else
		rotX -= speed;
	arm->Rotation.xRot = rotX;

	// TODO: Set arms to inherit rotations of parent bones.
	arm->Rotation.zRot = 0;
}

void SmashItem(short itemNum)
{
	auto* item = &g_Level.Items[itemNum];

	if (item->ObjectNumber >= ID_SMASH_OBJECT1 && item->ObjectNumber <= ID_SMASH_OBJECT8)
		SmashObject(itemNum);
}

void LaraGun(ITEM_INFO* laraItem)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	if (laraInfo->LeftArm.FlashGun > 0)
		--laraInfo->LeftArm.FlashGun;
	if (laraInfo->RightArm.FlashGun > 0)
		--laraInfo->RightArm.FlashGun;

	if (laraInfo->Control.WeaponControl.GunType == WEAPON_TORCH)
	{
		DoFlameTorch();
		return;
	}

	if (laraItem->HitPoints <= 0)
		laraInfo->Control.HandStatus = HandStatus::Free;
	else if (laraInfo->Control.HandStatus == HandStatus::Free)
	{
		// Draw weapon.
		if (TrInput & IN_DRAW)
			laraInfo->Control.WeaponControl.RequestGunType = laraInfo->Control.WeaponControl.LastGunType;
		// Draw flare.
		else if (TrInput & IN_FLARE &&
			(g_GameFlow->GetLevel(CurrentLevel)->LaraType != LaraType::Young))
		{
			if (laraInfo->Control.WeaponControl.GunType == WEAPON_FLARE)
			{
			//	if (!laraInfo->leftArm.frameNumber)	// NO
				{
					laraInfo->Control.HandStatus = HandStatus::UndrawWeapon;
				}
			}
			else if (laraInfo->NumFlares)
			{
				if (laraInfo->NumFlares != -1)
					laraInfo->NumFlares--;

				laraInfo->Control.WeaponControl.RequestGunType = WEAPON_FLARE;
			}
		}

		if (TrInput & IN_DRAW ||
			laraInfo->Control.WeaponControl.RequestGunType != laraInfo->Control.WeaponControl.GunType)
		{
			if ((laraItem->ActiveState == LS_CROUCH_IDLE ||
				laraItem->ActiveState == LS_CROUCH_TURN_LEFT ||
				laraItem->ActiveState == LS_CROUCH_TURN_RIGHT) &&
				(laraInfo->Control.WeaponControl.RequestGunType == WEAPON_HK ||
					laraInfo->Control.WeaponControl.RequestGunType == WEAPON_CROSSBOW ||
					laraInfo->Control.WeaponControl.RequestGunType == WEAPON_SHOTGUN ||
					laraInfo->Control.WeaponControl.RequestGunType == WEAPON_HARPOON_GUN))
			{
				if (laraInfo->Control.WeaponControl.GunType == WEAPON_FLARE)
					laraInfo->Control.WeaponControl.RequestGunType = WEAPON_FLARE;
			}
			else if (laraInfo->Control.WeaponControl.RequestGunType == WEAPON_FLARE ||
				(laraInfo->Vehicle == NO_ITEM &&
					(laraInfo->Control.WeaponControl.RequestGunType == WEAPON_HARPOON_GUN ||
						laraInfo->Control.WaterStatus == WaterStatus::Dry ||
						(laraInfo->Control.WaterStatus == WaterStatus::Wade &&
							laraInfo->WaterSurfaceDist > -Weapons[laraInfo->Control.WeaponControl.GunType].GunHeight))))
			{
				if (laraInfo->Control.WeaponControl.GunType == WEAPON_FLARE)
				{
					CreateFlare(laraItem, ID_FLARE_ITEM, 0);
					UndrawFlareMeshes(laraItem);
					laraInfo->Flare.ControlLeft = false;
					laraInfo->Flare.Life = 0;
				}

				laraInfo->Control.WeaponControl.GunType = laraInfo->Control.WeaponControl.RequestGunType;
				InitialiseNewWeapon(laraItem);
				laraInfo->RightArm.FrameNumber = 0;
				laraInfo->LeftArm.FrameNumber = 0;
				laraInfo->Control.HandStatus = HandStatus::DrawWeapon;
			}
			else
			{
				laraInfo->Control.WeaponControl.LastGunType = laraInfo->Control.WeaponControl.RequestGunType;

				if (laraInfo->Control.WeaponControl.GunType != WEAPON_FLARE)
					laraInfo->Control.WeaponControl.GunType = laraInfo->Control.WeaponControl.RequestGunType;
				else
					laraInfo->Control.WeaponControl.RequestGunType = WEAPON_FLARE;
			}
		}
	}
	else if (laraInfo->Control.HandStatus == HandStatus::WeaponReady)
	{
		if (TrInput & IN_DRAW ||
			laraInfo->Control.WeaponControl.RequestGunType != laraInfo->Control.WeaponControl.GunType)
		{
			laraInfo->Control.HandStatus = HandStatus::UndrawWeapon;
		}
		else if (laraInfo->Control.WeaponControl.GunType != WEAPON_HARPOON_GUN &&
			laraInfo->Control.WaterStatus != WaterStatus::Dry &&
			(laraInfo->Control.WaterStatus != WaterStatus::Wade ||
				laraInfo->WaterSurfaceDist < -Weapons[laraInfo->Control.WeaponControl.GunType].GunHeight))
		{
			laraInfo->Control.HandStatus = HandStatus::UndrawWeapon;
		}
	}
	else if (TrInput & IN_FLARE &&
		laraInfo->Control.HandStatus == HandStatus::Busy &&
		laraItem->ActiveState == LS_CRAWL_IDLE &&
		laraItem->AnimNumber == LA_CRAWL_IDLE)
	{
		laraInfo->Control.WeaponControl.RequestGunType = WEAPON_FLARE;
	}

	switch (laraInfo->Control.HandStatus)
	{
	case HandStatus::DrawWeapon:
		if (laraInfo->Control.WeaponControl.GunType != WEAPON_FLARE &&
			laraInfo->Control.WeaponControl.GunType != WEAPON_NONE)
		{
			laraInfo->Control.WeaponControl.LastGunType = laraInfo->Control.WeaponControl.GunType;
		}

		switch (laraInfo->Control.WeaponControl.GunType)
		{
		case WEAPON_PISTOLS:
		case WEAPON_REVOLVER:
		case WEAPON_UZI:
			if (Camera.type != CameraType::Look && Camera.type != CameraType::Heavy)
				Camera.type = CameraType::Combat;

			DrawPistols(laraItem, laraInfo->Control.WeaponControl.GunType);
			break;

		case WEAPON_SHOTGUN:
		case WEAPON_CROSSBOW:
		case WEAPON_HK:
		case WEAPON_GRENADE_LAUNCHER:
		case WEAPON_ROCKET_LAUNCHER:
		case WEAPON_HARPOON_GUN:
			if (Camera.type != CameraType::Look && Camera.type != CameraType::Heavy)
				Camera.type = CameraType::Combat;

			DrawShotgun(laraItem, laraInfo->Control.WeaponControl.GunType);
			break;

		case WEAPON_FLARE:
			DrawFlare(laraItem);
			break;

		default:
			laraInfo->Control.HandStatus = HandStatus::Free;
			break;
		}

		break;

	case HandStatus::Special:
		DrawFlare(laraItem);
		break;

	case HandStatus::UndrawWeapon:
		laraInfo->meshPtrs[LM_HEAD] = Objects[ID_LARA_SKIN].meshIndex + LM_HEAD;

		switch (laraInfo->Control.WeaponControl.GunType)
		{
		case WEAPON_PISTOLS:
		case WEAPON_REVOLVER:
		case WEAPON_UZI:
			UndrawPistols(laraItem, laraInfo->Control.WeaponControl.GunType);
			break;

		case WEAPON_SHOTGUN:
		case WEAPON_CROSSBOW:
		case WEAPON_HK:
		case WEAPON_GRENADE_LAUNCHER:
		case WEAPON_ROCKET_LAUNCHER:
		case WEAPON_HARPOON_GUN:
			UndrawShotgun(laraItem, laraInfo->Control.WeaponControl.GunType);
			break;

		case WEAPON_FLARE:
			UndrawFlare(laraItem);
			break;

		default:
			return;
		}

		break;

	case HandStatus::WeaponReady:
		if (!(TrInput & IN_ACTION))
			laraInfo->meshPtrs[LM_HEAD] = Objects[ID_LARA_SKIN].meshIndex + LM_HEAD;
		else
			laraInfo->meshPtrs[LM_HEAD] = Objects[ID_LARA_SCREAM].meshIndex + LM_HEAD;

		if (Camera.type != CameraType::Look &&
			Camera.type != CameraType::Heavy)
		{
			Camera.type = CameraType::Combat;
		}

		if (TrInput & IN_ACTION)
		{
			if (!GetAmmo(laraItem, laraInfo->Control.WeaponControl.GunType))
			{
				laraInfo->Control.WeaponControl.RequestGunType = Objects[ID_PISTOLS_ITEM].loaded ? WEAPON_PISTOLS : WEAPON_NONE;
				return;
			}
		}

		switch (laraInfo->Control.WeaponControl.GunType)
		{
		case WEAPON_PISTOLS:
		case WEAPON_UZI:
			PistolHandler(laraItem, laraInfo->Control.WeaponControl.GunType);

			break;

		case WEAPON_SHOTGUN:
		case WEAPON_CROSSBOW:
		case WEAPON_HK:
		case WEAPON_GRENADE_LAUNCHER:
		case WEAPON_ROCKET_LAUNCHER:
		case WEAPON_HARPOON_GUN:
		case WEAPON_REVOLVER:
			RifleHandler(laraItem, laraInfo->Control.WeaponControl.GunType);
			break;

		default:
			return;
		}

		break;

	case HandStatus::Free:
		if (laraInfo->Control.WeaponControl.GunType == WEAPON_FLARE)
		{
			if (laraInfo->Vehicle != NO_ITEM ||
				CheckForHoldingState((LaraState)laraItem->ActiveState))
			{
				if (laraInfo->Flare.ControlLeft)
				{
					if (laraInfo->LeftArm.FrameNumber)
					{
						if (++laraInfo->LeftArm.FrameNumber == 110)
							laraInfo->LeftArm.FrameNumber = 0;
					}
				}
				else
				{
					laraInfo->LeftArm.FrameNumber = 95;
					laraInfo->Flare.ControlLeft = true;
				}
			}
			else
				laraInfo->Flare.ControlLeft = false;

			DoFlareInHand(laraItem, laraInfo->Flare.Life);
			SetFlareArm(laraItem, laraInfo->LeftArm.FrameNumber);
		}

		break;

	case HandStatus::Busy:
		if (laraInfo->Control.WeaponControl.GunType == WEAPON_FLARE)
		{
			if (laraInfo->meshPtrs[LM_LHAND] == Objects[ID_LARA_FLARE_ANIM].meshIndex + LM_LHAND)
			{
				laraInfo->Flare.ControlLeft = (laraInfo->Vehicle != NO_ITEM || CheckForHoldingState((LaraState)laraItem->ActiveState));
				DoFlareInHand(laraItem, laraInfo->Flare.Life);
				SetFlareArm(laraItem, laraInfo->LeftArm.FrameNumber);
			}
		}

		break;
	}
}

Ammo& GetAmmo(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	return laraInfo->Weapons[weaponType].Ammo[laraInfo->Weapons[weaponType].SelectedAmmo];
}

void InitialiseNewWeapon(ITEM_INFO* laraItem)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	laraInfo->LeftArm.FrameNumber = 0;
	laraInfo->RightArm.FrameNumber = 0;
	laraInfo->LeftArm.Rotation = PHD_3DPOS();
	laraInfo->RightArm.Rotation = PHD_3DPOS();
	laraInfo->target = nullptr;
	laraInfo->LeftArm.Locked = false;
	laraInfo->RightArm.Locked = false;
	laraInfo->LeftArm.FlashGun = 0;
	laraInfo->RightArm.FlashGun = 0;

	switch (laraInfo->Control.WeaponControl.GunType)
	{
	case WEAPON_PISTOLS:
	case WEAPON_UZI:
		laraInfo->RightArm.FrameBase = Objects[ID_PISTOLS_ANIM].frameBase;
		laraInfo->LeftArm.FrameBase = Objects[ID_PISTOLS_ANIM].frameBase;

		if (laraInfo->Control.HandStatus != HandStatus::Free)
			DrawPistolMeshes(laraItem, laraInfo->Control.WeaponControl.GunType);

		break;

	case WEAPON_SHOTGUN:
	case WEAPON_REVOLVER:
	case WEAPON_HK:
	case WEAPON_GRENADE_LAUNCHER:
	case WEAPON_HARPOON_GUN:
	case WEAPON_ROCKET_LAUNCHER:
		laraInfo->RightArm.FrameBase = Objects[WeaponObject(laraInfo->Control.WeaponControl.GunType)].frameBase;
		laraInfo->LeftArm.FrameBase = Objects[WeaponObject(laraInfo->Control.WeaponControl.GunType)].frameBase;

		if (laraInfo->Control.HandStatus != HandStatus::Free)
			DrawShotgunMeshes(laraItem, laraInfo->Control.WeaponControl.GunType);

		break;

	case WEAPON_FLARE:
		laraInfo->RightArm.FrameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;
		laraInfo->LeftArm.FrameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;

		if (laraInfo->Control.HandStatus != HandStatus::Free)
			DrawFlareMeshes(laraItem);

		break;

	default:
		laraInfo->RightArm.FrameBase = g_Level.Anims[laraItem->AnimNumber].framePtr;
		laraInfo->LeftArm.FrameBase = g_Level.Anims[laraItem->AnimNumber].framePtr;
		break;
	}
}

GAME_OBJECT_ID WeaponObjectMesh(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* laraInfo = GetLaraInfo(laraItem);

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

void HitTarget(ITEM_INFO* laraItem, ITEM_INFO* target, GAME_VECTOR* hitPos, int damage, int grenade)
{	
	auto* laraInfo = GetLaraInfo(laraItem);

	target->HitStatus = true;

	if (target->Data.is<CREATURE_INFO>())
		((CREATURE_INFO*)target->Data)->hurtByLara = true;

	auto* obj = &Objects[target->ObjectNumber];

	if (hitPos != nullptr)
	{
		if (obj->hitEffect != HIT_NONE)
		{
			switch (obj->hitEffect)
			{
			case HIT_BLOOD:
				if (target->ObjectNumber == ID_BADDY2 &&
					(target->ActiveState == 8 || GetRandomControl() & 1) &&
					(laraInfo->Control.WeaponControl.GunType == WEAPON_PISTOLS ||
						laraInfo->Control.WeaponControl.GunType == WEAPON_SHOTGUN ||
						laraInfo->Control.WeaponControl.GunType == WEAPON_UZI))
				{
					// Baddy2 gun hitting sword
					SoundEffect(SFX_TR4_BAD_SWORD_RICO, &target->Position, 0);
					TriggerRicochetSpark(hitPos, laraItem->Position.yRot, 3, 0);
					return;
				}
				else
					DoBloodSplat(hitPos->x, hitPos->y, hitPos->z, (GetRandomControl() & 3) + 3, target->Position.yRot, target->RoomNumber);

				break;

			case HIT_RICOCHET:
				TriggerRicochetSpark(hitPos, laraItem->Position.yRot, 3, 0);
				break;

			case HIT_SMOKE:
				TriggerRicochetSpark(hitPos, laraItem->Position.yRot, 3, -5);

				if (target->ObjectNumber == ID_ROMAN_GOD1 ||
					target->ObjectNumber == ID_ROMAN_GOD2)
				{
					SoundEffect(SFX_TR5_SWORDGODHITMETAL, &target->Position, 0);
				}

				break;
			}
		}
	}

	if (!obj->undead || grenade ||
		target->HitPoints == NOT_TARGETABLE)
	{
		if (target->HitPoints > 0)
		{
			Statistics.Level.AmmoHits++;

			if (target->HitPoints >= damage)
				target->HitPoints -= damage;
			else
				target->HitPoints = 0;
		}
	}
}

FireWeaponType FireWeapon(LaraWeaponType weaponType, ITEM_INFO* target, ITEM_INFO* src, short* angles)
{
	auto* laraInfo = GetLaraInfo(src);

	Ammo& ammo = GetAmmo(src, weaponType);
	if (ammo.getCount() == 0 && !ammo.hasInfinite())
		return FireWeaponType::NoAmmo;
	if (!ammo.hasInfinite())
		ammo--;

	auto* weapon = &Weapons[weaponType];
	int r;

	PHD_VECTOR muzzleOffset;
	GetLaraJointPosition(&muzzleOffset, LM_RHAND);

	PHD_VECTOR pos;
	pos.x = src->Position.xPos;
	pos.y = muzzleOffset.y;
	pos.z = src->Position.zPos;
	PHD_3DPOS rotation;
	rotation.xRot = angles[1] + (GetRandomControl() - 16384) * weapon->ShotAccuracy / 65536;
	rotation.yRot = angles[0] + (GetRandomControl() - 16384) * weapon->ShotAccuracy / 65536;
	rotation.zRot = 0;

	// Calculate ray from rotation angles
	float x =  sin(TO_RAD(rotation.yRot)) * cos(TO_RAD(rotation.xRot));
	float y = -sin(TO_RAD(rotation.xRot));
	float z =  cos(TO_RAD(rotation.yRot)) * cos(TO_RAD(rotation.xRot));
	Vector3 direction = Vector3(x, y, z);
	direction.Normalize();

	Vector3 source = Vector3(pos.x, pos.y, pos.z);
	Vector3 destination = source + direction * weapon->TargetDist;
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

	laraInfo->Control.WeaponControl.HasFired = true;
	laraInfo->Control.WeaponControl.Fired = true;
	
	GAME_VECTOR vSrc;
	vSrc.x = pos.x;
	vSrc.y = pos.y;
	vSrc.z = pos.z;
	short roomNumber = src->RoomNumber;
	GetFloor(pos.x, pos.y, pos.z, &roomNumber);
	vSrc.roomNumber = roomNumber;

	if (best < 0)
	{
		GAME_VECTOR vDest;
		vDest.x = destination.x;
		vDest.y = destination.y;
		vDest.z = destination.z;
		GetTargetOnLOS(&vSrc, &vDest, false, true);
		return FireWeaponType::Miss;
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
			target->HitPoints--;
			ricochet_angle = (mGetAngle(lara->pos.zPos, lara->pos.xPos, target->pos.zPos, target->pos.xPos) >> 4) & 4095;
			TriggerRicochetSparks(&vDest, ricochet_angle, 16, 0);
			SoundEffect(SFX_TR4_LARA_RICOCHET, &target->pos, 0);		// play RICOCHET Sample
		}
		else if (target->objectNumber == ID_SHIVA) //So must be Shiva
		{
			z = target->pos.zPos - lara_item->pos.zPos;
			x = target->pos.xPos - lara_item->pos.xPos;
			angle = 0x8000 + phd_atan(z, x) - target->pos.yRot;

			if ((target->ActiveState > 1 && target->ActiveState < 5) && angle < 0x4000 && angle > -0x4000)
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
				HitTarget(src, target, &vDest, weapon->Damage, false);
		//}
		
		return FireWeaponType::PossibleHit;
	}
}

void FindTargetPoint(ITEM_INFO* item, GAME_VECTOR* target)
{
	auto* bounds = (BOUNDING_BOX*)GetBestFrame(item);
	int x = (int)(bounds->X1 + bounds->X2) / 2;
	int y = (int) bounds->Y1 + (bounds->Y2 - bounds->Y1) / 3;
	int z = (int)(bounds->Z1 + bounds->Z2) / 2;

	float c = phd_cos(item->Position.yRot);
	float s = phd_sin(item->Position.yRot);

	target->x = item->Position.xPos + c * x + s * z;
	target->y = item->Position.yPos + y;
	target->z = item->Position.zPos + c * z - s * x;
	target->roomNumber = item->RoomNumber;
}

void LaraTargetInfo(ITEM_INFO* laraItem, WeaponInfo* weaponInfo)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	if (laraInfo->target == nullptr)
	{
		laraInfo->RightArm.Locked = false;
		laraInfo->LeftArm.Locked = false;
		laraInfo->targetAngles[1] = 0;
		laraInfo->targetAngles[0] = 0;
		return;
	}

	short angles[2];

	PHD_VECTOR muzzleOffset;
	GetLaraJointPosition(&muzzleOffset, LM_RHAND);

	GAME_VECTOR src;
	src.x = laraItem->Position.xPos;
	src.y = muzzleOffset.y;
	src.z = laraItem->Position.zPos;
	src.roomNumber = laraItem->RoomNumber;

	GAME_VECTOR targetPoint;
	FindTargetPoint(laraInfo->target, &targetPoint);
	phd_GetVectorAngles(targetPoint.x - src.x, targetPoint.y - src.y, targetPoint.z - src.z, angles);

	angles[0] -= laraItem->Position.yRot;
	angles[1] -= laraItem->Position.xRot;

	if (LOS(&src, &targetPoint))
	{
		if (angles[0] >= weaponInfo->LockAngles[0] &&
			angles[0] <= weaponInfo->LockAngles[1] &&
			angles[1] >= weaponInfo->LockAngles[2] &&
			angles[1] <= weaponInfo->LockAngles[3])
		{
			laraInfo->RightArm.Locked = true;
			laraInfo->LeftArm.Locked = true;
		}
		else
		{
			if (laraInfo->LeftArm.Locked)
			{
				if (angles[0] < weaponInfo->LeftAngles[0] ||
					angles[0] > weaponInfo->LeftAngles[1] ||
					angles[1] < weaponInfo->LeftAngles[2] ||
					angles[1] > weaponInfo->LeftAngles[3])
					laraInfo->LeftArm.Locked = false;
			}

			if (laraInfo->RightArm.Locked)
			{
				if (angles[0] < weaponInfo->RightAngles[0] ||
					angles[0] > weaponInfo->RightAngles[1] ||
					angles[1] < weaponInfo->RightAngles[2] ||
					angles[1] > weaponInfo->RightAngles[3])
					laraInfo->RightArm.Locked = false;
			}
		}
	}
	else
	{
		laraInfo->RightArm.Locked = false;
		laraInfo->LeftArm.Locked = false;
	}

	laraInfo->targetAngles[0] = angles[0];
	laraInfo->targetAngles[1] = angles[1];
}

bool CheckForHoldingState(LaraState state)
{
#if 0
	if (laraInfo->ExtraAnim != NO_ITEM)
		return false;
#endif

	int* holdState = HoldStates;
	while (*holdState >= 0)
	{
		if (state == *holdState)
			return true;

		holdState++;
	}
	
	return false;
}

void LaraGetNewTarget(ITEM_INFO* laraItem, WeaponInfo* weaponInfo)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	if (BinocularRange)
	{
		laraInfo->target = nullptr;
		return;
	}

	PHD_VECTOR muzzleOffset;
	GetLaraJointPosition(&muzzleOffset, LM_RHAND);

	GAME_VECTOR src;
	src.x = laraItem->Position.xPos;
	src.y = muzzleOffset.y;
	src.z = laraItem->Position.zPos;
	src.roomNumber = laraItem->RoomNumber;

	ITEM_INFO* bestItem = NULL;
	short bestYrot = MAXSHORT;
	int bestDistance = MAXINT;
	int maxDistance = weaponInfo->TargetDist;
	int targets = 0;
	for (int slot = 0; slot < ActiveCreatures.size(); ++slot)
	{
		if (ActiveCreatures[slot]->itemNum != NO_ITEM)
		{
			auto* item = &g_Level.Items[ActiveCreatures[slot]->itemNum];
			if (item->HitPoints > 0)
			{
				int x = item->Position.xPos - src.x;
				int y = item->Position.yPos - src.y;
				int z = item->Position.zPos - src.z;
				if (abs(x) <= maxDistance && abs(y) <= maxDistance && abs(z) <= maxDistance)
				{
					int distance = SQUARE(x) + SQUARE(y) + SQUARE(z);
					if (distance < SQUARE(maxDistance))
					{
						GAME_VECTOR target;
						FindTargetPoint(item, &target);
						if (LOS(&src, &target))
						{
							short angle[2];
							phd_GetVectorAngles(target.x - src.x, target.y - src.y, target.z - src.z, angle);
							angle[0] -= laraItem->Position.yRot + laraInfo->ExtraTorsoRot.yRot;
							angle[1] -= laraItem->Position.xRot + laraInfo->ExtraTorsoRot.xRot;

							if (angle[0] >= weaponInfo->LockAngles[0] && angle[0] <= weaponInfo->LockAngles[1] && angle[1] >= weaponInfo->LockAngles[2] && angle[1] <= weaponInfo->LockAngles[3])
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
		laraInfo->target = NULL;
	else
	{
		for (int slot = 0; slot < MAX_TARGETS; ++slot)
		{
			if (!TargetList[slot])
				laraInfo->target = NULL;

			if (TargetList[slot] == laraInfo->target)
				break;
		}

		if (laraInfo->Control.HandStatus != HandStatus::Free || TrInput & IN_LOOKSWITCH)
		{
			if (!laraInfo->target)
			{
				laraInfo->target = bestItem;
				LastTargets[0] = NULL;
			}
			else if (TrInput & IN_LOOKSWITCH)
			{
				laraInfo->target = NULL;
				bool flag = true;

				for (int match = 0; match < MAX_TARGETS && TargetList[match]; ++match)
				{
					bool loop = false;
					for (int slot = 0; slot < MAX_TARGETS && LastTargets[slot]; ++slot)
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
		for (int slot = 7; slot > 0; --slot)
			LastTargets[slot] = LastTargets[slot - 1];
		
		LastTargets[0] = laraInfo->target;
	}

	LaraTargetInfo(laraItem, weaponInfo);
}

HolsterSlot HolsterSlotForWeapon(LaraWeaponType weaponType)
{
	switch(weaponType)
	{
		case WEAPON_PISTOLS:
			return HolsterSlot::Pistols;
		case WEAPON_UZI:
			return HolsterSlot::Uzis;
		case WEAPON_REVOLVER:
			return HolsterSlot::Revolver;
		case WEAPON_SHOTGUN:
			return HolsterSlot::Shotgun;
		case WEAPON_HK:
			return HolsterSlot::HK;
		case WEAPON_HARPOON_GUN:
			return HolsterSlot::Harpoon;
		case WEAPON_CROSSBOW:
			return HolsterSlot::Crowssbow;
		case WEAPON_GRENADE_LAUNCHER:
			return HolsterSlot::GrenadeLauncher;
		case WEAPON_ROCKET_LAUNCHER:
			return HolsterSlot::RocketLauncher;
		default:
			return HolsterSlot::Empty;
	}
}
