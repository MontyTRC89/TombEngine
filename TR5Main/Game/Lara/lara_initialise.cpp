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
	if (Lara.itemNumber == NO_ITEM)
		return;

	short itemNumber = Lara.itemNumber;

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

	Lara.look = true;
	Lara.itemNumber = itemNumber;
	Lara.hitDirection = -1;
	Lara.sprintTimer = LARA_SPRINT_MAX;
	Lara.air = LARA_AIR_MAX;
	Lara.weaponItem = NO_ITEM;
	PoisonFlag = 0;
	Lara.poisoned = 0;
	Lara.waterSurfaceDist = 100;

	if (Lara.Weapons[static_cast<int>(LARA_WEAPON_TYPE::WEAPON_PISTOLS)].Present)
	{
		Lara.holsterInfo.leftHolster = HOLSTER_SLOT::Pistols;
		Lara.holsterInfo.rightHolster = HOLSTER_SLOT::Pistols;
	}
	else
	{
		Lara.holsterInfo.leftHolster = HOLSTER_SLOT::Empty;
		Lara.holsterInfo.rightHolster = HOLSTER_SLOT::Empty;
	}
	if (Lara.Weapons[static_cast<int>(LARA_WEAPON_TYPE::WEAPON_SHOTGUN)].Present)
	{
		Lara.holsterInfo.backHolster = HOLSTER_SLOT::Shotgun;
	}
	else
	{
		Lara.holsterInfo.backHolster = HOLSTER_SLOT::Empty;
	}

	Lara.location = -1;
	Lara.highestLocation = -1;
	Lara.ropeParameters.Ptr = -1;
	LaraItem->HitPoints = LARA_HEALTH_MAX;
	Lara.gunStatus = LG_HANDS_FREE;

	LARA_WEAPON_TYPE gun = WEAPON_NONE;

	if (Objects[ID_HK_ITEM].loaded)
		gun = WEAPON_HK;

	if (Objects[ID_PISTOLS_ITEM].loaded)
		gun = WEAPON_PISTOLS;

	Lara.lastGunType = Lara.gunType = Lara.requestGunType = gun;

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

	if (Lara.gunType == WEAPON_HK)
		Lara.holsterInfo.backHolster = HOLSTER_SLOT::HK;
	else if (!Lara.Weapons[WEAPON_SHOTGUN].Present)
	{
		if (Lara.Weapons[WEAPON_HK].Present)
			Lara.holsterInfo.backHolster = HOLSTER_SLOT::HK;
	}
	else
		Lara.holsterInfo.backHolster = HOLSTER_SLOT::Empty;

	Lara.gunStatus = LG_HANDS_FREE;
	Lara.leftArm.frameNumber = 0;
	Lara.rightArm.frameNumber = 0;
	Lara.target = NULL;
	Lara.rightArm.lock = false;
	Lara.leftArm.lock = false;
}

void InitialiseLaraAnims(ITEM_INFO* item)
{
	if (TestEnvironment(ENV_FLAG_WATER, item))
	{
		Lara.waterStatus = LW_UNDERWATER;
		item->VerticalVelocity = 0;
		SetAnimation(item, LA_UNDERWATER_IDLE);
	}
	else
	{
		Lara.waterStatus = LW_ABOVE_WATER;
		SetAnimation(item, LA_STAND_SOLID);
	}
}

void InitialiseLaraLoad(short itemNum)
{
	Lara.itemNumber = itemNum;
	LaraItem = &g_Level.Items[itemNum];
}
