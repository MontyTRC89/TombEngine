#include "framework.h"
#include "Game/Lara/lara_initialise.h"

#include "Game/health.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_tests.h"
#include "Specific/level.h"
#include "Specific/setup.h"

void InitialiseLara(int restore)
{
	if (Lara.ItemNumber == NO_ITEM)
		return;

	short itemNumber = Lara.ItemNumber;

	LaraItem->Data = &Lara;
	LaraItem->Collidable = false;
	LaraItem->Location.roomNumber = LaraItem->RoomNumber;
	LaraItem->Location.yNumber = LaraItem->Position.yPos;

	if (restore)
	{
		LaraInfo backup;
		memcpy(&backup, &Lara, sizeof(LaraInfo));
		ZeroMemory(&Lara, sizeof(LaraInfo));
	}
	else
	{
		ZeroMemory(&Lara, sizeof(LaraInfo));
		Lara.ExtraAnim = NO_ITEM;
		Lara.Vehicle = NO_ITEM;
	}

	Lara.Control.CanLook = true;
	Lara.ItemNumber = itemNumber;
	Lara.hitDirection = -1;
	Lara.SprintEnergy = LARA_SPRINT_MAX;
	Lara.Air = LARA_AIR_MAX;
	Lara.Control.WeaponControl.WeaponItem = NO_ITEM;
	PoisonFlag = 0;
	Lara.poisoned = 0;
	Lara.WaterSurfaceDist = 100;

	if (Lara.Weapons[static_cast<int>(LARA_WEAPON_TYPE::WEAPON_PISTOLS)].Present)
	{
		Lara.Control.WeaponControl.HolsterInfo.LeftHolster = HolsterSlot::Pistols;
		Lara.Control.WeaponControl.HolsterInfo.RightHolster = HolsterSlot::Pistols;
	}
	else
	{
		Lara.Control.WeaponControl.HolsterInfo.LeftHolster = HolsterSlot::Empty;
		Lara.Control.WeaponControl.HolsterInfo.RightHolster = HolsterSlot::Empty;
	}
	if (Lara.Weapons[static_cast<int>(LARA_WEAPON_TYPE::WEAPON_SHOTGUN)].Present)
	{
		Lara.Control.WeaponControl.HolsterInfo.BackHolster = HolsterSlot::Shotgun;
	}
	else
	{
		Lara.Control.WeaponControl.HolsterInfo.BackHolster = HolsterSlot::Empty;
	}

	Lara.location = -1;
	Lara.highestLocation = -1;
	Lara.Control.RopeControl.Ptr = -1;
	LaraItem->HitPoints = LARA_HEALTH_MAX;
	Lara.Control.HandStatus = HandStatus::Free;

	LARA_WEAPON_TYPE gun = WEAPON_NONE;

	if (Objects[ID_HK_ITEM].loaded)
		gun = WEAPON_HK;

	if (Objects[ID_PISTOLS_ITEM].loaded)
		gun = WEAPON_PISTOLS;

	Lara.Control.WeaponControl.LastGunType = Lara.Control.WeaponControl.GunType = Lara.Control.WeaponControl.RequestGunType = gun;

	LaraInitialiseMeshes();

	if (gun == WEAPON_PISTOLS)
	{
		Lara.Weapons[WEAPON_PISTOLS].Present = true;
		Lara.Weapons[WEAPON_PISTOLS].Ammo[WEAPON_AMMO1].setInfinite(true);
	}
	else if (gun == WEAPON_HK)
	{
		Lara.Weapons[WEAPON_HK].Present = true;
		Lara.Weapons[WEAPON_HK].Ammo[WEAPON_AMMO1] = 100;
	}

	Lara.Binoculars = true;

	if (!restore)
	{
		if (Objects[ID_FLARE_INV_ITEM].loaded)
			Lara.NumFlares = 3;

		Lara.NumSmallMedipacks = 3;
		Lara.NumLargeMedipacks = 1;
	}

	InitialiseLaraAnims(LaraItem);
	Lara.BeetleLife = 3;
}

void LaraInitialiseMeshes()
{
	for (int i = 0; i < NUM_LARA_MESHES; i++)
	{
		//Meshes[i] = Meshes[MESHES(ID_LARA_SKIN, i)];
		//LARA_MESHES(ID_LARA, MESHES(ID_LARA_SKIN, i));
		Lara.meshPtrs[i] = Objects[ID_LARA_SKIN].meshIndex + i;
	}

	/* Hardcoded code */

	if (Lara.Control.WeaponControl.GunType == WEAPON_HK)
		Lara.Control.WeaponControl.HolsterInfo.BackHolster = HolsterSlot::HK;
	else if (!Lara.Weapons[WEAPON_SHOTGUN].Present)
	{
		if (Lara.Weapons[WEAPON_HK].Present)
			Lara.Control.WeaponControl.HolsterInfo.BackHolster = HolsterSlot::HK;
	}
	else
		Lara.Control.WeaponControl.HolsterInfo.BackHolster = HolsterSlot::Empty;

	Lara.Control.HandStatus = HandStatus::Free;
	Lara.LeftArm.FrameNumber = 0;
	Lara.RightArm.FrameNumber = 0;
	Lara.target = NULL;
	Lara.RightArm.Locked = false;
	Lara.LeftArm.Locked = false;
}

void InitialiseLaraAnims(ITEM_INFO* item)
{
	if (TestEnvironment(ENV_FLAG_WATER, item))
	{
		Lara.Control.WaterStatus = WaterStatus::Underwater;
		item->VerticalVelocity = 0;
		SetAnimation(item, LA_UNDERWATER_IDLE);
	}
	else
	{
		Lara.Control.WaterStatus = WaterStatus::Dry;
		SetAnimation(item, LA_STAND_SOLID);
	}
}

void InitialiseLaraLoad(short itemNum)
{
	Lara.ItemNumber = itemNum;
	LaraItem = &g_Level.Items[itemNum];
}
