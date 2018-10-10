#include "inventory.h"
#include "draw.h"
#include "control.h"
#include "larafire.h"
#include "sound.h"
#include "gameflow.h"
#include "sound.h"
#include "savegame.h"

#include "..\Global\global.h"
#include "..\Specific\input.h"

Inventory* g_Inventory;
extern GameFlow* g_GameFlow;

void Inject_Inventory()
{

}

Inventory::Inventory()
{
	ZeroMemory(&m_objectsTable[0], sizeof(InventoryObjectDefinition) * INVENTORY_TABLE_SIZE);

	// Copy the old table
	for (__int32 i = 0; i < 100; i++)
	{
		m_objectsTable[i].objectNumber = InventoryObjectsList[i].objectNumber;
		m_objectsTable[i].objectName = InventoryObjectsList[i].objectNumber;
		m_objectsTable[i].meshBits = InventoryObjectsList[i].meshBits;
	}

	// Assign new objects
	m_objectsTable[INV_OBJECT_PASSAPORT].objectNumber = ID_INVENTORY_PASSPORT;
	m_objectsTable[INV_OBJECT_PASSAPORT].objectName = STRING_INV_PASSPORT;
	m_objectsTable[INV_OBJECT_PASSAPORT].meshBits = -1;

	m_objectsTable[INV_OBJECT_KEYS].objectNumber = ID_INVENTORY_KEYS;
	m_objectsTable[INV_OBJECT_KEYS].objectName = STRING_INV_CONTROLS;
	m_objectsTable[INV_OBJECT_KEYS].meshBits = -1;

	m_objectsTable[INV_OBJECT_SUNGLASSES].objectNumber = ID_INVENTORY_SUNGLASSES;
	m_objectsTable[INV_OBJECT_SUNGLASSES].objectName = STRING_INV_DISPLAY;
	m_objectsTable[INV_OBJECT_SUNGLASSES].meshBits = -1;

	m_objectsTable[INV_OBJECT_POLAROID].objectNumber = ID_INVENTORY_POLAROID;
	m_objectsTable[INV_OBJECT_POLAROID].objectName = STRING_INV_LARA_HOME;
	m_objectsTable[INV_OBJECT_POLAROID].meshBits = -1;
	m_objectsTable[INV_OBJECT_POLAROID].rotY = 16384;

	m_objectsTable[INV_OBJECT_HEADPHONES].objectNumber = ID_INVENTORY_HEADPHONES;
	m_objectsTable[INV_OBJECT_HEADPHONES].objectName = STRING_INV_SOUND;
	m_objectsTable[INV_OBJECT_HEADPHONES].meshBits = -1;
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

void Inventory::Initialise()
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
		//Lara.uzisTypeCarried = 1;
		//Lara.numUziAmmo = 1000;

		/*Lara.shotgunTypeCarried = 1;
		Lara.numShotgunAmmo1 = 1000;
		Lara.numShotgunAmmo2 = 1000;

		Lara.numRevolverAmmo = 1000;
		Lara.numShotgunAmmo2 = 1000;
		Lara.crowbar = 1;

		Lara.sixshooterTypeCarried = 1;*/
		//Lara.uzisTypeCarried = 1;
		//Lara.numUziAmmo = 10000;
				//Lara.crossbowTypeCarried = 1;
		//Lara.numCrossbowAmmo1 = 1000;
	}
	
	// Now fill the rings
	if (!(gfLevelFlags & 1))
	{
		if (Lara.pistolsTypeCarried & 1)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_PISTOLS);

		if (Lara.uzisTypeCarried & 1)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_UZIS);

		if (Lara.numUziAmmo)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_UZI_AMMO);
		
		if (Lara.sixshooterTypeCarried & 1)
		{
			if (Lara.sixshooterTypeCarried & 4)
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_REVOLVER_LASER);
			else
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_REVOLVER);
		}
		
		if (Lara.numRevolverAmmo)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_REVOLVER_AMMO);
		
		if (Lara.shotgunTypeCarried & 1)
		{
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_SHOTGUN);
			//if (Lara.shotgunTypeCarried & 0x10)
			//	CurrentShotGunAmmoType = 1;
		}
		else
		{
			if (Lara.numShotgunAmmo1)
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_SHOTGUN_AMMO1);
			if (Lara.numShotgunAmmo2)
				InsertObject(INV_RING_WEAPONS, INV_OBJECT_SHOTGUN_AMMO2);
		}

		if (Lara.HKtypeCarried & 1)
		{
			if (Lara.HKtypeCarried & 2)
				InsertObject(INV_RING_WEAPONS, 8);
			else
				InsertObject(INV_RING_WEAPONS, 7);

			if (Lara.HKtypeCarried & 0x10)
			{
				//CurrentGrenadeGunAmmoType = 1;
			}
			else if (Lara.HKtypeCarried & 0x20)
			{
				//CurrentGrenadeGunAmmoType = 2;
			}
		}
		
		if (Lara.numHKammo1)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_HK_AMMO1);
		
		if (Lara.crossbowTypeCarried & 1)
		{
			if (CurrentLevel < 0xBu || CurrentLevel > 0xEu)
			{
				if (Lara.crossbowTypeCarried & 4)
					InsertObject(INV_RING_WEAPONS, 6);
				else
					InsertObject(INV_RING_WEAPONS, 5);
				//if (Lara.crossbowTypeCarried & 0x10)
				//	CurrentCrossBowAmmoType = 1;
			}
			else
			{
				InsertObject(INV_RING_WEAPONS, 95);
				//CurrentCrossBowAmmoType = 0;
			}
		}
		
		if (CurrentLevel < 0xBu || CurrentLevel > 0xEu)
		{
			if (Lara.numCrossbowAmmo1)
				InsertObject(INV_RING_WEAPONS, 15);
			if (Lara.numCrossbowAmmo2)
				InsertObject(INV_RING_WEAPONS, 16);
		}

		if (Lara.numCrossbowAmmo1)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_CROSSBOW_AMMO1);

		if (Lara.laserSight)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_LASERSIGHT);

		if (Lara.silencer)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_SILENCER);

		if (Lara.binoculars)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_BINOCULARS);

		if (Lara.numFlares)
			InsertObject(INV_RING_WEAPONS, INV_OBJECT_FLARES);
	}

	//InsertObject(INV_RING_OPTIONS, 26);

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
		ring->objects[j].scale = 2.0f;
	}
	  
	InsertObject(INV_RING_OPTIONS, INV_OBJECT_PASSAPORT);
	InsertObject(INV_RING_OPTIONS, INV_OBJECT_SUNGLASSES);
	InsertObject(INV_RING_OPTIONS, INV_OBJECT_HEADPHONES);
	InsertObject(INV_RING_OPTIONS, INV_OBJECT_KEYS);

	m_activeRing = INV_RING_WEAPONS;
	m_type = INV_TYPE_GAME;
	m_deltaMovement = 0;
	m_movement = INV_MOVE_STOPPED;
	m_type = INV_TYPE_GAME;
	InventoryItemChosen = -1;
}

