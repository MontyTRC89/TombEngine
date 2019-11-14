#include "pickup.h"
#include "Lara.h"
#include "draw.h"
#include "inventory.h"
#include "effects.h"
#include "effect2.h"
#include "control.h"
#include "sphere.h"
#include "debris.h"
#include "box.h"
#include "healt.h"
#include "items.h"
#include "collide.h"
#include "Lara.h"
#include "switch.h"
#include "larafire.h"
#include "laraflar.h"
#include "lara1gun.h"
#include "lara2gun.h"

#include "..\Global\global.h"

static __int16 PickUpBounds[12] = // offset 0xA1338
{
	0xFF00, 0x0100, 0xFF38, 0x00C8, 0xFF00, 0x0100, 0xF8E4, 0x071C, 0x0000, 0x0000,
	0x0000, 0x0000
};
static PHD_VECTOR PickUpPosition = { 0, 0, 0xFFFFFF9C }; // offset 0xA1350
static __int16 HiddenPickUpBounds[12] = // offset 0xA135C
{
	0xFF00, 0x0100, 0xFF9C, 0x0064, 0xFCE0, 0xFF00, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0x0000, 0x0000
};
static PHD_VECTOR HiddenPickUpPosition = { 0, 0, 0xFFFFFD4E }; // offset 0xA1374
static __int16 CrowbarPickUpBounds[12] = // offset 0xA1380
{
	0xFF00, 0x0100, 0xFF9C, 0x0064, 0x00C8, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0x0000, 0x0000
};
static PHD_VECTOR CrowbarPickUpPosition = { 0, 0, 0xD7 }; // offset 0xA1398
static __int16 JobyCrowPickUpBounds[12] = // offset 0xA13A4
{
	0xFE00, 0x0000, 0xFF9C, 0x0064, 0x0000, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0x0000, 0x0000
};
static PHD_VECTOR JobyCrowPickUpPosition = { 0xFFFFFF20, 0, 0xF0 }; // offset 0xA13BC
static __int16 PlinthPickUpBounds[12] = // offset 0xA13C8
{
	0xFF00, 0x0100, 0xFD80, 0x0280, 0xFE01, 0x0000, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0x0000, 0x0000
};
static PHD_VECTOR PlinthPickUpPosition = { 0, 0, 0xFFFFFE34 }; // offset 0xA13E0
static __int16 PickUpBoundsUW[12] = // offset 0xA13EC
{
	0xFE00, 0x0200, 0xFE00, 0x0200, 0xFE00, 0x0200, 0xE002, 0x1FFE, 0xE002, 0x1FFE,
	0xE002, 0x1FFE
};
static PHD_VECTOR PickUpPositionUW = { 0, 0xFFFFFF38, 0xFFFFFEA2 }; // offset 0xA1404
static __int16 KeyHoleBounds[12] = // offset 0xA1410
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0x0000, 0x019C, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
static PHD_VECTOR KeyHolePosition = { 0, 0, 0x138 }; // offset 0xA1428
static __int16 PuzzleBounds[12] = // offset 0xA1434
{
	0x0000, 0x0000, 0xFF00, 0x0100, 0x0000, 0x0000, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
static __int16 SOBounds[12] = // offset 0xA144C
{
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
static PHD_VECTOR SOPos = { 0, 0, 0 }; // offset 0xA1464
__int16 SearchCollectFrames[4] =
{
	0x00B4, 0x0064, 0x0099, 0x0053
};
__int16 SearchAnims[4] =
{
	0x01D0, 0x01D1, 0x01D2, 0x01D8
};
__int16 SearchOffsets[4] =
{
	0x00A0, 0x0060, 0x00A0, 0x0070
};
static __int16 MSBounds[12] = // offset 0xA1488
{
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};

__int32 KeyTriggerActive;
__int32 NumRPickups;
__int16 RPickups[16];

extern LaraExtraInfo g_LaraExtra;
extern Inventory* g_Inventory;

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

	case ID_WATERSKIN1_EMPTY:
		g_LaraExtra.Waterskin1.Present = true;
		g_LaraExtra.Waterskin1.Quantity = 0;
		break;

	case ID_WATERSKIN2_EMPTY:
		g_LaraExtra.Waterskin2.Present = true;
		g_LaraExtra.Waterskin2.Quantity = 0;
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

	g_Inventory->LoadObjects(false);
}

void __cdecl RemoveObjectFromInventory(__int16 objectNumber)
{
	if (objectNumber >= ID_PUZZLE_ITEM1 && objectNumber <= ID_PUZZLE_ITEM8)
		Lara.puzzleItems[objectNumber - ID_PUZZLE_ITEM1]--;

	else if (objectNumber >= ID_PUZZLE_ITEM1_COMBO1 && objectNumber <= ID_PUZZLE_ITEM8_COMBO2)
		Lara.puzzleItemsCombo &= ~(1 << (objectNumber - ID_PUZZLE_ITEM1_COMBO1));

	else if (objectNumber >= ID_KEY_ITEM1 && objectNumber <= ID_KEY_ITEM8)
		Lara.keyItems &= ~(1 << (objectNumber - ID_KEY_ITEM1));

	else if (objectNumber >= ID_KEY_ITEM1_COMBO1 && objectNumber <= ID_KEY_ITEM8_COMBO2)
		Lara.keyItemsCombo &= ~(1 << (objectNumber - ID_KEY_ITEM1_COMBO1));

	else if (objectNumber >= ID_PICKUP_ITEM1 && objectNumber <= ID_PICKUP_ITEM4)
		Lara.pickupItems &= ~(1 << (objectNumber - ID_PICKUP_ITEM1));

	else if (objectNumber >= ID_PICKUP_ITEM1_COMBO1 && objectNumber <= ID_PICKUP_ITEM4_COMBO2)
		Lara.pickupItemsCombo &= ~(1 << (objectNumber - ID_PICKUP_ITEM1_COMBO1));

	g_Inventory->LoadObjects(false);
}

void __cdecl CollectCarriedItems(ITEM_INFO* item) 
{
	__int16 pickupNumber = item->carriedItem;
	if (pickupNumber != NO_ITEM)
	{
		while (pickupNumber != NO_ITEM)
		{
			ITEM_INFO* pickup = &Items[pickupNumber];

			AddDisplayPickup(pickup->objectNumber);
			KillItem(pickupNumber);

			pickupNumber = pickup->carriedItem;
		}
	}
}

/*void __cdecl SearchObjectCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	UNIMPLEMENTED();
}

void SearchObjectControl(__int16 itemNumber)//52D54, 531B8
{
	UNIMPLEMENTED();
}*/

__int32 __cdecl PickupTrigger(__int16 itemNum) 
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->flags & IFLAG_KILLED && item->status != ITEM_INVISIBLE && item->itemFlags[3] != 1 && item->triggerFlags & 0x80)
	{
		return 0;
	}

	KillItem(itemNum);

	return 1;
}

