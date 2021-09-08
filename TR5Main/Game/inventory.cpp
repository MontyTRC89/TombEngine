#include "framework.h"
#ifndef NEW_INV
#include "inventory.h"
#include "draw.h"
#include "control.h"
#include "lara_fire.h"
#include "gameflow.h"
#include "Sound\sound.h"
#include "savegame.h"
#include "Lara.h"
#include "camera.h"
#include "spotcam.h"

#include "input.h"
#include "configuration.h"
#include "lara_one_gun.h"
#include "lara_two_guns.h"
#include "level.h"
#include "input.h"
using namespace TEN::Renderer;
using std::vector;
Inventory g_Inventory;
extern GameFlow* g_GameFlow;

void CombinePuzzle(int action, short object)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.Puzzles[object - _INV_OBJECT_PUZZLE1]++;
		Lara.PuzzlesCombo[object - _INV_OBJECT_PUZZLE1_COMBO1]--;
		Lara.PuzzlesCombo[object - _INV_OBJECT_PUZZLE1_COMBO1 + 1]--;
	}
	else
	{
		Lara.Puzzles[object - _INV_OBJECT_PUZZLE1]--;
		Lara.PuzzlesCombo[object - _INV_OBJECT_PUZZLE1_COMBO1]++;
		Lara.PuzzlesCombo[object - _INV_OBJECT_PUZZLE1_COMBO1 + 1]++;
	}
}

void CombineKey(int action, short object)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.Keys[object - _INV_OBJECT_KEY1]++;
		Lara.KeysCombo[object - _INV_OBJECT_KEY1_COMBO1]--;
		Lara.KeysCombo[object - _INV_OBJECT_KEY1_COMBO1 + 1]--;
	}
	else
	{
		Lara.Keys[object - _INV_OBJECT_KEY1]--;
		Lara.KeysCombo[object - _INV_OBJECT_KEY1_COMBO1]++;
		Lara.KeysCombo[object - _INV_OBJECT_KEY1_COMBO1 + 1]++;
	}
}

void CombinePickup(int action, short object)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.Pickups[object - _INV_OBJECT_PICKUP1]++;
		Lara.PickupsCombo[object - _INV_OBJECT_PICKUP1_COMBO1]--;
		Lara.PickupsCombo[object - _INV_OBJECT_PICKUP1_COMBO1 + 1] = false;
	}
	else
	{
		Lara.Pickups[object - _INV_OBJECT_PICKUP1]--;
		Lara.PickupsCombo[object - _INV_OBJECT_PICKUP1_COMBO1]++;
		Lara.PickupsCombo[object - _INV_OBJECT_PICKUP1_COMBO1 + 1]++;
	}
}

void CombineExamine(int action, short object)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.Examines[object - _INV_OBJECT_EXAMINE1]++;
		Lara.ExaminesCombo[object - _INV_OBJECT_EXAMINE1_COMBO1]--;
		Lara.ExaminesCombo[object - _INV_OBJECT_EXAMINE1_COMBO1 + 1]--;
	}
	else
	{
		Lara.Examines[object - _INV_OBJECT_EXAMINE1]--;
		Lara.ExaminesCombo[object - _INV_OBJECT_EXAMINE1_COMBO1]++;
		Lara.ExaminesCombo[object - _INV_OBJECT_EXAMINE1_COMBO1 + 1]++;
	}
}

void CombineRevolverLasersight(int action, short object)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.Lasersight = false;
		Lara.Weapons[WEAPON_REVOLVER].HasLasersight = true;
	}
	else
	{
		Lara.Lasersight = true;
		Lara.Weapons[WEAPON_REVOLVER].HasLasersight = false;
	}

	if (Lara.gunStatus && Lara.gunType == WEAPON_REVOLVER)
	{
		undraw_pistol_mesh_right(WEAPON_REVOLVER);
		draw_pistol_meshes(WEAPON_REVOLVER);
	}
}

void CombineCrossbowLasersight(int action, short object)
{
	if (action == INV_COMBINE_COMBINE)
	{
		Lara.Lasersight = false;
		Lara.Weapons[WEAPON_CROSSBOW].HasLasersight = true;
	}
	else
	{
		Lara.Lasersight = true;
		Lara.Weapons[WEAPON_CROSSBOW].HasLasersight = false;
	}

	if (Lara.gunStatus && Lara.gunType == WEAPON_CROSSBOW)
	{
		undraw_shotgun_meshes(WEAPON_CROSSBOW);
		draw_shotgun_meshes(WEAPON_CROSSBOW);
	}
}