INVENTORY_RESULT Inventory::DoInventory()
{
	Initialise();

	// If Lara is dead, then we can use only the passport
	if (LaraItem->hitPoints <= 0 && CurrentLevel > 0)
	{
		m_rings[INV_RING_PUZZLES].draw = false;
		m_rings[INV_RING_WEAPONS].draw = false;
		m_rings[INV_RING_OPTIONS].draw = true;

		m_activeRing = INV_RING_OPTIONS;
		m_rings[m_activeRing].currentObject = 0;

		INVENTORY_RESULT passportResult = DoPassport();

		// Fade out
		g_Renderer->FadeOut();
		for (__int32 i = 0; i < FADE_FRAMES_COUNT; i++)
			g_Renderer->DrawInventory();

		return passportResult;
	}

	m_rings[INV_RING_PUZZLES].draw = true;
	m_rings[INV_RING_WEAPONS].draw = true;
	m_rings[INV_RING_OPTIONS].draw = true;

	m_activeRing = INV_RING_WEAPONS;

	INVENTORY_RESULT result = INVENTORY_RESULT::INVENTORY_RESULT_NONE;

	g_Renderer->DumpGameScene();
	g_Renderer->DrawInventory();

	while (!ResetFlag)
	{
		SetDebounce = true;
		S_UpdateInput();
		SetDebounce = false;

		GameTimer++;

		// Handle input
		if (DbInput & 0x200000)
		{
			SoundEffect(SFX_MENU_SELECT, NULL, 0);

			// Exit from inventory
			GlobalEnterInventory = -1;
			return INVENTORY_RESULT::INVENTORY_RESULT_NONE;
		}
		else if (DbInput & 1 &&
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
		else if (DbInput & 2 && (m_activeRing == INV_RING_PUZZLES || m_activeRing == INV_RING_WEAPONS))
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
		else if (DbInput & 4)
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

			m_rings[m_activeRing].movement = 0;
		}
		else if (DbInput & 8)
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

			m_rings[m_activeRing].movement = 0;
		}
		else if (DbInput & 0x100000)
		{
			// Handle action 
			if (m_activeRing == INV_RING_OPTIONS)
			{
				if (m_rings[INV_RING_OPTIONS].objects[m_rings[INV_RING_OPTIONS].currentObject].inventoryObject == INV_OBJECT_PASSAPORT)
				{
					INVENTORY_RESULT passportResult = DoPassport();
					if (passportResult == INVENTORY_RESULT::INVENTORY_RESULT_NEW_GAME ||
						passportResult == INVENTORY_RESULT::INVENTORY_RESULT_EXIT_TO_TILE ||
						passportResult == INVENTORY_RESULT::INVENTORY_RESULT_LOAD_GAME)
					{
						// Fade out
						g_Renderer->FadeOut();
						for (__int32 i = 0; i < FADE_FRAMES_COUNT; i++)
							g_Renderer->DrawInventory();

						return passportResult;
					}
				}

				if (m_rings[INV_RING_OPTIONS].objects[m_rings[INV_RING_OPTIONS].currentObject].inventoryObject == INV_OBJECT_KEYS)
					DoControlsSettings();

				if (m_rings[INV_RING_OPTIONS].objects[m_rings[INV_RING_OPTIONS].currentObject].inventoryObject == INV_OBJECT_SUNGLASSES)
					DoGraphicsSettings();

				if (m_rings[INV_RING_OPTIONS].objects[m_rings[INV_RING_OPTIONS].currentObject].inventoryObject == INV_OBJECT_HEADPHONES)
					DoSoundSettings();
			}
			else if (m_activeRing == INV_RING_WEAPONS || m_activeRing == INV_RING_PUZZLES)
			{
				UseCurrentItem();

				// Exit from inventory
				GlobalEnterInventory = -1;
				return INVENTORY_RESULT::INVENTORY_RESULT_USE_ITEM;
			}
		}

		g_Renderer->DrawInventory();
		g_Renderer->SyncRenderer();
	}

	return result;
}

