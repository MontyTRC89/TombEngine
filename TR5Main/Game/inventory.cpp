#include "inventory.h"
#include "draw.h"
#include "control.h"
#include "larafire.h"
#include "sound.h"
#include "gameflow.h"
#include "sound.h"
#include "savegame.h"
#include "lara.h"

#include "..\Global\global.h"
#include "..\Specific\input.h"
#include "..\Specific\config.h"
#include "lara1gun.h"
#include "lara2gun.h"

Inventory* g_Inventory;
extern GameFlow* g_GameFlow;
extern LaraExtraInfo g_LaraExtra;

void CombinePuzzle1(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.puzzleItems[0] = true;
		Lara.puzzleItemsCombo &= (~(3 << 0));
	}
	else
	{
		Lara.puzzleItems[0] = false;
		Lara.puzzleItemsCombo |= (3 << 0);
	}
}

void CombinePuzzle2(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.puzzleItems[1] = true;
		Lara.puzzleItemsCombo &= (~(3 << 2));
	}
	else
	{
		Lara.puzzleItems[1] = false;
		Lara.puzzleItemsCombo |= (3 << 2);
	}
}

void CombinePuzzle3(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.puzzleItems[2] = true;
		Lara.puzzleItemsCombo &= (~(3 << 4));
	}
	else
	{
		Lara.puzzleItems[2] = false;
		Lara.puzzleItemsCombo |= (3 << 4);
	}
}

void CombinePuzzle4(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.puzzleItems[3] = true;
		Lara.puzzleItemsCombo &= (~(3 << 6));
	}
	else
	{
		Lara.puzzleItems[3] = false;
		Lara.puzzleItemsCombo |= (3 << 6);
	}
}

void CombinePuzzle5(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.puzzleItems[4] = true;
		Lara.puzzleItemsCombo &= (~(3 << 8));
	}
	else
	{
		Lara.puzzleItems[4] = false;
		Lara.puzzleItemsCombo |= (3 << 8);
	}
}

void CombinePuzzle6(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.puzzleItems[5] = true;
		Lara.puzzleItemsCombo &= (~(3 << 10));
	}
	else
	{
		Lara.puzzleItems[5] = false;
		Lara.puzzleItemsCombo |= (3 << 10);
	}
}

void CombinePuzzle7(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.puzzleItems[6] = true;
		Lara.puzzleItemsCombo &= (~(3 << 12));
	}
	else
	{
		Lara.puzzleItems[6] = false;
		Lara.puzzleItemsCombo |= (3 << 12);
	}
}

void CombinePuzzle8(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.puzzleItems[7] = true;
		Lara.puzzleItemsCombo &= (~(3 << 14));
	}
	else
	{
		Lara.puzzleItems[7] = false;
		Lara.puzzleItemsCombo |= (3 << 14);
	}
}

void CombineKey1(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.keyItems |= (1<<0);
		Lara.puzzleItemsCombo &= (~(3 << 0));
	}
	else
	{
		Lara.keyItems &= ~(1 << 0);
		Lara.puzzleItemsCombo |= (3 << 0);
	}
}

void CombineKey2(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.keyItems |= (1 << 1);
		Lara.puzzleItemsCombo &= (~(3 << 2));
	}
	else
	{
		Lara.keyItems &= ~(1 << 1);
		Lara.puzzleItemsCombo |= (3 << 2);
	}
}

void CombineKey3(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.keyItems |= (1 << 2);
		Lara.puzzleItemsCombo &= (~(3 << 4));
	}
	else
	{
		Lara.keyItems &= ~(1 << 2);
		Lara.puzzleItemsCombo |= (3 << 4);
	}
}

void CombineKey4(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.keyItems |= (1 << 3);
		Lara.puzzleItemsCombo &= (~(3 << 6));
	}
	else
	{
		Lara.keyItems &= ~(1 << 3);
		Lara.puzzleItemsCombo |= (3 << 6);
	}
}

void CombineKey5(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.keyItems |= (1 << 4);
		Lara.puzzleItemsCombo &= (~(3 << 8));
	}
	else
	{
		Lara.keyItems &= ~(1 << 4);
		Lara.puzzleItemsCombo |= (3 << 8);
	}
}

void CombineKey6(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.keyItems |= (1 << 5);
		Lara.puzzleItemsCombo &= (~(3 << 10));
	}
	else
	{
		Lara.keyItems &= ~(1 << 5);
		Lara.puzzleItemsCombo |= (3 << 10);
	}
}

void CombineKey7(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.keyItems |= (1 << 6);
		Lara.puzzleItemsCombo &= (~(3 << 12));
	}
	else
	{
		Lara.keyItems &= ~(1 << 6);
		Lara.puzzleItemsCombo |= (3 << 12);
	}
}

void CombineKey8(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.keyItems |= (1 << 7);
		Lara.puzzleItemsCombo &= (~(3 << 14));
	}
	else
	{
		Lara.keyItems &= ~(1 << 7);
		Lara.puzzleItemsCombo |= (3 << 14);
	}
}

void CombinePickup1(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.pickupItems |= (1 << 0);
		Lara.pickupItemsCombo &= (~(3 << 0));
	}
	else
	{
		Lara.pickupItems &= ~(1 << 0);
		Lara.pickupItemsCombo |= (3 << 0);
	}
}

void CombinePickup2(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.pickupItems |= (1 << 1);
		Lara.pickupItemsCombo &= (~(3 << 2));
	}
	else
	{
		Lara.pickupItems &= ~(1 << 1);
		Lara.pickupItemsCombo |= (3 << 2);
	}
}

void CombinePickup3(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.pickupItems |= (1 << 2);
		Lara.pickupItemsCombo &= (~(3 << 4));
	}
	else
	{
		Lara.pickupItems &= ~(1 << 2);
		Lara.pickupItemsCombo |= (3 << 4);
	}
}

void CombinePickup4(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.pickupItems |= (1 << 3);
		Lara.pickupItemsCombo &= (~(3 << 6));
	}
	else
	{
		Lara.pickupItems &= ~(1 << 3);
		Lara.pickupItemsCombo |= (3 << 6);
	}
}

void CombineRevolverLasersight(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.laserSight = true;
		g_LaraExtra.Weapons[WEAPON_REVOLVER].HasLasersight = true;
	}
	else
	{
		Lara.laserSight = false;
		g_LaraExtra.Weapons[WEAPON_REVOLVER].HasLasersight = false;
	}

	if (Lara.gunStatus && Lara.gunType == WEAPON_REVOLVER)
	{
		UndrawPistolMeshRight(WEAPON_REVOLVER);
		DrawPistolMeshes(WEAPON_REVOLVER);
	}
}

void CombineCrossbowLasersight(__int32 action)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.laserSight = true;
		g_LaraExtra.Weapons[WEAPON_CROSSBOW].HasLasersight = true;
	}
	else
	{
		Lara.laserSight = false;
		g_LaraExtra.Weapons[WEAPON_CROSSBOW].HasLasersight = false;
	}

	if (Lara.gunStatus && Lara.gunType == WEAPON_CROSSBOW)
	{
		UndrawShotgunMeshes(WEAPON_CROSSBOW);
		DrawShotgunMeshes(WEAPON_CROSSBOW);
	}
}

void Inject_Inventory()
{

}

