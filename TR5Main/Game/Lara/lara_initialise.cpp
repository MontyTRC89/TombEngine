#include "framework.h"
#include "lara.h"
#include "lara_initialise.h"
#include "health.h"
#include "items.h"
#include "setup.h"
#include "level.h"

void InitialiseLara(int restore)
{
	if (Lara.itemNumber == NO_ITEM)
		return;

	short itemNumber = Lara.itemNumber;

	LaraItem->data = &Lara;
	LaraItem->collidable = false;
	LaraItem->location.roomNumber = LaraItem->roomNumber;
	LaraItem->location.yNumber = LaraItem->pos.yPos;

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
	Lara.air = 1800;
	Lara.weaponItem = NO_ITEM;
	PoisonFlag = 0;
	Lara.poisoned = 0;
	Lara.waterSurfaceDist = 100;
	if (Lara.Weapons[static_cast<int>(LARA_WEAPON_TYPE::WEAPON_PISTOLS)].Present) {
		Lara.holsterInfo.leftHolster = HOLSTER_SLOT::Pistols;
		Lara.holsterInfo.rightHolster = HOLSTER_SLOT::Pistols;
	}
	else {
		Lara.holsterInfo.leftHolster = HOLSTER_SLOT::Empty;
		Lara.holsterInfo.rightHolster = HOLSTER_SLOT::Empty;
	}
	if (Lara.Weapons[static_cast<int>(LARA_WEAPON_TYPE::WEAPON_SHOTGUN)].Present) {
		Lara.holsterInfo.backHolster = HOLSTER_SLOT::Shotgun;
	}
	else {
		Lara.holsterInfo.backHolster = HOLSTER_SLOT::Empty;
	}


	Lara.location = -1;
	Lara.highestLocation = -1;
	Lara.ropePtr = -1;
	LaraItem->hitPoints = 1000;
	Lara.gunStatus = LG_NO_ARMS;
	memset(&Lara.NewAnims, 0, sizeof(AnimsNew));	//make sure script changes these AFTER Lara is initialized?

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
	DashTimer = 120;
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
	{
		Lara.holsterInfo.backHolster = HOLSTER_SLOT::HK;
	}
	else if (!Lara.Weapons[WEAPON_SHOTGUN].Present)
	{
		if (Lara.Weapons[WEAPON_HK].Present)
			Lara.holsterInfo.backHolster = HOLSTER_SLOT::HK;
	}
	else
	{
		Lara.holsterInfo.backHolster = HOLSTER_SLOT::Empty;
	}

	Lara.gunStatus = LG_NO_ARMS;
	Lara.leftArm.frameNumber = 0;
	Lara.rightArm.frameNumber = 0;
	Lara.target = NULL;
	Lara.rightArm.lock = false;
	Lara.leftArm.lock = false;
}

void InitialiseLaraAnims(ITEM_INFO* item)
{
	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		Lara.waterStatus = LW_UNDERWATER;
		item->goalAnimState = LS_UNDERWATER_STOP;
		item->currentAnimState = LS_UNDERWATER_STOP;
		item->fallspeed = 0;
		item->animNumber = LA_UNDERWATER_IDLE;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	}
	else
	{
		Lara.waterStatus = LW_ABOVE_WATER;
		item->goalAnimState = LS_STOP;
		item->currentAnimState = LS_STOP;
		item->animNumber = LA_STAND_SOLID;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	}
}

void InitialiseLaraLoad(short itemNum)
{
	Lara.itemNumber = itemNum;
	LaraItem = &g_Level.Items[itemNum];
}