Inventory::Inventory()
{
	ZeroMemory(&m_objectsTable[0], sizeof(InventoryObjectDefinition) * _INVENTORY_TABLE_SIZE);

	// Create objects table
	m_objectsTable[_INV_OBJECT_UZIS] = InventoryObjectDefinition(ID_UZI_ITEM, STRING_UZI, -1, 0);
	m_objectsTable[_INV_OBJECT_PISTOLS] = InventoryObjectDefinition(ID_PISTOLS_ITEM, STRING_PISTOLS, -1, 0);
	m_objectsTable[_INV_OBJECT_SHOTGUN] = InventoryObjectDefinition(ID_SHOTGUN_ITEM, STRING_SHOTGUN, -1, 0);
	m_objectsTable[_INV_OBJECT_REVOLVER] = InventoryObjectDefinition(ID_REVOLVER_ITEM, STRING_REVOLVER, -1, 0);
	m_objectsTable[_INV_OBJECT_REVOLVER_LASER] = InventoryObjectDefinition(ID_REVOLVER_ITEM, STRING_REVOLVER_LASER, -1, 0);
	m_objectsTable[_INV_OBJECT_HK] = InventoryObjectDefinition(ID_HK_ITEM, STRING_HK, -1, 0);
	m_objectsTable[_INV_OBJECT_SHOTGUN_AMMO1] = InventoryObjectDefinition(ID_SHOTGUN_AMMO1_ITEM, STRING_SHOTGUN_AMMO1, -1, 0);
	m_objectsTable[_INV_OBJECT_SHOTGUN_AMMO2] = InventoryObjectDefinition(ID_SHOTGUN_AMMO2_ITEM, STRING_SHOTGUN_AMMO2, -1, 0);
	m_objectsTable[_INV_OBJECT_HK_AMMO1] = InventoryObjectDefinition(ID_HK_AMMO_ITEM, STRING_HK_AMMO, -1, 0);
	m_objectsTable[_INV_OBJECT_REVOLVER_AMMO] = InventoryObjectDefinition(ID_REVOLVER_AMMO_ITEM, STRING_REVOLVER_AMMO, -1, 0);
	m_objectsTable[_INV_OBJECT_UZI_AMMO] = InventoryObjectDefinition(ID_UZI_AMMO_ITEM, STRING_UZI_AMMO, -1, 0);
	m_objectsTable[_INV_OBJECT_PISTOLS_AMMO] = InventoryObjectDefinition(ID_PISTOLS_AMMO_ITEM, STRING_PISTOLS_AMMO, -1, 0);
	m_objectsTable[_INV_OBJECT_LASERSIGHT] = InventoryObjectDefinition(ID_LASERSIGHT_ITEM, STRING_LASERSIGHT, -1, 0);
	m_objectsTable[_INV_OBJECT_SILENCER] = InventoryObjectDefinition(ID_SILENCER_ITEM, STRING_SILENCER, -1, 0);
	m_objectsTable[_INV_OBJECT_LARGE_MEDIPACK] = InventoryObjectDefinition(ID_BIGMEDI_ITEM, STRING_LARGE_MEDIPACK, -1, 0x8000);
	m_objectsTable[_INV_OBJECT_SMALL_MEDIPACK] = InventoryObjectDefinition(ID_SMALLMEDI_ITEM, STRING_SMALL_MEDIPACK, -1, 0);
	m_objectsTable[_INV_OBJECT_BINOCULARS] = InventoryObjectDefinition(ID_BINOCULARS_ITEM, STRING_BINOCULARS, -1, 0);
	m_objectsTable[_INV_OBJECT_FLARES] = InventoryObjectDefinition(ID_FLARE_INV_ITEM, STRING_FLARES, -1, 0);
	m_objectsTable[_INV_OBJECT_TIMEX] = InventoryObjectDefinition(ID_COMPASS_ITEM, STRING_TIMEX, -1, 0);
	m_objectsTable[_INV_OBJECT_CROWBAR] = InventoryObjectDefinition(ID_CROWBAR_ITEM, STRING_CROWBAR, -1, 0);
	m_objectsTable[_INV_OBJECT_GRENADE_LAUNCHER] = InventoryObjectDefinition(ID_GRENADE_GUN_ITEM, STRING_GRENADE_LAUNCHER, -1, 0);
	m_objectsTable[_INV_OBJECT_GRENADE_AMMO1] = InventoryObjectDefinition(ID_GRENADE_AMMO1_ITEM, STRING_GRENADE_AMMO1, -1, 0);
	m_objectsTable[_INV_OBJECT_GRENADE_AMMO2] = InventoryObjectDefinition(ID_GRENADE_AMMO2_ITEM, STRING_GRENADE_AMMO2, -1, 0);
	m_objectsTable[_INV_OBJECT_GRENADE_AMMO3] = InventoryObjectDefinition(ID_GRENADE_AMMO3_ITEM, STRING_GRENADE_AMMO3, -1, 0);
	m_objectsTable[_INV_OBJECT_HARPOON_GUN] = InventoryObjectDefinition(ID_HARPOON_ITEM, STRING_HARPOON_GUN, -1, 0);
	m_objectsTable[_INV_OBJECT_HARPOON_AMMO] = InventoryObjectDefinition(ID_HARPOON_AMMO_ITEM, STRING_HARPOON_AMMO, -1, 0);
	m_objectsTable[_INV_OBJECT_ROCKET_LAUNCHER] = InventoryObjectDefinition(ID_ROCKET_LAUNCHER_ITEM, STRING_ROCKET_LAUNCHER, -1, 0);
	m_objectsTable[_INV_OBJECT_ROCKET_AMMO] = InventoryObjectDefinition(ID_ROCKET_LAUNCHER_AMMO_ITEM, STRING_ROCKET_AMMO, -1, 0);
	m_objectsTable[_INV_OBJECT_CROSSBOW] = InventoryObjectDefinition(ID_CROSSBOW_ITEM, STRING_CROSSBOW, -1, 0);
	m_objectsTable[_INV_OBJECT_CROSSBOW_LASER] = InventoryObjectDefinition(ID_CROSSBOW_ITEM, STRING_CROSSBOW_LASER, -1, 0);
	m_objectsTable[_INV_OBJECT_CROSSBOW_AMMO1] = InventoryObjectDefinition(ID_CROSSBOW_AMMO1_ITEM, STRING_CROSSBOW_AMMO1, -1, 0);
	m_objectsTable[_INV_OBJECT_CROSSBOW_AMMO2] = InventoryObjectDefinition(ID_CROSSBOW_AMMO2_ITEM, STRING_CROSSBOW_AMMO2, -1, 0);
	m_objectsTable[_INV_OBJECT_CROSSBOW_AMMO3] = InventoryObjectDefinition(ID_CROSSBOW_AMMO3_ITEM, STRING_CROSSBOW_AMMO3, -1, 0);
	m_objectsTable[_INV_OBJECT_PASSPORT] = InventoryObjectDefinition(ID_INVENTORY_PASSPORT, STRING_PASSPORT, -1, 0);
	m_objectsTable[_INV_OBJECT_KEYS] = InventoryObjectDefinition(ID_INVENTORY_KEYS, STRING_CONTROLS, -1, 0x4000);
	m_objectsTable[_INV_OBJECT_SUNGLASSES] = InventoryObjectDefinition(ID_INVENTORY_SUNGLASSES, STRING_DISPLAY, -1, 0);
	m_objectsTable[_INV_OBJECT_POLAROID] = InventoryObjectDefinition(ID_INVENTORY_POLAROID, STRING_LARA_HOME, -1, 0);
	m_objectsTable[_INV_OBJECT_HEADPHONES] = InventoryObjectDefinition(ID_INVENTORY_HEADPHONES, STRING_SOUND, -1, 0);
	m_objectsTable[_INV_OBJECT_DIARY] = InventoryObjectDefinition(ID_DIARY_ITEM, STRING_DIARY, -1, 0);
	m_objectsTable[_INV_OBJECT_WATERSKIN1] = InventoryObjectDefinition(ID_WATERSKIN1_EMPTY, STRING_WATERSKIN1_EMPTY, -1, 0);
	m_objectsTable[_INV_OBJECT_WATERSKIN2] = InventoryObjectDefinition(ID_WATERSKIN2_EMPTY, STRING_WATERSKIN2_EMPTY, -1, 0);

	for (int i = 0; i < NUM_PUZZLES; i++)
		m_objectsTable[_INV_OBJECT_PUZZLE1 + i] = InventoryObjectDefinition(ID_PUZZLE_ITEM1 + i, STRING_PISTOLS, -1, 0);

	for (int i = 0; i < NUM_PUZZLES * 2; i++)
		m_objectsTable[_INV_OBJECT_PUZZLE1_COMBO1 + i] = InventoryObjectDefinition(ID_PUZZLE_ITEM1_COMBO1 + i, STRING_PISTOLS, -1, 0);

	for (int i = 0; i < NUM_KEYS; i++)
		m_objectsTable[_INV_OBJECT_KEY1 + i] = InventoryObjectDefinition(ID_KEY_ITEM1 + i, STRING_PISTOLS, -1, 0);

	for (int i = 0; i < NUM_KEYS * 2; i++)
		m_objectsTable[_INV_OBJECT_KEY1_COMBO1 + i] = InventoryObjectDefinition(ID_KEY_ITEM1_COMBO1 + i, STRING_PISTOLS, -1, 0);

	for (int i = 0; i < NUM_PICKUPS; i++)
		m_objectsTable[_INV_OBJECT_PICKUP1 + i] = InventoryObjectDefinition(ID_PICKUP_ITEM1 + i, STRING_PISTOLS, -1, 0);

	for (int i = 0; i < 3; i++)
		m_objectsTable[_INV_OBJECT_EXAMINE1 + i] = InventoryObjectDefinition(ID_EXAMINE1 + i, STRING_PISTOLS, -1, 0);

	// Add combinations
	for (int i = 0; i < NUM_PUZZLES; i++)
		AddCombination(_INV_OBJECT_PUZZLE1_COMBO1 + 2 * i, _INV_OBJECT_PUZZLE1_COMBO2 + 2 * i, _INV_OBJECT_PUZZLE1 + i, CombinePuzzle);

	for (int i = 0; i < NUM_KEYS; i++)
		AddCombination(_INV_OBJECT_KEY1_COMBO1 + 2 * i, _INV_OBJECT_KEY1_COMBO2 + 2 * i, _INV_OBJECT_KEY1 + i, CombineKey);

	for (int i = 0; i < NUM_PICKUPS; i++)
		AddCombination(_INV_OBJECT_PICKUP1_COMBO1 + 2 * i, _INV_OBJECT_PICKUP1_COMBO2 + 2 * i, _INV_OBJECT_PICKUP1 + i, CombinePickup);

	for (int i = 0; i < NUM_EXAMINES; i++)
		AddCombination(_INV_OBJECT_EXAMINE1_COMBO1 + 2 * i, _INV_OBJECT_EXAMINE1_COMBO2 + 2 * i, _INV_OBJECT_EXAMINE1 + i, CombineExamine);

	AddCombination(_INV_OBJECT_REVOLVER, _INV_OBJECT_LASERSIGHT, _INV_OBJECT_REVOLVER_LASER, CombineRevolverLasersight);
	AddCombination(_INV_OBJECT_CROSSBOW, _INV_OBJECT_LASERSIGHT, _INV_OBJECT_CROSSBOW_LASER, CombineCrossbowLasersight);

	m_rings[INV_RING_PUZZLES].y = -INV_RINGS_OFFSET;
	m_rings[INV_RING_WEAPONS].y = 0;
	m_rings[INV_RING_OPTIONS].y = INV_RINGS_OFFSET;
	m_rings[INV_RING_CHOOSE_AMMO].y = 0;
	m_rings[INV_RING_COMBINE].y = 0;

	m_rings[INV_RING_PUZZLES].titleStringIndex = STRING_TITLE_PUZZLES;
	m_rings[INV_RING_WEAPONS].titleStringIndex = STRING_TITLE_ITEMS;
	m_rings[INV_RING_OPTIONS].titleStringIndex = STRING_TITLE_SETTINGS;
	m_rings[INV_RING_CHOOSE_AMMO].titleStringIndex = STRING_TITLE_CHOOSE_AMMO;
	m_rings[INV_RING_COMBINE].titleStringIndex = STRING_TITLE_COMBINE;
}

Inventory::~Inventory()
{

}

InventoryRing* Inventory::GetRing(int index)
{
	return &m_rings[index];
}

int Inventory::GetActiveRing()
{
	return m_activeRing;
}

void Inventory::SetActiveRing(int index)
{
	m_activeRing = index;
}

void Inventory::InsertObject(int ring, int objectNumber)
{
	m_rings[ring].objects[m_rings[ring].numObjects].inventoryObject = objectNumber;
	m_rings[ring].numObjects++;
}