Inventory::Inventory()
{
	ZeroMemory(&m_objectsTable[0], sizeof(InventoryObjectDefinition) * INVENTORY_TABLE_SIZE);

	// Create objects table
	m_objectsTable[INV_OBJECT_UZIS] = InventoryObjectDefinition(ID_UZI_ITEM, STRING_INV_UZI, -1, 0);
	m_objectsTable[INV_OBJECT_PISTOLS] = InventoryObjectDefinition(ID_PISTOLS_ITEM, STRING_INV_PISTOLS, -1, 0);
	m_objectsTable[INV_OBJECT_SHOTGUN] = InventoryObjectDefinition(ID_SHOTGUN_ITEM, STRING_INV_SHOTGUN, -1, 0);
	m_objectsTable[INV_OBJECT_REVOLVER] = InventoryObjectDefinition(ID_REVOLVER_ITEM, STRING_INV_REVOLVER, -1, 0);
	m_objectsTable[INV_OBJECT_REVOLVER_LASER] = InventoryObjectDefinition(ID_REVOLVER_ITEM, STRING_INV_REVOLVER_LASER, -1, 0);
	m_objectsTable[INV_OBJECT_HK] = InventoryObjectDefinition(ID_HK_ITEM, STRING_INV_HK, -1, 0);
	m_objectsTable[INV_OBJECT_SHOTGUN_AMMO1] = InventoryObjectDefinition(ID_SHOTGUN_AMMO1_ITEM, STRING_INV_SHOTGUN_AMMO1, -1, 0);
	m_objectsTable[INV_OBJECT_SHOTGUN_AMMO2] = InventoryObjectDefinition(ID_SHOTGUN_AMMO2_ITEM, STRING_INV_SHOTGUN_AMMO2, -1, 0);
	m_objectsTable[INV_OBJECT_HK_AMMO1] = InventoryObjectDefinition(ID_HK_AMMO_ITEM, STRING_INV_HK_AMMO, -1, 0);
	m_objectsTable[INV_OBJECT_REVOLVER_AMMO] = InventoryObjectDefinition(ID_REVOLVER_AMMO_ITEM, STRING_INV_REVOLVER_AMMO, -1, 0);
	m_objectsTable[INV_OBJECT_UZI_AMMO] = InventoryObjectDefinition(ID_UZI_AMMO_ITEM, STRING_INV_UZI_AMMO, -1, 0);
	m_objectsTable[INV_OBJECT_PISTOLS_AMMO] = InventoryObjectDefinition(ID_PISTOLS_AMMO_ITEM, STRING_INV_PISTOLS_AMMO, -1, 0);
	m_objectsTable[INV_OBJECT_LASERSIGHT] = InventoryObjectDefinition(ID_LASERSIGHT_ITEM, STRING_INV_LASERSIGHT, -1, 0);
	m_objectsTable[INV_OBJECT_SILENCER] = InventoryObjectDefinition(ID_SILENCER_ITEM, STRING_INV_SILENCER, -1, 0);
	m_objectsTable[INV_OBJECT_LARGE_MEDIPACK] = InventoryObjectDefinition(ID_BIGMEDI_ITEM, STRING_INV_LARGE_MEDIPACK, -1, 0);
	m_objectsTable[INV_OBJECT_SMALL_MEDIPACK] = InventoryObjectDefinition(ID_SMALLMEDI_ITEM, STRING_INV_SMALL_MEDIPACK, -1, 0);
	m_objectsTable[INV_OBJECT_BINOCULARS] = InventoryObjectDefinition(ID_BINOCULARS_ITEM, STRING_INV_BINOCULARS, -1, 0);
	m_objectsTable[INV_OBJECT_FLARES] = InventoryObjectDefinition(ID_FLARE_INV_ITEM, STRING_INV_FLARES, -1, 0);
	m_objectsTable[INV_OBJECT_TIMEX] = InventoryObjectDefinition(ID_COMPASS_ITEM, STRING_INV_TIMEX, -1, 0);
	m_objectsTable[INV_OBJECT_CROWBAR] = InventoryObjectDefinition(ID_CROWBAR_ITEM, STRING_INV_CROWBAR, -1, 0);
	m_objectsTable[INV_OBJECT_GRENADE_LAUNCHER] = InventoryObjectDefinition(ID_GRENADE_ITEM, STRING_INV_GRENADE_LAUNCHER, -1, 0);
	m_objectsTable[INV_OBJECT_GRENADE_AMMO1] = InventoryObjectDefinition(ID_GRENADE_AMMO1_ITEM, STRING_INV_GRENADE_AMMO1, -1, 0);
	m_objectsTable[INV_OBJECT_GRENADE_AMMO2] = InventoryObjectDefinition(ID_GRENADE_AMMO2_ITEM, STRING_INV_GRENADE_AMMO2, -1, 0);
	m_objectsTable[INV_OBJECT_GRENADE_AMMO3] = InventoryObjectDefinition(ID_GRENADE_AMMO3_ITEM, STRING_INV_GRENADE_AMMO3, -1, 0);
	m_objectsTable[INV_OBJECT_HARPOON_GUN] = InventoryObjectDefinition(ID_HARPOON_ITEM, STRING_INV_HARPOON_GUN, -1, 0);
	m_objectsTable[INV_OBJECT_HARPOON_AMMO] = InventoryObjectDefinition(ID_HARPOON_AMMO_ITEM, STRING_INV_HARPOON_AMMO, -1, 0);
	m_objectsTable[INV_OBJECT_ROCKET_LAUNCHER] = InventoryObjectDefinition(ID_ROCKET_LAUNCHER_ITEM, STRING_INV_ROCKET_LAUNCHER, -1, 0);
	m_objectsTable[INV_OBJECT_ROCKET_AMMO] = InventoryObjectDefinition(ID_ROCKET_LAUNCHER_AMMO_ITEM, STRING_INV_ROCKET_AMMO, -1, 0);
	m_objectsTable[INV_OBJECT_CROSSBOW] = InventoryObjectDefinition(ID_CROSSBOW_ITEM, STRING_INV_CROSSBOW, -1, 0);
	m_objectsTable[INV_OBJECT_CROSSBOW_LASER] = InventoryObjectDefinition(ID_CROSSBOW_ITEM, STRING_INV_CROSSBOW_LASER, -1, 0);
	m_objectsTable[INV_OBJECT_CROSSBOW_AMMO1] = InventoryObjectDefinition(ID_CROSSBOW_AMMO1_ITEM, STRING_INV_CROSSBOW_AMMO1, -1, 0);
	m_objectsTable[INV_OBJECT_CROSSBOW_AMMO2] = InventoryObjectDefinition(ID_CROSSBOW_AMMO2_ITEM, STRING_INV_CROSSBOW_AMMO2, -1, 0);
	m_objectsTable[INV_OBJECT_CROSSBOW_AMMO3] = InventoryObjectDefinition(ID_CROSSBOW_AMMO3_ITEM, STRING_INV_CROSSBOW_AMMO3, -1, 0);
	m_objectsTable[INV_OBJECT_PASSAPORT] = InventoryObjectDefinition(ID_INVENTORY_PASSPORT, STRING_INV_PASSPORT, -1, 0);
	m_objectsTable[INV_OBJECT_KEYS] = InventoryObjectDefinition(ID_INVENTORY_KEYS, STRING_INV_CONTROLS, -1, 0);
	m_objectsTable[INV_OBJECT_SUNGLASSES] = InventoryObjectDefinition(ID_INVENTORY_SUNGLASSES, STRING_INV_DISPLAY, -1, 0);
	m_objectsTable[INV_OBJECT_POLAROID] = InventoryObjectDefinition(ID_INVENTORY_POLAROID, STRING_INV_LARA_HOME, -1, 0);
	m_objectsTable[INV_OBJECT_HEADPHONES] = InventoryObjectDefinition(ID_INVENTORY_HEADPHONES, STRING_INV_SOUND, -1, 0);
	m_objectsTable[INV_OBJECT_DIARY] = InventoryObjectDefinition(ID_DIARY, STRING_INV_DIARY, -1, 0);
	m_objectsTable[INV_OBJECT_WATERSKIN1] = InventoryObjectDefinition(ID_WATERSKIN1_EMPTY, STRING_INV_WATERSKIN1_EMPTY, -1, 0);
	m_objectsTable[INV_OBJECT_WATERSKIN2] = InventoryObjectDefinition(ID_WATERSKIN2_EMPTY, STRING_INV_WATERSKIN2_EMPTY, -1, 0);

	// Add combinations
	AddCombination(INV_OBJECT_PUZZLE1_COMBO1, INV_OBJECT_PUZZLE1_COMBO2, INV_OBJECT_PUZZLE1, CombinePuzzle1);
	AddCombination(INV_OBJECT_PUZZLE2_COMBO1, INV_OBJECT_PUZZLE2_COMBO2, INV_OBJECT_PUZZLE2, CombinePuzzle2);
	AddCombination(INV_OBJECT_PUZZLE3_COMBO1, INV_OBJECT_PUZZLE3_COMBO2, INV_OBJECT_PUZZLE3, CombinePuzzle3);
	AddCombination(INV_OBJECT_PUZZLE4_COMBO1, INV_OBJECT_PUZZLE4_COMBO2, INV_OBJECT_PUZZLE4, CombinePuzzle4);
	AddCombination(INV_OBJECT_PUZZLE5_COMBO1, INV_OBJECT_PUZZLE5_COMBO2, INV_OBJECT_PUZZLE5, CombinePuzzle5);
	AddCombination(INV_OBJECT_PUZZLE6_COMBO1, INV_OBJECT_PUZZLE6_COMBO2, INV_OBJECT_PUZZLE6, CombinePuzzle6);
	AddCombination(INV_OBJECT_PUZZLE7_COMBO1, INV_OBJECT_PUZZLE7_COMBO2, INV_OBJECT_PUZZLE7, CombinePuzzle7);
	AddCombination(INV_OBJECT_PUZZLE8_COMBO1, INV_OBJECT_PUZZLE8_COMBO2, INV_OBJECT_PUZZLE8, CombinePuzzle8);
	AddCombination(INV_OBJECT_KEY1_COMBO1, INV_OBJECT_KEY1_COMBO2, INV_OBJECT_KEY1, CombineKey1);
	AddCombination(INV_OBJECT_KEY2_COMBO1, INV_OBJECT_KEY2_COMBO2, INV_OBJECT_KEY2, CombineKey2);
	AddCombination(INV_OBJECT_KEY3_COMBO1, INV_OBJECT_KEY3_COMBO2, INV_OBJECT_KEY3, CombineKey3);
	AddCombination(INV_OBJECT_KEY4_COMBO1, INV_OBJECT_KEY4_COMBO2, INV_OBJECT_KEY4, CombineKey4);
	AddCombination(INV_OBJECT_KEY5_COMBO1, INV_OBJECT_KEY5_COMBO2, INV_OBJECT_KEY5, CombineKey5);
	AddCombination(INV_OBJECT_KEY6_COMBO1, INV_OBJECT_KEY6_COMBO2, INV_OBJECT_KEY6, CombineKey6);
	AddCombination(INV_OBJECT_KEY7_COMBO1, INV_OBJECT_KEY7_COMBO2, INV_OBJECT_KEY7, CombineKey7);
	AddCombination(INV_OBJECT_KEY8_COMBO1, INV_OBJECT_KEY8_COMBO2, INV_OBJECT_KEY8, CombineKey8);
	AddCombination(INV_OBJECT_PICKUP1_COMBO1, INV_OBJECT_PICKUP1_COMBO2, INV_OBJECT_PICKUP1, CombinePickup1);
	AddCombination(INV_OBJECT_PICKUP2_COMBO1, INV_OBJECT_PICKUP2_COMBO2, INV_OBJECT_PICKUP2, CombinePickup2);
	AddCombination(INV_OBJECT_PICKUP3_COMBO1, INV_OBJECT_PICKUP3_COMBO2, INV_OBJECT_PICKUP3, CombinePickup3);
	AddCombination(INV_OBJECT_PICKUP4_COMBO1, INV_OBJECT_PICKUP4_COMBO2, INV_OBJECT_PICKUP4, CombinePickup4);
	AddCombination(INV_OBJECT_REVOLVER, INV_OBJECT_LASERSIGHT, INV_OBJECT_REVOLVER_LASER, CombineRevolverLasersight);
	AddCombination(INV_OBJECT_CROSSBOW, INV_OBJECT_LASERSIGHT, INV_OBJECT_CROSSBOW_LASER, CombineCrossbowLasersight);
}

Inventory::~Inventory()
{

}

InventoryRing* Inventory::GetRing(__int32 index)
{
	return &m_rings[index];
}

__int32 Inventory::GetActiveRing()
{
	return m_activeRing;
}

void Inventory::SetActiveRing(__int32 index)
{
	m_activeRing = index;
}

void Inventory::InsertObject(__int32 ring, __int32 objectNumber)
{
	m_rings[ring].objects[m_rings[ring].numObjects].inventoryObject = objectNumber;
	m_rings[ring].numObjects++;
}