void __cdecl ActivateKey()
{
	KeyTriggerActive = 1;
}

void __cdecl ActivateCamera()
{
	KeyTriggerActive = 2;
}

__int32 __cdecl KeyTrigger(__int16 itemNum) 
{
	ITEM_INFO* item = &Items[itemNum];
	__int32 oldkey;

	if ((item->status != ITEM_ACTIVE || Lara.gunStatus == LG_HANDS_BUSY) && (!KeyTriggerActive || Lara.gunStatus != LG_HANDS_BUSY))
		return -1;

	oldkey = KeyTriggerActive;

	if (!oldkey)
		item->status = ITEM_DEACTIVATED;

	KeyTriggerActive = false;

	return oldkey;
}

void __cdecl PuzzleHoleCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &Items[itemNum];
	__int32 flag = 0;
	
	if (item->triggerFlags >= 0)
	{
		if (item->triggerFlags <= 1024)
		{
			if (item->triggerFlags && item->triggerFlags != 999 && item->triggerFlags != 998)
				flag = 3;
		}
		else
			flag = 2;
	}
	else
		flag = 1;

	if (!(TrInput & IN_ACTION && InventoryItemChosen == NO_ITEM)
		|| BinocularRange
		|| Lara.gunStatus
		|| l->currentAnimState != STATE_LARA_STOP
		|| l->animNumber != ANIMATION_LARA_STAY_IDLE
		|| !GetKeyTrigger(&Items[itemNum]))
	{
		if (!Lara.isMoving && (__int16)Lara.generalPtr == itemNum || (__int16)Lara.generalPtr != itemNum)
		{
			if ((__int16)Lara.generalPtr == itemNum && l->currentAnimState == STATE_LARA_INSERT_PUZZLE)
			{
				if (l->frameNumber == Anims[ANIMATION_LARA_USE_PUZZLE].frameBase + 80 && item->itemFlags[0])
				{
					if (flag == 3)
						l->itemFlags[0] = item->triggerFlags;
					else
						l->itemFlags[0] = 0;
					PuzzleDone(item, itemNum);
					item->itemFlags[0] = 0;
					return;
				}
			}

			if ((__int16)Lara.generalPtr == itemNum)
			{
				if (l->currentAnimState != STATE_LARA_MISC_CONTROL)
				{
					if (flag != 2)
						ObjectCollision(itemNum, l, coll);
					return;
				}
				if (l->animNumber == ANIMATION_LARA_PUT_TRIDENT)
				{
					if (l->frameNumber == Anims[l->animNumber].frameBase + 180)
					{
						PuzzleDone(item, itemNum);
						return;
					}
				}
			}
			if (l->currentAnimState == STATE_LARA_MISC_CONTROL)
				return;

			if (flag != 2)
				ObjectCollision(itemNum, l, coll);
			return;
		}
	}

	__int16 oldYrot = item->pos.yRot;
	__int16* bounds = GetBoundsAccurate(item);

	PuzzleBounds[0] = bounds[0] - 256;
	PuzzleBounds[1] = bounds[1] + 256;
	PuzzleBounds[4] = bounds[4] - 256;
	PuzzleBounds[5] = bounds[5] + 256;

	if (item->triggerFlags == 1058)
	{
		PuzzleBounds[0] = bounds[0] - 256 - 300;
		PuzzleBounds[1] = bounds[1] + 256 + 300;
		PuzzleBounds[4] = bounds[4] - 256 - 300;
		PuzzleBounds[5] = bounds[5] + 256 + 300;

		item->pos.yRot = l->pos.yRot;

		/*v13 = word_509C0E;
		v12 = word_509C0C;
		v11 = word_509C06;
		v10 = word_509C04;*/
	}

	/*if (v20 == 7 && gfCurrentLevel >= 0xBu && gfCurrentLevel <= 0xCu)
	{
		word_509C0C = v12 - 512;
		word_509C04 = v10 - 512;
		word_509C06 = v11 + 512;
		word_509C0E = v13 + 512;
		v19 = 0;
	}*/

	if (TestLaraPosition(PuzzleBounds, item, l))
	{
		PHD_VECTOR pos;
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;
		
		/*if (item->objectNumber == ID_PUZZLE_HOLE8 && gfCurrentLevel >= 0xBu && gfCurrentLevel <= 0xCu)
		{
			pos.z = coll->midType - 100;
			if (MoveLaraPosition(&pos, item, l))
			{
				Lara.isMoving = false;
				l->animNumber = ANIMATION_LARA_STAY_IDLE;
				l->currentAnimState = STATE_LARA_STOP;
				l->frameNumber = Anims[l->animNumber].frameBase;
				//GLOBAL_invkeypadcombination = item->triggerFlags;
				//GLOBAL_enterinventory = -559038737;
			}
			else
			{
				Lara.generalPtr = (void*)itemNum;
				InventoryItemChosen = NO_ITEM
			}
			return;
		}*/

		if (!Lara.isMoving)
		{
			if (InventoryItemChosen == NO_ITEM)
			{
				if (g_Inventory->IsObjectPresentInInventory(item->objectNumber - 70))
					EnterInventoryItem = item->objectNumber - 70;
				item->pos.yRot = oldYrot;
				return;
			}
			if (InventoryItemChosen != item->objectNumber - 70)
			{
				item->pos.yRot = oldYrot;
				return;
			}
		}

		pos.z = bounds[4] - 100;
		if (flag != 2 || item->triggerFlags == 1036)
		{
			if (!MoveLaraPosition(&pos, item, l))
			{
				Lara.generalPtr = (void*)itemNum;
				InventoryItemChosen = NO_ITEM;
				item->pos.yRot = oldYrot;
				return;
			}
		}

		RemoveObjectFromInventory(item->objectNumber - 70);

		if (flag == 1)
		{
			l->currentAnimState = STATE_LARA_MISC_CONTROL;
			l->animNumber = -item->triggerFlags;
			if (l->animNumber != ANIMATION_LARA_PUT_TRIDENT)
				PuzzleDone(item, itemNum);
		}
		else
		{
			l->animNumber = ANIMATION_LARA_USE_PUZZLE;
			l->currentAnimState = STATE_LARA_INSERT_PUZZLE;
			item->itemFlags[0] = 1;
		}

		l->frameNumber = Anims[l->animNumber].frameBase;
		Lara.isMoving = false;
		Lara.headYrot = 0;
		Lara.headXrot = 0;
		Lara.torsoYrot = 0;
		Lara.torsoXrot = 0;
		Lara.gunStatus = 1;
		item->flags |= 0x20;
		Lara.generalPtr = (void*)itemNum;
		InventoryItemChosen = NO_ITEM;
		item->pos.yRot = oldYrot;
		return;
	}

	if (Lara.isMoving)
	{
		if ((__int16)Lara.generalPtr == itemNum)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		}
	}

	item->pos.yRot = oldYrot;
}