void Inventory::LoadObjects(bool isReload)
{
	// Reset the objects in inventory
	for (int i = 0; i < NUM_INVENTORY_RINGS; i++)
	{
		m_rings[i].numObjects = 0;
		m_rings[i].rotation = 0;
		m_rings[i].currentObject = 0;
		m_rings[i].selectedIndex = 0;

		if (!isReload)
		{
			m_rings[i].focusState = INV_FOCUS_STATE_NONE;
		}

		for (int j = 0; j < NUM_INVENTORY_OBJECTS_PER_RING; j++)
		{
			m_rings[i].objects[j].inventoryObject = NO_ITEM;
			m_rings[i].objects[j].rotation = 0;
			m_rings[i].objects[j].scale = INV_OBJECTS_SCALE;
		}
	}

	/*Lara.Weapons[WEAPON_REVOLVER].Present = true;
	Lara.Weapons[WEAPON_REVOLVER].Ammo[0] = 1000;
	Lara.Weapons[WEAPON_REVOLVER].SelectedAmmo = WEAPON_AMMO1;
	Lara.Weapons[WEAPON_REVOLVER].HasLasersight = true;

	// DEBUG
	{
		/*Lara.Weapons[WEAPON_SHOTGUN].Present = true;
		Lara.Weapons[WEAPON_SHOTGUN].Ammo[0] = 1000;
		Lara.Weapons[WEAPON_SHOTGUN].Ammo[1] = 1000;
		Lara.Weapons[WEAPON_SHOTGUN].SelectedAmmo = WEAPON_AMMO1;

		Lara.Weapons[WEAPON_REVOLVER].Present = true;
		Lara.Weapons[WEAPON_REVOLVER].Ammo[0] = 1000;
		Lara.Weapons[WEAPON_REVOLVER].SelectedAmmo = WEAPON_AMMO1;

		Lara.laserSight = true;

		/*Lara.Weapons[WEAPON_CROSSBOW].Present = true;
		Lara.Weapons[WEAPON_CROSSBOW].Ammo[0] = 1000;
		Lara.Weapons[WEAPON_CROSSBOW].SelectedAmmo = WEAPON_AMMO1;

		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Present = true;
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[0] = 1000;
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].SelectedAmmo = WEAPON_AMMO1;

		Lara.Weapons[WEAPON_HARPOON_GUN].Present = true;
		Lara.Weapons[WEAPON_HARPOON_GUN].Ammo[0] = 1000;
	}*/

	// Now fill the rings
	if (g_GameFlow->GetLevel(CurrentLevel)->LaraType != LARA_YOUNG)
	{
		// Pistols
		if (Lara.Weapons[WEAPON_PISTOLS].Present)
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_PISTOLS);

		// Uzi
		if (Lara.Weapons[WEAPON_UZI].Present)
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_UZIS);
		else if (Lara.Weapons[WEAPON_UZI].Ammo[0])
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_UZI_AMMO);

		// Revolver
		if (Lara.Weapons[WEAPON_REVOLVER].Present)
		{
			if (Lara.Weapons[WEAPON_REVOLVER].HasLasersight)
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_REVOLVER_LASER);
			else
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_REVOLVER);
		}
		else
		{
			if (Lara.Weapons[WEAPON_REVOLVER].Ammo[0])
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_REVOLVER_AMMO);
		}

		// Shotgun
		if (Lara.Weapons[WEAPON_SHOTGUN].Present)
		{
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_SHOTGUN);
			//if (Lara.shotgunTypeCarried & 0x10)
			//	CurrentShotGunAmmoType = 1;
		}
		else
		{
			if (Lara.Weapons[WEAPON_SHOTGUN].Ammo[0])
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_SHOTGUN_AMMO1);
			if (Lara.Weapons[WEAPON_SHOTGUN].Ammo[1])
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_SHOTGUN_AMMO2);
		}

		// HK
		if (Lara.Weapons[WEAPON_HK].Present)
		{
			if (Lara.Weapons[WEAPON_HK].HasSilencer)
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_HK_LASER);
			else
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_HK);
		}
		else if (Lara.Weapons[WEAPON_HK].Ammo[0])
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_HK_AMMO1);

		// Crossbow
		if (Lara.Weapons[WEAPON_CROSSBOW].Present)
		{
			if (Lara.Weapons[WEAPON_CROSSBOW].HasLasersight)
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_CROSSBOW_LASER);
			else
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_CROSSBOW);
		}
		else
		{
			if (Lara.Weapons[WEAPON_CROSSBOW].Ammo[0])
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_CROSSBOW_AMMO1);
			if (Lara.Weapons[WEAPON_CROSSBOW].Ammo[1])
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_CROSSBOW_AMMO2);
			if (Lara.Weapons[WEAPON_CROSSBOW].Ammo[2])
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_CROSSBOW_AMMO3);
		}

		// Grenade launcher
		if (Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Present)
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_GRENADE_LAUNCHER);
		else
		{
			if (Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[0])
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_GRENADE_AMMO1);
			if (Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[1])
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_GRENADE_AMMO2);
			if (Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[2])
				InsertObject(INV_RING_WEAPONS, _INV_OBJECT_GRENADE_AMMO3);
		}

		// Harpoon
		if (Lara.Weapons[WEAPON_HARPOON_GUN].Present)
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_HARPOON_GUN);
		else if (Lara.Weapons[WEAPON_HARPOON_GUN].Ammo[0])
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_HARPOON_AMMO);

		// Rocket launcher
		if (Lara.Weapons[WEAPON_ROCKET_LAUNCHER].Present)
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_ROCKET_LAUNCHER);
		else if (Lara.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[0])
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_ROCKET_AMMO);

		// Lasersight
		if (Lara.Lasersight)
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_LASERSIGHT);

		// Silencer
		if (Lara.Silencer)
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_SILENCER);

		// Binoculars
		if (Lara.Binoculars)
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_BINOCULARS);

		// Flares
		if (Lara.NumFlares)
			InsertObject(INV_RING_WEAPONS, _INV_OBJECT_FLARES);
	}

	if (Lara.NumSmallMedipacks)
		InsertObject(INV_RING_WEAPONS, _INV_OBJECT_SMALL_MEDIPACK);

	if (Lara.NumLargeMedipacks)
		InsertObject(INV_RING_WEAPONS, _INV_OBJECT_LARGE_MEDIPACK);

	if (Lara.Crowbar)
		InsertObject(INV_RING_WEAPONS, _INV_OBJECT_CROWBAR);

	int i;
	for (i = 0; i < NUM_PUZZLES; i++)
	{
		if (Lara.Puzzles[i])
			InsertObject(INV_RING_PUZZLES, i + _INV_OBJECT_PUZZLE1);
	}

	for (i = 0; i < NUM_PUZZLES * 2; i++)
	{
		if (Lara.PuzzlesCombo[i])
			InsertObject(INV_RING_PUZZLES, i + _INV_OBJECT_PUZZLE1_COMBO1);
	}

	for (i = 0; i < NUM_KEYS; i++)
	{
		if (Lara.Keys[i])
			InsertObject(INV_RING_PUZZLES, i + _INV_OBJECT_KEY1);
	}

	for (i = 0; i < NUM_KEYS * 2; i++)
	{
		if (Lara.KeysCombo[i])
			InsertObject(INV_RING_PUZZLES, i + _INV_OBJECT_KEY1_COMBO1);
	}

	for (i = 0; i < NUM_PICKUPS; i++)
	{
		if (Lara.Pickups[i])
			InsertObject(INV_RING_PUZZLES, i + _INV_OBJECT_PICKUP1);
	}

	for (i = 0; i < NUM_PICKUPS * 2; i++)
	{
		if (Lara.PickupsCombo[i])
			InsertObject(INV_RING_PUZZLES, i + _INV_OBJECT_PICKUP1_COMBO1);
	}

	for (i = 0; i < NUM_EXAMINES; i++)
	{
		if (Lara.Examines[i])
			InsertObject(INV_RING_PUZZLES, i + _INV_OBJECT_EXAMINE1);
	}

	for (i = 0; i < NUM_EXAMINES * 2; i++)
	{
		if (Lara.ExaminesCombo[i])
			InsertObject(INV_RING_PUZZLES, i + _INV_OBJECT_EXAMINE1_COMBO1);
	}

	if (Lara.wetcloth == 2)
		InsertObject(INV_RING_PUZZLES, _INV_OBJECT_WETCLOTH1);

	if (Lara.wetcloth == 1)
		InsertObject(INV_RING_PUZZLES, _INV_OBJECT_WETCLOTH2);

	if (Lara.bottle)
		InsertObject(INV_RING_PUZZLES, _INV_OBJECT_BOTTLE);


	if (Lara.small_waterskin)
		InsertObject(INV_RING_PUZZLES, (Lara.small_waterskin - 1) + _INV_OBJECT_WATERSKIN1);

	if (Lara.big_waterskin)
		InsertObject(INV_RING_PUZZLES, (Lara.big_waterskin - 1) + _INV_OBJECT_WATERSKIN2);

	InventoryRing* ring = &m_rings[INV_RING_OPTIONS];

	// Reset the objects in inventory
	ring->numObjects = 0;
	ring->rotation = 0;
	ring->currentObject = 0;
	ring->focusState = INV_FOCUS_STATE_NONE;

	for (int j = 0; j < NUM_INVENTORY_OBJECTS_PER_RING; j++)
	{
		ring->objects[j].inventoryObject = -1;
		ring->objects[j].rotation = 0;
		ring->objects[j].scale = 2.0f;
	}

	InsertObject(INV_RING_OPTIONS, _INV_OBJECT_PASSPORT);
	InsertObject(INV_RING_OPTIONS, _INV_OBJECT_SUNGLASSES);
	InsertObject(INV_RING_OPTIONS, _INV_OBJECT_HEADPHONES);
	InsertObject(INV_RING_OPTIONS, _INV_OBJECT_KEYS);
}

void Inventory::SelectObject(int r, int object, float scale)
{
	if (object != NO_ITEM)
	{
		InventoryRing* ring = &m_rings[r];

		for (int i = 0; i < ring->numObjects; i++)
		{
			if (ring->objects[i].inventoryObject == object)
			{
				ring->currentObject = i;
				ring->objects[i].scale = scale;
				break;
			}
		}
	}
}

void Inventory::Initialise()
{
	LoadObjects(false);

	m_activeRing = INV_RING_WEAPONS;
	m_type = INV_TYPE_GAME;
	m_deltaMovement = 0;
	m_movement = INV_MOVE_STOPPED;
	m_type = INV_TYPE_GAME;
	m_selectedObject = NO_ITEM;
}


short Inventory::GetEnterObject()
{
	return m_enterObject;
}

short Inventory::GetSelectedObject()
{
	return m_selectedObject;
}

void Inventory::SetEnterObject(short objNum)
{
	m_enterObject = objNum;
}

void Inventory::SetSelectedObject(short objNum)
{
	m_selectedObject = objNum;
}