void Inventory::LoadObjects()
{
	// Reset the objects in inventory
	for (__int32 i = 0; i < NUM_INVENTORY_RINGS; i++)
	{
		m_rings[i].numObjects = 0;
		m_rings[i].movement = 0;
		m_rings[i].currentObject = 0;
		m_rings[i].focusState = INV_FOCUS_STATE_NONE;

		for (__int32 j = 0; j < NUM_INVENTORY_OBJECTS_PER_RING; j++)
		{
			m_rings[i].objects[j].inventoryObject = -1;
			m_rings[i].objects[j].rotation = 0;
			m_rings[i].objects[j].scale = INV_OBJECT_SCALE;
		}
	}

	// DEBUG
	{
		g_LaraExtra.Weapons[WEAPON_SHOTGUN].Present = true;
		g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[0] = 1000;
		g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[1] = 1000;
		g_LaraExtra.Weapons[WEAPON_SHOTGUN].SelectedAmmo = WEAPON_AMMO1;

		g_LaraExtra.Weapons[WEAPON_REVOLVER].Present = true;
		g_LaraExtra.Weapons[WEAPON_REVOLVER].Ammo[0] = 1000;
		g_LaraExtra.Weapons[WEAPON_REVOLVER].SelectedAmmo = WEAPON_AMMO1;

		Lara.laserSight = true;

		/*g_LaraExtra.Weapons[WEAPON_CROSSBOW].Present = true;
		g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[0] = 1000;
		g_LaraExtra.Weapons[WEAPON_CROSSBOW].SelectedAmmo = WEAPON_AMMO1;

		g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Present = true;
		g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[0] = 1000;
		g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].SelectedAmmo = WEAPON_AMMO1;

		g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].Present = true;
		g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].Ammo[0] = 1000;*/
	}

	// Now fill the rings
	if (g_GameFlow->GetLevel(CurrentLevel)->LaraType != LARA_DRAW_TYPE::LARA_YOUNG)
	{
		// Pistols
		if (g_LaraExtra.Weapons[WEAPON_PISTOLS].Present)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_PISTOLS);

		// Uzi
		if (g_LaraExtra.Weapons[WEAPON_UZI].Present)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_UZIS);
		else if (g_LaraExtra.Weapons[WEAPON_UZI].Ammo[0])
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_UZI_AMMO);

		// Revolver
		if (g_LaraExtra.Weapons[WEAPON_REVOLVER].Present)
		{
			if (g_LaraExtra.Weapons[WEAPON_REVOLVER].HasLasersight)
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_REVOLVER_LASER);
			else
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_REVOLVER);
		}
		else
		{
			if (g_LaraExtra.Weapons[WEAPON_REVOLVER].Ammo[0])
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_REVOLVER_AMMO);
		}

		// Shotgun
		if (g_LaraExtra.Weapons[WEAPON_SHOTGUN].Present)
		{
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_SHOTGUN);
			//if (Lara.shotgunTypeCarried & 0x10)
			//	CurrentShotGunAmmoType = 1;
		}
		else
		{
			if (g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[0])
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_SHOTGUN_AMMO1);
			if (g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[1])
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_SHOTGUN_AMMO2);
		}

		// HK
		if (g_LaraExtra.Weapons[WEAPON_HK].Present)
		{
			if (g_LaraExtra.Weapons[WEAPON_HK].HasSilencer)
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_HK_LASER);
			else
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_HK);
		}
		else if (g_LaraExtra.Weapons[WEAPON_HK].Ammo[0])
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_HK_AMMO1);

		// Crossbow
		if (g_LaraExtra.Weapons[WEAPON_CROSSBOW].Present)
		{
			if (g_LaraExtra.Weapons[WEAPON_CROSSBOW].HasLasersight)
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_CROSSBOW_LASER);
			else
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_CROSSBOW);
		}
		else
		{
			if (g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[0])
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_CROSSBOW_AMMO1);
			if (g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[1])
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_CROSSBOW_AMMO2);
			if (g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[2])
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_CROSSBOW_AMMO3);
		}

		// Grenade launcher
		if (g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Present)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_GRENADE_LAUNCHER);
		else
		{
			if (g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[0])
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_GRENADE_AMMO1);
			if (g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[1])
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_GRENADE_AMMO2);
			if (g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[2])
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_GRENADE_AMMO3);
		}

		// Harpoon
		if (g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].Present)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_HARPOON_GUN);
		else if (g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].Ammo[0])
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_HARPOON_AMMO);

		// Rocket launcher
		if (g_LaraExtra.Weapons[WEAPON_ROCKET_LAUNCHER].Present)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_ROCKET_LAUNCHER);
		else if (g_LaraExtra.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[0])
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_ROCKET_AMMO);

		// Lasersight
		if (Lara.laserSight)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_LASERSIGHT);

		// Silencer
		if (Lara.silencer)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_SILENCER);

		// Binoculars
		if (Lara.binoculars)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_BINOCULARS);

		// Flares
		if (Lara.numFlares)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_FLARES);
	}

	if (Lara.numSmallMedipack)
		InsertObject(INV_RING_WEAPONS, INV_OBJECT_SMALL_MEDIPACK);

	if (Lara.numLargeMedipack)
		InsertObject(INV_RING_WEAPONS, INV_OBJECT_LARGE_MEDIPACK);

	if (Lara.crowbar)
		InsertObject(INV_RING_WEAPONS, INV_OBJECT_CROWBAR);

	__int32 i = 0;
	do
	{
		if (Lara.puzzleItems[i])
			InsertObject(INV_RING_PUZZLES, i + INV_OBJECT_PUZZLE1);
		i++;
	} while (i < 8);

	i = 0;
	do
	{
		if ((1 << i) & Lara.puzzleItemsCombo)
			InsertObject(INV_RING_PUZZLES, i + INV_OBJECT_PUZZLE1_COMBO1);
		i++;
	} while (i < 16);

	i = 0;
	do
	{
		if ((1 << i) & Lara.keyItems)
			InsertObject(INV_RING_PUZZLES, i + INV_OBJECT_KEY1);
		i++;
	} while (i < 8);

	i = 0;
	do
	{
		if ((1 << i) & Lara.keyItemsCombo)
			InsertObject(INV_RING_PUZZLES, i + INV_OBJECT_KEY1_COMBO1);
		i++;
	} while (i < 16);

	i = 0;
	do
	{
		if ((1 << i) & Lara.pickupItems)
			InsertObject(INV_RING_PUZZLES, i + INV_OBJECT_PICKUP1);
		i++;
	} while (i < 4);

	i = 0;
	do
	{
		if ((1 << i) & Lara.pickupItemsCombo)
			InsertObject(INV_RING_PUZZLES, i + INV_OBJECT_PICKUP1_COMBO1);
		i++;
	} while (i < 8);

	if (Lara.examine1)
		InsertObject(INV_RING_PUZZLES, INV_OBJECT_EXAMINE1);

	if (Lara.examine2)
		InsertObject(INV_RING_PUZZLES, INV_OBJECT_EXAMINE2);

	if (Lara.examine3)
		InsertObject(INV_RING_PUZZLES, INV_OBJECT_EXAMINE3);

	if (Lara.wetcloth == 2)
		InsertObject(INV_RING_PUZZLES, INV_OBJECT_WETCLOTH1);

	if (Lara.wetcloth == 1)
		InsertObject(INV_RING_PUZZLES, INV_OBJECT_WETCLOTH2);

	if (Lara.bottle)
		InsertObject(INV_RING_PUZZLES, INV_OBJECT_BOTTLE);

	if (g_LaraExtra.Waterskin1.Present)
		InsertObject(INV_RING_PUZZLES, INV_OBJECT_WATERSKIN1);

	if (g_LaraExtra.Waterskin2.Present)
		InsertObject(INV_RING_PUZZLES, INV_OBJECT_WATERSKIN2);

	InventoryRing * ring = &m_rings[INV_RING_OPTIONS];

	// Reset the objects in inventory
	ring->numObjects = 0;
	ring->movement = 0;
	ring->currentObject = 0;
	ring->focusState = INV_FOCUS_STATE_NONE;

	for (__int32 j = 0; j < NUM_INVENTORY_OBJECTS_PER_RING; j++)
	{
		ring->objects[j].inventoryObject = -1;
		ring->objects[j].rotation = 0;
		ring->objects[j].scale = 2.0f;
	}

	InsertObject(INV_RING_OPTIONS, INV_OBJECT_PASSAPORT);
	InsertObject(INV_RING_OPTIONS, INV_OBJECT_SUNGLASSES);
	InsertObject(INV_RING_OPTIONS, INV_OBJECT_HEADPHONES);
	InsertObject(INV_RING_OPTIONS, INV_OBJECT_KEYS);
}

void Inventory::Initialise()
{
	LoadObjects();

	m_activeRing = INV_RING_WEAPONS;
	m_type = INV_TYPE_GAME;
	m_deltaMovement = 0;
	m_movement = INV_MOVE_STOPPED;
	m_type = INV_TYPE_GAME;
	InventoryItemChosen = -1;
}

