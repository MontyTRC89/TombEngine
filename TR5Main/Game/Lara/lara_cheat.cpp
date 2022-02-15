#include "framework.h"
#include "Game/Lara/lara_cheat.h"

#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Scripting/GameFlowScript.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

int NoCheatCounter;

void lara_as_swimcheat(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	if (TrInput & IN_FORWARD)
	{
		item->Position.xRot -= ANGLE(3);
	}
	else if (TrInput & IN_BACK)
	{
		item->Position.xRot += ANGLE(3);
	}

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= 613;

		if (info->Control.TurnRate < -ANGLE(6))
			info->Control.TurnRate = -ANGLE(6);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += 613;

		if (info->Control.TurnRate > ANGLE(6))
			info->Control.TurnRate = ANGLE(6);
	}

	if (TrInput & IN_ACTION)
	{
		TriggerDynamicLight(item->Position.xPos, item->Position.yPos, item->Position.zPos, 31, 255, 255, 255);
	}

	if (TrInput & IN_OPTION)
	{
		info->Control.TurnRate = -ANGLE(12);
	}

	if (TrInput & IN_JUMP)
	{
		item->VerticalVelocity += 16;

		if (item->VerticalVelocity > 400)
			item->VerticalVelocity = 400;
	}
	else
	{
		if (item->VerticalVelocity >= 8)
			item->VerticalVelocity -= item->VerticalVelocity >> 3;
		else
			item->VerticalVelocity = 0;
	}
}

void LaraCheatyBits(ITEM_INFO* item)
{
	auto* info = GetLaraInfo(item);

	if (g_GameFlow->FlyCheat)
	{
		if (KeyMap[DIK_O])
		{
			if (info->Vehicle == NO_ITEM)
			{

				LaraCheatGetStuff(item);

				DelsGiveLaraItemsCheat(item);

				item->Position.yPos -= 128;

				if (info->Control.WaterStatus != WaterStatus::FlyCheat)
				{
					info->Control.WaterStatus = WaterStatus::FlyCheat;
					SetAnimation(item, LA_DOZY);
					item->Airborne = false;
					item->Position.xRot = ANGLE(30);
					item->VerticalVelocity = 30;
					item->HitPoints = 1000;
					info->poisoned = 0;
					info->Air = 1800;
					info->Control.Count.Death = 0;
					ResetLaraFlex(item);
				}
			}
			else if (!NoCheatCounter)
			{
				SayNo();
				NoCheatCounter = 15;
			}
		}
	}
	if (NoCheatCounter)
		NoCheatCounter--;
}