int Inventory::DoInventory()
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

		g_Renderer.DumpGameScene();

		OpenRing(m_activeRing, true);

		int passportResult = DoPassport();

		// Fade out
		//g_Renderer.FadeOut();
		//for (int i = 0; i < FADE_FRAMES_COUNT; i++)
		//{
		UpdateSceneAndDrawInventory();
		//}

		return passportResult;
	}

	m_rings[INV_RING_PUZZLES].draw = false;
	m_rings[INV_RING_WEAPONS].draw = false;
	m_rings[INV_RING_OPTIONS].draw = false;
	m_rings[INV_RING_COMBINE].draw = false;
	m_rings[INV_RING_CHOOSE_AMMO].draw = false;

	if (m_enterObject != NO_ITEM)
	{
		for (int r = 0; r < 3; r++)
			for (int o = 0; o < m_rings[r].numObjects; o++)
				if (m_objectsTable[m_rings[r].objects[o].inventoryObject].objectNumber == m_enterObject)
				{
					m_activeRing = r;
					m_rings[m_activeRing].currentObject = o;
					m_rings[m_activeRing].draw = true;
				}

		m_enterObject = NO_ITEM;
	}
	else
	{
		m_activeRing = INV_RING_WEAPONS;
		m_rings[m_activeRing].draw = true;
	}

	int result = INV_RESULT_NONE;

	g_Renderer.DumpGameScene();

	SoundEffect(SFX_TR3_MENU_SPININ, NULL, 0);

	OpenRing(m_activeRing, true);

	while (true /*!ResetFlag*/)
	{
		SetDebounce = true;
		S_UpdateInput();
		SetDebounce = false;

		GameTimer++;

		// Handle input
		if (DbInput & IN_DESELECT)
		{
			//SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

			// Exit from inventory
			m_enterObject = NO_ITEM;
			result = INV_RESULT_NONE;
			break;
		}
		else if (DbInput & IN_FORWARD && (m_activeRing == INV_RING_WEAPONS && m_rings[INV_RING_PUZZLES].numObjects != 0 || m_activeRing == INV_RING_OPTIONS))
		{
			SoundEffect(SFX_TR4_MENU_ROTATE, NULL, 0);

			int newRing = INV_RING_WEAPONS;
			if (m_activeRing == INV_RING_WEAPONS)
				newRing = INV_RING_PUZZLES;
			else
				newRing = INV_RING_WEAPONS;

			SwitchRing(m_activeRing, newRing, -1);
			m_activeRing = newRing;
			m_movement = 0;
			continue;
		}
		else if (DbInput & IN_BACK && (m_activeRing == INV_RING_PUZZLES || m_activeRing == INV_RING_WEAPONS))
		{
			SoundEffect(SFX_TR4_MENU_ROTATE, NULL, 0);

			int newRing = INV_RING_WEAPONS;
			if (m_activeRing == INV_RING_WEAPONS)
				newRing = INV_RING_OPTIONS;
			else
				newRing = INV_RING_WEAPONS;

			SwitchRing(m_activeRing, newRing, 1);
			m_activeRing = newRing;
			m_movement = 0;
			continue;
		}
		else if (TrInput & IN_LEFT)
		{
			SoundEffect(SFX_TR4_MENU_ROTATE, NULL, 0);

			// Change object left
			float deltaAngle = 360.0f / m_rings[m_activeRing].numObjects / INV_NUM_FRAMES_ROTATE;
			m_rings[m_activeRing].rotation = 0;

			for (int i = 0; i < INV_NUM_FRAMES_ROTATE; i++)
			{
				m_rings[m_activeRing].rotation += deltaAngle;
				UpdateSceneAndDrawInventory();
			}

			if (m_rings[m_activeRing].currentObject == m_rings[m_activeRing].numObjects - 1)
				m_rings[m_activeRing].currentObject = 0;
			else
				m_rings[m_activeRing].currentObject++;

			m_rings[m_activeRing].selectedIndex = INV_ACTION_USE;
			m_rings[m_activeRing].rotation = 0;
		}
		else if (TrInput & IN_RIGHT)
		{
			SoundEffect(SFX_TR4_MENU_ROTATE, NULL, 0);

			// Change object right
			float deltaAngle = 360.0f / m_rings[m_activeRing].numObjects / INV_NUM_FRAMES_ROTATE;
			m_rings[m_activeRing].rotation = 0;

			for (int i = 0; i < INV_NUM_FRAMES_ROTATE; i++)
			{
				m_rings[m_activeRing].rotation -= deltaAngle;
				UpdateSceneAndDrawInventory();
			}

			if (m_rings[m_activeRing].currentObject == 0)
				m_rings[m_activeRing].currentObject = m_rings[m_activeRing].numObjects - 1;
			else
				m_rings[m_activeRing].currentObject--;

			m_rings[m_activeRing].selectedIndex = INV_ACTION_USE;
			m_rings[m_activeRing].rotation = 0;
		}
		else if (DbInput & IN_SELECT)
		{
			// Handle action 
			if (m_activeRing == INV_RING_OPTIONS)
			{
				if (m_rings[INV_RING_OPTIONS].objects[m_rings[INV_RING_OPTIONS].currentObject].inventoryObject == _INV_OBJECT_PASSPORT)
				{
					int passportResult = DoPassport();
					if (passportResult == INV_RESULT_NEW_GAME ||
						passportResult == INV_RESULT_EXIT_TO_TILE ||
						passportResult == INV_RESULT_LOAD_GAME)
					{
						// Fade out
						g_Renderer.fadeOut();
						for (int i = 0; i < FADE_FRAMES_COUNT; i++)
						{
							UpdateSceneAndDrawInventory();
						}

						return passportResult;
					}
				}

				short currentObject = m_rings[INV_RING_OPTIONS].objects[m_rings[INV_RING_OPTIONS].currentObject].inventoryObject;

				if (currentObject == _INV_OBJECT_KEYS)
					DoControlsSettings();

				if (currentObject == _INV_OBJECT_SUNGLASSES)
					DoGraphicsSettings();

				if (currentObject == _INV_OBJECT_HEADPHONES)
					DoSoundSettings();
			}
			else if (m_activeRing == INV_RING_WEAPONS || m_activeRing == INV_RING_PUZZLES)
			{
				short currentObject = m_rings[m_activeRing].objects[m_rings[m_activeRing].currentObject].inventoryObject;
				int result = INV_RESULT_NONE;

				if (IsCurrentObjectPuzzle())
					// Puzzles have Use, Combine, Separe
					result = DoPuzzle();
				else if (IsCurrentObjectWeapon())
					// Weapons have Use, Select Ammo
					result = DoWeapon();
				else if (IsCurrentObjectExamine())
					// Examines have just Examine
					DoExamine();
				else if (currentObject == _INV_OBJECT_TIMEX)
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
					m_enterObject = NO_ITEM;
					result = INV_RESULT_USE_ITEM;
					break;
				}
			}
		}

		UpdateSceneAndDrawInventory();
	}

	SoundEffect(SFX_TR3_MENU_SPINOUT, NULL, 0);

	CloseRing(m_activeRing, true);

	return result;
}

bool Inventory::IsCurrentObjectWeapon()
{
	short currentObject = m_rings[m_activeRing].objects[m_rings[m_activeRing].currentObject].inventoryObject;
	return (currentObject == _INV_OBJECT_PISTOLS || currentObject == _INV_OBJECT_UZIS ||
		currentObject == _INV_OBJECT_REVOLVER_LASER || currentObject == _INV_OBJECT_CROSSBOW_LASER ||
		currentObject == _INV_OBJECT_REVOLVER || currentObject == _INV_OBJECT_SHOTGUN ||
		currentObject == _INV_OBJECT_HK || currentObject == _INV_OBJECT_CROSSBOW ||
		currentObject == _INV_OBJECT_ROCKET_LAUNCHER || currentObject == _INV_OBJECT_GRENADE_LAUNCHER ||
		currentObject == _INV_OBJECT_HARPOON_GUN);
}

bool Inventory::IsCurrentObjectPuzzle()
{
	short currentObject = m_rings[m_activeRing].objects[m_rings[m_activeRing].currentObject].inventoryObject;
	return (currentObject >= _INV_OBJECT_PUZZLE1 && currentObject <= _INV_OBJECT_PICKUP4_COMBO2);
}

bool Inventory::IsCurrentObjectGeneric()
{
	short currentObject = m_rings[m_activeRing].objects[m_rings[m_activeRing].currentObject].inventoryObject;
	return (!IsCurrentObjectPuzzle() && !IsCurrentObjectExamine() && !IsCurrentObjectWeapon() &&
		currentObject != _INV_OBJECT_TIMEX);
}

bool Inventory::IsCurrentObjectExamine()
{
	short currentObject = m_rings[m_activeRing].objects[m_rings[m_activeRing].currentObject].inventoryObject;
	return (currentObject >= _INV_OBJECT_EXAMINE1 && currentObject <= _INV_OBJECT_EXAMINE1);
}

int Inventory::DoPuzzle()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->framePtr = 0;
	ring->selectedIndex = 0;
	ring->numActions = 0;

	int result = INV_RESULT_NONE;
	bool closeObject = false;
	short currentObject = ring->objects[ring->currentObject].inventoryObject;

	ring->numActions = 0;
	ring->actions[ring->numActions++] = INV_ACTION_USE;
	if (IsObjectCombinable(currentObject))
		ring->actions[ring->numActions++] = INV_ACTION_COMBINE;
	//if (IsObjectSeparable(currentObject)) 
	//	ring->actions[ring->numActions++] = INV_ACTION_SEPARE;

	// If only use action then select the weapon directly
	if (ring->numActions == 1)
		return INV_RESULT_USE_ITEM;

	PopupObject();

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

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex > 0)
				ring->selectedIndex--;
		}
		else if (DbInput & IN_BACK)
		{
			closeObject = false;

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex < ring->numActions)
				ring->selectedIndex++;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

			if (ring->actions[ring->selectedIndex] == INV_ACTION_USE)
			{
				result = INV_RESULT_USE_ITEM;
				closeObject = true;
				break;
			}
			else if (ring->actions[ring->selectedIndex] == INV_ACTION_COMBINE)
			{
				if (DoCombine())
				{
					ring->actions[1] = INV_ACTION_SEPARE;
				}
			}
			else if (ring->actions[ring->selectedIndex] == INV_ACTION_SEPARE)
			{
				if (DoSepare())
				{
					ring->actions[1] = INV_ACTION_COMBINE;
				}
			}
		}

		UpdateSceneAndDrawInventory();
	}

	PopoverObject();

	return result;
}

int Inventory::DoWeapon()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->framePtr = 0;
	ring->selectedIndex = 0;
	ring->numActions = 0;

	int result = INV_RESULT_NONE;
	bool closeObject = false;
	short currentObject = ring->objects[ring->currentObject].inventoryObject;

	ring->numActions = 0;
	ring->actions[ring->numActions++] = INV_ACTION_USE;
	if (IsObjectCombinable(currentObject)) ring->actions[ring->numActions++] = INV_ACTION_COMBINE;
	if (IsObjectSeparable(currentObject)) ring->actions[ring->numActions++] = INV_ACTION_SEPARE;
	if (HasWeaponMultipleAmmos(currentObject)) ring->actions[ring->numActions++] = INV_ACTION_SELECT_AMMO;

	// If only use action then select the weapon directly
	if (ring->numActions == 1)
		return INV_RESULT_USE_ITEM;

	PopupObject();

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

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex > 0)
				ring->selectedIndex--;
		}
		else if (DbInput & IN_BACK)
		{
			closeObject = false;

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex < ring->numActions)
				ring->selectedIndex++;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

			if (ring->actions[ring->selectedIndex] == INV_ACTION_USE)
			{
				result = INV_RESULT_USE_ITEM;
				closeObject = true;
				break;
			}
			else if (ring->actions[ring->selectedIndex] == INV_ACTION_COMBINE)
			{
				if (DoCombine())
				{
					ring->actions[1] = INV_ACTION_SEPARE;
				}
			}
			else if (ring->actions[ring->selectedIndex] == INV_ACTION_SEPARE)
			{
				if (DoSepare())
				{
					ring->actions[1] = INV_ACTION_COMBINE;
				}
			}
			else if (ring->actions[ring->selectedIndex] == INV_ACTION_SELECT_AMMO)
			{
				DoSelectAmmo();
			}
		}

		UpdateSceneAndDrawInventory();
	}

	PopoverObject();

	return result;
}