__int32 Inventory::DoInventory()
{
	Initialise();

	// If Lara is dead, then we can use only the passport
	if (LaraItem->hitPoints <= 0 && CurrentLevel > 0)
	{
		m_rings[INV_RING_PUZZLES].draw = false;
		m_rings[INV_RING_WEAPONS].draw = false;
		m_rings[INV_RING_OPTIONS].draw = true;
		m_rings[INV_RING_COMBINE].draw = false;
		m_rings[INV_RING_CHOOSE_AMMO].draw = false;

		m_activeRing = INV_RING_OPTIONS;
		m_rings[m_activeRing].currentObject = 0;

		__int32 passportResult = DoPassport();

		// Fade out
		g_Renderer->FadeOut();
		for (__int32 i = 0; i < FADE_FRAMES_COUNT; i++)
		{
			g_Renderer->DrawInventory();
		}

		return passportResult;
	}

	m_rings[INV_RING_PUZZLES].draw = true;
	m_rings[INV_RING_WEAPONS].draw = true;
	m_rings[INV_RING_OPTIONS].draw = true;
	m_rings[INV_RING_COMBINE].draw = false;
	m_rings[INV_RING_CHOOSE_AMMO].draw = false;

	m_activeRing = INV_RING_WEAPONS;

	__int32 result = INV_RESULT_NONE;

	g_Renderer->DumpGameScene();
	g_Renderer->DrawInventory();

	while (!ResetFlag)
	{
		SetDebounce = true;
		S_UpdateInput();
		SetDebounce = false;

		GameTimer++;

		// Handle input
		if (DbInput & IN_DESELECT)
		{
			SoundEffect(SFX_MENU_SELECT, NULL, 0);

			// Exit from inventory
			GlobalEnterInventory = -1;
			return INV_RESULT_NONE;
		}
		else if (DbInput & IN_FORWARD &&
			(m_activeRing == INV_RING_WEAPONS && m_rings[INV_RING_PUZZLES].numObjects != 0 ||
			 m_activeRing == INV_RING_OPTIONS))
		{
			SoundEffect(SFX_MENU_ROTATE, NULL, 0);

			// Go to the upper ring
			for (__int32 i = 0; i < 8; i++)
			{
				m_movement -= 1024.0f;
				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}

			if (m_activeRing == INV_RING_WEAPONS)
				m_activeRing = INV_RING_PUZZLES;
			else
				m_activeRing = INV_RING_WEAPONS;

			m_movement = 0;

			continue;
		}
		else if (DbInput & IN_BACK && (m_activeRing == INV_RING_PUZZLES || m_activeRing == INV_RING_WEAPONS))
		{
			SoundEffect(SFX_MENU_ROTATE, NULL, 0);

			// Go to the lower ring
			for (__int32 i = 0; i < 8; i++)
			{
				m_movement += 1024.0f;
				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}

			if (m_activeRing == INV_RING_WEAPONS)
				m_activeRing = INV_RING_OPTIONS;
			else
				m_activeRing = INV_RING_WEAPONS;

			m_movement = 0;

			continue;
		}
		else if (DbInput & IN_LEFT)
		{
			SoundEffect(SFX_MENU_ROTATE, NULL, 0);

			// Change object right
			float deltaAngle = 360.0f / m_rings[m_activeRing].numObjects / 8.0f;
			m_rings[m_activeRing].movement = 0;

			for (__int32 i = 0; i < 8; i++)
			{
				m_rings[m_activeRing].movement += deltaAngle;

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}

			if (m_rings[m_activeRing].currentObject == m_rings[m_activeRing].numObjects - 1)
				m_rings[m_activeRing].currentObject = 0;
			else
				m_rings[m_activeRing].currentObject++;

			m_rings[m_activeRing].selectedIndex = INV_ACTION_USE;
			m_rings[m_activeRing].movement = 0;
		}
		else if (DbInput & IN_RIGHT)
		{
			SoundEffect(SFX_MENU_ROTATE, NULL, 0);

			// Change object left
			float deltaAngle = 360.0f / m_rings[m_activeRing].numObjects / 8.0f;
			m_rings[m_activeRing].movement = 0;

			for (__int32 i = 0; i < 8; i++)
			{
				m_rings[m_activeRing].movement -= deltaAngle;

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}

			if (m_rings[m_activeRing].currentObject == 0)
				m_rings[m_activeRing].currentObject = m_rings[m_activeRing].numObjects - 1;
			else
				m_rings[m_activeRing].currentObject--;

			m_rings[m_activeRing].selectedIndex = INV_ACTION_USE;
			m_rings[m_activeRing].movement = 0;
		}
		else if (DbInput & IN_SELECT)
		{
			// Handle action 
			if (m_activeRing == INV_RING_OPTIONS)
			{
				if (m_rings[INV_RING_OPTIONS].objects[m_rings[INV_RING_OPTIONS].currentObject].inventoryObject == INV_OBJECT_PASSAPORT)
				{
					__int32 passportResult = DoPassport();
					if (passportResult == INV_RESULT_NEW_GAME ||
						passportResult == INV_RESULT_EXIT_TO_TILE ||
						passportResult == INV_RESULT_LOAD_GAME)
					{
						// Fade out
						g_Renderer->FadeOut();
						for (__int32 i = 0; i < FADE_FRAMES_COUNT; i++)
							g_Renderer->DrawInventory();

						return passportResult;
					}
				}

				__int16 currentObject = m_rings[INV_RING_OPTIONS].objects[m_rings[INV_RING_OPTIONS].currentObject].inventoryObject;

				if (currentObject == INV_OBJECT_KEYS)
					DoControlsSettings();

				if (currentObject == INV_OBJECT_SUNGLASSES)
					DoGraphicsSettings();

				if (currentObject == INV_OBJECT_HEADPHONES)
					DoSoundSettings();
			}
			else if (m_activeRing == INV_RING_WEAPONS || m_activeRing == INV_RING_PUZZLES)
			{
				__int16 currentObject = m_rings[m_activeRing].objects[m_rings[m_activeRing].currentObject].inventoryObject;
				__int32 result = INV_RESULT_NONE;

				if (IsCurrentObjectPuzzle())
					// Puzzles have Use, Combine, Separe
					result = DoPuzzle();
				else if (IsCurrentObjectWeapon())
					// Weapons have Use, Select Ammo
					result = DoWeapon();
				else if (IsCurrentObjectExamine())
					// Examines have just Examine
					DoExamine();
				else if (currentObject == INV_OBJECT_TIMEX)
					// Do statistics
					DoStatistics();
				else
					// All other objects have just Use
					result = DoGenericObject();

				// If an item is set to be used, then use it and close inventory
				if (result == INV_RESULT_USE_ITEM)
				{
					UseCurrentItem();

					// Exit from inventory
					GlobalEnterInventory = -1;
					return INV_RESULT_USE_ITEM;
				}
			}
		}

		g_Renderer->DrawInventory();
		g_Renderer->SyncRenderer();
	}

	return result;
}

bool Inventory::IsCurrentObjectWeapon()
{
	__int16 currentObject = m_rings[m_activeRing].objects[m_rings[m_activeRing].currentObject].inventoryObject;
	return (currentObject == INV_OBJECT_PISTOLS || currentObject == INV_OBJECT_UZIS ||
		currentObject == INV_OBJECT_REVOLVER_LASER || currentObject == INV_OBJECT_CROSSBOW_LASER ||
		currentObject == INV_OBJECT_REVOLVER || currentObject == INV_OBJECT_SHOTGUN ||
		currentObject == INV_OBJECT_HK || currentObject == INV_OBJECT_CROSSBOW ||
		currentObject == INV_OBJECT_ROCKET_LAUNCHER || currentObject == INV_OBJECT_GRENADE_LAUNCHER ||
		currentObject == INV_OBJECT_HARPOON_GUN);
}

bool Inventory::IsCurrentObjectPuzzle()
{
	__int16 currentObject = m_rings[m_activeRing].objects[m_rings[m_activeRing].currentObject].inventoryObject;
	return (currentObject >= INV_OBJECT_PUZZLE1 && currentObject <= INV_OBJECT_PICKUP4_COMBO2);
}

bool Inventory::IsCurrentObjectGeneric()
{
	__int16 currentObject = m_rings[m_activeRing].objects[m_rings[m_activeRing].currentObject].inventoryObject;
	return (!IsCurrentObjectPuzzle() && !IsCurrentObjectExamine() && !IsCurrentObjectWeapon() &&
		currentObject != INV_OBJECT_TIMEX);
}

bool Inventory::IsCurrentObjectExamine()
{
	__int16 currentObject = m_rings[m_activeRing].objects[m_rings[m_activeRing].currentObject].inventoryObject;
	return (currentObject >= INV_OBJECT_EXAMINE1 && currentObject <= INV_OBJECT_EXAMINE1);
}

__int32 Inventory::DoPuzzle()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->frameIndex = 0;

	return INV_RESULT_NONE;
}

__int32 Inventory::DoWeapon()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->frameIndex = 0;
	ring->selectedIndex = 0;
	ring->numActions = 0;

	PopupObject();

	__int32 result = INV_RESULT_NONE;
	bool closeObject = false;
	__int16 currentObject = ring->objects[ring->currentObject].inventoryObject;

	ring->actions[ring->numActions++] = INV_ACTION_USE;
	if (IsObjectCombinable(currentObject)) ring->actions[ring->numActions++] = INV_ACTION_COMBINE;
	if (IsObjectSeparable(currentObject)) ring->actions[ring->numActions++] = INV_ACTION_SEPARE;
	ring->actions[ring->numActions++] = INV_ACTION_SELECT_AMMO;

	// Do the menu
	while (true)
	{
		// Handle input
		SetDebounce = true;
		S_UpdateInput();
		SetDebounce = false;

		GameTimer++;

		// Handle input
		if (DbInput & IN_DESELECT || closeObject)
		{
			closeObject = true;
			break;
		}
		else if (DbInput & IN_FORWARD)
		{
			closeObject = false;

			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex > 0)
				ring->selectedIndex--;
		}
		else if (DbInput & IN_BACK)
		{
			closeObject = false;

			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex < ring->numActions)
				ring->selectedIndex++;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_MENU_SELECT, NULL, 0);

			if (ring->actions[ring->selectedIndex] == INV_ACTION_USE)
			{
				result = INV_RESULT_USE_ITEM;
				closeObject = true;
				break;
			}
			else if (ring->actions[ring->selectedIndex] == INV_ACTION_COMBINE)
			{
				DoCombine();
			}
			else if (ring->actions[ring->selectedIndex] == INV_ACTION_SEPARE)
			{
				DoSepare();
			}
			else if (ring->actions[ring->selectedIndex] == INV_ACTION_SELECT_AMMO)
			{
				DoSelectAmmo();
			}
		}

		g_Renderer->DrawInventory();
		g_Renderer->SyncRenderer();
	}

	PopoverObject();

	return result;
}

bool Inventory::IsObjectPresentInInventory(__int16 object)
{
	for (__int32 r = 0; r < 3; r++)
		for (__int32 o = 0; o < m_rings[r].numObjects; o++)
			if (m_rings[r].objects[o].inventoryObject == object)
				return true;
	return false;
}

bool Inventory::IsObjectCombinable(__int16 object)
{
	for (__int32 i = 0; i < m_combinations.size(); i++)
		if (m_combinations[i].piece1 == object || m_combinations[i].piece2 == object)
			return true;
	return false;
}

void Inventory::AddCombination(__int16 piece1, __int16 piece2, __int16 combinedObject, void (*f) (__int32))
{
	InventoryObjectCombination combination;
	combination.piece1 = piece1;
	combination.piece2 = piece2;
	combination.combinedObject = combinedObject;
	combination.combineRoutine = f;
	m_combinations.push_back(combination);
}

__int32 Inventory::DoGenericObject()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->frameIndex = 0;

	return INV_RESULT_USE_ITEM;
}

void Inventory::DoStatistics()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->frameIndex = 0;

}

void Inventory::DoExamine()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->frameIndex = 0;

}

