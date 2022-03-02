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

void lara_as_swimcheat(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_FORWARD)
		item->Position.xRot -= ANGLE(3.0f);
	else if (TrInput & IN_BACK)
		item->Position.xRot += ANGLE(3.0f);

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= ANGLE(3.4f);
		if (lara->Control.TurnRate < -ANGLE(6.0f))
			lara->Control.TurnRate = -ANGLE(6.0f);
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += ANGLE(3.4f);
		if (lara->Control.TurnRate > ANGLE(6.0f))
			lara->Control.TurnRate = ANGLE(6.0f);
	}

	if (TrInput & IN_ACTION)
		TriggerDynamicLight(item->Position.xPos, item->Position.yPos, item->Position.zPos, 31, 255, 255, 255);

	if (TrInput & IN_OPTION)
		lara->Control.TurnRate = -ANGLE(12.0f);

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
	auto* lara = GetLaraInfo(item);

	if (g_GameFlow->FlyCheat)
	{
		if (KeyMap[DIK_O])
		{
			if (lara->Vehicle == NO_ITEM)
			{
				LaraCheatGetStuff(item);
				DelsGiveLaraItemsCheat(item);

				item->Position.yPos -= CLICK(0.5f);

				if (lara->Control.WaterStatus != WaterStatus::FlyCheat)
				{
					SetAnimation(item, LA_DOZY);
					item->Position.xRot = ANGLE(30.0f);
					item->VerticalVelocity = 30;
					item->Airborne = false;
					item->HitPoints = 1000;

					ResetLaraFlex(item);
					lara->Control.WaterStatus = WaterStatus::FlyCheat;
					lara->Poisoned = 0;
					lara->Air = 1800;
					lara->Control.Count.Death = 0;
				}
			}
			else if (!lara->Control.Count.NoCheat)
			{
				lara->Control.Count.NoCheat = 15;
				SayNo();
			}
		}
	}

	if (lara->Control.Count.NoCheat)
		lara->Control.Count.NoCheat--;
}