void __cdecl PuzzleDoneCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	if (Items[itemNum].triggerFlags - 998 > 1)
	{
		ObjectCollision(itemNum, l, coll);
	}
}

void __cdecl KeyHoleCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &Items[itemNum];
	if (Items[itemNum].triggerFlags == 1 && item->objectNumber == ID_KEY_HOLE8)
	{
		if (item->itemFlags[3])
		{
			item->itemFlags[3]--;
			if (!item->itemFlags[3])
				item->meshBits = 2;
		}
	}

	if ((!(TrInput & IN_ACTION) && InventoryItemChosen == NO_ITEM
		|| BinocularRange
		|| Lara.gunStatus
		|| l->currentAnimState != STATE_LARA_STOP
		|| l->animNumber != ANIMATION_LARA_STAY_IDLE)
		&& (!Lara.isMoving || (__int16)Lara.generalPtr != itemNum))
	{
		if (item->objectNumber < ID_KEY_HOLE6)
			ObjectCollision(itemNum, l, coll);
	}
	else
	{
		if (TestLaraPosition(KeyHoleBounds, item, l))
		{
			if (!Lara.isMoving)
			{
				if (item->status != ITEM_INACTIVE)
					return;
				if (InventoryItemChosen == NO_ITEM)
				{
					if (g_Inventory->IsObjectPresentInInventory(item->objectNumber - 62))
						EnterInventoryItem = item->objectNumber - 62;
					return;
				}
				if (InventoryItemChosen != item->objectNumber - 62)
					return;
			}
			
			if (MoveLaraPosition(&KeyHolePosition, item, l))
			{
				if (item->objectNumber == ID_KEY_HOLE8)
					l->animNumber = ANIMATION_LARA_USE_KEYCARD;
				else
				{
					RemoveObjectFromInventory(item->objectNumber - 62);
					l->animNumber = ANIMATION_LARA_USE_KEY;
				}
				l->currentAnimState = STATE_LARA_INSERT_KEY;
				l->frameNumber = Anims[l->animNumber].frameBase;
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
				item->flags |= 0x20;
				item->status = ITEM_ACTIVE;
				if (item->triggerFlags == 1 && item->objectNumber == ID_KEY_HOLE8)
				{
					item->itemFlags[3] = 92;
					InventoryItemChosen = NO_ITEM;
					return;
				}
			}
			else
			{
				Lara.generalPtr = (void*)itemNum;
			}

			InventoryItemChosen = NO_ITEM;
			return;
		}

		if (Lara.isMoving && (__int16)Lara.generalPtr == itemNum)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		}
	}

	return;
}