void Inventory::DoCombine()
{
	InventoryRing* ring = &m_rings[m_activeRing];

	// Enable the secondary GUI
	m_activeRing = INV_RING_COMBINE;

	// Fill the secondary ring
	InventoryRing* combineRing = &m_rings[INV_RING_COMBINE];
	combineRing->draw = true;
	combineRing->numObjects = 0;
	
	// Fill the objects ring
	combineRing->numObjects = 0;
	__int16 currentObject = ring->objects[ring->currentObject].inventoryObject;

	for (__int32 i = 0; i < m_combinations.size(); i++)
	{
		InventoryObjectCombination* combination = &m_combinations[i];
		
		// Add piece 1
		if (currentObject != combination->piece1 && IsObjectPresentInInventory(combination->piece1))
		{
			combineRing->objects[combineRing->numObjects++].inventoryObject = combination->piece1;
		}

		// Add piece 2
		if (currentObject != combination->piece2 && IsObjectPresentInInventory(combination->piece2))
		{
			combineRing->objects[combineRing->numObjects++].inventoryObject = combination->piece2;
		}
	}

	// If no objects then exit 
	if (combineRing->numObjects == 0)
		return;

	combineRing->selectedIndex = 0;
	bool closeObject = false;

	// Do the menu
	while (true)
	{
		// Handle input
		SetDebounce = true;
		S_UpdateInput();
		SetDebounce = false;

		GameTimer++;

		// Handle input
		if (DbInput & IN_DESELECT || closeObject)
		{
			closeObject = true;
			break;
		}
		else if (DbInput & IN_LEFT)
		{
			closeObject = false;

			SoundEffect(SFX_MENU_ROTATE, NULL, 0);

			// Change object left
			float deltaAngle = 360.0f / combineRing->numObjects / 8.0f;
			combineRing->movement = 0;

			for (__int32 i = 0; i < 8; i++)
			{
				combineRing->movement += deltaAngle;

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}

			if (combineRing->currentObject > 0)
				combineRing->currentObject--;
			else
				combineRing->currentObject = combineRing->numObjects - 1;

			combineRing->movement = 0;
		}
		else if (DbInput & IN_RIGHT)
		{
			closeObject = false;

			// Change object right
			float deltaAngle = 360.0f / combineRing->numObjects / 8.0f;
			combineRing->movement = 0;

			for (__int32 i = 0; i < 8; i++)
			{
				combineRing->movement -= deltaAngle;

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}

			SoundEffect(SFX_MENU_ROTATE, NULL, 0);
			if (combineRing->currentObject < combineRing->numObjects - 1)
				combineRing->currentObject++;
			else
				combineRing->currentObject = 0;

			combineRing->movement = 0;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_MENU_SELECT, NULL, 0);

			// Check if can be combined
			__int16 currentObject = combineRing->objects[combineRing->currentObject].inventoryObject;
			for (__int32 i = 0; i < m_combinations.size(); i++)
			{
				InventoryObjectCombination* combination = &m_combinations[i];
				if (combination->piece1 == currentObject && combination->piece2 == ring->objects[ring->currentObject].inventoryObject || 
					combination->piece2 == currentObject && combination->piece1 == ring->objects[ring->currentObject].inventoryObject)
				{
					// I can do the combination
					SoundEffect(SFX_MENU_COMBINE, NULL, 0);
					combination->combineRoutine(INV_COMBINE_COMBINE);
					LoadObjects();
					closeObject = true;
					break;
				}
			}

			if (!closeObject)
				SayNo();
		}

		g_Renderer->DrawInventory();
		g_Renderer->SyncRenderer();
	}

	// Reset secondary GUI
	m_activeRing = INV_RING_WEAPONS;
	combineRing->draw = false;
}

void Inventory::DoSepare()
{
	__int16 currentObject = m_secondaryRing.currentObject;
	for (__int32 i = 0; i < m_combinations.size(); i++)
	{
		InventoryObjectCombination* combination = &m_combinations[i];
		if (combination->combinedObject == currentObject)
		{
			// I can do the separation
			SoundEffect(SFX_MENU_COMBINE, NULL, 0);
			combination->combineRoutine(INV_COMBINE_SEPARE);
			LoadObjects();
			return;
		}
	}
}

