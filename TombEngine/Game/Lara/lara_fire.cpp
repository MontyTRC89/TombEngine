#include "framework.h"
#include "Game/Lara/lara_fire.h"

#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/control/los.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/lara_two_guns.h"
#include "Game/misc.h"
#include "Game/savegame.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/ScriptInterfaceObjectsHandler.h"
#include "ScriptInterfaceGame.h"
#include "ScriptInterfaceLevel.h"
#include "Sound/sound.h"
#include "Specific/configuration.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Entities::Generic;
using namespace TEN::Input;
using namespace TEN::Math;

ItemInfo* LastTargets[MAX_TARGETS];
ItemInfo* TargetList[MAX_TARGETS];

int FlashGrenadeAftershockTimer = 0;

WeaponInfo Weapons[(int)LaraWeaponType::NumWeapons] =
{
	// No weapons
	{
		{ EulerAngles::Zero, EulerAngles::Zero },
		{ EulerAngles::Zero, EulerAngles::Zero },
		{ EulerAngles::Zero, EulerAngles::Zero },
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
		{ EulerAngles(ANGLE(-60.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(60.0f), ANGLE(60.0f), 0) },
		{ EulerAngles(ANGLE(-80.0f), ANGLE(-170.0f), 0), EulerAngles(ANGLE(80.0f), ANGLE(60.0f), 0) },
		{ EulerAngles(ANGLE(-80.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(80.0f), ANGLE(170.0f), 0) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		650,
		SECTOR(8),
		1,
		9,
		3,
		0,
		SFX_TR4_PISTOL_FIRE,
		0
	},

	// Revolver
	{
		{ EulerAngles(ANGLE(-60.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(60.0f), ANGLE(60.0f), 0) },
		{ EulerAngles(ANGLE(-80.0f), ANGLE(-10.0f), 0), EulerAngles(ANGLE(80.0f), ANGLE(10.0f), 0) },
		{ EulerAngles::Zero, EulerAngles::Zero },
		ANGLE(10.0f),
		ANGLE(4.0f),
		650,
		SECTOR(8),
		21,
		16,
		3,
		0,
		SFX_TR4_REVOLVER_FIRE,
		0
	},

	// Uzis
	{
		{ EulerAngles(ANGLE(-60.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(60.0f), ANGLE(60.0f), 0) },
		{ EulerAngles(ANGLE(-80.0f), ANGLE(-170.0f), 0), EulerAngles(ANGLE(80.0f), ANGLE(60.0f), 0) },
		{ EulerAngles(ANGLE(-80.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(80.0f), ANGLE(170.0f), 0) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		650,
		SECTOR(8),
		1,
		3,
		3,
		0,
		SFX_TR4_UZI_FIRE,
		0
	},

	// Shotgun
	{
		{ EulerAngles(ANGLE(-55.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(60.0f), 0) },
		{ EulerAngles(ANGLE(-65.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0) },
		{ EulerAngles(ANGLE(-65.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0) },
		ANGLE(10.0f),
		0,
		500,
		SECTOR(8),
		3,
		9,
		3,
		9,
		SFX_TR4_SHOTGUN_FIRE,
		0
	},

	// HK
	{
		{ EulerAngles(ANGLE(-55.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(60.0f), 0) },
		{ EulerAngles(ANGLE(-65.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0) },
		{ EulerAngles(ANGLE(-65.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0) },
		ANGLE(10.0f),
		ANGLE(4.0f),
		500,
		SECTOR(12),
		4,
		0,
		3,
		16,
		0,     // FIRE/SILENCER_FIRE
		0
	},

	// Crossbow
	{
		{ EulerAngles(ANGLE(-55.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(60.0f), 0) },
		{ EulerAngles(ANGLE(-65.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0) },
		{ EulerAngles(ANGLE(-65.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		500,
		SECTOR(8),
		5,
		0,
		2,
		9,
		SFX_TR4_CROSSBOW_FIRE,
		20
	},

	// Flare
	{
		{ EulerAngles::Zero, EulerAngles::Zero },
		{ EulerAngles::Zero, EulerAngles::Zero },
		{ EulerAngles::Zero, EulerAngles::Zero },
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
		{ EulerAngles(ANGLE(-55.0f), ANGLE(-30.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(30.0f), 0) },
		{ EulerAngles(ANGLE(-55.0f), ANGLE(-30.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(30.0f), 0) },
		{ EulerAngles(ANGLE(-55.0f), ANGLE(-30.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(30.0f), 0) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		400,
		SECTOR(8),
		3,
		0,
		2,
		0,
		SFX_TR4_UZI_FIRE,
		0
	},

	// Grenade launcher
	{
		{ EulerAngles(ANGLE(-55.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(60.0f), 0) },
		{ EulerAngles(ANGLE(-65.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0) },
		{ EulerAngles(ANGLE(-65.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		500,
		SECTOR(8),
		20,
		0,
		2,
		9,
		0,
		30
	},

	// Harpoon gun
	{
		{ EulerAngles(ANGLE(-65.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(60.0f), 0) },
		{ EulerAngles(ANGLE(-75.0f), ANGLE(-20.0f), 0), EulerAngles(ANGLE(75.0f), ANGLE(20.0f), 0) },
		{ EulerAngles(ANGLE(-75.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(75.0f), ANGLE(80.0f), 0) },
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
		{ EulerAngles(ANGLE(-55.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(60.0f), 0) },
		{ EulerAngles(ANGLE(-65.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0) },
		{ EulerAngles(ANGLE(-65.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0) },
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
		{ EulerAngles(ANGLE(-55.0f), ANGLE(-30.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(30.0f), 0) },
		{ EulerAngles(ANGLE(-55.0f), ANGLE(-30.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(30.0f), 0) },
		{ EulerAngles(ANGLE(-55.0f), ANGLE(-30.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(30.0f), 0) },
		ANGLE(10.0f),
		ANGLE(8.0f),
		400,
		SECTOR(8),
		3,
		0,
		0,
		0,
		SFX_TR4_UZI_FIRE,
		0
	}
};

// States in which Lara will hold a flare out in front.
const std::vector<LaraState> FlarePoseStates =
{
	LS_WALK_FORWARD,
	LS_RUN_FORWARD,
	LS_IDLE,
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
	LS_SOFT_SPLAT
};
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

void AimWeapon(ItemInfo* laraItem, WeaponInfo* weaponInfo, ArmInfo* arm)
{
	auto* lara = GetLaraInfo(laraItem);

	auto targetArmOrient = EulerAngles::Zero;
	if (arm->Locked)
		targetArmOrient = lara->TargetArmOrient;

	int speed = weaponInfo->AimSpeed;

	// Rotate arms on y axis toward target.
	short rotY = arm->Orientation.y;
	if (rotY >= (targetArmOrient.y - speed) && rotY <= (targetArmOrient.y + speed))
		rotY = targetArmOrient.y;
	else if (rotY < targetArmOrient.y)
		rotY += speed;
	else
		rotY -= speed;
	arm->Orientation.y = rotY;

	// Rotate arms on x axis toward target.
	short rotX = arm->Orientation.x;
	if (rotX >= (targetArmOrient.x - speed) && rotX <= (targetArmOrient.x + speed))
		rotX = targetArmOrient.x;
	else if (rotX < targetArmOrient.x)
		rotX += speed;
	else
		rotX -= speed;
	arm->Orientation.x = rotX;

	// TODO: Set arms to inherit rotations of parent bones.
	arm->Orientation.z = 0;
}

void SmashItem(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->ObjectNumber >= ID_SMASH_OBJECT1 && item->ObjectNumber <= ID_SMASH_OBJECT8)
		SmashObject(itemNumber);
}

void LaraGun(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	if (lara->LeftArm.GunFlash > 0)
		--lara->LeftArm.GunFlash;
	if (lara->RightArm.GunFlash > 0)
		--lara->RightArm.GunFlash;
	if (lara->RightArm.GunSmoke > 0)
		--lara->RightArm.GunSmoke;
	if (lara->LeftArm.GunSmoke > 0)
		--lara->LeftArm.GunSmoke;

	if (FlashGrenadeAftershockTimer)
		FlashGrenadeAftershockTimer--;

	if (lara->Control.Weapon.GunType == LaraWeaponType::Torch)
	{
		DoFlameTorch();
		return;
	}

	if (laraItem->HitPoints <= 0)
	{
		lara->Control.HandStatus = HandStatus::Free;
	}
	else if (lara->Control.HandStatus == HandStatus::Free)
	{
		// Draw weapon.
		if (TrInput & IN_DRAW)
			lara->Control.Weapon.RequestGunType = lara->Control.Weapon.LastGunType;
		// Draw flare.
		else if (TrInput & IN_FLARE &&
			(g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() != LaraType::Young))
		{
			if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
			{
			//	if (!lara->leftArm.frameNumber)	// NO
				{
					lara->Control.HandStatus = HandStatus::WeaponUndraw;
				}
			}
			else if (lara->Inventory.TotalFlares)
			{
				if (lara->Inventory.TotalFlares != -1)
					lara->Inventory.TotalFlares--;

				lara->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			}
		}

		if (TrInput & IN_DRAW || lara->Control.Weapon.RequestGunType != lara->Control.Weapon.GunType)
		{
			if (lara->Control.IsLow && 
				lara->Control.Weapon.RequestGunType >= LaraWeaponType::Shotgun && 
				lara->Control.Weapon.RequestGunType != LaraWeaponType::Flare && 
				lara->Control.Weapon.RequestGunType != LaraWeaponType::Torch)
			{
				if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
					lara->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			}
			else if (lara->Control.Weapon.RequestGunType == LaraWeaponType::Flare ||
				(lara->Vehicle == NO_ITEM &&
					(lara->Control.Weapon.RequestGunType == LaraWeaponType::HarpoonGun ||
						lara->Control.WaterStatus == WaterStatus::Dry ||
						(lara->Control.WaterStatus == WaterStatus::Wade &&
							lara->WaterSurfaceDist > -Weapons[(int)lara->Control.Weapon.GunType].GunHeight))))
			{
				if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
				{
					CreateFlare(laraItem, ID_FLARE_ITEM, 0);
					UndrawFlareMeshes(laraItem);
					lara->Flare.ControlLeft = false;
					lara->Flare.Life = 0;
				}

				lara->Control.Weapon.GunType = lara->Control.Weapon.RequestGunType;
				InitialiseNewWeapon(laraItem);
				lara->RightArm.FrameNumber = 0;
				lara->LeftArm.FrameNumber = 0;
				lara->Control.HandStatus = HandStatus::WeaponDraw;
			}
			else
			{
				lara->Control.Weapon.LastGunType = lara->Control.Weapon.RequestGunType;

				if (lara->Control.Weapon.GunType != LaraWeaponType::Flare)
					lara->Control.Weapon.GunType = lara->Control.Weapon.RequestGunType;
				else
					lara->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			}
		}
	}
	else if (lara->Control.HandStatus == HandStatus::WeaponReady)
	{
		if (TrInput & IN_DRAW ||
			lara->Control.Weapon.RequestGunType != lara->Control.Weapon.GunType)
		{
			lara->Control.HandStatus = HandStatus::WeaponUndraw;
		}
		else if (lara->Control.Weapon.GunType != LaraWeaponType::HarpoonGun &&
			lara->Control.WaterStatus != WaterStatus::Dry &&
			(lara->Control.WaterStatus != WaterStatus::Wade ||
				lara->WaterSurfaceDist < -Weapons[(int)lara->Control.Weapon.GunType].GunHeight))
		{
			lara->Control.HandStatus = HandStatus::WeaponUndraw;
		}
	}
	else if (TrInput & IN_FLARE &&
		lara->Control.HandStatus == HandStatus::Busy &&
		laraItem->Animation.ActiveState == LS_CRAWL_IDLE &&
		laraItem->Animation.AnimNumber == LA_CRAWL_IDLE)
	{
		lara->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
	}

	switch (lara->Control.HandStatus)
	{
	case HandStatus::WeaponDraw:
		if (lara->Control.Weapon.GunType != LaraWeaponType::Flare &&
			lara->Control.Weapon.GunType != LaraWeaponType::None)
		{
			lara->Control.Weapon.LastGunType = lara->Control.Weapon.GunType;
		}

		switch (lara->Control.Weapon.GunType)
		{
		case LaraWeaponType::Pistol:
		case LaraWeaponType::Revolver:
		case LaraWeaponType::Uzi:
			if (Camera.type != CameraType::Look && Camera.type != CameraType::Heavy)
				Camera.type = CameraType::Combat;

			DrawPistols(laraItem, lara->Control.Weapon.GunType);
			break;

		case LaraWeaponType::Shotgun:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::HK:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
			if (Camera.type != CameraType::Look && Camera.type != CameraType::Heavy)
				Camera.type = CameraType::Combat;

			DrawShotgun(laraItem, lara->Control.Weapon.GunType);
			break;

		case LaraWeaponType::Flare:
			DrawFlare(laraItem);
			break;

		default:
			lara->Control.HandStatus = HandStatus::Free;
			break;
		}

		break;

	case HandStatus::Special:
		DrawFlare(laraItem);
		break;

	case HandStatus::WeaponUndraw:
		lara->MeshPtrs[LM_HEAD] = Objects[ID_LARA_SKIN].meshIndex + LM_HEAD;

		switch (lara->Control.Weapon.GunType)
		{
		case LaraWeaponType::Pistol:
		case LaraWeaponType::Revolver:
		case LaraWeaponType::Uzi:
			UndrawPistols(laraItem, lara->Control.Weapon.GunType);
			break;

		case LaraWeaponType::Shotgun:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::HK:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
			UndrawShotgun(laraItem, lara->Control.Weapon.GunType);
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
			lara->MeshPtrs[LM_HEAD] = Objects[ID_LARA_SKIN].meshIndex + LM_HEAD;
		else
			lara->MeshPtrs[LM_HEAD] = Objects[ID_LARA_SCREAM].meshIndex + LM_HEAD;

		if (Camera.type != CameraType::Look &&
			Camera.type != CameraType::Heavy)
		{
			Camera.type = CameraType::Combat;
		}

		if (TrInput & IN_ACTION)
		{
			if (!GetAmmo(laraItem, lara->Control.Weapon.GunType))
			{
				lara->Control.Weapon.RequestGunType = Objects[ID_PISTOLS_ITEM].loaded ? LaraWeaponType::Pistol : LaraWeaponType::None;
				return;
			}
		}

		switch (lara->Control.Weapon.GunType)
		{
		case LaraWeaponType::Pistol:
		case LaraWeaponType::Uzi:
			PistolHandler(laraItem, lara->Control.Weapon.GunType);
			break;

		case LaraWeaponType::Shotgun:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::HK:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
		case LaraWeaponType::Revolver:
			RifleHandler(laraItem, lara->Control.Weapon.GunType);
			break;

		default:
			return;
		}

		break;

	case HandStatus::Free:
		if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
		{
			if (lara->Vehicle != NO_ITEM ||
				CheckLaraState((LaraState)laraItem->Animation.ActiveState, FlarePoseStates))
			{
				if (lara->Flare.ControlLeft)
				{
					if (lara->LeftArm.FrameNumber)
					{
						if (++lara->LeftArm.FrameNumber == 110)
							lara->LeftArm.FrameNumber = 0;
					}
				}
				else
				{
					lara->LeftArm.FrameNumber = 95;
					lara->Flare.ControlLeft = true;
				}
			}
			else
				lara->Flare.ControlLeft = false;

			DoFlareInHand(laraItem, lara->Flare.Life);
			SetFlareArm(laraItem, lara->LeftArm.FrameNumber);
		}

		break;

	case HandStatus::Busy:
		if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
		{
			if (lara->MeshPtrs[LM_LHAND] == Objects[ID_LARA_FLARE_ANIM].meshIndex + LM_LHAND)
			{
				lara->Flare.ControlLeft = (lara->Vehicle != NO_ITEM || CheckLaraState((LaraState)laraItem->Animation.ActiveState, FlarePoseStates));
				DoFlareInHand(laraItem, lara->Flare.Life);
				SetFlareArm(laraItem, lara->LeftArm.FrameNumber);
			}
		}

		break;
	}
}

Ammo& GetAmmo(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	return lara->Weapons[(int)weaponType].Ammo[(int)lara->Weapons[(int)weaponType].SelectedAmmo];
}

void InitialiseNewWeapon(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	lara->LeftArm.FrameNumber = 0;
	lara->RightArm.FrameNumber = 0;
	lara->LeftArm.Orientation = EulerAngles::Zero;
	lara->RightArm.Orientation = EulerAngles::Zero;
	lara->TargetEntity = nullptr;
	lara->LeftArm.Locked = false;
	lara->RightArm.Locked = false;
	lara->LeftArm.GunFlash = 0;
	lara->RightArm.GunFlash = 0;

	switch (lara->Control.Weapon.GunType)
	{
	case LaraWeaponType::Pistol:
	case LaraWeaponType::Uzi:
		lara->RightArm.FrameBase = Objects[ID_PISTOLS_ANIM].frameBase;
		lara->LeftArm.FrameBase = Objects[ID_PISTOLS_ANIM].frameBase;

		if (lara->Control.HandStatus != HandStatus::Free)
			DrawPistolMeshes(laraItem, lara->Control.Weapon.GunType);

		break;

	case LaraWeaponType::Shotgun:
	case LaraWeaponType::Revolver:
	case LaraWeaponType::HK:
	case LaraWeaponType::GrenadeLauncher:
	case LaraWeaponType::HarpoonGun:
	case LaraWeaponType::RocketLauncher:
		lara->RightArm.FrameBase = Objects[WeaponObject(lara->Control.Weapon.GunType)].frameBase;
		lara->LeftArm.FrameBase = Objects[WeaponObject(lara->Control.Weapon.GunType)].frameBase;

		if (lara->Control.HandStatus != HandStatus::Free)
			DrawShotgunMeshes(laraItem, lara->Control.Weapon.GunType);

		break;

	case LaraWeaponType::Flare:
		lara->RightArm.FrameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;
		lara->LeftArm.FrameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;

		if (lara->Control.HandStatus != HandStatus::Free)
			DrawFlareMeshes(laraItem);

		break;

	default:
		lara->RightArm.FrameBase = g_Level.Anims[laraItem->Animation.AnimNumber].framePtr;
		lara->LeftArm.FrameBase = g_Level.Anims[laraItem->Animation.AnimNumber].framePtr;
		break;
	}
}

GAME_OBJECT_ID WeaponObjectMesh(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	switch (weaponType)
	{
	case LaraWeaponType::Revolver:
		return (lara->Weapons[(int)LaraWeaponType::Revolver].HasLasersight ? ID_LARA_REVOLVER_LASER : ID_REVOLVER_ANIM);

	case LaraWeaponType::Uzi:
		return ID_UZI_ANIM;

	case LaraWeaponType::Shotgun:
		return ID_SHOTGUN_ANIM;

	case LaraWeaponType::HK:
		return ID_HK_ANIM;

	case LaraWeaponType::Crossbow:
		return (lara->Weapons[(int)LaraWeaponType::Crossbow].HasLasersight ? ID_LARA_CROSSBOW_LASER : ID_CROSSBOW_ANIM);
		
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

void HitTarget(ItemInfo* laraItem, ItemInfo* targetEntity, GameVector* hitPos, int damage, int grenade)
{	
	auto* lara = GetLaraInfo(laraItem);

	targetEntity->HitStatus = true;

	if (targetEntity->IsCreature())
		GetCreatureInfo(targetEntity)->HurtByLara = true;

	auto* object = &Objects[targetEntity->ObjectNumber];

	if (hitPos != nullptr)
	{
		if (object->hitEffect != HIT_NONE)
		{
			switch (object->hitEffect)
			{
			case HIT_BLOOD:
				if (targetEntity->ObjectNumber == ID_BADDY2 &&
					(targetEntity->Animation.ActiveState == 8 || GetRandomControl() & 1) &&
					(lara->Control.Weapon.GunType == LaraWeaponType::Pistol ||
						lara->Control.Weapon.GunType == LaraWeaponType::Shotgun ||
						lara->Control.Weapon.GunType == LaraWeaponType::Uzi))
				{
					// Baddy2 gun hitting sword
					SoundEffect(SFX_TR4_BADDY_SWORD_RICOCHET, &targetEntity->Pose);
					TriggerRicochetSpark(hitPos, laraItem->Pose.Orientation.y, 3, 0);
					return;
				}
				else
					DoBloodSplat(hitPos->x, hitPos->y, hitPos->z, (GetRandomControl() & 3) + 3, targetEntity->Pose.Orientation.y, targetEntity->RoomNumber);

				break;

			case HIT_RICOCHET:
				TriggerRicochetSpark(hitPos, laraItem->Pose.Orientation.y, 3, 0);
				break;

			case HIT_SMOKE:
				TriggerRicochetSpark(hitPos, laraItem->Pose.Orientation.y, 3, -5);

				if (targetEntity->ObjectNumber == ID_ROMAN_GOD1 ||
					targetEntity->ObjectNumber == ID_ROMAN_GOD2)
				{
					SoundEffect(SFX_TR5_SWORD_GOD_HIT_METAL, &targetEntity->Pose);
				}

				break;
			}
		}
	}

	if (!object->undead || grenade)
	{
		if (targetEntity->HitPoints > 0)
		{
			Statistics.Level.AmmoHits++;
			DoDamage(targetEntity, damage);
		}
	}

	if (!targetEntity->LuaCallbackOnHitName.empty())
	{
		short index = g_GameScriptEntities->GetIndexByName(targetEntity->LuaName);
		g_GameScript->ExecuteFunction(targetEntity->LuaCallbackOnHitName, index);
	}
}

FireWeaponType FireWeapon(LaraWeaponType weaponType, ItemInfo* targetEntity, ItemInfo* originEntity, EulerAngles armOrient)
{
	auto* lara = GetLaraInfo(originEntity);

	auto& ammo = GetAmmo(originEntity, weaponType);
	if (ammo.getCount() == 0 && !ammo.hasInfinite())
		return FireWeaponType::NoAmmo;

	if (!ammo.hasInfinite())
		ammo--;

	auto* weapon = &Weapons[(int)weaponType];

	auto wobbledArmOrient = EulerAngles(
		armOrient.x + (Random::GenerateAngle(0, ANGLE(180.0f)) - ANGLE(90.0f)) * weapon->ShotAccuracy / 65536,
		armOrient.y + (Random::GenerateAngle(0, ANGLE(180.0f)) - ANGLE(90.0f)) * weapon->ShotAccuracy / 65536,
		0
	);

	auto muzzleOffset = GetLaraJointPosition(LM_RHAND);
	auto pos = Vector3i(originEntity->Pose.Position.x, muzzleOffset.y, originEntity->Pose.Position.z);

	// Calculate ray from wobbled orientation.
	auto directionNorm = wobbledArmOrient.ToDirection();
	auto origin = pos.ToVector3();
	auto target = origin + (directionNorm * weapon->TargetDist);
	auto ray = Ray(origin, directionNorm);

	int num = GetSpheres(targetEntity, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
	int bestItemNumber = NO_ITEM;
	float bestDistance = FLT_MAX;

	for (int i = 0; i < num; i++)
	{
		auto sphere = BoundingSphere(Vector3(CreatureSpheres[i].x, CreatureSpheres[i].y, CreatureSpheres[i].z), CreatureSpheres[i].r);
		float distance;
		if (ray.Intersects(sphere, distance))
		{
			if (distance < bestDistance)
			{
				bestDistance = distance;
				bestItemNumber = i;
			}
		}
	}

	lara->Control.Weapon.HasFired = true;
	lara->Control.Weapon.Fired = true;
	
	auto vOrigin = GameVector(pos);
	short roomNumber = originEntity->RoomNumber;
	GetFloor(pos.x, pos.y, pos.z, &roomNumber);
	vOrigin.roomNumber = roomNumber;

	if (bestItemNumber < 0)
	{
		auto vTarget = GameVector(target);
		GetTargetOnLOS(&vOrigin, &vTarget, false, true);
		return FireWeaponType::Miss;
	}
	else
	{
		Statistics.Game.AmmoHits++;

		target = origin + (directionNorm * bestDistance);

		auto vTarget = GameVector(target);

		// TODO: Enable when slot is created.
		/*
		if (target->ObjectNumber == ID_TRIBEBOSS)
		{
			long dx, dy, dz;

			dx = (vDest.x - vSrc.x) >> 5;
			dy = (vDest.y - vSrc.y) >> 5;
			dz = (vDest.z - vSrc.z) >> 5;
			FindClosestShieldPoint(vDest.x - dx, vDest.y - dy, vDest.z - dz, target);
		}
		else if (target->ObjectNumber == ID_ARMY_WINSTON || target->ObjectNumber == ID_LONDONBOSS) //Don't want blood on Winston - never get the stains out
		{
			short ricochet_angle;
			target->HitStatus = true; //need to do this to maintain defence state
			target->HitPoints--;
			ricochet_angle = (mGetAngle(lara->Pose.Position.z, lara->Pose.Position.x, target->Pose.Position.z, target->Pose.Position.x) >> 4) & 4095;
			TriggerRicochetSparks(&vDest, ricochet_angle, 16, 0);
			SoundEffect(SFX_TR4_WEAPON_RICOCHET, &target->Pose);		// play RICOCHET Sample
		}
		else if (target->ObjectNumber == ID_SHIVA) //So must be Shiva
		{
			z = target->Pose.Position.z - lara_item->Pose.Position.z;
			x = target->Pose.Position.x - lara_item->Pose.Position.x;
			angle = 0x8000 + phd_atan(z, x) - target->Pose.Orientation.y;

			if ((target->ActiveState > 1 && target->ActiveState < 5) && angle < 0x4000 && angle > -0x4000)
			{
				target->HitStatus = true; //need to do this to maintain defence state
				ricochet_angle = (mGetAngle(lara->Pose.Position.z, lara->Pose.Position.x, target->Pose.Position.z, target->Pose.Position.x) >> 4) & 4095;
				TriggerRicochetSparks(&vDest, ricochet_angle, 16, 0);
				SoundEffect(SFX_TR4_WEAPON_RICOCHET, &target->Pose); // play RICOCHET Sample
			}
			else //Shiva's not in defence mode or has its back to Lara
				HitTarget(target, &vDest, weapon->damage, 0);
		}
		else
		{*/

			// NOTE: it seems that items for being hit by Lara in the normal way must have GetTargetOnLOS returning false
			// it's really weird but we decided to replicate original behaviour until we'll fully understand what is happening
			// with weapons
			if (!GetTargetOnLOS(&vOrigin, &vTarget, false, true))
				HitTarget(originEntity, targetEntity, &vTarget, weapon->Damage, false);
		//}
		
		return FireWeaponType::PossibleHit;
	}
}

GameVector FindTargetPoint(ItemInfo* item)
{
	auto* bounds = (BOUNDING_BOX*)GetBestFrame(item);

	auto center = Vector3i(
		int(bounds->X1 + bounds->X2) / 2,
		(int)bounds->Y1 + bounds->Height() / 3,
		int(bounds->Z1 + bounds->Z2) / 2
	);

	float sinY = phd_sin(item->Pose.Orientation.y);
	float cosY = phd_cos(item->Pose.Orientation.y);

	return GameVector(
		item->Pose.Position.x + ((center.x * cosY) + (center.z * sinY)),
		item->Pose.Position.y + center.y,
		item->Pose.Position.z + ((center.z * cosY) - (center.x * sinY)),
		item->RoomNumber
	);
}

void LaraTargetInfo(ItemInfo* laraItem, WeaponInfo* weaponInfo)
{
	auto* lara = GetLaraInfo(laraItem);

	if (lara->TargetEntity == nullptr)
	{
		lara->RightArm.Locked = false;
		lara->LeftArm.Locked = false;
		lara->TargetArmOrient = EulerAngles::Zero;
		return;
	}

	auto origin = GameVector(
		laraItem->Pose.Position.x,
		GetLaraJointPosition(LM_RHAND).y, // Muzzle offset.
		laraItem->Pose.Position.z,
		laraItem->RoomNumber
	);
	auto target = FindTargetPoint(lara->TargetEntity);

	auto orient = Geometry::GetOrientTowardPoint(origin.ToVector3(), target.ToVector3()) - laraItem->Pose.Orientation;

	if (LOS(&origin, &target))
	{
		if (orient.x >= weaponInfo->LockAngles[0].x &&
			orient.y >= weaponInfo->LockAngles[0].y &&
			orient.x <= weaponInfo->LockAngles[1].x &&
			orient.y <= weaponInfo->LockAngles[1].y)
		{
			lara->RightArm.Locked = true;
			lara->LeftArm.Locked = true;
		}
		else
		{
			if (lara->LeftArm.Locked)
			{
				if (orient.x < weaponInfo->LeftAngles[0].x ||
					orient.y < weaponInfo->LeftAngles[0].y ||
					orient.x > weaponInfo->LeftAngles[1].x ||
					orient.y > weaponInfo->LeftAngles[1].y)
				{
					lara->LeftArm.Locked = false;
				}
			}

			if (lara->RightArm.Locked)
			{
				if (orient.x < weaponInfo->RightAngles[0].x ||
					orient.y < weaponInfo->RightAngles[0].y ||
					orient.x > weaponInfo->RightAngles[1].x ||
					orient.y > weaponInfo->RightAngles[1].y)
				{
					lara->RightArm.Locked = false;
				}
			}
		}
	}
	else
	{
		lara->RightArm.Locked = false;
		lara->LeftArm.Locked = false;
	}

	lara->TargetArmOrient = orient;
}

void LaraGetNewTarget(ItemInfo* laraItem, WeaponInfo* weaponInfo)
{
	if (!g_Configuration.AutoTarget)
		return;

	auto* lara = GetLaraInfo(laraItem);

	if (BinocularRange)
	{
		lara->TargetEntity = nullptr;
		return;
	}

	auto origin = GameVector(
		laraItem->Pose.Position.x,
		GetLaraJointPosition(LM_RHAND).y, // Muzzle offset.
		laraItem->Pose.Position.z,
		laraItem->RoomNumber
	);

	ItemInfo* bestEntity = nullptr;
	float bestDistance = FLT_MAX;
	short bestYOrient = MAXSHORT;
	unsigned int numTargets = 0;
	float maxDistance = weaponInfo->TargetDist;

	for (auto* activeCreature : ActiveCreatures)
	{
		// Continue loop if no item.
		if (activeCreature->ItemNumber == NO_ITEM)
			continue;

		auto* item = &g_Level.Items[activeCreature->ItemNumber];

		// Assess whether creature is alive.
		if (item->HitPoints <= 0)
			continue;

		// Assess distance.
		float distance = Vector3::Distance(origin.ToVector3(), item->Pose.Position.ToVector3());
		if (distance > maxDistance)
			continue;

		// Assess line of sight.
		auto target = FindTargetPoint(item);
		if (!LOS(&origin, &target))
			continue;

		// Assess whether relative orientation falls within weapon's lock constraints.
		auto orient = Geometry::GetOrientTowardPoint(origin.ToVector3(), target.ToVector3()) - (laraItem->Pose.Orientation + lara->ExtraTorsoRot);
		if (orient.x >= weaponInfo->LockAngles[0].x &&
			orient.y >= weaponInfo->LockAngles[0].y &&
			orient.x <= weaponInfo->LockAngles[1].x &&
			orient.y <= weaponInfo->LockAngles[1].y)
		{
			TargetList[numTargets] = item;
			++numTargets;

			if (distance < bestDistance &&
				abs(orient.y) < (bestYOrient + ANGLE(15.0f)))
			{
				bestEntity = item;
				bestDistance = distance;
				bestYOrient = abs(orient.y);
			}
		}
	}

	TargetList[numTargets] = nullptr;
	if (!TargetList[0])
		lara->TargetEntity = nullptr;
	else
	{
		for (int slot = 0; slot < MAX_TARGETS; ++slot)
		{
			if (!TargetList[slot])
				lara->TargetEntity = nullptr;

			if (TargetList[slot] == lara->TargetEntity)
				break;
		}

		if (lara->Control.HandStatus != HandStatus::Free || TrInput & IN_LOOKSWITCH)
		{
			if (!lara->TargetEntity)
			{
				lara->TargetEntity = bestEntity;
				LastTargets[0] = nullptr;
			}
			else if (TrInput & IN_LOOKSWITCH)
			{
				lara->TargetEntity = nullptr;
				bool flag = true;

				for (int match = 0; match < MAX_TARGETS && TargetList[match]; ++match)
				{
					bool doLoop = false;
					for (int slot = 0; slot < MAX_TARGETS && LastTargets[slot]; ++slot)
					{
						if (LastTargets[slot] == TargetList[match])
						{
							doLoop = true;
							break;
						}
					}

					if (!doLoop)
					{
						lara->TargetEntity = TargetList[match];
						if (lara->TargetEntity)
							flag = false;

						break;
					}
				}

				if (flag)
				{
					lara->TargetEntity = bestEntity;
					LastTargets[0] = nullptr;
				}
			}
		}
	}

	if (lara->TargetEntity != LastTargets[0])
	{
		for (int slot = 7; slot > 0; --slot)
			LastTargets[slot] = LastTargets[slot - 1];
		
		LastTargets[0] = lara->TargetEntity;
	}

	LaraTargetInfo(laraItem, weaponInfo);
}

HolsterSlot HolsterSlotForWeapon(LaraWeaponType weaponType)
{
	switch (weaponType)
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
