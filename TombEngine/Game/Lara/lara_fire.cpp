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
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Entities::Generic;
using namespace TEN::Input;
using namespace TEN::Math;

ItemInfo* LastTargets[MAX_TARGETS];
ItemInfo* TargetList[MAX_TARGETS];

int FlashGrenadeAftershockTimer = 0;

// States in which Lara will hold an active flare out in front.
const auto FlarePoseStates = std::vector<int>
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
	LS_SWITCH_DOWN,
	LS_SWITCH_UP,
	LS_WADE_FORWARD,
	LS_CROUCH_IDLE,
	LS_CROUCH_TURN_LEFT,
	LS_CROUCH_TURN_RIGHT,
	LS_SOFT_SPLAT
};

WeaponInfo Weapons[(int)LaraWeaponType::NumWeapons] =
{
	// No weapon
	{
		std::pair(EulerAngles::Zero, EulerAngles::Zero),
		std::pair(EulerAngles::Zero, EulerAngles::Zero),
		std::pair(EulerAngles::Zero, EulerAngles::Zero),
		0,
		0,
		0,
		0.0f,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// Pistols
	{
		std::pair(EulerAngles(ANGLE(-80.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(80.0f), ANGLE(60.0f), 0)),
		std::pair(EulerAngles(ANGLE(-80.0f), ANGLE(-170.0f), 0), EulerAngles(ANGLE(80.0f), ANGLE(60.0f), 0)),
		std::pair(EulerAngles(ANGLE(-80.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(80.0f), ANGLE(170.0f), 0)),
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
		std::pair(EulerAngles(ANGLE(-80.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(80.0f), ANGLE(60.0f), 0)),
		std::pair(EulerAngles(ANGLE(-80.0f), ANGLE(-10.0f), 0), EulerAngles(ANGLE(80.0f), ANGLE(10.0f), 0)),
		std::pair(EulerAngles::Zero, EulerAngles::Zero),
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
		std::pair(EulerAngles(ANGLE(-80.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(80.0f), ANGLE(60.0f), 0)),
		std::pair(EulerAngles(ANGLE(-80.0f), ANGLE(-170.0f), 0), EulerAngles(ANGLE(80.0f), ANGLE(60.0f), 0)),
		std::pair(EulerAngles(ANGLE(-80.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(80.0f), ANGLE(170.0f), 0)),
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
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(60.0f), 0)),
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0)),
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0)),
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
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(60.0f), 0)),
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0)),
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0)),
		ANGLE(10.0f),
		ANGLE(4.0f),
		500,
		SECTOR(12),
		4,
		0,
		3,
		16,
		0, // FIRE / SILENCER_FIRE
		0
	},

	// Crossbow
	{
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(60.0f), 0)),
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0)),
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0)),
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
		std::pair(EulerAngles::Zero, EulerAngles::Zero),
		std::pair(EulerAngles::Zero, EulerAngles::Zero),
		std::pair(EulerAngles::Zero, EulerAngles::Zero),
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

	// Torch
	{
		std::pair(EulerAngles(ANGLE(-55.0f), ANGLE(-30.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(30.0f), 0)),
		std::pair(EulerAngles(ANGLE(-55.0f), ANGLE(-30.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(30.0f), 0)),
		std::pair(EulerAngles(ANGLE(-55.0f), ANGLE(-30.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(30.0f), 0)),
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
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(60.0f), 0)),
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0)),
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0)),
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
		std::pair(EulerAngles(ANGLE(-75.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(75.0f), ANGLE(60.0f), 0)),
		std::pair(EulerAngles(ANGLE(-75.0f), ANGLE(-20.0f), 0), EulerAngles(ANGLE(75.0f), ANGLE(20.0f), 0)),
		std::pair(EulerAngles(ANGLE(-75.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(75.0f), ANGLE(80.0f), 0)),
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
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-60.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(60.0f), 0)),
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0)),
		std::pair(EulerAngles(ANGLE(-70.0f), ANGLE(-80.0f), 0), EulerAngles(ANGLE(65.0f), ANGLE(80.0f), 0)),
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
		std::pair(EulerAngles(ANGLE(-55.0f), ANGLE(-30.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(30.0f), 0)),
		std::pair(EulerAngles(ANGLE(-55.0f), ANGLE(-30.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(30.0f), 0)),
		std::pair(EulerAngles(ANGLE(-55.0f), ANGLE(-30.0f), 0), EulerAngles(ANGLE(55.0f), ANGLE(30.0f), 0)),
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

void InitialiseNewWeapon(ItemInfo& laraItem)
{
	auto& lara = *GetLaraInfo(&laraItem);

	lara.TargetEntity = nullptr;
	lara.LeftArm.FrameNumber =
	lara.RightArm.FrameNumber = 0;
	lara.LeftArm.Orientation =
	lara.RightArm.Orientation = EulerAngles::Zero;
	lara.LeftArm.Locked =
	lara.RightArm.Locked = false;
	lara.LeftArm.GunFlash =
	lara.RightArm.GunFlash = 0;

	switch (lara.Control.Weapon.GunType)
	{
	case LaraWeaponType::Pistol:
	case LaraWeaponType::Uzi:
		lara.RightArm.FrameBase = Objects[ID_PISTOLS_ANIM].frameBase;
		lara.LeftArm.FrameBase = Objects[ID_PISTOLS_ANIM].frameBase;

		if (lara.Control.HandStatus != HandStatus::Free)
			DrawPistolMeshes(laraItem, lara.Control.Weapon.GunType);

		break;

	case LaraWeaponType::Shotgun:
	case LaraWeaponType::Revolver:
	case LaraWeaponType::HK:
	case LaraWeaponType::GrenadeLauncher:
	case LaraWeaponType::HarpoonGun:
	case LaraWeaponType::RocketLauncher:
		lara.RightArm.FrameBase = Objects[GetWeaponObjectID(lara.Control.Weapon.GunType)].frameBase;
		lara.LeftArm.FrameBase = Objects[GetWeaponObjectID(lara.Control.Weapon.GunType)].frameBase;

		if (lara.Control.HandStatus != HandStatus::Free)
			DrawShotgunMeshes(laraItem, lara.Control.Weapon.GunType);

		break;

	case LaraWeaponType::Flare:
		lara.RightArm.FrameBase = Objects[ID_FLARE_ANIM].frameBase;
		lara.LeftArm.FrameBase = Objects[ID_FLARE_ANIM].frameBase;

		if (lara.Control.HandStatus != HandStatus::Free)
			DrawFlareMeshes(laraItem);

		break;

	default:
		lara.RightArm.FrameBase = g_Level.Anims[laraItem.Animation.AnimNumber].FramePtr;
		lara.LeftArm.FrameBase = g_Level.Anims[laraItem.Animation.AnimNumber].FramePtr;
		break;
	}
}

Ammo& GetAmmo(LaraInfo& lara, LaraWeaponType weaponType)
{
	return lara.Weapons[(int)weaponType].Ammo[(int)lara.Weapons[(int)weaponType].SelectedAmmo];
}

GameVector GetTargetPoint(ItemInfo& targetEntity)
{
	const auto& bounds = GetBestFrame(&targetEntity)->boundingBox;

	auto center = Vector3i(
		(bounds.X1 + bounds.X2) / 2,
		bounds.Y1 + (bounds.GetHeight() / 3),
		(bounds.Z1 + bounds.Z2) / 2);

	float sinY = phd_sin(targetEntity.Pose.Orientation.y);
	float cosY = phd_cos(targetEntity.Pose.Orientation.y);

	return GameVector(
		targetEntity.Pose.Position.x + ((center.x * cosY) + (center.z * sinY)),
		targetEntity.Pose.Position.y + center.y,
		targetEntity.Pose.Position.z + ((center.z * cosY) - (center.x * sinY)),
		targetEntity.RoomNumber);
}

HolsterSlot GetWeaponHolsterSlot(LaraWeaponType weaponType)
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
		return HolsterSlot::Crossbow;

	case LaraWeaponType::GrenadeLauncher:
		return HolsterSlot::GrenadeLauncher;

	case LaraWeaponType::RocketLauncher:
		return HolsterSlot::RocketLauncher;

	default:
		return HolsterSlot::Empty;
	}
}