void Inventory::DoSelectAmmo()
{
	InventoryRing* ring = &m_rings[m_activeRing];

	// Enable the secondary GUI
	m_activeRing = INV_RING_CHOOSE_AMMO;

	// Fill the secondary ring
	InventoryRing* ammoRing = &m_rings[INV_RING_CHOOSE_AMMO];
	ammoRing->draw = true;
	ammoRing->numObjects = 0;

	switch (ring->objects[ring->currentObject].inventoryObject)
	{
	case INV_OBJECT_SHOTGUN:
		if (g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[0] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = INV_OBJECT_SHOTGUN_AMMO1;
		if (g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[1] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = INV_OBJECT_SHOTGUN_AMMO2;
		ammoRing->selectedIndex = g_LaraExtra.Weapons[WEAPON_SHOTGUN].SelectedAmmo;

		break;

	case INV_OBJECT_GRENADE_LAUNCHER:
		if (g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[0] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = INV_OBJECT_GRENADE_AMMO1;
		if (g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[1] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = INV_OBJECT_GRENADE_AMMO2;
		if (g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[2] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = INV_OBJECT_GRENADE_AMMO3;
		ammoRing->selectedIndex = g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].SelectedAmmo;
		
		break;

	case INV_OBJECT_CROSSBOW:
	case INV_OBJECT_CROSSBOW_LASER:
		if (g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[0] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = INV_OBJECT_CROSSBOW_AMMO1;
		if (g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[1] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = INV_OBJECT_CROSSBOW_AMMO2;
		if (g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[2] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = INV_OBJECT_CROSSBOW_AMMO3;
		ammoRing->selectedIndex = g_LaraExtra.Weapons[WEAPON_CROSSBOW].SelectedAmmo;
		
		break;
	}

	// If no objects then exit 
	if (ammoRing->numObjects == 0)
		return;

	bool closeObject = false;

	// Do the menu
	while (true)
	{
		// Handle input
		SetDebounce = true;
		S_UpdateInput();
		SetDebounce = false;

		GameTimer++;

		// Handle input
		if (DbInput & IN_DESELECT || closeObject)
		{
			closeObject = true;
			break;
		}
		else if (DbInput & IN_LEFT)
		{
			closeObject = false;

			SoundEffect(SFX_MENU_ROTATE, NULL, 0);

			// Change object left
			float deltaAngle = 360.0f / ammoRing->numObjects / 8.0f;
			ammoRing->movement = 0;

			for (__int32 i = 0; i < 8; i++)
			{
				ammoRing->movement += deltaAngle;

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}

			if (ammoRing->currentObject > 0)
				ammoRing->currentObject--;
			else
				ammoRing->currentObject = ammoRing->numObjects - 1;

			ammoRing->movement = 0;
		}
		else if (DbInput & IN_RIGHT)
		{
			closeObject = false;

			// Change object right
			float deltaAngle = 360.0f / ammoRing->numObjects / 8.0f;
			ammoRing->movement = 0;

			for (__int32 i = 0; i < 8; i++)
			{
				ammoRing->movement -= deltaAngle;

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}

			SoundEffect(SFX_MENU_ROTATE, NULL, 0);
			if (ammoRing->currentObject < ammoRing->numObjects - 1)
				ammoRing->currentObject++;
			else
				ammoRing->currentObject = 0;

			ammoRing->movement = 0;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_MENU_SELECT, NULL, 0);

			// Choose ammo
			switch (ring->objects[ring->currentObject].inventoryObject)
			{
			case INV_OBJECT_SHOTGUN:
				g_LaraExtra.Weapons[WEAPON_SHOTGUN].SelectedAmmo = ring->selectedIndex;
				break;

			case INV_OBJECT_GRENADE_LAUNCHER:
				g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].SelectedAmmo = ring->selectedIndex;
				break;

			case INV_OBJECT_CROSSBOW:
			case INV_OBJECT_CROSSBOW_LASER:
				g_LaraExtra.Weapons[WEAPON_CROSSBOW].SelectedAmmo = ring->selectedIndex;
				break;

			}

			closeObject = true;
			break;
		}

		g_Renderer->DrawInventory();
		g_Renderer->SyncRenderer();
	}

	// Reset secondary GUI
	m_activeRing = INV_RING_WEAPONS;
	ammoRing->draw = false;
}

void Inventory::UseCurrentItem()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	InventoryObject* inventoryObject = &ring->objects[ring->currentObject];
	__int16 objectNumber = m_objectsTable[inventoryObject->inventoryObject].objectNumber;

	LaraItem->meshBits = -1;

	__int32 binocularRange = BinocularRange;
	BinocularRange = 0;
	OldLaraBusy = false;

	// Small medipack
	if (objectNumber == ID_SMALLMEDI_ITEM)
	{
		if ((LaraItem->hitPoints <= 0 || LaraItem->hitPoints >= 1000) && !Lara.poisoned)
		{
			SayNo();
			return;
		}

		if (Lara.numSmallMedipack != -1)
			Lara.numSmallMedipack--;

		Lara.dpoisoned = 0;
		LaraItem->hitPoints += 500;
		if (LaraItem->hitPoints > 1000)
			LaraItem->hitPoints = 1000;
		
		SoundEffect(116, 0, 2);
		Savegame.Game.HealthUsed++;

		SoundEffect(SFX_MENU_MEDI, NULL, 0);

		return;
	}

	// Big medipack
	if (objectNumber == ID_BIGMEDI_ITEM)
	{
		if ((LaraItem->hitPoints <= 0 || LaraItem->hitPoints >= 1000) && !Lara.poisoned)
		{
			SayNo();
			return;
		}

		if (Lara.numLargeMedipack != -1)
			Lara.numLargeMedipack--;

		Lara.dpoisoned = 0;
		LaraItem->hitPoints += 1000;
		if (LaraItem->hitPoints > 1000)
			LaraItem->hitPoints = 1000;

		SoundEffect(116, 0, 2);
		Savegame.Game.HealthUsed++;

		SoundEffect(SFX_MENU_MEDI, NULL, 0);

		return;
	}

	// Binoculars
	if (objectNumber == ID_BINOCULARS_ITEM)
	{
		if (LaraItem->currentAnimState == 2 && LaraItem->animNumber == 103 || Lara.isDucked && !(TrInput & 0x20000000))
		{
			if (!SniperCameraActive && !UseSpotCam && !TrackCameraInit)
			{
				OldLaraBusy = true;
				BinocularRange = 128;
				if (Lara.gunStatus)
					Lara.gunStatus = LG_UNDRAW_GUNS;
			}
		}

		if (binocularRange)
			BinocularRange = binocularRange;
		else
			BinocularOldCamera = Camera.oldType;

		return;
	}

	// Crowbar and puzzles
	if (objectNumber == ID_CROWBAR_ITEM ||
		objectNumber >= ID_PUZZLE_ITEM1 && objectNumber <= ID_PUZZLE_ITEM8 ||
		objectNumber >= ID_PUZZLE_ITEM1_COMBO1 && objectNumber <= ID_PUZZLE_ITEM8_COMBO2 ||
		objectNumber >= ID_KEY_ITEM1 && objectNumber <= ID_KEY_ITEM8 ||
		objectNumber >= ID_KEY_ITEM1_COMBO1 && objectNumber <= ID_KEY_ITEM8_COMBO2 ||
		objectNumber >= ID_PICKUP_ITEM1 && objectNumber <= ID_PICKUP_ITEM3 ||
		objectNumber >= ID_PICKUP_ITEM1_COMBO1 && objectNumber <= ID_PICKUP_ITEM3_COMBO2)
	{
		// Only if above water
		if (Lara.waterStatus == LW_ABOVE_WATER)
		{
			InventoryItemChosen = objectNumber;
			
			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
			
			return;
		}
		else
		{
			SayNo();
			return;
		}
	}

	// Flares
	if (objectNumber == ID_FLARE_INV_ITEM)
	{
		if (Lara.waterStatus == LW_ABOVE_WATER)
		{
			InventoryItemChosen = objectNumber;
			
			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
			
			return;
		}
		else
		{
			SayNo();
			return;
		}
	}

	bool canUseWeapons = !(LaraItem->currentAnimState == STATE_LARA_CRAWL_IDLE || 
						   LaraItem->currentAnimState == STATE_LARA_CRAWL_FORWARD ||
						   LaraItem->currentAnimState == STATE_LARA_CRAWL_TURN_LEFT || 
						   LaraItem->currentAnimState == STATE_LARA_CRAWL_TURN_RIGHT ||
						   LaraItem->currentAnimState == STATE_LARA_CRAWL_BACK || 
						   LaraItem->currentAnimState == STATE_LARA_CRAWL_TO_CLIMB ||
						   LaraItem->currentAnimState == STATE_LARA_CROUCH_IDLE || 
						   LaraItem->currentAnimState == STATE_LARA_CROUCH_TURN_LEFT ||
						   LaraItem->currentAnimState == STATE_LARA_CROUCH_TURN_RIGHT || 
						   Lara.waterStatus != LW_ABOVE_WATER);

	// Pistols
	if (objectNumber == ID_PISTOLS_ITEM)
	{
		if (canUseWeapons)
		{
			Lara.requestGunType = WEAPON_PISTOLS; 
			if (!Lara.gunStatus && Lara.gunType == WEAPON_PISTOLS)
				Lara.gunStatus = LG_DRAW_GUNS;
		
			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
		}
		else
		{
			SayNo();
		}

		return;
	}

	// Uzis
	if (objectNumber == ID_UZI_ITEM)
	{
		if (canUseWeapons)
		{
			Lara.requestGunType = WEAPON_UZI;
			if (!Lara.gunStatus && Lara.gunType == WEAPON_UZI)
				Lara.gunStatus = LG_DRAW_GUNS;
		
			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
		}
		else
		{
			SayNo();
		}

		return;
	}

	// Revolver
	if (objectNumber == ID_REVOLVER_ITEM)
	{
		if (canUseWeapons)
		{
			Lara.requestGunType = WEAPON_REVOLVER;
			if (!Lara.gunStatus && Lara.gunType == WEAPON_REVOLVER)
				Lara.gunStatus = LG_DRAW_GUNS;
	
			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
		}
		else
		{
			SayNo();
		}

		return;
	}

	// Shotgun
	if (objectNumber == ID_SHOTGUN_ITEM)
	{
		if (canUseWeapons)
		{
			Lara.requestGunType = WEAPON_SHOTGUN;
			if (!Lara.gunStatus && Lara.gunType == WEAPON_SHOTGUN)
				Lara.gunStatus = LG_DRAW_GUNS;
		
			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
		}
		else
		{
			SayNo();
		}

		return;
	}

	// Grenade launcher
	if (objectNumber == ID_GRENADE_ITEM)
	{
		if (canUseWeapons)
		{
			Lara.requestGunType = WEAPON_GRENADE_LAUNCHER;
			if (!Lara.gunStatus && Lara.gunType == WEAPON_GRENADE_LAUNCHER)
				Lara.gunStatus = LG_DRAW_GUNS;

			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
		}
		else
		{
			SayNo();
		}

		return;
	}

	// Harpoon gun
	if (objectNumber == ID_HARPOON_ITEM)
	{
		if (canUseWeapons)
		{
			Lara.requestGunType = WEAPON_HARPOON_GUN;
			if (!Lara.gunStatus && Lara.gunType == WEAPON_HARPOON_GUN)
				Lara.gunStatus = LG_DRAW_GUNS;

			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
		}
		else
		{
			SayNo();
		}

		return;
	}

	// Crossbow/grappling gun
	if (objectNumber == ID_CROSSBOW_ITEM)
	{
		if (canUseWeapons)
		{
			Lara.requestGunType = WEAPON_CROSSBOW;
			if (!Lara.gunStatus && Lara.gunType == WEAPON_CROSSBOW)
				Lara.gunStatus = LG_DRAW_GUNS;
		
			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
		}
		else
		{
			SayNo();
		}

		return;
	}

	// HK
	if (objectNumber == ID_HK_ITEM)
	{
		if (canUseWeapons)
		{
			Lara.requestGunType = WEAPON_HK;
			if (!Lara.gunStatus && Lara.gunType == WEAPON_HK)
				Lara.gunStatus = LG_DRAW_GUNS;
		
			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
		}
		else
		{
			SayNo();
		}

		return;
	}

	// Flares
	if (objectNumber == ID_FLARE_INV_ITEM)
	{
		if (!Lara.gunStatus)
		{
			if (LaraItem->currentAnimState != STATE_LARA_CRAWL_IDLE &&
				LaraItem->currentAnimState != STATE_LARA_CRAWL_FORWARD &&
				LaraItem->currentAnimState != STATE_LARA_CRAWL_TURN_LEFT &&
				LaraItem->currentAnimState != STATE_LARA_CRAWL_TURN_RIGHT &&
				LaraItem->currentAnimState != STATE_LARA_CRAWL_BACK &&
				LaraItem->currentAnimState != STATE_LARA_CRAWL_TO_CLIMB && 
				Lara.waterStatus == LW_ABOVE_WATER)
			{
				if (Lara.gunType != WEAPON_FLARE)
				{
					TrInput = 0x80000;
					LaraGun();
					TrInput = 0;

					SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				}
			}
		}
		else
		{
			SayNo();
		}

		return;
	}

	SayNo();
	return;
}

float Inventory::GetVerticalOffset()
{
	return m_movement;
}

void Inventory::InitialiseTitle()
{
	InventoryRing* ring = &m_rings[INV_RING_OPTIONS];

	// Reset the objects in inventory
	ring->numObjects = 0;
	ring->movement = 0;
	ring->currentObject = 0;
	ring->focusState = INV_FOCUS_STATE_NONE;

	for (__int32 j = 0; j < NUM_INVENTORY_OBJECTS_PER_RING; j++)
	{
		ring->objects[j].inventoryObject = -1;
		ring->objects[j].rotation = 0;
		ring->objects[j].scale = INV_OBJECT_SCALE;
	}

	InsertObject(INV_RING_OPTIONS, INV_OBJECT_PASSAPORT);
	InsertObject(INV_RING_OPTIONS, INV_OBJECT_POLAROID);
	InsertObject(INV_RING_OPTIONS, INV_OBJECT_SUNGLASSES);
	InsertObject(INV_RING_OPTIONS, INV_OBJECT_HEADPHONES);
	InsertObject(INV_RING_OPTIONS, INV_OBJECT_KEYS);

	m_activeRing = INV_RING_OPTIONS;
	m_deltaMovement = 0;
	m_movement = INV_MOVE_STOPPED;
	m_type = INV_TYPE_TITLE;
	InventoryItemChosen = -1;
}

__int32 Inventory::DoTitleInventory()
{
	InitialiseTitle();

	m_rings[INV_RING_PUZZLES].draw = false;
	m_rings[INV_RING_WEAPONS].draw = false;
	m_rings[INV_RING_OPTIONS].draw = true;

	InventoryRing* ring = &m_rings[INV_RING_OPTIONS];
	m_activeRing = INV_RING_OPTIONS;

	// Fade in
	g_Renderer->FadeIn();
	for (__int32 i = 0; i < FADE_FRAMES_COUNT; i++)
		g_Renderer->DrawInventory();

	__int32 result = INV_RESULT_NONE;

	while (!ResetFlag)
	{
		SetDebounce = true;
		S_UpdateInput();
		SetDebounce = false;

		GameTimer++;

		// Handle input
		if (DbInput & IN_LEFT)
		{
			SoundEffect(SFX_MENU_ROTATE, NULL, 0);

			// Change object right
			float deltaAngle = 360.0f / ring->numObjects / 8.0f;
			ring->movement = 0;

			for (__int32 i = 0; i < 8; i++)
			{
				ring->movement += deltaAngle;

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}

			if (ring->currentObject == ring->numObjects - 1)
				ring->currentObject = 0;
			else
				ring->currentObject++;

			ring->movement = 0;
		}
		else if (DbInput & IN_RIGHT)
		{
			SoundEffect(SFX_MENU_ROTATE, NULL, 0);

			// Change object left
			float deltaAngle = 360.0f / ring->numObjects / 8.0f;
			ring->movement = 0;

			for (__int32 i = 0; i < 8; i++)
			{
				ring->movement -= deltaAngle;

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}

			if (ring->currentObject == 0)
				ring->currentObject = ring->numObjects - 1;
			else
				ring->currentObject--;

			ring->movement = 0;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_MENU_SELECT, NULL, 0);

			if (ring->objects[ring->currentObject].inventoryObject == INV_OBJECT_PASSAPORT)
			{
				__int32 passportResult = DoPassport();
				if (passportResult == INV_RESULT_NEW_GAME ||
					passportResult == INV_RESULT_EXIT_GAME ||
					passportResult == INV_RESULT_LOAD_GAME)
				{
					// Fade out
					g_Renderer->FadeOut();
					for (__int32 i = 0; i < FADE_FRAMES_COUNT; i++)
						g_Renderer->DrawInventory();

					return passportResult;
				}
			}

			if (ring->objects[ring->currentObject].inventoryObject == INV_OBJECT_KEYS)
				DoControlsSettings();

			if (ring->objects[ring->currentObject].inventoryObject == INV_OBJECT_SUNGLASSES)
				DoGraphicsSettings();

			if (ring->objects[ring->currentObject].inventoryObject == INV_OBJECT_HEADPHONES)
				DoSoundSettings();
		}

		g_Renderer->DrawInventory();
		g_Renderer->SyncRenderer();
	}

	// Fade out
	g_Renderer->FadeOut();
	for (__int32 i = 0; i < FADE_FRAMES_COUNT; i++)
		g_Renderer->DrawInventory();

	return result;
}

InventoryObjectDefinition* Inventory::GetInventoryObject(__int32 index)
{
	return &m_objectsTable[index];
}

__int32 Inventory::DoPassport()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->frameIndex = 0;

	__int16 choice = 0;
	
	vector<__int32> choices;
	if (m_type == INV_TYPE_TITLE)
	{
		choices.push_back(INV_WHAT_PASSPORT_NEW_GAME);
		choices.push_back(INV_WHAT_PASSPORT_SELECT_LEVEL);
		choices.push_back(INV_WHAT_PASSPORT_LOAD_GAME);
		choices.push_back(INV_WHAT_PASSPORT_EXIT_GAME);
	}
	else
	{
		choices.push_back(INV_WHAT_PASSPORT_NEW_GAME);
		choices.push_back(INV_WHAT_PASSPORT_LOAD_GAME);
		if (LaraItem->hitPoints > 0 || CurrentLevel == 0)
			choices.push_back(INV_WHAT_PASSPORT_SAVE_GAME);
		choices.push_back(INV_WHAT_PASSPORT_EXIT_TO_TITLE);
	}

	ring->passportAction = choices[0];

	PopupObject();

	// Open the passport
	for (__int32 i = 0; i < 14; i++)
	{
		g_Renderer->DrawInventory();
		g_Renderer->SyncRenderer();
		ring->frameIndex++;
	}

	bool moveLeft = false;
	bool moveRight = false;
	bool closePassport = false;

	__int32 result = INV_RESULT_NONE;

	// Do the passport
	while (true)
	{
		// Handle input
		SetDebounce = true;
		S_UpdateInput();
		SetDebounce = false;

		GameTimer++;

		// Handle input
		if (DbInput & IN_DESELECT || closePassport)
		{
			moveLeft = false;
			moveRight = false;
			closePassport = false;

			break;
		}
		else if (DbInput & IN_LEFT || moveLeft)
		{
			moveLeft = false;
			moveRight = false;
			closePassport = false;

			if (choice > 0)
			{
				ring->frameIndex = 19;
				for (__int32 i = 0; i < 5; i++)
				{
					g_Renderer->DrawInventory();
					g_Renderer->SyncRenderer();
					ring->frameIndex--;
				}

				choice--;
			}
		}
		else if (DbInput & IN_RIGHT || moveRight)
		{
			moveLeft = false;
			moveRight = false;
			closePassport = false;
			
			if (choice < choices.size() - 1)
			{
				ring->frameIndex = 14;
				for (__int32 i = 0; i < 5; i++)
				{
					g_Renderer->DrawInventory();
					g_Renderer->SyncRenderer();
					ring->frameIndex++;
				}

				choice++;
			}
		}

		if (choices[choice] == INV_WHAT_PASSPORT_LOAD_GAME)
		{
			// Load game
			__int32 selectedSavegame = 0;
			while (true)
			{
				SetDebounce = 1;
				S_UpdateInput();
				SetDebounce = 0;

				// Process input
				if (DbInput & IN_DESELECT)
				{
					if (CurrentLevel == 0 || LaraItem->hitPoints > 0)
					{
						moveLeft = false;
						moveRight = false;
						closePassport = true;
					}

					break;
				}
				else if (DbInput & IN_FORWARD && selectedSavegame > 0)
				{
					SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
					selectedSavegame--;
					continue;
				}
				else if (DbInput & IN_BACK && selectedSavegame < MAX_SAVEGAMES - 1)
				{
					SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
					selectedSavegame++;
					continue;
				}
				else if (DbInput & IN_LEFT)
				{
					moveLeft = true;
					moveRight = false;
					closePassport = false;

					break;
				}
				else if (DbInput & IN_RIGHT)
				{
					moveLeft = false;
					moveRight = true;
					closePassport = false;

					break;
				}
				else if (DbInput & IN_SELECT)
				{
					SoundEffect(SFX_MENU_SELECT, NULL, 0);

					//ReadSavegame(selectedSavegame);
					g_GameFlow->SelectedSaveGame = selectedSavegame;
					result = INV_RESULT_LOAD_GAME;
					moveLeft = false;
					moveRight = false;
					closePassport = true;

					break;
				}

				ring->selectedIndex = selectedSavegame;
				ring->passportAction = INV_WHAT_PASSPORT_LOAD_GAME;

				LoadSavegameInfos();

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}
		}
		else if (choices[choice] == INV_WHAT_PASSPORT_SAVE_GAME)
		{
			// Save game
			__int32 selectedSavegame = 0;
			while (true)
			{
				SetDebounce = 1;
				S_UpdateInput();
				SetDebounce = 0;

				// Process input
				if (DbInput & IN_DESELECT)
				{
					if (CurrentLevel == 0 || LaraItem->hitPoints > 0)
					{
						moveLeft = false;
						moveRight = false;
						closePassport = true;
					}

					break;
				}
				else if (DbInput & IN_FORWARD && selectedSavegame > 0)
				{
					SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
					selectedSavegame--;
					continue;
				}
				else if (DbInput & IN_BACK && selectedSavegame < MAX_SAVEGAMES - 1)
				{
					SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
					selectedSavegame++;
					continue;
				}
				else if (DbInput & IN_LEFT)
				{
					moveLeft = true;
					moveRight = false;
					closePassport = false;

					break;
				}
				else if (DbInput & IN_RIGHT)
				{
					moveLeft = false;
					moveRight = true;
					closePassport = false;

					break;
				}
				else if (DbInput & IN_SELECT)
				{
					SoundEffect(SFX_MENU_SELECT, NULL, 0);

					// Use the new savegame system
					char fileName[255];
					ZeroMemory(fileName, 255);
					sprintf(fileName, "savegame.%d", selectedSavegame);
					SaveGame::Save(fileName);

					moveLeft = false;
					moveRight = false;
					closePassport = true;

					break;
				}

				ring->selectedIndex = selectedSavegame;
				ring->passportAction = INV_WHAT_PASSPORT_SAVE_GAME;

				LoadSavegameInfos();

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}
		}
		else if (choices[choice] == INV_WHAT_PASSPORT_SELECT_LEVEL)
		{
			// Save game
			__int32 selectedLevel = 0;
			while (true)
			{
				SetDebounce = 1;
				S_UpdateInput();
				SetDebounce = 0;

				// Process input
				if (DbInput & IN_DESELECT)
				{
					if (CurrentLevel == 0 || LaraItem->hitPoints > 0)
					{
						moveLeft = false;
						moveRight = false;
						closePassport = true;
					}

					break;
				}
				else if (DbInput & IN_FORWARD && selectedLevel > 0)
				{
					SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
					selectedLevel--;
					continue;
				}
				else if (DbInput & IN_BACK && selectedLevel < g_GameFlow->GetNumLevels() - 1)
				{
					SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
					selectedLevel++;
					continue;
				}
				else if (DbInput & IN_LEFT)
				{
					moveLeft = true;
					moveRight = false;
					closePassport = false;

					break;
				}
				else if (DbInput & IN_RIGHT)
				{
					moveLeft = false;
					moveRight = true;
					closePassport = false;

					break;
				}
				else if (DbInput & IN_SELECT)
				{
					SoundEffect(SFX_MENU_SELECT, NULL, 0);

					result = INV_RESULT_NEW_GAME;
					g_GameFlow->SelectedLevelForNewGame = selectedLevel + 1;

					moveLeft = false;
					moveRight = false;
					closePassport = true;

					break;
				}

				ring->selectedIndex = selectedLevel;
				ring->passportAction = INV_WHAT_PASSPORT_SELECT_LEVEL;

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}
		}
		else if (choices[choice] == INV_WHAT_PASSPORT_NEW_GAME)
		{
			// New game
			while (true)
			{
				SetDebounce = 1;
				S_UpdateInput();
				SetDebounce = 0;

				// Process input
				if (DbInput & IN_DESELECT)
				{
					if (CurrentLevel == 0 || LaraItem->hitPoints > 0)
					{
						moveLeft = false;
						moveRight = false;
						closePassport = true;
					}

					break;
				}
				else if (DbInput & IN_LEFT)
				{
					moveLeft = true;
					moveRight = false;
					closePassport = false;

					break;
				}
				else if (DbInput & IN_RIGHT)
				{
					moveLeft = false;
					moveRight = true;
					closePassport = false;

					break;
				}
				else if (DbInput & IN_SELECT)
				{
					SoundEffect(SFX_MENU_SELECT, NULL, 0);

					result = INV_RESULT_NEW_GAME;
					moveLeft = false;
					moveRight = false;
					closePassport = true;

					break;
				}

				ring->passportAction = INV_WHAT_PASSPORT_NEW_GAME;

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}
		}
		else if (choices[choice] == INV_WHAT_PASSPORT_EXIT_GAME)
		{
			// Exit game
			while (true)
			{
				SetDebounce = 1;
				S_UpdateInput();
				SetDebounce = 0;

				// Process input
				if (DbInput & IN_DESELECT)
				{
					if (CurrentLevel == 0 || LaraItem->hitPoints > 0)
					{
						moveLeft = false;
						moveRight = false;
						closePassport = true;
					}

					break;
				}
				else if (DbInput & IN_LEFT)
				{
					moveLeft = true;
					moveRight = false;
					closePassport = false;

					break;
				}
				else if (DbInput & IN_RIGHT)
				{
					moveLeft = false;
					moveRight = true;
					closePassport = false;

					break;
				}
				else if (DbInput & IN_SELECT)
				{
					SoundEffect(SFX_MENU_SELECT, NULL, 0);

					result = INV_RESULT_EXIT_GAME;
					moveLeft = false;
					moveRight = false;
					closePassport = true;

					break;
				}

				ring->passportAction = INV_WHAT_PASSPORT_EXIT_GAME;

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}
		}
		else if (choices[choice] == INV_WHAT_PASSPORT_EXIT_TO_TITLE)
		{
			// Exit game
			while (true)
			{
				SetDebounce = 1;
				S_UpdateInput();
				SetDebounce = 0;

				// Process input
				if (DbInput & IN_DESELECT)
				{
					if (CurrentLevel == 0 || LaraItem->hitPoints > 0)
					{
						moveLeft = false;
						moveRight = false;
						closePassport = true;
					}

					break;
				}
				else if (DbInput & IN_LEFT)
				{
					moveLeft = true;
					moveRight = false;
					closePassport = false;

					break;
				}
				else if (DbInput & IN_RIGHT)
				{
					moveLeft = false;
					moveRight = true;
					closePassport = false;

					break;
				}
				else if (DbInput & IN_SELECT)
				{
					SoundEffect(SFX_MENU_SELECT, NULL, 0);

					result = INV_RESULT_EXIT_TO_TILE;
					moveLeft = false;
					moveRight = false;
					closePassport = true;

					break;
				}

				ring->passportAction = INV_WHAT_PASSPORT_EXIT_TO_TITLE;

				g_Renderer->DrawInventory();
				g_Renderer->SyncRenderer();
			}
		}
		else
		{
			g_Renderer->DrawInventory();
			g_Renderer->SyncRenderer();
		}
	}

	// Close the passport
	ring->frameIndex = 24;
	for (__int32 i = 24; i < 30; i++)
	{
		g_Renderer->DrawInventory();
		g_Renderer->SyncRenderer();
		ring->frameIndex++;
	}

	ring->frameIndex = 0;

	PopoverObject();

	return result;
}

__int32	Inventory::PopupObject()
{
	InventoryRing* ring = &m_rings[m_activeRing];

	__int32 steps = 8;
	__int32 deltaAngle = (0 - ring->objects[ring->currentObject].rotation) / steps;
	float deltaScale = INV_OBJECT_SCALE / (float)steps;

	ring->focusState = INV_FOCUS_STATE_POPUP;
	for (__int32 i = 0; i < steps; i++)
	{
		g_Renderer->DrawInventory();
		g_Renderer->SyncRenderer();

		ring->objects[ring->currentObject].rotation += deltaAngle;
		ring->objects[ring->currentObject].scale += deltaScale;
	}
	ring->focusState = INV_FOCUS_STATE_FOCUSED;

	return 0;
}

__int32	Inventory::PopoverObject()
{
	InventoryRing* ring = &m_rings[m_activeRing];

	__int32 steps = 8;
	__int32 deltaAngle = (0 - ring->objects[ring->currentObject].rotation) / steps;
	float deltaScale = INV_OBJECT_SCALE / (float)steps;

	ring->focusState = INV_FOCUS_STATE_POPOVER;
	for (__int32 i = 0; i < steps; i++)
	{
		g_Renderer->DrawInventory();
		g_Renderer->SyncRenderer();

		ring->objects[ring->currentObject].rotation -= deltaAngle;
		ring->objects[ring->currentObject].scale -= deltaScale;
	}
	ring->focusState = INV_FOCUS_STATE_NONE;

	return 0;
}

__int32 Inventory::GetType()
{
	return m_type;
}

void Inventory::DoControlsSettings()
{

}

void Inventory::DoGraphicsSettings()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->frameIndex = 0;
	ring->selectedIndex = 0;

	PopupObject();

	// Copy configuration to a temporary object
	memcpy(&ring->Configuration, &g_Configuration, sizeof(GameConfiguration));

	// Get current display mode
	vector<RendererVideoAdapter>* adapters = g_Renderer->GetAdapters();
	RendererVideoAdapter* adapter = &(*adapters)[ring->Configuration.Adapter];
	ring->SelectedVideoMode = 0;
	for (__int32 i = 0; i < adapter->DisplayModes.size(); i++)
	{
		RendererDisplayMode* mode = &adapter->DisplayModes[i];
		if (mode->Width == ring->Configuration.Width && mode->Height == ring->Configuration.Height && 
			mode->RefreshRate == ring->Configuration.RefreshRate)
		{
			ring->SelectedVideoMode = i;
			break;
		}
	}

	bool closeObject = false;

	// Do the menu
	while (true)
	{
		// Handle input
		SetDebounce = true;
		S_UpdateInput();
		SetDebounce = false;

		GameTimer++;

		// Handle input
		if (DbInput & IN_DESELECT || closeObject)
		{
			closeObject = true;
			break;
		}
		else if (DbInput & IN_LEFT)
		{
			closeObject = false;

			switch (ring->selectedIndex)
			{
			case INV_DISPLAY_RESOLUTION:
				SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				if (ring->SelectedVideoMode > 0)
					ring->SelectedVideoMode--;

				break;

			case INV_DISPLAY_SHADOWS:
				SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableShadows = !ring->Configuration.EnableShadows;
				break;

			case INV_DISPLAY_CAUSTICS:
				SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableCaustics = !ring->Configuration.EnableCaustics;
				break;

			case INV_DISPLAY_VOLUMETRIC_FOG:
				SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableVolumetricFog = !ring->Configuration.EnableVolumetricFog;
				break;
			}
		}
		else if (DbInput & IN_RIGHT)
		{
			closeObject = false;

			switch (ring->selectedIndex)
			{
			case INV_DISPLAY_RESOLUTION:
				SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				//if (ring->SelectedVideoMode < adapter->DisplayModes.size() - 1)
					ring->SelectedVideoMode++;
				break;

			case INV_DISPLAY_SHADOWS:
				SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableShadows = !ring->Configuration.EnableShadows;
				break;

			case INV_DISPLAY_CAUSTICS:
				SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableCaustics = !ring->Configuration.EnableCaustics;
				break;

			case INV_DISPLAY_VOLUMETRIC_FOG:
				SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableVolumetricFog = !ring->Configuration.EnableVolumetricFog;
				break;
			}
		}
		else if (DbInput & IN_FORWARD)
		{
			closeObject = false;

			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex > 0)
				ring->selectedIndex--;
		}
		else if (DbInput & IN_BACK)
		{
			closeObject = false;

			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex < INV_DISPLAY_COUNT)
				ring->selectedIndex++;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_MENU_SELECT, NULL, 0);

			if (ring->selectedIndex == INV_DISPLAY_APPLY)
			{
				// Save the configuration
				RendererDisplayMode* mode = &adapter->DisplayModes[ring->SelectedVideoMode];
				ring->Configuration.Width = mode->Width;
				ring->Configuration.Height = mode->Height;
				ring->Configuration.RefreshRate = mode->RefreshRate;
				memcpy(&g_Configuration, &ring->Configuration, sizeof(GameConfiguration));
				SaveConfiguration();

				// Reset screen and go back
				g_Renderer->ChangeScreenResolution(ring->Configuration.Width, ring->Configuration.Height, 
												   ring->Configuration.RefreshRate, ring->Configuration.Windowed);
				closeObject = true;

				break;
			}
			else if (ring->selectedIndex == INV_DISPLAY_CANCEL)
			{
				SoundEffect(SFX_MENU_SELECT, NULL, 0);

				closeObject = true;
				break;
			}
			else
			{

			}
		}

		g_Renderer->DrawInventory();
		g_Renderer->SyncRenderer();
	}

	PopoverObject();
}