void __cdecl PickUpCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];

	__int16 oldXrot = item->pos.xRot;
	__int16 oldYrot = item->pos.yRot;
	__int16 oldZrot = item->pos.zRot;

	if (item->status == ITEM_INVISIBLE)
		return;

	__int16 triggerFlags = item->triggerFlags & 0x3F;
	if (triggerFlags == 5 || triggerFlags == 10)
		return;

	if (item->objectNumber == ID_FLARE_ITEM && Lara.gunType == WEAPON_FLARE)
		return;

	item->pos.yRot = l->pos.yRot;
	item->pos.zRot = 0;

	if (Lara.waterStatus && Lara.waterStatus != LW_WADE)
	{
		if (Lara.waterStatus == LW_UNDERWATER)
		{
			item->pos.xRot = -ANGLE(25);
			if (TrInput & IN_ACTION
				&& item->objectNumber != ID_BURNING_TORCH_ITEM
				&& l->currentAnimState == STATE_LARA_UNDERWATER_STOP
				&& !Lara.gunStatus
				&& TestLaraPosition(PickUpBoundsUW, item, l)
				|| Lara.isMoving && (__int16)Lara.generalPtr == itemNum)
			{
				if (TestLaraPosition(PickUpBoundsUW, item, l))
				{
					if (MoveLaraPosition(&PickUpPositionUW, item, l))
					{
						if (item->objectNumber == ID_FLARE_ITEM)
						{
							l->animNumber = ANIMATION_LARA_UNDERWATER_FLARE_PICKUP;
							l->currentAnimState = STATE_LARA_FLARE_PICKUP;
							l->fallspeed = 0;
						}
						else
						{
							l->animNumber = ANIMATION_LARA_UNDERWATER_PICKUP;
							l->currentAnimState = STATE_LARA_PICKUP;
						}
						l->goalAnimState = STATE_LARA_UNDERWATER_STOP;
						l->frameNumber = Anims[l->animNumber].frameBase;
						Lara.isMoving = false;
						Lara.gunStatus = LG_HANDS_BUSY;
					}
					Lara.generalPtr = (void*)itemNum;
				}
				else
				{
					if (Lara.isMoving)
					{
						if ((__int16)Lara.generalPtr == itemNum)
						{
							Lara.isMoving = false;
							Lara.gunStatus = LG_NO_ARMS;
						}
					}
				}

				item->pos.xRot = oldXrot;
				item->pos.yRot = oldYrot;
				item->pos.zRot = oldZrot;
				return;
			}
			else if ((__int16)Lara.generalPtr != itemNum
				|| l->currentAnimState != STATE_LARA_PICKUP
				|| l->frameNumber != Anims[ANIMATION_LARA_UNDERWATER_PICKUP].frameBase + 18)
			{
				if ((__int16)Lara.generalPtr == itemNum
					&& l->currentAnimState == STATE_LARA_FLARE_PICKUP
					&& l->frameNumber == Anims[ANIMATION_LARA_UNDERWATER_FLARE_PICKUP].frameBase + 20)
				{
					Lara.requestGunType = WEAPON_FLARE;
					Lara.gunType = WEAPON_FLARE;
					InitialiseNewWeapon();
					Lara.gunStatus = LG_SPECIAL;
					Lara.flareAge = (__int32)(item->data) & 0x7FFF;
					DrawFlareMeshes();
					KillItem(itemNum);
				}

				item->pos.xRot = oldXrot;
				item->pos.yRot = oldYrot;
				item->pos.zRot = oldZrot;
				return;
			}

			AddDisplayPickup(item->objectNumber);
			if (!(item->triggerFlags & 0xC0))
			{
				KillItem(itemNum);
			}
			else
			{
			LABEL_124:
				item->itemFlags[3] = 1;
				item->flags |= 0x20;
				item->status = ITEM_ACTIVE;
			}
		}

		item->pos.xRot = oldXrot;
		item->pos.yRot = oldYrot;
		item->pos.zRot = oldZrot;
		return;
	}
	else if (!(TrInput & IN_ACTION) && (InventoryItemChosen == NO_ITEM || triggerFlags != 2)
		|| BinocularRange
		|| (l->currentAnimState != STATE_LARA_STOP || l->animNumber != ANIMATION_LARA_STAY_IDLE || Lara.gunStatus)
		&& (l->currentAnimState != STATE_LARA_CROUCH_IDLE || l->animNumber != ANIMATION_LARA_CROUCH_IDLE || Lara.gunStatus)
		&& (l->currentAnimState != STATE_LARA_CRAWL_IDLE || l->animNumber != ANIMATION_LARA_CRAWL_IDLE))
	{
		if (!Lara.isMoving)
		{
			if ((__int16)Lara.generalPtr == itemNum)
			{
				if (l->currentAnimState != STATE_LARA_PICKUP && l->currentAnimState != STATE_LARA_HOLE)
				{
					if ((__int16)Lara.generalPtr == itemNum
						&& l->currentAnimState == STATE_LARA_FLARE_PICKUP
						&& (l->animNumber == ANIMATION_LARA_CROUCH_PICKUP_FLARE &&
							l->frameNumber == Anims[ANIMATION_LARA_CROUCH_PICKUP_FLARE].frameBase + 22)
						|| l->frameNumber == Anims[ANIMATION_LARA_FLARE_PICKUP].frameBase + 58)
					{
						Lara.requestGunType = WEAPON_FLARE;
						Lara.gunType = WEAPON_FLARE;
						InitialiseNewWeapon();
						Lara.gunStatus = LG_SPECIAL;
						Lara.flareAge = (__int16)(item->data) & 0x7FFF;
					}
				}
				else
				{
					if (l->frameNumber == Anims[ANIMATION_LARA_PICKUP].frameBase + 15
						|| l->frameNumber == Anims[ANIMATION_LARA_CROUCH_PICKUP].frameBase + 22
						|| l->frameNumber == Anims[ANIMATION_LARA_CROUCH_PICKUP].frameBase + 20
						|| l->frameNumber == Anims[ANIMATION_LARA_PICKUP_PEDESTAL_LOW].frameBase + 29
						|| l->frameNumber == Anims[ANIMATION_LARA_PICKUP_PEDESTAL_HIGH].frameBase + 45
						|| l->frameNumber == Anims[ANIMATION_LARA_HOLE_GRAB].frameBase + 42
						|| l->frameNumber == Anims[ANIMATION_LARA_CROWBAR_USE_ON_WALL2].frameBase + 183
						|| (l->animNumber == ANIMATION_LARA_CROWBAR_USE_ON_WALL && l->frameNumber != Anims[ANIMATION_LARA_CROWBAR_USE_ON_WALL].frameBase + 123))
					{
						if (item->objectNumber == ID_BURNING_TORCH_ITEM)
						{
							AddDisplayPickup(ID_BURNING_TORCH_ITEM);
							//sub_402FC7();
							Lara.litTorch = (item->itemFlags[3] & 1);
						}
						else
						{
							if (item->objectNumber != ID_FLARE_ITEM)
							{
								AddDisplayPickup(item->objectNumber);
								if (item->triggerFlags & 0x100)
								{
									for (__int32 i = 0; i < LevelItems; i++)
									{
										if (Items[i].objectNumber == item->objectNumber)
											KillItem(i);
									}
								}
							}
							if (item->triggerFlags & 0xC0)
							{
								item->itemFlags[3] = 1;
								item->flags |= 0x20;
								item->status = ITEM_ACTIVE;
							}
						}
					}
					KillItem(itemNum);
				}
			}
		}

		if ((__int16)Lara.generalPtr != itemNum)
		{
			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			return;
		}
	}
	
	__int32 flag = 0;
	__int16* plinth = NULL;
	item->pos.xRot = 0;
	switch (triggerFlags)
	{
	case 1: // Pickup from wall hole
		if (Lara.isDucked || !TestLaraPosition(HiddenPickUpBounds, item, l))
		{
			if(Lara.isMoving)
			{
				if ((__int16)Lara.generalPtr == itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_NO_ARMS;
				}
			}

			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			return;
		}
		else if (MoveLaraPosition(&HiddenPickUpPosition, item, l))
		{
			l->animNumber = ANIMATION_LARA_HOLE_GRAB;
			l->currentAnimState = STATE_LARA_HOLE;
			flag = 1;
		}
		Lara.generalPtr = (void*)itemNum;
		break;

	case 2: // Pickup with crowbar
		item->pos.yRot = oldYrot;
		if (Lara.isDucked || !TestLaraPosition(CrowbarPickUpBounds, item, l))
		{
			if (!Lara.isMoving)
			{
				item->pos.xRot = oldXrot;
				item->pos.yRot = oldYrot;
				item->pos.zRot = oldZrot;
				return;
			}

			if ((__int16)Lara.generalPtr == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}

			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			return;
		}
		if (!Lara.isMoving)
		{
			if (InventoryItemChosen == NO_ITEM)
			{
				if (g_Inventory->IsObjectPresentInInventory(ID_CROWBAR_ITEM))
					EnterInventoryItem = ID_CROWBAR_ITEM;
				item->pos.xRot = oldXrot;
				item->pos.yRot = oldYrot;
				item->pos.zRot = oldZrot;
				return;
			}
			if (InventoryItemChosen != ID_CROWBAR_ITEM)
			{
				item->pos.xRot = oldXrot;
				item->pos.yRot = oldYrot;
				item->pos.zRot = oldZrot;
				return;
			}
			InventoryItemChosen = NO_ITEM;
		}
		if (MoveLaraPosition(&CrowbarPickUpPosition, item, l))
		{
			l->animNumber = ANIMATION_LARA_CROWBAR_USE_ON_WALL;
			l->currentAnimState = STATE_LARA_PICKUP;
			item->status = ITEM_ACTIVE;
			AddActiveItem(itemNum);
			AnimateItem(item);
			flag = 1;
		}

		Lara.generalPtr = (void*)itemNum;
		break;

	case 3:
	case 4:
	case 7:
	case 8:
		plinth = FindPlinth(item);
		if (!plinth)
		{
			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			return;
		}

		PlinthPickUpBounds[0] = plinth[0];
		PlinthPickUpBounds[1] = plinth[1];
		PlinthPickUpBounds[3] = l->pos.yPos - item->pos.yPos + 100;
		PlinthPickUpBounds[5] = plinth[5] + 320;
		PlinthPickUpPosition.z = -200 - plinth[5];

		if (TestLaraPosition(PlinthPickUpBounds, item, l) && !Lara.isDucked)
		{
			if (item->pos.yPos == l->pos.yPos)
				PlinthPickUpPosition.y = 0;
			else
				PlinthPickUpPosition.y = l->pos.yPos - item->pos.yPos;
			if (MoveLaraPosition(&PlinthPickUpPosition, item, l))
			{
				if (triggerFlags == 3 || triggerFlags == 7)
				{
					l->animNumber = ANIMATION_LARA_PICKUP_PEDESTAL_HIGH;
					l->currentAnimState = STATE_LARA_PICKUP;
				}
				else
				{
					l->animNumber = ANIMATION_LARA_PICKUP_PEDESTAL_LOW;
					l->currentAnimState = STATE_LARA_PICKUP;
				}
				flag = 1;
			}
			Lara.generalPtr = (void*)itemNum;
			break;
		}

		if (!Lara.isMoving)
		{
			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			return;
		}
		
		if ((__int16)Lara.generalPtr == itemNum)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		}
		
		item->pos.xRot = oldXrot;
		item->pos.yRot = oldYrot;
		item->pos.zRot = oldZrot;
		return;

	case 9: // Pickup object and conver it to crowbar (like submarine level)
		item->pos.yRot = oldYrot;
		if (!TestLaraPosition(JobyCrowPickUpBounds, item, l))
		{
			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			return;
		}
		if (MoveLaraPosition(&JobyCrowPickUpPosition, item, l))
		{
			l->animNumber = ANIMATION_LARA_CROWBAR_USE_ON_WALL2;
			l->currentAnimState = STATE_LARA_PICKUP;
			item->status = ITEM_ACTIVE;
			AddActiveItem(itemNum);
			flag = 1;
		}
		Lara.generalPtr = (void*)itemNum;
		break;

	default:
		if (!TestLaraPosition(PickUpBounds, item, l))
		{
			if (!Lara.isMoving)
			{
				item->pos.xRot = oldXrot;
				item->pos.yRot = oldYrot;
				item->pos.zRot = oldZrot;
				return;
			}
			
			if ((__int16)Lara.generalPtr == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}

			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			return;
		}

		PickUpPosition.y = l->pos.yPos - item->pos.yPos;

		if (l->currentAnimState == STATE_LARA_CROUCH_IDLE)
		{
			AlignLaraPosition(&PickUpPosition, item, l);
			if (item->objectNumber == ID_FLARE_ITEM)
			{
				l->animNumber = ANIMATION_LARA_CROUCH_PICKUP_FLARE;
				l->currentAnimState = STATE_LARA_FLARE_PICKUP;
				flag = 1;
				Lara.generalPtr = (void*)itemNum;
				break;
			}
			l->animNumber = ANIMATION_LARA_CROUCH_PICKUP;
		}
		else
		{
			if (l->currentAnimState == STATE_LARA_CRAWL_IDLE)
			{
				l->goalAnimState = STATE_LARA_CROUCH_IDLE;
				Lara.generalPtr = (void*)itemNum;
				break;;
			}
			if (!MoveLaraPosition(&PickUpPosition, item, l))
			{
				Lara.generalPtr = (void*)itemNum;
				break;
			}
			if (item->objectNumber == ID_FLARE_ITEM)
			{
				l->animNumber = ANIMATION_LARA_FLARE_PICKUP;
				l->currentAnimState = STATE_LARA_FLARE_PICKUP;
				flag = 1;
				Lara.generalPtr = (void*)itemNum;
				break;
			}
			l->animNumber = ANIMATION_LARA_PICKUP;
		}
		l->currentAnimState = STATE_LARA_PICKUP;
		flag = 1;
		Lara.generalPtr = (void*)itemNum;
	}

	if (flag)
	{
		Lara.headYrot = 0;
		Lara.headXrot = 0;
		Lara.torsoYrot = 0;
		Lara.torsoXrot = 0;
		l->frameNumber = Anims[l->animNumber].frameBase;
		Lara.isMoving = FAILED_ACCESS_ACE_FLAG;
		Lara.gunStatus = LG_HANDS_BUSY;
	}

	item->pos.xRot = oldXrot;
	item->pos.yRot = oldYrot;
	item->pos.zRot = oldZrot;
}