bool Inventory::IsObjectPresentInInventory(short object)
{
	for (int r = 0; r < 3; r++)
		for (int o = 0; o < m_rings[r].numObjects; o++)
			if (m_objectsTable[m_rings[r].objects[o].inventoryObject].objectNumber == object)
				return true;
	return false;
}

bool Inventory::IsInventoryObjectPresentInInventory(short object)
{
	for (int r = 0; r < 3; r++)
		for (int o = 0; o < m_rings[r].numObjects; o++)
			if (m_rings[r].objects[o].inventoryObject == object)
				return true;
	return false;
}

int Inventory::FindObjectIndex(short object)
{
	for (int r = 0; r < 3; r++)
		for (int o = 0; o < m_rings[r].numObjects; o++)
			if (m_rings[r].objects[o].inventoryObject == object)
				return o;
	return -1;
}

int Inventory::FindObjectRing(short object)
{
	for (int r = 0; r < 3; r++)
		for (int o = 0; o < m_rings[r].numObjects; o++)
			if (m_rings[r].objects[o].inventoryObject == object)
				return r;
	return -1;
}

bool Inventory::IsObjectCombinable(short object)
{
	for (int i = 0; i < m_combinations.size(); i++)
		if (m_combinations[i].piece1 == object || m_combinations[i].piece2 == object)
			return true;
	return false;
}

void Inventory::AddCombination(short piece1, short piece2, short combinedObject, void (*f) (int, short))
{
	InventoryObjectCombination combination;
	combination.piece1 = piece1;
	combination.piece2 = piece2;
	combination.combinedObject = combinedObject;
	combination.combineRoutine = f;
	m_combinations.push_back(combination);
}

int Inventory::DoGenericObject()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->framePtr = 0;

	return INV_RESULT_USE_ITEM;
}

void Inventory::DoStatistics()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->framePtr = 0;

}

void Inventory::DoExamine()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->framePtr = 0;

}

bool Inventory::DoCombine()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	int oldRing = m_activeRing;

	// Fill the objects ring
	InventoryRing* combineRing = &m_rings[INV_RING_COMBINE];
	combineRing->numObjects = 0;

	short currentObject = ring->objects[ring->currentObject].inventoryObject;

	for (int i = 0; i < m_combinations.size(); i++)
	{
		InventoryObjectCombination* combination = &m_combinations[i];

		// Add piece 1
		if (currentObject != combination->piece1 && IsInventoryObjectPresentInInventory(combination->piece1))
		{
			bool found = false;
			for (int j = 0; j < combineRing->numObjects; j++)
			{
				if (combineRing->objects[j].inventoryObject == combination->piece1)
				{
					found = true;
					break;
				}
			}
			if (!found)
				combineRing->objects[combineRing->numObjects++].inventoryObject = combination->piece1;
		}

		// Add piece 2
		if (currentObject != combination->piece2 && IsInventoryObjectPresentInInventory(combination->piece2))
		{
			bool found = false;
			for (int j = 0; j < combineRing->numObjects; j++)
			{
				if (combineRing->objects[j].inventoryObject == combination->piece2)
				{
					found = true;
					break;
				}
			}
			if (!found)
				combineRing->objects[combineRing->numObjects++].inventoryObject = combination->piece2;
		}
	}

	// If no objects then exit 
	if (combineRing->numObjects == 0)
	{
		return false;
	}

	ring->draw = false;
	combineRing->draw = true;
	combineRing->selectedIndex = 0;

	OpenRing(INV_RING_COMBINE, false);

	bool closeObject = false;
	bool combined = false;

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
		else if (TrInput & IN_LEFT && combineRing->numObjects > 1)
		{
			closeObject = false;

			SoundEffect(SFX_TR4_MENU_ROTATE, NULL, 0);

			// Change object left
			float deltaAngle = 360.0f / combineRing->numObjects / INV_NUM_FRAMES_ROTATE;
			combineRing->rotation = 0;

			for (int i = 0; i < INV_NUM_FRAMES_ROTATE; i++)
			{
				combineRing->rotation += deltaAngle;

				UpdateSceneAndDrawInventory();
			}

			if (combineRing->currentObject > 0)
				combineRing->currentObject--;
			else
				combineRing->currentObject = combineRing->numObjects - 1;

			combineRing->rotation = 0;
		}
		else if (TrInput & IN_RIGHT && combineRing->numObjects > 1)
		{
			closeObject = false;

			// Change object right
			float deltaAngle = 360.0f / combineRing->numObjects / INV_NUM_FRAMES_ROTATE;
			combineRing->rotation = 0;

			for (int i = 0; i < INV_NUM_FRAMES_ROTATE; i++)
			{
				combineRing->rotation -= deltaAngle;

				UpdateSceneAndDrawInventory();
			}

			SoundEffect(SFX_TR4_MENU_ROTATE, NULL, 0);
			if (combineRing->currentObject < combineRing->numObjects - 1)
				combineRing->currentObject++;
			else
				combineRing->currentObject = 0;

			combineRing->rotation = 0;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

			// Check if can be combined
			short currentObject = combineRing->objects[combineRing->currentObject].inventoryObject;
			for (int i = 0; i < m_combinations.size(); i++)
			{
				InventoryObjectCombination* combination = &m_combinations[i];
				if (combination->piece1 == currentObject && combination->piece2 == ring->objects[ring->currentObject].inventoryObject ||
					combination->piece2 == currentObject && combination->piece1 == ring->objects[ring->currentObject].inventoryObject)
				{
					// I can do the combination
					SoundEffect(SFX_TR4_MENU_COMBINE, NULL, 0);
					combination->combineRoutine(INV_COMBINE_COMBINE, combination->combinedObject);
					LoadObjects(true);
					SelectObject(oldRing, combination->combinedObject, 2 * INV_OBJECTS_SCALE);
					closeObject = true;
					combined = true;
					break;
				}
			}

			if (!closeObject)
				SayNo();
		}

		UpdateSceneAndDrawInventory();
	}

	CloseRing(INV_RING_COMBINE, false);

	m_activeRing = oldRing;
	combineRing->draw = false;
	ring->draw = true;
	ring->selectedIndex = 0;

	return combined;
}

bool Inventory::DoSepare()
{
	short currentObject = m_rings[m_activeRing].objects[m_rings[m_activeRing].currentObject].inventoryObject;

	for (int i = 0; i < m_combinations.size(); i++)
	{
		InventoryObjectCombination* combination = &m_combinations[i];
		if (combination->combinedObject == currentObject)
		{
			// Separation can be done
			SoundEffect(SFX_TR4_MENU_COMBINE, NULL, 0);
			combination->combineRoutine(INV_COMBINE_SEPARE, combination->combinedObject);
			LoadObjects(true);
			SelectObject(m_activeRing, combination->piece1, 2 * INV_OBJECTS_SCALE);
			return true;
		}
	}

	return false;
}

