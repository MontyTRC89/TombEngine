#include "pickup.h"
#include "lara.h"
#include "draw.h"

#include "..\Global\global.h"

extern LaraExtraInfo g_LaraExtra;

__int32 __cdecl DrawAllPickups()
{
	if (Pickups[CurrentPickup].life > 0)
	{
		if (PickupX > 0)
		{
			PickupX += -PickupX >> 5;
			return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);
		}
		else
		{
			Pickups[CurrentPickup].life--;
			return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);
		}
	} 
	else if (Pickups[CurrentPickup].life == 0)
	{
		if (PickupX < 128)
		{
			if (PickupVel < 16)
				PickupVel++;
			PickupX += PickupVel >> 2;
			return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);
		}
		else
		{
			Pickups[CurrentPickup].life = -1;
			PickupVel = 0;
			return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);
		}
	}

	__int32 pickupIndex = CurrentPickup;
	__int32 i = 0;
	do
	{
		if (Pickups[pickupIndex].life > 0)
			break;
		pickupIndex = (pickupIndex + 1) & 7;
		i++;
	} while (i < 8);

	CurrentPickup = pickupIndex;
	if (i != 8)
		return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);

	CurrentPickup = 0;

	return 0;
}

void __cdecl PickedUpObject(__int16 objectNumber)
{
	switch (objectNumber)
	{
		case ID_UZI_ITEM:
			if (!(g_LaraExtra.Weapons[WEAPON_UZI].Present))
			{
				g_LaraExtra.Weapons[WEAPON_UZI].Present = true;
				g_LaraExtra.Weapons[WEAPON_UZI].SelectedAmmo = 0;
			}

		if (g_LaraExtra.Weapons[WEAPON_UZI].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_UZI].Ammo[0] += 30;

		break;

	case ID_PISTOLS_ITEM:
		if (!(g_LaraExtra.Weapons[WEAPON_PISTOLS].Present))
		{
			g_LaraExtra.Weapons[WEAPON_PISTOLS].Present = true;
			g_LaraExtra.Weapons[WEAPON_PISTOLS].SelectedAmmo = 0;
		}

		g_LaraExtra.Weapons[WEAPON_PISTOLS].Ammo[0] = -1;

		break;

	case ID_SHOTGUN_ITEM:
		if (!(g_LaraExtra.Weapons[WEAPON_SHOTGUN].Present))
		{
			g_LaraExtra.Weapons[WEAPON_SHOTGUN].Present = true;
			g_LaraExtra.Weapons[WEAPON_SHOTGUN].SelectedAmmo = 0;
		}

		if (g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[0] += 36;

		break;

	case ID_REVOLVER_ITEM:
		if (!(g_LaraExtra.Weapons[WEAPON_REVOLVER].Present))
		{
			g_LaraExtra.Weapons[WEAPON_REVOLVER].Present = true;
			g_LaraExtra.Weapons[WEAPON_REVOLVER].SelectedAmmo = 0;
		}

		if (g_LaraExtra.Weapons[WEAPON_REVOLVER].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_REVOLVER].Ammo[0] += 6;

		break;

	case ID_CROSSBOW_ITEM:
		if (!(g_LaraExtra.Weapons[WEAPON_CROSSBOW].Present))
		{
			g_LaraExtra.Weapons[WEAPON_CROSSBOW].Present = true;
			g_LaraExtra.Weapons[WEAPON_CROSSBOW].SelectedAmmo = 0;
		}

		if (g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[0] += 10;

			break;

	case ID_HK_ITEM:
		if (!(g_LaraExtra.Weapons[WEAPON_CROSSBOW].Present))
		{
			g_LaraExtra.Weapons[WEAPON_HK].Present = true;
			g_LaraExtra.Weapons[WEAPON_HK].SelectedAmmo = 0;
		}

		if (g_LaraExtra.Weapons[WEAPON_HK].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_HK].Ammo[0] += 30;

		break;

	case ID_HARPOON_ITEM:
		if (!(g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].Present))
		{
			g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].Present = true;
			g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].SelectedAmmo = 0;
		}

		if (g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].Ammo[0] += 10;

		break;

	case ID_GRENADE_ITEM:
		if (!(g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Present))
		{
			g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Present = true;
			g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].SelectedAmmo = 0;
		}

		if (g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[0] += 10;

		break;

	case ID_ROCKET_LAUNCHER_ITEM:
		if (!(g_LaraExtra.Weapons[WEAPON_ROCKET_LAUNCHER].Present))
		{
			g_LaraExtra.Weapons[WEAPON_ROCKET_LAUNCHER].Present = true;
			g_LaraExtra.Weapons[WEAPON_ROCKET_LAUNCHER].SelectedAmmo = 0;
		}

		if (g_LaraExtra.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[0] += 10;

		break;

	case ID_SHOTGUN_AMMO1_ITEM:
		if (g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[0] += 36;

		break;

	case ID_SHOTGUN_AMMO2_ITEM:
		if (g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[1] != -1)
			g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[1] += 36;

		break;

	case ID_HK_AMMO_ITEM:
		if (g_LaraExtra.Weapons[WEAPON_HK].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_HK].Ammo[0] += 30;

		break;

	case ID_CROSSBOW_AMMO1_ITEM:
		if (g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[0] += 10;

		break;

	case ID_CROSSBOW_AMMO2_ITEM:
		if (g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[1] != -1)
			g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[1] += 10;

		break;

	case ID_CROSSBOW_AMMO3_ITEM:
		if (g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[2] != -1)
			g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[2] += 10;

		break;

	case ID_GRENADE_AMMO1_ITEM:
		if (g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[0] += 10;

		break;

	case ID_GRENADE_AMMO2_ITEM:
		if (g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[1] != -1)
			g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[1] += 10;

		break;

	case ID_GRENADE_AMMO3_ITEM:
		if (g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[2] != -1)
			g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[2] += 10;

		break;

	case ID_REVOLVER_AMMO_ITEM:
		if (g_LaraExtra.Weapons[WEAPON_REVOLVER].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_REVOLVER].Ammo[0] += 6;

		break;

	case ID_ROCKET_LAUNCHER_AMMO_ITEM:
		if (g_LaraExtra.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[0] += 10;

		break;

	case ID_HARPOON_AMMO_ITEM:
		if (g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].Ammo[0] += 10;

		break;

	case ID_UZI_AMMO_ITEM:
		if (g_LaraExtra.Weapons[WEAPON_UZI].Ammo[0] != -1)
			g_LaraExtra.Weapons[WEAPON_UZI].Ammo[0] += 30;

		break;

	case ID_FLARE_INV_ITEM:
		if (Lara.numFlares != -1)
			Lara.numFlares += 12;
		break;

	case ID_SILENCER_ITEM:
		if (!(g_LaraExtra.Weapons[WEAPON_UZI].HasSilencer || g_LaraExtra.Weapons[WEAPON_PISTOLS].HasSilencer || 
			  g_LaraExtra.Weapons[WEAPON_SHOTGUN].HasSilencer || g_LaraExtra.Weapons[WEAPON_REVOLVER].HasSilencer || 
			  g_LaraExtra.Weapons[WEAPON_CROSSBOW].HasSilencer || g_LaraExtra.Weapons[WEAPON_HK].HasSilencer))
			Lara.silencer = true;
		break;

	case ID_LASERSIGHT_ITEM:
		if (!(g_LaraExtra.Weapons[WEAPON_UZI].HasSilencer || g_LaraExtra.Weapons[WEAPON_PISTOLS].HasSilencer ||
			  g_LaraExtra.Weapons[WEAPON_SHOTGUN].HasSilencer || g_LaraExtra.Weapons[WEAPON_REVOLVER].HasSilencer ||
			  g_LaraExtra.Weapons[WEAPON_CROSSBOW].HasSilencer || g_LaraExtra.Weapons[WEAPON_HK].HasSilencer))
			Lara.laserSight = true;
		break;

	case ID_BIGMEDI_ITEM:
		if (Lara.numLargeMedipack != -1)
			Lara.numLargeMedipack++;
		break;

	case ID_SMALLMEDI_ITEM:
		if (Lara.numSmallMedipack != -1)
			Lara.numSmallMedipack++;
		break;

	case ID_BINOCULARS_ITEM:
		Lara.binoculars = 1;
		break;

	default:
		if (objectNumber >= ID_PUZZLE_ITEM1 && objectNumber <= ID_PUZZLE_ITEM8)
			Lara.puzzleItems[objectNumber - ID_PUZZLE_ITEM1]++;

		else if (objectNumber >= ID_PUZZLE_ITEM1_COMBO1 && objectNumber <= ID_PUZZLE_ITEM8_COMBO2)
			Lara.puzzleItemsCombo |= 1 << (objectNumber - ID_PUZZLE_ITEM1_COMBO1);

		else if (objectNumber >= ID_KEY_ITEM1 && objectNumber <= ID_KEY_ITEM8)
			Lara.keyItems |= 1 << (objectNumber - ID_KEY_ITEM1);

		else if (objectNumber >= ID_KEY_ITEM1_COMBO1 && objectNumber <= ID_KEY_ITEM8_COMBO2)
			Lara.keyItemsCombo |= 1 << (objectNumber - ID_KEY_ITEM1_COMBO1);

		else if (objectNumber >= ID_PICKUP_ITEM1 && objectNumber <= ID_PICKUP_ITEM4)
			Lara.pickupItems |= 1 << (objectNumber - ID_PICKUP_ITEM1);

		else if (objectNumber >= ID_PICKUP_ITEM1_COMBO1 && objectNumber <= ID_PICKUP_ITEM4_COMBO2)
			Lara.pickupItemsCombo |= 1 << (objectNumber - ID_PICKUP_ITEM1_COMBO1);

		else if (objectNumber == ID_GOLDROSE_ITEM)
		{
			IsAtmospherePlaying = 0;
			S_CDPlay(6, 0);
			Lara.pickupItems |= 8;
			Savegame.Level.Secrets++;
			Savegame.Game.Secrets++;
		}

		else if (objectNumber == ID_CROWBAR_ITEM)
			Lara.crowbar = true;

		else if (objectNumber == ID_EXAMINE1)
			Lara.examine1 = true;

		else if (objectNumber == ID_EXAMINE2)
			Lara.examine2 = true;

		else if (objectNumber == ID_EXAMINE3)
			Lara.examine3 = true;

		else if (objectNumber == ID_DIARY)
			g_LaraExtra.Diary.Present = true;

		break;
	}
}

void Inject_Pickup()
{
	INJECT(0x0043A130, DrawAllPickups);
	INJECT(0x00463B60, PickedUpObject);
}