void __cdecl RegeneratePickups()
{
	for (__int32 i = 0; i < NumRPickups; i++)
	{
		ITEM_INFO* item = &Items[RPickups[i]];

		if (item->status == ITEM_INVISIBLE)
		{
			__int16 ammo = 0;

			if (item->objectNumber == ID_CROSSBOW_AMMO1_ITEM)
				ammo = g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO1];
			else if (item->objectNumber == ID_CROSSBOW_AMMO2_ITEM)
				ammo = g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO2];
			else if (item->objectNumber == ID_CROSSBOW_AMMO3_ITEM)
				ammo = g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO3];

			else if(item->objectNumber == ID_GRENADE_AMMO1_ITEM)
				ammo = g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO1];
			else if (item->objectNumber == ID_GRENADE_AMMO2_ITEM)
				ammo = g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO2];
			else if (item->objectNumber == ID_GRENADE_AMMO3_ITEM)
				ammo = g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO3];

			else if (item->objectNumber == ID_HK_AMMO_ITEM)
				ammo = g_LaraExtra.Weapons[WEAPON_HK].Ammo[WEAPON_AMMO1];

			else if (item->objectNumber == ID_UZI_AMMO_ITEM)
				ammo = g_LaraExtra.Weapons[WEAPON_UZI].Ammo[WEAPON_AMMO1];

			else if (item->objectNumber == ID_HARPOON_AMMO_ITEM)
				ammo = g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].Ammo[WEAPON_AMMO1];

			else if (item->objectNumber == ID_ROCKET_LAUNCHER_AMMO_ITEM)
				ammo = g_LaraExtra.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[WEAPON_AMMO1];

			else if (item->objectNumber == ID_REVOLVER_AMMO_ITEM)
				ammo = g_LaraExtra.Weapons[WEAPON_REVOLVER].Ammo[WEAPON_AMMO1];

			else if (item->objectNumber == ID_SHOTGUN_AMMO1_ITEM)
				ammo = g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[WEAPON_AMMO1];
			else if (item->objectNumber == ID_SHOTGUN_AMMO1_ITEM)
				ammo = g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[WEAPON_AMMO2];

			if (ammo == 0)
			{
				item->status = ITEM_INACTIVE;
			}
		}
	}
}