void Inventory::UseCurrentItem()
{
	InventoryRing* ring = &m_rings[m_activeRing];
	InventoryObject* inventoryObject = &ring->objects[ring->currentObject];
	__int16 objectNumber = m_objectsTable[inventoryObject->inventoryObject].objectNumber;

	LaraItem->meshBits = -1;

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
		/*
		if ( lara_item->current_anim_state == 2 && lara_item->anim_number == 103
        || (LOBYTE(objectNum) = *(&lara + 68), *(&lara + 69) & 8) && !(input & 0x20000000) )
      {
        LOBYTE(objectNum) = SniperCamActive;
        if ( !SniperCamActive && !bUseSpotCam && !bTrackCamInit )
        {
          oldLaraBusy = 1;
          BinocularRange = 128;
          if ( lara.gun_status )
            lara.gun_status = 3;
        }
      }
      if ( v0 )
        BinocularRange = v0;
      else
        BinocularOldCamera = dword_EEF964;
      return objectNum;
	  */
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

	bool canUseWeapons = !(LaraItem->currentAnimState == 80 || LaraItem->currentAnimState == 81 || 
						   LaraItem->currentAnimState == 84 || LaraItem->currentAnimState == 85 || 
						   LaraItem->currentAnimState == 86 || LaraItem->currentAnimState == 88 || 
						   LaraItem->currentAnimState == 71 || LaraItem->currentAnimState == 105 || 
						   LaraItem->currentAnimState == 106 || Lara.waterStatus != LW_ABOVE_WATER);

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
			return;
		}
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
			return;
		}
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
			return;
		}
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
			return;
		}
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
			return;
		}
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
			return;
		}
	}

	// Flares
	if (objectNumber == ID_FLARE_INV_ITEM)
	{
		if (!Lara.gunStatus)
		{
			if (LaraItem->currentAnimState != 80
				&& LaraItem->currentAnimState != 81
				&& LaraItem->currentAnimState != 84
				&& LaraItem->currentAnimState != 85
				&& LaraItem->currentAnimState != 86
				&& LaraItem->currentAnimState != 88
				&& Lara.waterStatus == LW_ABOVE_WATER)
			{
				if (Lara.gunType != WEAPON_FLARE)
				{
					TrInput = 0x80000;
					LaraGun();
					TrInput = 0;

					SoundEffect(SFX_MENU_CHOOSE, NULL, 0);
				}
				return;
			}
		}
		else
		{
			SayNo();
			return;
		}
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

INVENTORY_RESULT Inventory::DoTitleInventory()
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

	INVENTORY_RESULT result = INVENTORY_RESULT::INVENTORY_RESULT_NONE;

	while (!ResetFlag)
	{
		SetDebounce = true;
		S_UpdateInput();
		SetDebounce = false;

		GameTimer++;

		// Handle input
		if (DbInput & 4)
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
		else if (DbInput & 8)
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
		else if (DbInput & 0x100000)
		{
			SoundEffect(SFX_MENU_SELECT, NULL, 0);

			if (ring->objects[ring->currentObject].inventoryObject == INV_OBJECT_PASSAPORT)
			{
				INVENTORY_RESULT passportResult = DoPassport();
				if (passportResult == INVENTORY_RESULT::INVENTORY_RESULT_NEW_GAME ||
					passportResult == INVENTORY_RESULT::INVENTORY_RESULT_EXIT_GAME ||
					passportResult == INVENTORY_RESULT::INVENTORY_RESULT_LOAD_GAME)
				{
					// Fade out
					g_Renderer->FadeOut();
					for (__int32 i = 0; i < FADE_FRAMES_COUNT; i++)
						g_Renderer->DrawInventory();

					return result;
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

INVENTORY_RESULT Inventory::DoPassport()
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

	INVENTORY_RESULT result = INVENTORY_RESULT::INVENTORY_RESULT_NONE;

	// Do the passport
	while (true)
	{
		// Handle input
		SetDebounce = true;
		S_UpdateInput();
		SetDebounce = false;

		GameTimer++;

		// Handle input
		if (DbInput & 0x200000 || closePassport)
		{
			moveLeft = false;
			moveRight = false;
			closePassport = false;

			break;
		}
		else if (DbInput & 4 || moveLeft)
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
		else if (DbInput & 8 || moveRight)
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
				if (DbInput & 0x200000)
				{
					if (CurrentLevel == 0 || LaraItem->hitPoints > 0)
					{
						moveLeft = false;
						moveRight = false;
						closePassport = true;
					}

					break;
				}
				else if (DbInput & 1 && selectedSavegame > 0)
				{
					selectedSavegame--;
					continue;
				}
				else if (DbInput & 2 && selectedSavegame < MAX_SAVEGAMES - 1)
				{
					selectedSavegame++;
					continue;
				}
				else if (DbInput & 4)
				{
					moveLeft = true;
					moveRight = false;
					closePassport = false;

					break;
				}
				else if (DbInput & 8)
				{
					moveLeft = false;
					moveRight = true;
					closePassport = false;

					break;
				}
				else if (DbInput & 0x100000)
				{
					ReadSavegame(selectedSavegame);
					result = INVENTORY_RESULT::INVENTORY_RESULT_LOAD_GAME;
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
				if (DbInput & 0x200000)
				{
					if (CurrentLevel == 0 || LaraItem->hitPoints > 0)
					{
						moveLeft = false;
						moveRight = false;
						closePassport = true;
					}

					break;
				}
				else if (DbInput & 1 && selectedSavegame > 0)
				{
					selectedSavegame--;
					continue;
				}
				else if (DbInput & 2 && selectedSavegame < MAX_SAVEGAMES - 1)
				{
					selectedSavegame++;
					continue;
				}
				else if (DbInput & 4)
				{
					moveLeft = true;
					moveRight = false;
					closePassport = false;

					break;
				}
				else if (DbInput & 8)
				{
					moveLeft = false;
					moveRight = true;
					closePassport = false;

					break;
				}
				else if (DbInput & 0x100000)
				{
					//CreateSavegame();
					//WriteSavegame(selectedSavegame);

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
				if (DbInput & 0x200000)
				{
					if (CurrentLevel == 0 || LaraItem->hitPoints > 0)
					{
						moveLeft = false;
						moveRight = false;
						closePassport = true;
					}

					break;
				}
				else if (DbInput & 1 && selectedLevel > 0)
				{
					selectedLevel--;
					continue;
				}
				else if (DbInput & 2 && selectedLevel < g_GameFlow->GetNumLevels() - 1)
				{
					selectedLevel++;
					continue;
				}
				else if (DbInput & 4)
				{
					moveLeft = true;
					moveRight = false;
					closePassport = false;

					break;
				}
				else if (DbInput & 8)
				{
					moveLeft = false;
					moveRight = true;
					closePassport = false;

					break;
				}
				else if (DbInput & 0x100000)
				{
					result = INVENTORY_RESULT::INVENTORY_RESULT_NEW_GAME;
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
				if (DbInput & 0x200000)
				{
					if (CurrentLevel == 0 || LaraItem->hitPoints > 0)
					{
						moveLeft = false;
						moveRight = false;
						closePassport = true;
					}

					break;
				}
				else if (DbInput & 4)
				{
					moveLeft = true;
					moveRight = false;
					closePassport = false;

					break;
				}
				else if (DbInput & 8)
				{
					moveLeft = false;
					moveRight = true;
					closePassport = false;

					break;
				}
				else if (DbInput & 0x100000)
				{
					result = INVENTORY_RESULT::INVENTORY_RESULT_NEW_GAME;
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
				if (DbInput & 0x200000)
				{
					if (CurrentLevel == 0 || LaraItem->hitPoints > 0)
					{
						moveLeft = false;
						moveRight = false;
						closePassport = true;
					}

					break;
				}
				else if (DbInput & 4)
				{
					moveLeft = true;
					moveRight = false;
					closePassport = false;

					break;
				}
				else if (DbInput & 8)
				{
					moveLeft = false;
					moveRight = true;
					closePassport = false;

					break;
				}
				else if (DbInput & 0x100000)
				{
					result = INVENTORY_RESULT::INVENTORY_RESULT_EXIT_GAME;
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
				if (DbInput & 0x200000)
				{
					if (CurrentLevel == 0 || LaraItem->hitPoints > 0)
					{
						moveLeft = false;
						moveRight = false;
						closePassport = true;
					}

					break;
				}
				else if (DbInput & 4)
				{
					moveLeft = true;
					moveRight = false;
					closePassport = false;

					break;
				}
				else if (DbInput & 8)
				{
					moveLeft = false;
					moveRight = true;
					closePassport = false;

					break;
				}
				else if (DbInput & 0x100000)
				{
					result = INVENTORY_RESULT::INVENTORY_RESULT_EXIT_TO_TILE;
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

}

void Inventory::DoSoundSettings()
{

}