void Inventory::DoSelectAmmo()
{
	InventoryRing* ring = &m_rings[m_activeRing];

	// Enable the secondary GUI
	m_activeRing = INV_RING_CHOOSE_AMMO;

	// Fill the secondary ring
	InventoryRing* ammoRing = &m_rings[INV_RING_CHOOSE_AMMO];
	ring->draw = false;
	ammoRing->draw = true;
	ammoRing->numObjects = 0;

	switch (ring->objects[ring->currentObject].inventoryObject)
	{
	case _INV_OBJECT_SHOTGUN:
		if (Lara.Weapons[WEAPON_SHOTGUN].Ammo[0] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = _INV_OBJECT_SHOTGUN_AMMO1;
		if (Lara.Weapons[WEAPON_SHOTGUN].Ammo[1] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = _INV_OBJECT_SHOTGUN_AMMO2;
		ammoRing->selectedIndex = Lara.Weapons[WEAPON_SHOTGUN].SelectedAmmo;

		break;

	case _INV_OBJECT_GRENADE_LAUNCHER:
		if (Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[0] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = _INV_OBJECT_GRENADE_AMMO1;
		if (Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[1] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = _INV_OBJECT_GRENADE_AMMO2;
		if (Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[2] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = _INV_OBJECT_GRENADE_AMMO3;
		ammoRing->selectedIndex = Lara.Weapons[WEAPON_GRENADE_LAUNCHER].SelectedAmmo;

		break;

	case _INV_OBJECT_CROSSBOW:
	case _INV_OBJECT_CROSSBOW_LASER:
		if (Lara.Weapons[WEAPON_CROSSBOW].Ammo[0] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = _INV_OBJECT_CROSSBOW_AMMO1;
		if (Lara.Weapons[WEAPON_CROSSBOW].Ammo[1] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = _INV_OBJECT_CROSSBOW_AMMO2;
		if (Lara.Weapons[WEAPON_CROSSBOW].Ammo[2] != 0)
			ammoRing->objects[ammoRing->numObjects++].inventoryObject = _INV_OBJECT_CROSSBOW_AMMO3;
		ammoRing->selectedIndex = Lara.Weapons[WEAPON_CROSSBOW].SelectedAmmo;

		break;
	}

	// If no objects then exit 
	if (ammoRing->numObjects == 0)
		return;

	OpenRing(INV_RING_CHOOSE_AMMO, false);

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
		else if (TrInput & IN_LEFT && ammoRing->numObjects > 1)
		{
			closeObject = false;

			SoundEffect(SFX_TR4_MENU_ROTATE, NULL, 0);

			// Change object left
			float deltaAngle = 360.0f / ammoRing->numObjects / INV_NUM_FRAMES_ROTATE;
			ammoRing->rotation = 0;

			for (int i = 0; i < INV_NUM_FRAMES_ROTATE; i++)
			{
				ammoRing->rotation += deltaAngle;

				UpdateSceneAndDrawInventory();
			}

			if (ammoRing->currentObject > 0)
				ammoRing->currentObject--;
			else
				ammoRing->currentObject = ammoRing->numObjects - 1;

			ammoRing->rotation = 0;
		}
		else if (TrInput & IN_RIGHT && ammoRing->numObjects > 1)
		{
			closeObject = false;

			// Change object right
			float deltaAngle = 360.0f / ammoRing->numObjects / INV_NUM_FRAMES_ROTATE;
			ammoRing->rotation = 0;

			for (int i = 0; i < INV_NUM_FRAMES_ROTATE; i++)
			{
				ammoRing->rotation -= deltaAngle;

				UpdateSceneAndDrawInventory();
			}

			SoundEffect(SFX_TR4_MENU_ROTATE, NULL, 0);
			if (ammoRing->currentObject < ammoRing->numObjects - 1)
				ammoRing->currentObject++;
			else
				ammoRing->currentObject = 0;

			ammoRing->rotation = 0;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

			// Choose ammo
			switch (ring->objects[ring->currentObject].inventoryObject)
			{
			case _INV_OBJECT_SHOTGUN:
				Lara.Weapons[WEAPON_SHOTGUN].SelectedAmmo = ring->selectedIndex;
				break;

			case _INV_OBJECT_GRENADE_LAUNCHER:
				Lara.Weapons[WEAPON_GRENADE_LAUNCHER].SelectedAmmo = ring->selectedIndex;
				break;

			case _INV_OBJECT_CROSSBOW:
			case _INV_OBJECT_CROSSBOW_LASER:
				Lara.Weapons[WEAPON_CROSSBOW].SelectedAmmo = ring->selectedIndex;
				break;

			}

			closeObject = true;
			break;
		}

		UpdateSceneAndDrawInventory();
	}

	CloseRing(INV_RING_CHOOSE_AMMO, false);

	// Reset secondary GUI
	m_activeRing = INV_RING_WEAPONS;
	ammoRing->draw = false;
	ring->draw = true;
}

void Inventory::UseCurrentItem()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	InventoryObject* inventoryObject = &ring->objects[ring->currentObject];
	short objectNumber = m_objectsTable[inventoryObject->inventoryObject].objectNumber;

	LaraItem->meshBits = -1;

	int binocularRange = BinocularRange;
	BinocularRange = 0;
	OldLaraBusy = false;

	// Small medipack
	if (objectNumber == ID_SMALLMEDI_ITEM)
	{

		if (Lara.NumSmallMedipacks == 0)
		{
			SayNo();
			return;
		}

		if ((LaraItem->hitPoints <= 0 || LaraItem->hitPoints >= 1000) && !Lara.poisoned)
		{
			SayNo();
			return;
		}

		if (Lara.NumSmallMedipacks != -1)
			Lara.NumSmallMedipacks--;

		Lara.poisoned = 0;
		LaraItem->hitPoints += 500;
		if (LaraItem->hitPoints > 1000)
			LaraItem->hitPoints = 1000;

		SoundEffect(116, 0, 2);
		Savegame.Game.HealthUsed++;

		SoundEffect(SFX_TR4_MENU_MEDI, NULL, 0);

		return;
	}

	// Big medipack
	if (objectNumber == ID_BIGMEDI_ITEM)
	{

		if (Lara.NumLargeMedipacks == 0)
		{
			SayNo();
			return;
		}

		if ((LaraItem->hitPoints <= 0 || LaraItem->hitPoints >= 1000) && !Lara.poisoned)
		{
			SayNo();
			return;
		}

		if (Lara.NumLargeMedipacks != -1)
			Lara.NumLargeMedipacks--;

		Lara.poisoned = 0;
		LaraItem->hitPoints += 1000;
		if (LaraItem->hitPoints > 1000)
			LaraItem->hitPoints = 1000;

		SoundEffect(116, 0, 2);
		Savegame.Game.HealthUsed++;

		SoundEffect(SFX_TR4_MENU_MEDI, NULL, 0);

		return;
	}

	// Binoculars
	if (objectNumber == ID_BINOCULARS_ITEM)
	{
		if (LaraItem->currentAnimState == LS_STOP && LaraItem->animNumber == LA_STAND_IDLE || Lara.isDucked && !(TrInput & IN_DUCK))
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
			m_selectedObject = objectNumber;

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);

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
			m_selectedObject = objectNumber;

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);

			return;
		}
		else
		{
			SayNo();
			return;
		}
	}

	// TODO: can cause problem with harpoongun in underwater and wading !
	bool canUseWeapons = (LaraItem->currentAnimState != LS_CRAWL_IDLE &&
		LaraItem->currentAnimState != LS_CRAWL_FORWARD &&
		LaraItem->currentAnimState != LS_CRAWL_TURN_LEFT &&
		LaraItem->currentAnimState != LS_CRAWL_TURN_RIGHT &&
		LaraItem->currentAnimState != LS_CRAWL_BACK &&
		LaraItem->currentAnimState != LS_CRAWL_TO_HANG &&
		LaraItem->currentAnimState != LS_CROUCH_IDLE &&
		LaraItem->currentAnimState != LS_CROUCH_TURN_LEFT &&
		LaraItem->currentAnimState != LS_CROUCH_TURN_RIGHT &&
		(Lara.waterStatus != LW_UNDERWATER
			|| (Lara.waterStatus == LW_UNDERWATER && objectNumber == ID_HARPOON_ITEM)));

	// Pistols
	if (objectNumber == ID_PISTOLS_ITEM)
	{
		if (canUseWeapons)
		{
			Lara.requestGunType = WEAPON_PISTOLS;
			if (!Lara.gunStatus && Lara.gunType == WEAPON_PISTOLS)
				Lara.gunStatus = LG_DRAW_GUNS;

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
		}
		else
		{
			SayNo();
		}

		return;
	}

	// Grenade launcher
	if (objectNumber == ID_GRENADE_GUN_ITEM)
	{
		if (canUseWeapons)
		{
			Lara.requestGunType = WEAPON_GRENADE_LAUNCHER;
			if (!Lara.gunStatus && Lara.gunType == WEAPON_GRENADE_LAUNCHER)
				Lara.gunStatus = LG_DRAW_GUNS;

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
		}
		else
		{
			SayNo();
		}

		return;
	}

	// Rocket launcher
	if (objectNumber == ID_ROCKET_LAUNCHER_ITEM)
	{
		if (canUseWeapons)
		{
			Lara.requestGunType = WEAPON_ROCKET_LAUNCHER;
			if (!Lara.gunStatus && Lara.gunType == WEAPON_ROCKET_LAUNCHER)
				Lara.gunStatus = LG_DRAW_GUNS;

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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
			if (LaraItem->currentAnimState != LS_CRAWL_IDLE &&
				LaraItem->currentAnimState != LS_CRAWL_FORWARD &&
				LaraItem->currentAnimState != LS_CRAWL_TURN_LEFT &&
				LaraItem->currentAnimState != LS_CRAWL_TURN_RIGHT &&
				LaraItem->currentAnimState != LS_CRAWL_BACK &&
				LaraItem->currentAnimState != LS_CRAWL_TO_HANG &&
				Lara.waterStatus == LW_ABOVE_WATER)
			{
				if (Lara.gunType != WEAPON_FLARE)
				{
					TrInput = IN_FLARE;
					LaraGun();
					TrInput = 0;

					SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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
	ring->rotation = 0;
	ring->currentObject = 0;
	ring->focusState = INV_FOCUS_STATE_NONE;

	for (int j = 0; j < NUM_INVENTORY_OBJECTS_PER_RING; j++)
	{
		ring->objects[j].inventoryObject = -1;
		ring->objects[j].rotation = 0;
		ring->objects[j].scale = INV_OBJECTS_SCALE;
	}

	InsertObject(INV_RING_OPTIONS, _INV_OBJECT_PASSPORT);
	InsertObject(INV_RING_OPTIONS, _INV_OBJECT_POLAROID);
	InsertObject(INV_RING_OPTIONS, _INV_OBJECT_SUNGLASSES);
	InsertObject(INV_RING_OPTIONS, _INV_OBJECT_HEADPHONES);
	InsertObject(INV_RING_OPTIONS, _INV_OBJECT_KEYS);

	m_activeRing = INV_RING_OPTIONS;
	m_deltaMovement = 0;
	m_movement = INV_MOVE_STOPPED;
	m_type = INV_TYPE_TITLE;
	m_selectedObject = NO_ITEM;
}

bool Inventory::UpdateSceneAndDrawInventory()
{
	int nframes;

	if (CurrentLevel == 0 && g_GameFlow->TitleType == TITLE_FLYBY)
	{
		g_Renderer.renderTitle();
		Camera.numberFrames = g_Renderer.SyncRenderer();

		nframes = Camera.numberFrames;
		ControlPhase(nframes, 0);
	}
	else
	{
		g_Renderer.renderInventory();
		g_Renderer.SyncRenderer();
	}

	return true;
}

int Inventory::DoTitleInventory()
{
	InitialiseTitle();

	m_rings[INV_RING_PUZZLES].draw = false;
	m_rings[INV_RING_WEAPONS].draw = false;
	m_rings[INV_RING_OPTIONS].draw = true;

	InventoryRing* ring = &m_rings[INV_RING_OPTIONS];
	m_activeRing = INV_RING_OPTIONS;

	// Fade in
	g_Renderer.fadeIn();
	for (int i = 0; i < FADE_FRAMES_COUNT; i++)
	{
		UpdateSceneAndDrawInventory();
	}

	CurrentAtmosphere = CDA_XA11_FLYBY1;
	S_CDPlay(CurrentAtmosphere, 1);

	OpenRing(INV_RING_OPTIONS, true);

	int result = INV_RESULT_NONE;

	while (true /*!ResetFlag*/)
	{
		SetDebounce = true;

		S_UpdateInput();
		SetDebounce = false;

		GameTimer++;

		// Handle input
		if (TrInput & IN_LEFT)
		{
			SoundEffect(SFX_TR4_MENU_ROTATE, NULL, 0);

			// Change object right
			float deltaAngle = 360.0f / ring->numObjects / INV_NUM_FRAMES_ROTATE;
			ring->rotation = 0;

			for (int i = 0; i < INV_NUM_FRAMES_ROTATE; i++)
			{
				ring->rotation += deltaAngle;

				UpdateSceneAndDrawInventory();
			}

			if (ring->currentObject == ring->numObjects - 1)
				ring->currentObject = 0;
			else
				ring->currentObject++;

			ring->rotation = 0;
		}
		else if (TrInput & IN_RIGHT)
		{
			SoundEffect(SFX_TR4_MENU_ROTATE, NULL, 0);

			// Change object left
			float deltaAngle = 360.0f / ring->numObjects / INV_NUM_FRAMES_ROTATE;
			ring->rotation = 0;

			for (int i = 0; i < INV_NUM_FRAMES_ROTATE; i++)
			{
				ring->rotation -= deltaAngle;

				UpdateSceneAndDrawInventory();
			}

			if (ring->currentObject == 0)
				ring->currentObject = ring->numObjects - 1;
			else
				ring->currentObject--;

			ring->rotation = 0;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

			if (ring->objects[ring->currentObject].inventoryObject == _INV_OBJECT_PASSPORT)
			{
				int passportResult = DoPassport();
				if (passportResult == INV_RESULT_NEW_GAME ||
					passportResult == INV_RESULT_EXIT_GAME ||
					passportResult == INV_RESULT_LOAD_GAME)
				{
					result = passportResult;
					break;
				}
			}

			if (ring->objects[ring->currentObject].inventoryObject == _INV_OBJECT_KEYS)
				DoControlsSettings();

			if (ring->objects[ring->currentObject].inventoryObject == _INV_OBJECT_SUNGLASSES)
				DoGraphicsSettings();

			if (ring->objects[ring->currentObject].inventoryObject == _INV_OBJECT_HEADPHONES)
				DoSoundSettings();
		}

		UpdateSceneAndDrawInventory();
	}

	CloseRing(INV_RING_OPTIONS, true);

	// Fade out
	g_Renderer.fadeOut();
	for (int i = 0; i < FADE_FRAMES_COUNT; i++)
	{
		UpdateSceneAndDrawInventory();
	}

	return result;
}

InventoryObjectDefinition* Inventory::GetInventoryObject(int index)
{
	return &m_objectsTable[index];
}

int Inventory::DoPassport()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->framePtr = 0;

	short choice = 0;

	vector<int> choices;
	if (m_type == INV_TYPE_TITLE)
	{
		choices.push_back(INV_WHAT_PASSPORT_NEW_GAME);
		choices.push_back(INV_WHAT_PASSPORT_SELECT_LEVEL);
		choices.push_back(INV_WHAT_PASSPORT_LOAD_GAME);
		choices.push_back(INV_WHAT_PASSPORT_EXIT_GAME);
	}
	else
	{
		choices.push_back(INV_WHAT_PASSPORT_LOAD_GAME);
		if (LaraItem->hitPoints > 0 || CurrentLevel == 0)
			choices.push_back(INV_WHAT_PASSPORT_SAVE_GAME);
		choices.push_back(INV_WHAT_PASSPORT_EXIT_TO_TITLE);
	}

	ring->passportAction = choices[0];

	SoundEffect(SFX_TR3_MENU_CHOOSE, NULL, 0);

	PopupObject();

	// Open the passport
	for (int i = 0; i < 14; i++)
	{
		UpdateSceneAndDrawInventory();
		ring->framePtr++;
	}

	bool moveLeft = false;
	bool moveRight = false;
	bool closePassport = false;

	int result = INV_RESULT_NONE;

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
			SoundEffect(SFX_TR3_MENU_PASSPORT, NULL, 0);

			moveLeft = false;
			moveRight = false;
			closePassport = false;

			if (choice > 0)
			{
				ring->framePtr = 19;
				for (int i = 0; i < 5; i++)
				{
					UpdateSceneAndDrawInventory();
					ring->framePtr--;
				}

				choice--;
			}
		}
		else if (DbInput & IN_RIGHT || moveRight)
		{
			SoundEffect(SFX_TR3_MENU_PASSPORT, NULL, 0);

			moveLeft = false;
			moveRight = false;
			closePassport = false;

			if (choice < choices.size() - 1)
			{
				ring->framePtr = 14;
				for (int i = 0; i < 5; i++)
				{
					UpdateSceneAndDrawInventory();
					ring->framePtr++;
				}

				choice++;
			}
		}

		if (choices[choice] == INV_WHAT_PASSPORT_LOAD_GAME)
		{
			// Load game
			int selectedSavegame = 0;
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
					SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
					selectedSavegame--;
					continue;
				}
				else if (DbInput & IN_BACK && selectedSavegame < MAX_SAVEGAMES - 1)
				{
					SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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
					SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

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

				UpdateSceneAndDrawInventory();
			}
		}
		else if (choices[choice] == INV_WHAT_PASSPORT_SAVE_GAME)
		{
			// Save game
			int selectedSavegame = 0;
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
					SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
					selectedSavegame--;
					continue;
				}
				else if (DbInput & IN_BACK && selectedSavegame < MAX_SAVEGAMES - 1)
				{
					SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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
					SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

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

				UpdateSceneAndDrawInventory();
			}
		}
		else if (choices[choice] == INV_WHAT_PASSPORT_SELECT_LEVEL)
		{
			// Save game
			int selectedLevel = 0;
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
					SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
					selectedLevel--;
					continue;
				}
				else if (DbInput & IN_BACK && selectedLevel < g_GameFlow->GetNumLevels() - 1)
				{
					SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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
					SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

					result = INV_RESULT_NEW_GAME;
					g_GameFlow->SelectedLevelForNewGame = selectedLevel + 1;

					moveLeft = false;
					moveRight = false;
					closePassport = true;

					break;
				}

				ring->selectedIndex = selectedLevel;
				ring->passportAction = INV_WHAT_PASSPORT_SELECT_LEVEL;

				UpdateSceneAndDrawInventory();
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
					SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

					result = INV_RESULT_NEW_GAME;
					moveLeft = false;
					moveRight = false;
					closePassport = true;

					break;
				}

				ring->passportAction = INV_WHAT_PASSPORT_NEW_GAME;

				UpdateSceneAndDrawInventory();
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
					SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

					result = INV_RESULT_EXIT_GAME;
					moveLeft = false;
					moveRight = false;
					closePassport = true;

					break;
				}

				ring->passportAction = INV_WHAT_PASSPORT_EXIT_GAME;

				UpdateSceneAndDrawInventory();
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
					SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

					result = INV_RESULT_EXIT_TO_TILE;
					moveLeft = false;
					moveRight = false;
					closePassport = true;

					break;
				}

				ring->passportAction = INV_WHAT_PASSPORT_EXIT_TO_TITLE;

				UpdateSceneAndDrawInventory();
			}
		}
		else
		{
			UpdateSceneAndDrawInventory();
		}
	}

	// Close the passport
	ring->framePtr = 24;
	for (int i = 24; i < 30; i++)
	{
		UpdateSceneAndDrawInventory();
		ring->framePtr++;
	}

	ring->framePtr = 0;

	PopoverObject();

	return result;
}