void __cdecl PickUpControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	__int16 roomNumber;
	__int16 triggerFlags = item->triggerFlags & 0x3F;
	switch (triggerFlags)
	{
	case 5:
		item->fallspeed += 6;
		item->pos.yPos += item->fallspeed;
		
		roomNumber = item->roomNumber;
		GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

		if (item->pos.yPos > item->itemFlags[0])
		{
			item->pos.yPos = item->itemFlags[0];
			if (item->fallspeed <= 64)
				item->triggerFlags &= 0xC0;
			else
				item->fallspeed = -item->fallspeed >> 2;
		}

		if (item->roomNumber != roomNumber)
			ItemNewRoom(itemNum, roomNumber);
		break;

	case 2:
	case 6:
	case 7:
	case 8:
	case 9:
		AnimateItem(item);
		break;

	case 11:
		//sub_401014(itemNum);
		break;

	}
}

__int16* __cdecl FindPlinth(ITEM_INFO* item)
{
	ROOM_INFO* room = &Rooms[item->roomNumber];
	MESH_INFO* mesh = room->mesh;

	__int32 found = -1;
	for (__int32 i = 0; i < room->numMeshes; i++)
	{
		MESH_INFO* mesh = &room->mesh[i];
		if (mesh->Flags & 1)
		{
			if (item->pos.xPos == mesh->x && item->pos.yPos == mesh->y && item->pos.zPos == mesh->z)
			{
				__int16* frame = GetBestFrame(item);
				STATIC_INFO* s = &StaticObjects[mesh->staticNumber];
				if (frame[0] <= s->xMaxc && frame[1] >= s->xMinc && frame[4] <= s->zMaxc && frame[5] >= s->zMinc && (s->xMinc || s->xMaxc))
				{
					found = i;
					break;
				}
			}
		}
	}

	if (found != -1)
		return &StaticObjects[found].xMinc;

	if (room->itemNumber == NO_ITEM)
		return NULL;

	__int16 itemNumber = room->itemNumber;
	for (itemNumber = room->itemNumber; itemNumber != NO_ITEM; itemNumber = Items[itemNumber].nextItem)
	{
		ITEM_INFO* current = &Items[itemNumber];

		if (Objects[current->objectNumber].collision != PickUpCollision
			&& item->pos.xPos == current->pos.xPos
			&& item->pos.yPos <= current->pos.yPos
			&& item->pos.zPos == current->pos.zPos
			&& (current->objectNumber != ID_HIGH_OBJECT1 || current->itemFlags[0] == 5))
		{
			break;
		}
	}

	if (itemNumber == NO_ITEM)
		return NULL;
	else
		return GetBestFrame(&Items[itemNumber]);
}