void LaraCheatGetStuff(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	lara->NumFlares = -1;
	lara->NumSmallMedipacks = -1;
	lara->NumLargeMedipacks = -1;

	if (Objects[ID_CROWBAR_ITEM].loaded)
		lara->Crowbar = true;

	if (Objects[ID_LASERSIGHT_ITEM].loaded)
		lara->Lasersight = true;

	if (Objects[ID_CLOCKWORK_BEETLE].loaded)
		lara->hasBeetleThings |= 1;

	if (Objects[ID_WATERSKIN1_EMPTY].loaded)
		lara->smallWaterskin = 1;

	if (Objects[ID_WATERSKIN2_EMPTY].loaded)
		lara->bigWaterskin = 1;

	if (Objects[ID_REVOLVER_ITEM].loaded)
	{
		lara->Weapons[WEAPON_REVOLVER].Present = true;
		lara->Weapons[WEAPON_REVOLVER].SelectedAmmo = WEAPON_AMMO1;
		lara->Weapons[WEAPON_REVOLVER].HasLasersight = false;
		lara->Weapons[WEAPON_REVOLVER].HasSilencer = false;
		lara->Weapons[WEAPON_REVOLVER].Ammo[WEAPON_AMMO1].setInfinite(true);
	}

	if (Objects[ID_UZI_ITEM].loaded)
	{
		lara->Weapons[WEAPON_UZI].Present = true;
		lara->Weapons[WEAPON_UZI].SelectedAmmo = WEAPON_AMMO1;
		lara->Weapons[WEAPON_UZI].HasLasersight = false;
		lara->Weapons[WEAPON_UZI].HasSilencer = false;
		lara->Weapons[WEAPON_UZI].Ammo[WEAPON_AMMO1].setInfinite(true);
	}

	if (Objects[ID_SHOTGUN_ITEM].loaded)
	{
		lara->Weapons[WEAPON_SHOTGUN].Present = true;
		lara->Weapons[WEAPON_SHOTGUN].SelectedAmmo = WEAPON_AMMO1;
		lara->Weapons[WEAPON_SHOTGUN].HasLasersight = false;
		lara->Weapons[WEAPON_SHOTGUN].HasSilencer = false;
		lara->Weapons[WEAPON_SHOTGUN].Ammo[WEAPON_AMMO1].setInfinite(true);
	}

	if (Objects[ID_HARPOON_ITEM].loaded)
	{			
		lara->Weapons[WEAPON_HARPOON_GUN].Present = true;
		lara->Weapons[WEAPON_HARPOON_GUN].SelectedAmmo = WEAPON_AMMO1;
		lara->Weapons[WEAPON_HARPOON_GUN].HasLasersight = false;
		lara->Weapons[WEAPON_HARPOON_GUN].HasSilencer = false;
		lara->Weapons[WEAPON_HARPOON_GUN].Ammo[WEAPON_AMMO1].setInfinite(true);
	}

	if (Objects[ID_GRENADE_GUN_ITEM].loaded)
	{
		lara->Weapons[WEAPON_GRENADE_LAUNCHER].Present = true;
		lara->Weapons[WEAPON_GRENADE_LAUNCHER].SelectedAmmo = WEAPON_AMMO1;
		lara->Weapons[WEAPON_GRENADE_LAUNCHER].HasSilencer = false;
		lara->Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO1].setInfinite(true);
		lara->Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO2].setInfinite(true);
		lara->Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO3].setInfinite(true);
	}

	if (Objects[ID_ROCKET_LAUNCHER_ITEM].loaded)
	{
		lara->Weapons[WEAPON_ROCKET_LAUNCHER].Present = true;
		lara->Weapons[WEAPON_ROCKET_LAUNCHER].SelectedAmmo = WEAPON_AMMO1;
		lara->Weapons[WEAPON_ROCKET_LAUNCHER].HasLasersight = false;
		lara->Weapons[WEAPON_ROCKET_LAUNCHER].HasSilencer = false;
		lara->Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[WEAPON_AMMO1].setInfinite(true);
	}

	if (Objects[ID_HK_ITEM].loaded)
	{
		lara->Weapons[WEAPON_HK].Present = true;
		lara->Weapons[WEAPON_HK].SelectedAmmo = WEAPON_AMMO1;
		lara->Weapons[WEAPON_HK].HasLasersight = false;
		lara->Weapons[WEAPON_HK].HasSilencer = false;
		lara->Weapons[WEAPON_HK].Ammo[WEAPON_AMMO1].setInfinite(true);
	}

	if (Objects[ID_CROSSBOW_ITEM].loaded)
	{			
		lara->Weapons[WEAPON_CROSSBOW].Present = true;
		lara->Weapons[WEAPON_CROSSBOW].SelectedAmmo = WEAPON_AMMO1;
		lara->Weapons[WEAPON_CROSSBOW].HasLasersight = false;
		lara->Weapons[WEAPON_CROSSBOW].HasSilencer = false;
		lara->Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO1].setInfinite(true);
		lara->Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO2].setInfinite(true);
		lara->Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO3].setInfinite(true);
	}
}

void DelsGiveLaraItemsCheat(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	for (int i = 0; i < 8; ++i)
	{
		if (Objects[ID_PUZZLE_ITEM1 + i].loaded)
			lara->Puzzles[i] = 1;

		lara->PuzzlesCombo[2 * i] = false;
		lara->PuzzlesCombo[2 * i + 1] = false;
	}

	for (int i = 0; i < 8; ++i)
	{
		if (Objects[ID_KEY_ITEM1 + i].loaded)
			lara->Keys[i] = 1;

		lara->KeysCombo[2 * i] = false;
		lara->KeysCombo[2 * i + 1] = false;
	}

	for (int i = 0; i < 3; ++i)
	{
		if (Objects[ID_PICKUP_ITEM1 + i].loaded)
			lara->Pickups[i] = 1;

		lara->PickupsCombo[2 * i] = false;
		lara->PickupsCombo[2 * i + 1] = false;
	}
	/* Hardcoded code */
}