void Inventory::DoSoundSettings()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->frameIndex = 0;
	ring->selectedIndex = 0;

	PopupObject();

	// Copy configuration to a temporary object
	memcpy(&ring->Configuration, &g_Configuration, sizeof(GameConfiguration));

	// Open the passport
	for (__int32 i = 0; i < 14; i++)
	{
		g_Renderer->DrawInventory();
		g_Renderer->SyncRenderer();
	}

	bool closeObject = false;
	__int32 oldVolume = ring->Configuration.MusicVolume;
	__int32 oldSfxVolume = ring->Configuration.SfxVolume;
	bool wasSoundEnabled = ring->Configuration.EnableSound;

	// Do the passport
	while (true)
	{
		// Handle input
		SetDebounce = true;
		S_UpdateInput();
		SetDebounce = false;

		GameTimer++;

		// Handle input
		if (DbInput & IN_DESELECT || closeObject)
		{
			closeObject = true;
			GlobalMusicVolume = oldVolume;
			GlobalFXVolume = oldSfxVolume;

			break;
		}
		else if (DbInput & IN_LEFT)
		{
			closeObject = false;

			switch (ring->selectedIndex)
			{
			case INV_SOUND_ENABLED:
				SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableSound = !ring->Configuration.EnableSound;

				break;

			case INV_SOUND_SPECIAL_EFFECTS:
				SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableAudioSpecialEffects = !ring->Configuration.EnableAudioSpecialEffects;
				break;

			case INV_SOUND_MUSIC_VOLUME:
				if (ring->Configuration.MusicVolume > 0)
				{
					ring->Configuration.MusicVolume--;
					GlobalMusicVolume = ring->Configuration.MusicVolume;
				}

				break;

			case INV_SOUND_SFX_VOLUME:
				if (ring->Configuration.SfxVolume > 0)
				{
					ring->Configuration.SfxVolume--;
					GlobalFXVolume = ring->Configuration.SfxVolume;
					SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				}

				break;
			}
		}
		else if (DbInput & IN_RIGHT)
		{
			closeObject = false;

			switch (ring->selectedIndex)
			{
			case INV_SOUND_ENABLED:
				SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableSound = !ring->Configuration.EnableSound;

				break;

			case INV_SOUND_SPECIAL_EFFECTS:
				SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableAudioSpecialEffects = !ring->Configuration.EnableAudioSpecialEffects;
				break;

			case INV_SOUND_MUSIC_VOLUME:
				if (ring->Configuration.MusicVolume < 100)
				{
					ring->Configuration.MusicVolume++;
					GlobalMusicVolume = ring->Configuration.MusicVolume;
				}

				break;

			case INV_SOUND_SFX_VOLUME:
				if (ring->Configuration.SfxVolume < 100)
				{
					ring->Configuration.SfxVolume++;
					GlobalFXVolume = ring->Configuration.SfxVolume;
					SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				}

				break;
			}
		}
		else if (DbInput & IN_FORWARD)
		{
			closeObject = false;

			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex > 0)
				ring->selectedIndex--;
		}
		else if (DbInput & IN_BACK)
		{
			closeObject = false;

			SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex < INV_DISPLAY_COUNT)
				ring->selectedIndex++;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_MENU_SELECT, NULL, 0);

			if (ring->selectedIndex == INV_DISPLAY_APPLY)
			{
				// Save the configuration
				GlobalMusicVolume = ring->Configuration.MusicVolume;
				GlobalFXVolume = ring->Configuration.SfxVolume;
				memcpy(&g_Configuration, &ring->Configuration, sizeof(GameConfiguration));
				SaveConfiguration();

				// Init or deinit the sound system
				if (wasSoundEnabled && !g_Configuration.EnableSound)
					Sound_DeInit();
				else if (!wasSoundEnabled && g_Configuration.EnableSound)
					Sound_Init();

				closeObject = true;

				break;
			}
			else if (ring->selectedIndex == INV_DISPLAY_CANCEL)
			{
				SoundEffect(SFX_MENU_SELECT, NULL, 0);

				closeObject = true;
				GlobalMusicVolume = oldVolume;
				GlobalFXVolume = oldSfxVolume;

				break;
			}
			else
			{

			}
		}

		g_Renderer->DrawInventory();
		g_Renderer->SyncRenderer();
	}

	PopoverObject();
}

__int32 Inventory::GetActiveGui()
{
	return m_activeGui;
}

/*InventoryRing* Inventory::GetSecondaryRing()
{
	return m_secondaryRing;
}
**/

bool Inventory::IsObjectSeparable(__int16 object)
{
	return false;
}