GAME_OBJECT_ID GetWeaponObjectID(LaraWeaponType weaponType)
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
		return ID_FLARE_ANIM;

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

GAME_OBJECT_ID GetWeaponObjectMeshID(ItemInfo& laraItem, LaraWeaponType weaponType)
{
	const auto& lara = *GetLaraInfo(&laraItem);

	switch (weaponType)
	{
	case LaraWeaponType::Revolver:
		return (lara.Weapons[(int)LaraWeaponType::Revolver].HasLasersight ? ID_LARA_REVOLVER_LASER : ID_REVOLVER_ANIM);

	case LaraWeaponType::Uzi:
		return ID_UZI_ANIM;

	case LaraWeaponType::Shotgun:
		return ID_SHOTGUN_ANIM;

	case LaraWeaponType::HK:
		return ID_HK_ANIM;

	case LaraWeaponType::Crossbow:
		return (lara.Weapons[(int)LaraWeaponType::Crossbow].HasLasersight ? ID_LARA_CROSSBOW_LASER : ID_CROSSBOW_ANIM);

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

void HandleWeapon(ItemInfo& laraItem)
{
	auto& lara = *GetLaraInfo(&laraItem);

	if (lara.LeftArm.GunFlash > 0)
		--lara.LeftArm.GunFlash;

	if (lara.RightArm.GunFlash > 0)
		--lara.RightArm.GunFlash;

	if (lara.RightArm.GunSmoke > 0)
		--lara.RightArm.GunSmoke;

	if (lara.LeftArm.GunSmoke > 0)
		--lara.LeftArm.GunSmoke;

	if (FlashGrenadeAftershockTimer)
		FlashGrenadeAftershockTimer--;

	if (lara.Control.Weapon.GunType == LaraWeaponType::Torch)
	{
		DoFlameTorch();
		return;
	}

	if (laraItem.HitPoints <= 0)
	{
		lara.Control.HandStatus = HandStatus::Free;
	}
	else if (lara.Control.HandStatus == HandStatus::Free)
	{
		// Draw weapon.
		if (IsHeld(In::DrawWeapon))
		{
			// No weapon - no any actions.
			if (lara.Control.Weapon.LastGunType != LaraWeaponType::None)
				lara.Control.Weapon.RequestGunType = lara.Control.Weapon.LastGunType;
		}
		// Draw flare.
		else if (IsHeld(In::Flare) && (g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() != LaraType::Young))
		{
			if (lara.Control.Weapon.GunType == LaraWeaponType::Flare)
			{
				if (!lara.LeftArm.FrameNumber)
				{
					lara.Control.HandStatus = HandStatus::WeaponUndraw;
				}
			}
			else if (lara.Inventory.TotalFlares)
			{
				if (lara.Inventory.TotalFlares != -1)
					lara.Inventory.TotalFlares--;

				lara.Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			}
		}

		if ((IsHeld(In::DrawWeapon) && lara.Control.Weapon.LastGunType != LaraWeaponType::None) ||
			lara.Control.Weapon.RequestGunType != lara.Control.Weapon.GunType)
		{
			if (lara.Control.IsLow && 
				lara.Control.Weapon.RequestGunType >= LaraWeaponType::Shotgun && 
				lara.Control.Weapon.RequestGunType != LaraWeaponType::Flare && 
				lara.Control.Weapon.RequestGunType != LaraWeaponType::Torch)
			{
				if (lara.Control.Weapon.GunType == LaraWeaponType::Flare)
					lara.Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			}
			else if (lara.Control.Weapon.RequestGunType == LaraWeaponType::Flare ||
				(lara.Vehicle == NO_ITEM &&
					(lara.Control.Weapon.RequestGunType == LaraWeaponType::HarpoonGun ||
						lara.Control.WaterStatus == WaterStatus::Dry ||
						(lara.Control.WaterStatus == WaterStatus::Wade &&
							lara.WaterSurfaceDist > -Weapons[(int)lara.Control.Weapon.GunType].GunHeight))))
			{
				if (lara.Control.Weapon.GunType == LaraWeaponType::Flare)
				{
					CreateFlare(laraItem, ID_FLARE_ITEM, 0);
					UndrawFlareMeshes(laraItem);
					lara.Flare.ControlLeft = false;
					lara.Flare.Life = 0;
				}

				lara.Control.Weapon.GunType = lara.Control.Weapon.RequestGunType;
				InitialiseNewWeapon(laraItem);
				lara.RightArm.FrameNumber = 0;
				lara.LeftArm.FrameNumber = 0;
				lara.Control.HandStatus = HandStatus::WeaponDraw;
			}
			else
			{
				lara.Control.Weapon.LastGunType = lara.Control.Weapon.RequestGunType;

				if (lara.Control.Weapon.GunType != LaraWeaponType::Flare)
					lara.Control.Weapon.GunType = lara.Control.Weapon.RequestGunType;
				else
					lara.Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			}
		}
	}
	else if (lara.Control.HandStatus == HandStatus::WeaponReady)
	{
		if (IsHeld(In::DrawWeapon) ||
			lara.Control.Weapon.RequestGunType != lara.Control.Weapon.GunType)
		{
			lara.Control.HandStatus = HandStatus::WeaponUndraw;
		}
		else if (lara.Control.Weapon.GunType != LaraWeaponType::HarpoonGun &&
			lara.Control.WaterStatus != WaterStatus::Dry &&
			(lara.Control.WaterStatus != WaterStatus::Wade ||
				lara.WaterSurfaceDist < -Weapons[(int)lara.Control.Weapon.GunType].GunHeight))
		{
			lara.Control.HandStatus = HandStatus::WeaponUndraw;
		}
	}
	else if (IsHeld(In::Flare) &&
		lara.Control.HandStatus == HandStatus::Busy &&
		laraItem.Animation.ActiveState == LS_CRAWL_IDLE &&
		laraItem.Animation.AnimNumber == LA_CRAWL_IDLE)
	{
		lara.Control.Weapon.RequestGunType = LaraWeaponType::Flare;
	}

	switch (lara.Control.HandStatus)
	{
	case HandStatus::WeaponDraw:
		if (lara.Control.Weapon.GunType != LaraWeaponType::Flare &&
			lara.Control.Weapon.GunType != LaraWeaponType::None)
		{
			lara.Control.Weapon.LastGunType = lara.Control.Weapon.GunType;
		}

		switch (lara.Control.Weapon.GunType)
		{
		case LaraWeaponType::Pistol:
		case LaraWeaponType::Revolver:
		case LaraWeaponType::Uzi:
			if (Camera.type != CameraType::Look && Camera.type != CameraType::Heavy)
				Camera.type = CameraType::Combat;

			DrawPistols(laraItem, lara.Control.Weapon.GunType);
			break;

		case LaraWeaponType::Shotgun:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::HK:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
			if (Camera.type != CameraType::Look && Camera.type != CameraType::Heavy)
				Camera.type = CameraType::Combat;

			DrawShotgun(laraItem, lara.Control.Weapon.GunType);
			break;

		case LaraWeaponType::Flare:
			DrawFlare(laraItem);
			break;

		default:
			lara.Control.HandStatus = HandStatus::Free;
			break;
		}

		break;

	case HandStatus::Special:
		DrawFlare(laraItem);
		break;

	case HandStatus::WeaponUndraw:
		laraItem.Model.MeshIndex[LM_HEAD] = laraItem.Model.BaseMesh + LM_HEAD;

		switch (lara.Control.Weapon.GunType)
		{
		case LaraWeaponType::Pistol:
		case LaraWeaponType::Revolver:
		case LaraWeaponType::Uzi:
			UndrawPistols(laraItem, lara.Control.Weapon.GunType);
			break;

		case LaraWeaponType::Shotgun:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::HK:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
			UndrawShotgun(laraItem, lara.Control.Weapon.GunType);
			break;

		case LaraWeaponType::Flare:
			UndrawFlare(laraItem);
			break;

		default:
			return;
		}

		break;

	case HandStatus::WeaponReady:
		if (!IsHeld(In::Action))
			laraItem.Model.MeshIndex[LM_HEAD] = laraItem.Model.BaseMesh + LM_HEAD;
		else
			laraItem.Model.MeshIndex[LM_HEAD] = Objects[ID_LARA_SCREAM].meshIndex + LM_HEAD;

		if (Camera.type != CameraType::Look &&
			Camera.type != CameraType::Heavy)
		{
			Camera.type = CameraType::Combat;
		}

		if (IsHeld(In::Action) && !LaserSight)
		{
			if (!GetAmmo(lara, lara.Control.Weapon.GunType))
			{
				bool hasPistols = (lara.Weapons[(int)LaraWeaponType::Pistol].Present && Objects[ID_PISTOLS_ITEM].loaded);
				lara.Control.Weapon.RequestGunType = hasPistols ? LaraWeaponType::Pistol : LaraWeaponType::None;
				return;
			}
		}

		switch (lara.Control.Weapon.GunType)
		{
		case LaraWeaponType::Pistol:
		case LaraWeaponType::Uzi:
			HandlePistols(laraItem, lara.Control.Weapon.GunType);
			break;

		case LaraWeaponType::Shotgun:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::HK:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
		case LaraWeaponType::Revolver:
			RifleHandler(laraItem, lara.Control.Weapon.GunType);
			LasersightWeaponHandler(laraItem, lara.Control.Weapon.GunType);
			break;

		default:
			return;
		}

		break;

	case HandStatus::Free:
		if (lara.Control.Weapon.GunType == LaraWeaponType::Flare)
		{
			if (lara.Vehicle != NO_ITEM || TestState(laraItem.Animation.ActiveState, FlarePoseStates))
			{
				if (lara.Flare.ControlLeft)
				{
					if (lara.LeftArm.FrameNumber)
					{
						if (++lara.LeftArm.FrameNumber == 110)
							lara.LeftArm.FrameNumber = 0;
					}
				}
				else
				{
					lara.LeftArm.FrameNumber = 95;
					lara.Flare.ControlLeft = true;
				}
			}
			else
			{
				lara.Flare.ControlLeft = false;
			}

			DoFlareInHand(laraItem, lara.Flare.Life);
			SetFlareArm(laraItem, lara.LeftArm.FrameNumber);
		}

		break;

	case HandStatus::Busy:
		if (lara.Control.Weapon.GunType == LaraWeaponType::Flare)
		{
			if (laraItem.Model.MeshIndex[LM_LHAND] == Objects[ID_FLARE_ANIM].meshIndex + LM_LHAND)
			{
				lara.Flare.ControlLeft = (lara.Vehicle != NO_ITEM || TestState(laraItem.Animation.ActiveState, FlarePoseStates));
				DoFlareInHand(laraItem, lara.Flare.Life);
				SetFlareArm(laraItem, lara.LeftArm.FrameNumber);
			}
		}

		break;
	}
}

void AimWeapon(ItemInfo& laraItem, ArmInfo& arm, const WeaponInfo& weaponInfo)
{
	const auto& lara = *GetLaraInfo(&laraItem);

	auto targetArmOrient = arm.Locked ? lara.TargetArmOrient : EulerAngles::Zero;
	arm.Orientation.InterpolateConstant(targetArmOrient, weaponInfo.AimSpeed);
}

FireWeaponType FireWeapon(LaraWeaponType weaponType, ItemInfo& targetEntity, ItemInfo& laraItem, const EulerAngles& armOrient)
{
	auto& lara = *GetLaraInfo(&laraItem);
	auto& ammo = GetAmmo(lara, weaponType);

	if (ammo.GetCount() == 0 && !ammo.HasInfinite())
		return FireWeaponType::NoAmmo;

	if (!ammo.HasInfinite())
		ammo--;

	const auto& weapon = Weapons[(int)weaponType];

	auto wobbledArmOrient = EulerAngles(
		armOrient.x + (Random::GenerateAngle(0, ANGLE(180.0f)) - ANGLE(90.0f)) * weapon.ShotAccuracy / 65536,
		armOrient.y + (Random::GenerateAngle(0, ANGLE(180.0f)) - ANGLE(90.0f)) * weapon.ShotAccuracy / 65536,
		0);

	auto muzzleOffset = GetJointPosition(&laraItem, LM_RHAND);
	auto pos = Vector3i(laraItem.Pose.Position.x, muzzleOffset.y, laraItem.Pose.Position.z);

	// Calculate ray from wobbled orientation.
	auto directionNorm = wobbledArmOrient.ToDirection();
	auto origin = pos.ToVector3();
	auto target = origin + (directionNorm * weapon.TargetDist);
	auto ray = Ray(origin, directionNorm);

	int num = GetSpheres(&targetEntity, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
	int bestJointIndex = NO_JOINT;
	float bestDistance = INFINITY;
	for (int i = 0; i < num; i++)
	{
		auto sphere = BoundingSphere(Vector3(CreatureSpheres[i].x, CreatureSpheres[i].y, CreatureSpheres[i].z), CreatureSpheres[i].r);
		float distance = 0.0f;
		if (ray.Intersects(sphere, distance))
		{
			if (distance < bestDistance)
			{
				bestDistance = distance;
				bestJointIndex = i;
			}
		}
	}

	lara.Control.Weapon.HasFired = true;
	lara.Control.Weapon.Fired = true;
	
	auto vOrigin = GameVector(pos);
	short roomNumber = laraItem.RoomNumber;
	GetFloor(pos.x, pos.y, pos.z, &roomNumber);
	vOrigin.RoomNumber = roomNumber;

	if (bestJointIndex < 0)
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
		
		// NOTE: It seems that entities hit by the player in the normal way must have GetTargetOnLOS return false.
		// It's strange, but this replicates original behaviour until we fully understand what is happening.
		if (!GetTargetOnLOS(&vOrigin, &vTarget, false, true))
			HitTarget(&laraItem, &targetEntity, &vTarget, weapon.Damage, false, bestJointIndex);

		return FireWeaponType::PossibleHit;
	}
}

void FindNewTarget(ItemInfo& laraItem, const WeaponInfo& weaponInfo)
{
	if (!g_Configuration.AutoTarget)
		return;

	auto& lara = *GetLaraInfo(&laraItem);

	if (BinocularRange)
	{
		lara.TargetEntity = nullptr;
		return;
	}

	auto origin = GameVector(
		laraItem.Pose.Position.x,
		GetJointPosition(&laraItem, LM_RHAND).y, // Muzzle offset.
		laraItem.Pose.Position.z,
		laraItem.RoomNumber);

	ItemInfo* bestEntity = nullptr;
	float bestDistance = INFINITY;
	short bestYOrient = MAXSHORT;
	unsigned int numTargets = 0;
	float maxDistance = weaponInfo.TargetDist;

	for (auto* creaturePtr : ActiveCreatures)
	{
		// Continue loop if no item.
		if (creaturePtr->ItemNumber == NO_ITEM)
			continue;

		auto& item = g_Level.Items[creaturePtr->ItemNumber];

		// Check whether creature is alive.
		if (item.HitPoints <= 0)
			continue;

		// Check distance.
		float distance = Vector3::Distance(origin.ToVector3(), item.Pose.Position.ToVector3());
		if (distance > maxDistance)
			continue;

		// Assess line of sight.
		auto target = GetTargetPoint(item);
		if (!LOS(&origin, &target))
			continue;

		// Assess whether relative orientation falls within weapon's lock constraints.
		auto orient = Geometry::GetOrientToPoint(origin.ToVector3(), target.ToVector3()) - (laraItem.Pose.Orientation + lara.ExtraTorsoRot);
		if (orient.x >= weaponInfo.LockOrientConstraint.first.x &&
			orient.y >= weaponInfo.LockOrientConstraint.first.y &&
			orient.x <= weaponInfo.LockOrientConstraint.second.x &&
			orient.y <= weaponInfo.LockOrientConstraint.second.y)
		{
			TargetList[numTargets] = &item;
			++numTargets;

			if (distance < bestDistance &&
				abs(orient.y) < (bestYOrient + ANGLE(15.0f)))
			{
				bestEntity = &item;
				bestDistance = distance;
				bestYOrient = abs(orient.y);
			}
		}
	}

	TargetList[numTargets] = nullptr;
	if (!TargetList[0])
	{
		lara.TargetEntity = nullptr;
	}
	else
	{
		for (int slot = 0; slot < MAX_TARGETS; ++slot)
		{
			if (!TargetList[slot])
				lara.TargetEntity = nullptr;

			if (TargetList[slot] == lara.TargetEntity)
				break;
		}

		if (lara.Control.HandStatus != HandStatus::Free || IsClicked(In::SwitchTarget))
		{
			if (lara.TargetEntity == nullptr)
			{
				lara.TargetEntity = bestEntity;
				LastTargets[0] = nullptr;
			}
			else if (IsClicked(In::SwitchTarget))
			{
				lara.TargetEntity = nullptr;
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
						lara.TargetEntity = TargetList[match];
						if (lara.TargetEntity)
							flag = false;

						break;
					}
				}

				if (flag)
				{
					lara.TargetEntity = bestEntity;
					LastTargets[0] = nullptr;
				}
			}
		}
	}

	if (lara.TargetEntity != LastTargets[0])
	{
		for (int slot = 7; slot > 0; --slot)
			LastTargets[slot] = LastTargets[slot - 1];
		
		LastTargets[0] = lara.TargetEntity;
	}

	LaraTargetInfo(laraItem, weaponInfo);
}