int	Inventory::PopupObject()
{
	InventoryRing* ring = &m_rings[m_activeRing];

	int steps = INV_NUM_FRAMES_POPUP;
	int deltaAngle;// = (0 - ring->objects[ring->currentObject].rotation) / steps;
	float deltaScale = INV_OBJECTS_SCALE / (float)steps;
	float deltaTilt = 90.0f / steps;
	float deltaDistance = INV_OBJECT_DISTANCE / steps;

	if (ring->objects[ring->currentObject].rotation < 65535 / 2)
	{
		deltaAngle = -ring->objects[ring->currentObject].rotation / steps;
	}
	else
	{
		deltaAngle = (65535 - ring->objects[ring->currentObject].rotation) / steps;
	}

	ring->focusState = INV_FOCUS_STATE_POPUP;

	for (int i = 0; i < steps; i++)
	{
		UpdateSceneAndDrawInventory();

		ring->objects[ring->currentObject].rotation += deltaAngle;
		ring->objects[ring->currentObject].scale += deltaScale;
	}

	ring->focusState = INV_FOCUS_STATE_FOCUSED;

	return 0;
}

int	Inventory::PopoverObject()
{
	InventoryRing* ring = &m_rings[m_activeRing];

	int steps = INV_NUM_FRAMES_POPUP;
	int deltaAngle;// = (0 - ring->objects[ring->currentObject].rotation) / steps;
	float deltaScale = INV_OBJECTS_SCALE / (float)steps;
	float deltaTilt = INV_OBJECT_TILT / steps;
	float deltaDistance = INV_OBJECT_DISTANCE / steps;

	if (ring->objects[ring->currentObject].rotation < 65535 / 2)
	{
		deltaAngle = -ring->objects[ring->currentObject].rotation / steps;
	}
	else
	{
		deltaAngle = (65535 - ring->objects[ring->currentObject].rotation) / steps;
	}


	ring->focusState = INV_FOCUS_STATE_POPOVER;

	for (int i = 0; i < steps; i++)
	{
		UpdateSceneAndDrawInventory();

		ring->objects[ring->currentObject].rotation -= deltaAngle;
		ring->objects[ring->currentObject].scale -= deltaScale;
	}

	ring->focusState = INV_FOCUS_STATE_NONE;

	return 0;
}

int Inventory::GetType()
{
	return m_type;
}