void LaraCheatGetStuff(ITEM_INFO* item)
{
	auto* info = GetLaraInfo(item);

	info->NumFlares = -1;
	info->NumSmallMedipacks = -1;
	info->NumLargeMedipacks = -1;

	if (Objects[ID_CROWBAR_ITEM].loaded)
		info->Crowbar = true;

	if (Objects[ID_LASERSIGHT_ITEM].loaded)
		info->Lasersight = true;

	if (Objects[ID_CLOCKWORK_BEETLE].loaded)
		info->hasBeetleThings |= 1;

	if (Objects[ID_WATERSKIN1_EMPTY].loaded)
		info->smallWaterskin = 1;

	if (Objects[ID_WATERSKIN2_EMPTY].loaded)
		info->bigWaterskin = 1;

	if (Objects[ID_REVOLVER_ITEM].loaded)
	{
		info->Weapons[WEAPON_REVOLVER].Present = true;
		info->Weapons[WEAPON_REVOLVER].SelectedAmmo = WEAPON_AMMO1;
		info->Weapons[WEAPON_REVOLVER].HasLasersight = false;
		info->Weapons[WEAPON_REVOLVER].HasSilencer = false;
		info->Weapons[WEAPON_REVOLVER].Ammo[WEAPON_AMMO1].setInfinite(true);
	}

	if (Objects[ID_UZI_ITEM].loaded)
	{
		info->Weapons[WEAPON_UZI].Present = true;
		info->Weapons[WEAPON_UZI].SelectedAmmo = WEAPON_AMMO1;
		info->Weapons[WEAPON_UZI].HasLasersight = false;
		info->Weapons[WEAPON_UZI].HasSilencer = false;
		info->Weapons[WEAPON_UZI].Ammo[WEAPON_AMMO1].setInfinite(true);
	}

	if (Objects[ID_SHOTGUN_ITEM].loaded)
	{
		info->Weapons[WEAPON_SHOTGUN].Present = true;
		info->Weapons[WEAPON_SHOTGUN].SelectedAmmo = WEAPON_AMMO1;
		info->Weapons[WEAPON_SHOTGUN].HasLasersight = false;
		info->Weapons[WEAPON_SHOTGUN].HasSilencer = false;
		info->Weapons[WEAPON_SHOTGUN].Ammo[WEAPON_AMMO1].setInfinite(true);
	}

	if (Objects[ID_HARPOON_ITEM].loaded)
	{			
		info->Weapons[WEAPON_HARPOON_GUN].Present = true;
		info->Weapons[WEAPON_HARPOON_GUN].SelectedAmmo = WEAPON_AMMO1;
		info->Weapons[WEAPON_HARPOON_GUN].HasLasersight = false;
		info->Weapons[WEAPON_HARPOON_GUN].HasSilencer = false;
		info->Weapons[WEAPON_HARPOON_GUN].Ammo[WEAPON_AMMO1].setInfinite(true);
	}

	if (Objects[ID_GRENADE_GUN_ITEM].loaded)
	{
		info->Weapons[WEAPON_GRENADE_LAUNCHER].Present = true;
		info->Weapons[WEAPON_GRENADE_LAUNCHER].SelectedAmmo = WEAPON_AMMO1;
		info->Weapons[WEAPON_GRENADE_LAUNCHER].HasSilencer = false;
		info->Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO1].setInfinite(true);
		info->Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO2].setInfinite(true);
		info->Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO3].setInfinite(true);
	}

	if (Objects[ID_ROCKET_LAUNCHER_ITEM].loaded)
	{
		info->Weapons[WEAPON_ROCKET_LAUNCHER].Present = true;
		info->Weapons[WEAPON_ROCKET_LAUNCHER].SelectedAmmo = WEAPON_AMMO1;
		info->Weapons[WEAPON_ROCKET_LAUNCHER].HasLasersight = false;
		info->Weapons[WEAPON_ROCKET_LAUNCHER].HasSilencer = false;
		info->Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[WEAPON_AMMO1].setInfinite(true);
	}

	if (Objects[ID_HK_ITEM].loaded)
	{
		info->Weapons[WEAPON_HK].Present = true;
		info->Weapons[WEAPON_HK].SelectedAmmo = WEAPON_AMMO1;
		info->Weapons[WEAPON_HK].HasLasersight = false;
		info->Weapons[WEAPON_HK].HasSilencer = false;
		info->Weapons[WEAPON_HK].Ammo[WEAPON_AMMO1].setInfinite(true);
	}

	if (Objects[ID_CROSSBOW_ITEM].loaded)
	{			
		info->Weapons[WEAPON_CROSSBOW].Present = true;
		info->Weapons[WEAPON_CROSSBOW].SelectedAmmo = WEAPON_AMMO1;
		info->Weapons[WEAPON_CROSSBOW].HasLasersight = false;
		info->Weapons[WEAPON_CROSSBOW].HasSilencer = false;
		info->Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO1].setInfinite(true);
		info->Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO2].setInfinite(true);
		info->Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO3].setInfinite(true);
	}
}

void DelsGiveLaraItemsCheat(ITEM_INFO* item)
{
	auto* info = GetLaraInfo(item);

	int i;

	for (i = 0; i < 8; ++i)
	{
		if (Objects[ID_PUZZLE_ITEM1 + i].loaded)
			info->Puzzles[i] = 1;
		info->PuzzlesCombo[2 * i] = false;
		info->PuzzlesCombo[2 * i + 1] = false;
	}
	for (i = 0; i < 8; ++i)
	{
		if (Objects[ID_KEY_ITEM1 + i].loaded)
			info->Keys[i] = 1;
		info->KeysCombo[2 * i] = false;
		info->KeysCombo[2 * i + 1] = false;
	}
	for (i = 0; i < 3; ++i)
	{
		if (Objects[ID_PICKUP_ITEM1 + i].loaded)
			info->Pickups[i] = 1;
		info->PickupsCombo[2 * i] = false;
		info->PickupsCombo[2 * i + 1] = false;
	}
	/* Hardcoded code */
}
