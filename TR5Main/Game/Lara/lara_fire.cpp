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

WeaponInfo Weapons[(int)LaraWeaponType::Total] =
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

GAME_OBJECT_ID WeaponObject(LaraWeaponType weaponType)
{
	switch (weaponType)
	{
	case LaraWeaponType::Uzi:
		return ID_UZI_ANIM;
	case LaraWeaponType::Shotgun:
		return ID_SHOTGUN_ANIM;
	case LaraWeaponType::Revolver:
		return ID_REVOLVER_ANIM;
	case LaraWeaponType::Crossbow:
		return ID_CROSSBOW_ANIM;
	case LaraWeaponType::HK:
		return ID_HK_ANIM;
	case LaraWeaponType::Flare:
		return ID_LARA_FLARE_ANIM;
	case LaraWeaponType::GrenadeLauncher:
		return ID_GRENADE_ANIM;
	case LaraWeaponType::RocketLauncher:
		return ID_ROCKET_ANIM;
	case LaraWeaponType::HarpoonGun:
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
		y = laraInfo->TargetArmAngles[0];
		x = laraInfo->TargetArmAngles[1];
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

	if (laraInfo->Control.Weapon.GunType == LaraWeaponType::Torch)
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
			laraInfo->Control.Weapon.RequestGunType = laraInfo->Control.Weapon.LastGunType;
		// Draw flare.
		else if (TrInput & IN_FLARE &&
			(g_GameFlow->GetLevel(CurrentLevel)->LaraType != LaraType::Young))
		{
			if (laraInfo->Control.Weapon.GunType == LaraWeaponType::Flare)
			{
			//	if (!laraInfo->leftArm.frameNumber)	// NO
				{
					laraInfo->Control.HandStatus = HandStatus::WeaponUndraw;
				}
			}
			else if (laraInfo->Inventory.TotalFlares)
			{
				if (laraInfo->Inventory.TotalFlares != -1)
					laraInfo->Inventory.TotalFlares--;

				laraInfo->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			}
		}

		if (TrInput & IN_DRAW ||
			laraInfo->Control.Weapon.RequestGunType != laraInfo->Control.Weapon.GunType)
		{
			if ((laraItem->Animation.ActiveState == LS_CROUCH_IDLE ||
				laraItem->Animation.ActiveState == LS_CROUCH_TURN_LEFT ||
				laraItem->Animation.ActiveState == LS_CROUCH_TURN_RIGHT) &&
				(laraInfo->Control.Weapon.RequestGunType == LaraWeaponType::HK ||
					laraInfo->Control.Weapon.RequestGunType == LaraWeaponType::Crossbow ||
					laraInfo->Control.Weapon.RequestGunType == LaraWeaponType::Shotgun ||
					laraInfo->Control.Weapon.RequestGunType == LaraWeaponType::HarpoonGun))
			{
				if (laraInfo->Control.Weapon.GunType == LaraWeaponType::Flare)
					laraInfo->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			}
			else if (laraInfo->Control.Weapon.RequestGunType == LaraWeaponType::Flare ||
				(laraInfo->Vehicle == NO_ITEM &&
					(laraInfo->Control.Weapon.RequestGunType == LaraWeaponType::HarpoonGun ||
						laraInfo->Control.WaterStatus == WaterStatus::Dry ||
						(laraInfo->Control.WaterStatus == WaterStatus::Wade &&
							laraInfo->WaterSurfaceDist > -Weapons[(int)laraInfo->Control.Weapon.GunType].GunHeight))))
			{
				if (laraInfo->Control.Weapon.GunType == LaraWeaponType::Flare)
				{
					CreateFlare(laraItem, ID_FLARE_ITEM, 0);
					UndrawFlareMeshes(laraItem);
					laraInfo->Flare.ControlLeft = false;
					laraInfo->Flare.Life = 0;
				}

				laraInfo->Control.Weapon.GunType = laraInfo->Control.Weapon.RequestGunType;
				InitialiseNewWeapon(laraItem);
				laraInfo->RightArm.FrameNumber = 0;
				laraInfo->LeftArm.FrameNumber = 0;
				laraInfo->Control.HandStatus = HandStatus::WeaponDraw;
			}
			else
			{
				laraInfo->Control.Weapon.LastGunType = laraInfo->Control.Weapon.RequestGunType;

				if (laraInfo->Control.Weapon.GunType != LaraWeaponType::Flare)
					laraInfo->Control.Weapon.GunType = laraInfo->Control.Weapon.RequestGunType;
				else
					laraInfo->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			}
		}
	}
	else if (laraInfo->Control.HandStatus == HandStatus::WeaponReady)
	{
		if (TrInput & IN_DRAW ||
			laraInfo->Control.Weapon.RequestGunType != laraInfo->Control.Weapon.GunType)
		{
			laraInfo->Control.HandStatus = HandStatus::WeaponUndraw;
		}
		else if (laraInfo->Control.Weapon.GunType != LaraWeaponType::HarpoonGun &&
			laraInfo->Control.WaterStatus != WaterStatus::Dry &&
			(laraInfo->Control.WaterStatus != WaterStatus::Wade ||
				laraInfo->WaterSurfaceDist < -Weapons[(int)laraInfo->Control.Weapon.GunType].GunHeight))
		{
			laraInfo->Control.HandStatus = HandStatus::WeaponUndraw;
		}
	}
	else if (TrInput & IN_FLARE &&
		laraInfo->Control.HandStatus == HandStatus::Busy &&
		laraItem->Animation.ActiveState == LS_CRAWL_IDLE &&
		laraItem->Animation.AnimNumber == LA_CRAWL_IDLE)
	{
		laraInfo->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
	}

	switch (laraInfo->Control.HandStatus)
	{
	case HandStatus::WeaponDraw:
		if (laraInfo->Control.Weapon.GunType != LaraWeaponType::Flare &&
			laraInfo->Control.Weapon.GunType != LaraWeaponType::None)
		{
			laraInfo->Control.Weapon.LastGunType = laraInfo->Control.Weapon.GunType;
		}

		switch (laraInfo->Control.Weapon.GunType)
		{
		case LaraWeaponType::Pistol:
		case LaraWeaponType::Revolver:
		case LaraWeaponType::Uzi:
			if (Camera.type != CameraType::Look && Camera.type != CameraType::Heavy)
				Camera.type = CameraType::Combat;

			DrawPistols(laraItem, laraInfo->Control.Weapon.GunType);
			break;

		case LaraWeaponType::Shotgun:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::HK:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
			if (Camera.type != CameraType::Look && Camera.type != CameraType::Heavy)
				Camera.type = CameraType::Combat;

			DrawShotgun(laraItem, laraInfo->Control.Weapon.GunType);
			break;

		case LaraWeaponType::Flare:
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

	case HandStatus::WeaponUndraw:
		laraInfo->MeshPtrs[LM_HEAD] = Objects[ID_LARA_SKIN].meshIndex + LM_HEAD;

		switch (laraInfo->Control.Weapon.GunType)
		{
		case LaraWeaponType::Pistol:
		case LaraWeaponType::Revolver:
		case LaraWeaponType::Uzi:
			UndrawPistols(laraItem, laraInfo->Control.Weapon.GunType);
			break;

		case LaraWeaponType::Shotgun:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::HK:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
			UndrawShotgun(laraItem, laraInfo->Control.Weapon.GunType);
			break;

		case LaraWeaponType::Flare:
			UndrawFlare(laraItem);
			break;

		default:
			return;
		}

		break;

	case HandStatus::WeaponReady:
		if (!(TrInput & IN_ACTION))
			laraInfo->MeshPtrs[LM_HEAD] = Objects[ID_LARA_SKIN].meshIndex + LM_HEAD;
		else
			laraInfo->MeshPtrs[LM_HEAD] = Objects[ID_LARA_SCREAM].meshIndex + LM_HEAD;

		if (Camera.type != CameraType::Look &&
			Camera.type != CameraType::Heavy)
		{
			Camera.type = CameraType::Combat;
		}

		if (TrInput & IN_ACTION)
		{
			if (!GetAmmo(laraItem, laraInfo->Control.Weapon.GunType))
			{
				laraInfo->Control.Weapon.RequestGunType = Objects[ID_PISTOLS_ITEM].loaded ? LaraWeaponType::Pistol : LaraWeaponType::None;
				return;
			}
		}

		switch (laraInfo->Control.Weapon.GunType)
		{
		case LaraWeaponType::Pistol:
		case LaraWeaponType::Uzi:
			PistolHandler(laraItem, laraInfo->Control.Weapon.GunType);

			break;

		case LaraWeaponType::Shotgun:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::HK:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
		case LaraWeaponType::Revolver:
			RifleHandler(laraItem, laraInfo->Control.Weapon.GunType);
			break;

		default:
			return;
		}

		break;

	case HandStatus::Free:
		if (laraInfo->Control.Weapon.GunType == LaraWeaponType::Flare)
		{
			if (laraInfo->Vehicle != NO_ITEM ||
				CheckForHoldingState((LaraState)laraItem->Animation.ActiveState))
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
		if (laraInfo->Control.Weapon.GunType == LaraWeaponType::Flare)
		{
			if (laraInfo->MeshPtrs[LM_LHAND] == Objects[ID_LARA_FLARE_ANIM].meshIndex + LM_LHAND)
			{
				laraInfo->Flare.ControlLeft = (laraInfo->Vehicle != NO_ITEM || CheckForHoldingState((LaraState)laraItem->Animation.ActiveState));
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

	return laraInfo->Weapons[(int)weaponType].Ammo[(int)laraInfo->Weapons[(int)weaponType].SelectedAmmo];
}

void InitialiseNewWeapon(ITEM_INFO* laraItem)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	laraInfo->LeftArm.FrameNumber = 0;
	laraInfo->RightArm.FrameNumber = 0;
	laraInfo->LeftArm.Rotation = PHD_3DPOS();
	laraInfo->RightArm.Rotation = PHD_3DPOS();
	laraInfo->TargetEntity = nullptr;
	laraInfo->LeftArm.Locked = false;
	laraInfo->RightArm.Locked = false;
	laraInfo->LeftArm.FlashGun = 0;
	laraInfo->RightArm.FlashGun = 0;

	switch (laraInfo->Control.Weapon.GunType)
	{
	case LaraWeaponType::Pistol:
	case LaraWeaponType::Uzi:
		laraInfo->RightArm.FrameBase = Objects[ID_PISTOLS_ANIM].frameBase;
		laraInfo->LeftArm.FrameBase = Objects[ID_PISTOLS_ANIM].frameBase;

		if (laraInfo->Control.HandStatus != HandStatus::Free)
			DrawPistolMeshes(laraItem, laraInfo->Control.Weapon.GunType);

		break;

	case LaraWeaponType::Shotgun:
	case LaraWeaponType::Revolver:
	case LaraWeaponType::HK:
	case LaraWeaponType::GrenadeLauncher:
	case LaraWeaponType::HarpoonGun:
	case LaraWeaponType::RocketLauncher:
		laraInfo->RightArm.FrameBase = Objects[WeaponObject(laraInfo->Control.Weapon.GunType)].frameBase;
		laraInfo->LeftArm.FrameBase = Objects[WeaponObject(laraInfo->Control.Weapon.GunType)].frameBase;

		if (laraInfo->Control.HandStatus != HandStatus::Free)
			DrawShotgunMeshes(laraItem, laraInfo->Control.Weapon.GunType);

		break;

	case LaraWeaponType::Flare:
		laraInfo->RightArm.FrameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;
		laraInfo->LeftArm.FrameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;

		if (laraInfo->Control.HandStatus != HandStatus::Free)
			DrawFlareMeshes(laraItem);

		break;

	default:
		laraInfo->RightArm.FrameBase = g_Level.Anims[laraItem->Animation.AnimNumber].framePtr;
		laraInfo->LeftArm.FrameBase = g_Level.Anims[laraItem->Animation.AnimNumber].framePtr;
		break;
	}
}

GAME_OBJECT_ID WeaponObjectMesh(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	switch (weaponType)
	{
	case LaraWeaponType::Revolver:
		return (laraInfo->Weapons[(int)LaraWeaponType::Revolver].HasLasersight == true ? ID_LARA_REVOLVER_LASER : ID_REVOLVER_ANIM);

	case LaraWeaponType::Uzi:
		return ID_UZI_ANIM;

	case LaraWeaponType::Shotgun:
		return ID_SHOTGUN_ANIM;

	case LaraWeaponType::HK:
		return ID_HK_ANIM;

	case LaraWeaponType::Crossbow:
		return (laraInfo->Weapons[(int)LaraWeaponType::Crossbow].HasLasersight == true ? ID_LARA_CROSSBOW_LASER : ID_CROSSBOW_ANIM);
		
	case LaraWeaponType::GrenadeLauncher:
		return ID_GRENADE_ANIM;

	case LaraWeaponType::HarpoonGun:
		return ID_HARPOON_ANIM;

	case LaraWeaponType::RocketLauncher:
		return ID_ROCKET_ANIM;

	default:
		return ID_PISTOLS_ANIM;
	}
}

void HitTarget(ITEM_INFO* laraItem, ITEM_INFO* target, GAME_VECTOR* hitPos, int damage, int grenade)
{	
	auto* laraInfo = GetLaraInfo(laraItem);

	target->HitStatus = true;

	if (target->Data.is<CreatureInfo>())
		((CreatureInfo*)target->Data)->HurtByLara = true;

	auto* obj = &Objects[target->ObjectNumber];

	if (hitPos != nullptr)
	{
		if (obj->hitEffect != HIT_NONE)
		{
			switch (obj->hitEffect)
			{
			case HIT_BLOOD:
				if (target->ObjectNumber == ID_GOON2 &&
					(target->Animation.ActiveState == 8 || GetRandomControl() & 1) &&
					(laraInfo->Control.Weapon.GunType == LaraWeaponType::Pistol ||
						laraInfo->Control.Weapon.GunType == LaraWeaponType::Shotgun ||
						laraInfo->Control.Weapon.GunType == LaraWeaponType::Uzi))
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
					SoundEffect(SFX_TR5_SWORD_GOD_HITMET, &target->Position, 0);
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

	auto* weapon = &Weapons[(int)weaponType];
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

	laraInfo->Control.Weapon.HasFired = true;
	laraInfo->Control.Weapon.Fired = true;
	
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

	if (laraInfo->TargetEntity == nullptr)
	{
		laraInfo->RightArm.Locked = false;
		laraInfo->LeftArm.Locked = false;
		laraInfo->TargetArmAngles[1] = 0;
		laraInfo->TargetArmAngles[0] = 0;
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
	FindTargetPoint(laraInfo->TargetEntity, &targetPoint);
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

	laraInfo->TargetArmAngles[0] = angles[0];
	laraInfo->TargetArmAngles[1] = angles[1];
}

void LaraGetNewTarget(ITEM_INFO* laraItem, WeaponInfo* weaponInfo)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	if (BinocularRange)
	{
		laraInfo->TargetEntity = nullptr;
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
		if (ActiveCreatures[slot]->ItemNumber != NO_ITEM)
		{
			auto* item = &g_Level.Items[ActiveCreatures[slot]->ItemNumber];
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
		laraInfo->TargetEntity = NULL;
	else
	{
		for (int slot = 0; slot < MAX_TARGETS; ++slot)
		{
			if (!TargetList[slot])
				laraInfo->TargetEntity = NULL;

			if (TargetList[slot] == laraInfo->TargetEntity)
				break;
		}

		if (laraInfo->Control.HandStatus != HandStatus::Free || TrInput & IN_LOOKSWITCH)
		{
			if (!laraInfo->TargetEntity)
			{
				laraInfo->TargetEntity = bestItem;
				LastTargets[0] = NULL;
			}
			else if (TrInput & IN_LOOKSWITCH)
			{
				laraInfo->TargetEntity = NULL;
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
						laraInfo->TargetEntity = TargetList[match];
						if (laraInfo->TargetEntity)
							flag = false;

						break;
					}
				}

				if (flag)
				{
					laraInfo->TargetEntity = bestItem;
					LastTargets[0] = NULL;
				}
			}
		}
	}

	if (laraInfo->TargetEntity != LastTargets[0])
	{
		for (int slot = 7; slot > 0; --slot)
			LastTargets[slot] = LastTargets[slot - 1];
		
		LastTargets[0] = laraInfo->TargetEntity;
	}

	LaraTargetInfo(laraItem, weaponInfo);
}

HolsterSlot HolsterSlotForWeapon(LaraWeaponType weaponType)
{
	switch(weaponType)
	{
		case LaraWeaponType::Pistol:
			return HolsterSlot::Pistols;
		case LaraWeaponType::Uzi:
			return HolsterSlot::Uzis;
		case LaraWeaponType::Revolver:
			return HolsterSlot::Revolver;
		case LaraWeaponType::Shotgun:
			return HolsterSlot::Shotgun;
		case LaraWeaponType::HK:
			return HolsterSlot::HK;
		case LaraWeaponType::HarpoonGun:
			return HolsterSlot::Harpoon;
		case LaraWeaponType::Crossbow:
			return HolsterSlot::Crowssbow;
		case LaraWeaponType::GrenadeLauncher:
			return HolsterSlot::GrenadeLauncher;
		case LaraWeaponType::RocketLauncher:
			return HolsterSlot::RocketLauncher;
		default:
			return HolsterSlot::Empty;
	}
}