void LaraTargetInfo(ItemInfo& laraItem, const WeaponInfo& weaponInfo)
{
	auto& lara = *GetLaraInfo(&laraItem);

	if (lara.TargetEntity == nullptr)
	{
		lara.RightArm.Locked = false;
		lara.LeftArm.Locked = false;
		lara.TargetArmOrient = EulerAngles::Zero;
		return;
	}

	auto origin = GameVector(
		laraItem.Pose.Position.x,
		GetJointPosition(&laraItem, LM_RHAND).y, // Muzzle offset.
		laraItem.Pose.Position.z,
		laraItem.RoomNumber);
	auto target = GetTargetPoint(*lara.TargetEntity);

	auto orient = Geometry::GetOrientToPoint(origin.ToVector3(), target.ToVector3()) - laraItem.Pose.Orientation;

	if (LOS(&origin, &target))
	{
		if (orient.x >= weaponInfo.LockOrientConstraint.first.x &&
			orient.y >= weaponInfo.LockOrientConstraint.first.y &&
			orient.x <= weaponInfo.LockOrientConstraint.second.x &&
			orient.y <= weaponInfo.LockOrientConstraint.second.y)
		{
			lara.RightArm.Locked = true;
			lara.LeftArm.Locked = true;
		}
		else
		{
			if (lara.LeftArm.Locked)
			{
				if (orient.x < weaponInfo.LeftOrientConstraint.first.x ||
					orient.y < weaponInfo.LeftOrientConstraint.first.y ||
					orient.x > weaponInfo.LeftOrientConstraint.second.x ||
					orient.y > weaponInfo.LeftOrientConstraint.second.y)
				{
					lara.LeftArm.Locked = false;
				}
			}

			if (lara.RightArm.Locked)
			{
				if (orient.x < weaponInfo.RightOrientConstraint.first.x ||
					orient.y < weaponInfo.RightOrientConstraint.first.y ||
					orient.x > weaponInfo.RightOrientConstraint.second.x ||
					orient.y > weaponInfo.RightOrientConstraint.second.y)
				{
					lara.RightArm.Locked = false;
				}
			}
		}
	}
	else
	{
		lara.RightArm.Locked = false;
		lara.LeftArm.Locked = false;
	}

	lara.TargetArmOrient = orient;
}

void HitTarget(ItemInfo* laraItem, ItemInfo* targetEntity, GameVector* hitPos, int damage, bool isExplosive, int bestJointIndex)
{
	const auto& object = Objects[targetEntity->ObjectNumber];

	targetEntity->HitStatus = true;
	if (targetEntity->IsCreature())
		GetCreatureInfo(targetEntity)->HurtByLara = true;

	if (hitPos != nullptr)
	{
		hitPos->RoomNumber = targetEntity->RoomNumber;
		object.HitRoutine(*targetEntity, *laraItem, *hitPos, damage, isExplosive, bestJointIndex);
	}
	else
	{
		object.HitRoutine(*targetEntity, *laraItem, std::nullopt, damage, isExplosive, bestJointIndex);
	}
}

void SmashItem(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];

	if (item.ObjectNumber >= ID_SMASH_OBJECT1 && item.ObjectNumber <= ID_SMASH_OBJECT8)
		SmashObject(itemNumber);
}