void __cdecl PuzzleDone(ITEM_INFO* item, __int16 itemNum)
{
	item->objectNumber += 8; 
	item->animNumber = Objects[item->objectNumber].animIndex;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->requiredAnimState = 0;
	item->goalAnimState = Anims[item->animNumber].currentAnimState;
	item->currentAnimState = Anims[item->animNumber].currentAnimState;

	AddActiveItem(itemNum);

	item->flags |= IFLAG_ACTIVATION_MASK;
	item->status = ITEM_ACTIVE;

	/*if (item->triggerFlags == 0x3E6 && LevelItems > 0)
	{
		int i;
		for (i = 0; i < level_items; i++)
		{
			if (Items[i].objectNumber == AIRLOCK_SWITCH
				&& Items[i].pos.x_pos == item->pos.x_pos
				&& Items[i].pos.z_pos == item->pos.z_pos)
			{
				FlipMap(Items[i].triggerFlags - 7);
				flipmap[Items[i].triggerFlags - 7] ^= IFLAG_ACTIVATION_MASK;
				Items[i].status = ITEM_INACTIVE;
				Items[i].flags |= 0x20;
			}
		}
	}*/
}

void Inject_Pickup()
{
	INJECT(0x0043A130, DrawAllPickups);
	INJECT(0x00463B60, PickedUpObject);
}