void Inventory::DoControlsSettings()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->framePtr = 0;
	ring->selectedIndex = 0;
	ring->waitingForKey = false;

	PopupObject();

	bool closeObject = false;

	// Copy configuration to a temporary object
	memcpy(&ring->Configuration.KeyboardLayout, &KeyboardLayout[1], NUM_CONTROLS);

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
			if (!ring->waitingForKey)
				closeObject = true;
			else
				ring->waitingForKey = false;

			break;
		}
		else if (DbInput & IN_FORWARD)
		{
			closeObject = false;

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex > 0)
				ring->selectedIndex--;
		}
		else if (DbInput & IN_BACK)
		{
			closeObject = false;

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex < NUM_CONTROLS + 2 - 1)
				ring->selectedIndex++;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);

			if (ring->selectedIndex == NUM_CONTROLS)
			{
				memcpy(KeyboardLayout[1], ring->Configuration.KeyboardLayout, NUM_CONTROLS);
				SaveConfiguration();

				closeObject = true;
				break;
			}
			else if (ring->selectedIndex == NUM_CONTROLS + 1)
			{
				closeObject = true;
				break;
			}
			else
			{
				UpdateSceneAndDrawInventory();
				continue;
			}

		}

		// If RETURN is pressed, then wait for a new key
		if (KeyMap[DIK_RETURN] & 0x80)
		{
			SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

			if (!ring->waitingForKey)
			{
				ring->waitingForKey = true;

				TrInput = 0;
				DbInput = 0;
				ZeroMemory(KeyMap, 256);

				while (true)
				{
					if (DbInput & IN_DESELECT)
					{
						ring->waitingForKey = false;
						break;
					}

					int selectedKey = 0;
					for (selectedKey = 0; selectedKey < 256; selectedKey++)
					{
						if (KeyMap[selectedKey] & 0x80)
							break;
					}

					if (selectedKey == 256)
						selectedKey = 0;

					if (selectedKey && g_KeyNames[selectedKey])
					{
						// Can't rededefine special keys or the inventory will be not usable
						if (!(selectedKey == DIK_RETURN || selectedKey == DIK_LEFT || selectedKey == DIK_RIGHT ||
							selectedKey == DIK_UP || selectedKey == DIK_DOWN))
						{
							if (selectedKey != DIK_ESCAPE)
							{
								KeyboardLayout[1][ring->selectedIndex] = selectedKey;
								DefaultConflict();
								ring->waitingForKey = false;
								break;
							}
						}
					}

					UpdateSceneAndDrawInventory();

					SetDebounce = true;
					S_UpdateInput();
					SetDebounce = false;
				}
			}
		}

		UpdateSceneAndDrawInventory();
	}

	PopoverObject();
}

void Inventory::DoGraphicsSettings()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->framePtr = 0;
	ring->selectedIndex = 0;

	PopupObject();

	// Copy configuration to a temporary object
	memcpy(&ring->Configuration, &g_Configuration, sizeof(GameConfiguration));

	// Get current display mode
	vector<RendererVideoAdapter>* adapters = g_Renderer.getAdapters();
	RendererVideoAdapter* adapter = &(*adapters)[ring->Configuration.Adapter];
	ring->SelectedVideoMode = 0;
	for (int i = 0; i < adapter->DisplayModes.size(); i++)
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
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
				if (ring->SelectedVideoMode > 0)
					ring->SelectedVideoMode--;

				break;

			case INV_DISPLAY_WINDOWED:
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
				ring->Configuration.Windowed = !ring->Configuration.Windowed;
				break;

			case INV_DISPLAY_SHADOWS:
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableShadows = !ring->Configuration.EnableShadows;
				break;

			case INV_DISPLAY_CAUSTICS:
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableCaustics = !ring->Configuration.EnableCaustics;
				break;

			case INV_DISPLAY_VOLUMETRIC_FOG:
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
				if (ring->SelectedVideoMode < adapter->DisplayModes.size() - 1)
					ring->SelectedVideoMode++;
				break;

			case INV_DISPLAY_WINDOWED:
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
				ring->Configuration.Windowed = !ring->Configuration.Windowed;
				break;

			case INV_DISPLAY_SHADOWS:
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableShadows = !ring->Configuration.EnableShadows;
				break;

			case INV_DISPLAY_CAUSTICS:
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableCaustics = !ring->Configuration.EnableCaustics;
				break;

			case INV_DISPLAY_VOLUMETRIC_FOG:
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableVolumetricFog = !ring->Configuration.EnableVolumetricFog;
				break;
			}
		}
		else if (DbInput & IN_FORWARD)
		{
			closeObject = false;

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex > 0)
				ring->selectedIndex--;
		}
		else if (DbInput & IN_BACK)
		{
			closeObject = false;

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex < INV_DISPLAY_COUNT)
				ring->selectedIndex++;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

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
				g_Renderer.changeScreenResolution(ring->Configuration.Width, ring->Configuration.Height,
					ring->Configuration.RefreshRate, ring->Configuration.Windowed);
				closeObject = true;

				break;
			}
			else if (ring->selectedIndex == INV_DISPLAY_CANCEL)
			{
				SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

				closeObject = true;
				break;
			}
			else
			{

			}
		}

		UpdateSceneAndDrawInventory();
	}

	PopoverObject();
}

void Inventory::DoSoundSettings()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	ring->framePtr = 0;
	ring->selectedIndex = 0;

	PopupObject();

	// Copy configuration to a temporary object
	memcpy(&ring->Configuration, &g_Configuration, sizeof(GameConfiguration));

	bool closeObject = false;
	int oldVolume = ring->Configuration.MusicVolume;
	int oldSfxVolume = ring->Configuration.SfxVolume;
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
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableSound = !ring->Configuration.EnableSound;

				break;

			case INV_SOUND_SPECIAL_EFFECTS:
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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
					SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
				ring->Configuration.EnableSound = !ring->Configuration.EnableSound;

				break;

			case INV_SOUND_SPECIAL_EFFECTS:
				SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
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
					SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
				}

				break;
			}
		}
		else if (DbInput & IN_FORWARD)
		{
			closeObject = false;

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex > 0)
				ring->selectedIndex--;
		}
		else if (DbInput & IN_BACK)
		{
			closeObject = false;

			SoundEffect(SFX_TR4_MENU_CHOOSE, NULL, 0);
			if (ring->selectedIndex < INV_DISPLAY_COUNT)
				ring->selectedIndex++;
		}
		else if (DbInput & IN_SELECT)
		{
			SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

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
				SoundEffect(SFX_TR4_MENU_SELECT, NULL, 0);

				closeObject = true;
				GlobalMusicVolume = oldVolume;
				GlobalFXVolume = oldSfxVolume;

				break;
			}
			else
			{

			}
		}

		UpdateSceneAndDrawInventory();
	}

	PopoverObject();
}

int Inventory::GetActiveGui()
{
	return m_activeGui;
}

bool Inventory::IsObjectSeparable(short object)
{
	for (int i = 0; i < m_combinations.size(); i++)
		if (m_combinations[i].combinedObject == object)
			return true;
	return false;
}

void Inventory::OpenRing(int r, bool animateCamera)
{
	InventoryRing* ring = &m_rings[r];

	m_activeRing = r;
	m_cameraY = ring->y;

	int numFrames = INV_NUM_FRAMES_OPEN_CLOSE;
	ring->rotation = 90;
	ring->distance = 0;
	float deltaAngle = ring->rotation / numFrames;
	float deltaShift = INV_OBJECTS_DISTANCE / numFrames;

	float deltaTilt = 0;
	m_cameraTilt = INV_CAMERA_TILT;
	if (animateCamera)
	{
		deltaTilt = (INV_CAMERA_ANIMATION_TILT - INV_CAMERA_TILT) / numFrames;
		m_cameraTilt = INV_CAMERA_ANIMATION_TILT;
	}

	for (int i = 0; i < numFrames; i++)
	{
		ring->distance += deltaShift;
		ring->rotation -= deltaAngle;

		m_cameraTilt -= deltaTilt;

		UpdateSceneAndDrawInventory();
	}

	m_cameraTilt = INV_CAMERA_TILT;
	m_cameraY = m_rings[r].y;
	ring->distance = INV_OBJECTS_DISTANCE;
	ring->rotation = 0;
}

void Inventory::CloseRing(int r, bool animateCamera)
{
	InventoryRing* ring = &m_rings[r];

	int numFrames = INV_NUM_FRAMES_OPEN_CLOSE;
	ring->rotation = 0;
	ring->distance = INV_OBJECTS_DISTANCE;
	float deltaAngle = 90.0f / numFrames;
	float deltaShift = INV_OBJECTS_DISTANCE / numFrames;

	float deltaTilt = 0;
	m_cameraTilt = INV_CAMERA_TILT;
	if (animateCamera)
	{
		deltaTilt = (INV_CAMERA_ANIMATION_TILT - INV_CAMERA_TILT) / numFrames;
		m_cameraTilt = INV_CAMERA_TILT;
	}

	for (int i = 0; i < numFrames; i++)
	{
		ring->distance -= deltaShift;
		ring->rotation += deltaAngle;

		m_cameraTilt += deltaTilt;

		UpdateSceneAndDrawInventory();
	}

	m_cameraTilt = (animateCamera ? INV_CAMERA_ANIMATION_TILT : INV_CAMERA_TILT);

	ring->distance = 0;
	ring->rotation = 90;
}

void Inventory::SwitchRing(int from, int to, float verticalShift)
{
	InventoryRing* ring1 = &m_rings[from];
	InventoryRing* ring2 = &m_rings[to];

	int numFrames = INV_NUM_FRAMES_OPEN_CLOSE;

	ring1->rotation = 0;
	ring1->distance = INV_OBJECTS_DISTANCE;
	ring2->rotation = 90;
	ring2->distance = 0;

	float deltaAngle = 90.0f / numFrames;
	float deltaShift = INV_OBJECTS_DISTANCE / numFrames;
	float deltaY = INV_RINGS_OFFSET * verticalShift / numFrames;

	m_cameraTilt = INV_CAMERA_TILT;

	ring1->draw = true;
	ring2->draw = false;

	for (int i = 0; i < numFrames; i++)
	{
		ring1->distance -= deltaShift;
		ring1->rotation += deltaAngle;

		ring2->distance += deltaShift;
		ring2->rotation -= deltaAngle;

		m_cameraY += deltaY;

		if (i >= 2)
		{
			ring1->draw = false;
			ring2->draw = true;
		}

		UpdateSceneAndDrawInventory();
	}

	ring1->distance = 0;
	ring1->rotation = 90;

	ring2->distance = INV_OBJECTS_DISTANCE;
	ring2->rotation = 0;

	m_cameraY = ring2->y;
}

float Inventory::GetCameraY()
{
	return m_cameraY;
}

float Inventory::GetCameraTilt()
{
	return m_cameraTilt;
}

bool Inventory::HasWeaponMultipleAmmos(short object)
{
	return (object == _INV_OBJECT_SHOTGUN || object == _INV_OBJECT_CROSSBOW || object == _INV_OBJECT_GRENADE_LAUNCHER);
}
#endif