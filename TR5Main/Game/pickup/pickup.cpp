#include "framework.h"
#include "pickup.h"
#include "Specific/phd_global.h"
#include "lara.h"
#include "animation.h"
#include "gui.h"
#include "room.h"
#include "effects/debris.h"
#include "health.h"
#include "items.h"
#include "lara_fire.h"
#include "lara_flare.h"
#include "lara_one_gun.h"
#include "lara_two_guns.h"
#include "setup.h"
#include "camera.h"
#include "level.h"
#include "input.h"
#include "Sound/sound.h"
#include "tr4_clockwork_beetle.h"
#include "pickup/pickup_ammo.h"
#include "pickup/pickup_key_items.h"
#include "pickup/pickup_weapon.h"
#include "pickup/pickup_consumable.h"
#include "pickup/pickup_misc_items.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Game/collide.h"

using namespace TEN::Entities::Generic;

static PHD_VECTOR PickUpPosition(0, 0, -100);
OBJECT_COLLISION_BOUNDS PickUpBounds = 
{ -256, 256, -200, 200, -256, 256, ANGLE(-10), ANGLE(10), 0, 0, 0, 0 };

static PHD_VECTOR HiddenPickUpPosition(0, 0, -690);
OBJECT_COLLISION_BOUNDS HiddenPickUpBounds =
{ -256, 256, -100, 100, -800, -256, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), 0, 0 };

static PHD_VECTOR CrowbarPickUpPosition(0, 0, 215);
OBJECT_COLLISION_BOUNDS CrowbarPickUpBounds =
{ -256, 256, -100, 100, 200, 512, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), 0, 0 };

static PHD_VECTOR JobyCrowPickUpPosition(-224, 0, 240);
OBJECT_COLLISION_BOUNDS JobyCrowPickUpBounds =
{ -512, 0, -100, 100, 0, 512, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), 0, 0 };

static PHD_VECTOR PlinthPickUpPosition(0, 0, -460);
OBJECT_COLLISION_BOUNDS PlinthPickUpBounds =
{ -256, 256, -640, 640, -511, 0, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), 0, 0 };

static PHD_VECTOR PickUpPositionUW(0, -200, -350);
OBJECT_COLLISION_BOUNDS PickUpBoundsUW =
{ -512, 512, -512, 512, -512, 512, ANGLE(-45),  ANGLE(45), ANGLE(-45),  ANGLE(45), ANGLE(-45),  ANGLE(45) };

static PHD_VECTOR SOPos(0, 0, 0);
OBJECT_COLLISION_BOUNDS SOBounds =
{ 0, 0, 0, 0, 0, 0, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), ANGLE(-10), ANGLE(10) };

short SearchCollectFrames[4] = { 180, 100, 153, 83 };
short SearchAnims[4] = { LA_LOOT_CABINET, LA_LOOT_DRAWER, LA_LOOT_SHELF, LA_LOOT_CHEST };
short SearchOffsets[4] = { 160, 96, 160, 112 };

OBJECT_COLLISION_BOUNDS MSBounds =
{ 0, 0, 0, 0, 0, 0, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), ANGLE(-10), ANGLE(10) };

int NumRPickups;
short RPickups[16];
short getThisItemPlease = NO_ITEM;
PHD_VECTOR OldPickupPos;

void PickedUpObject(GAME_OBJECT_ID objID, int count)
{
	// see if the items fit into one of these easy groups
	if (!TryAddWeapon(Lara, objID, count)
		&& !TryAddAmmo(Lara, objID, count)
		&& !TryAddKeyItem(Lara, objID, count)
		&& !TryAddConsumable(Lara, objID, count)
		&& !TryAddMiscItem(Lara, objID))
	{
		// item isn't any of the above; do nothing
	}
}

int GetInventoryCount(GAME_OBJECT_ID objID)
{
	auto boolResult = HasWeapon(Lara, objID);
	if (boolResult.has_value())
	{
		return int{ boolResult.value() };
	}

	auto intResult = GetAmmoCount(Lara, objID);
	if (intResult.has_value())
	{
		return intResult.value();
	}

	intResult = GetKeyItemCount(Lara, objID);
	if (intResult.has_value())
	{
		return intResult.value();
	}

	intResult = GetConsumableCount(Lara, objID);
	if (intResult.has_value())
	{
		return intResult.value();
	}

	boolResult = HasMiscItem(Lara, objID);
	if (boolResult.has_value())
	{
		return int{ boolResult.value() };
	}
	return 0;
}

void RemoveObjectFromInventory(GAME_OBJECT_ID objID, int count)
{
	// see if the items fit into one of these easy groups
	if (!TryRemoveWeapon(Lara, objID, count) && 
		!TryRemoveAmmo(Lara, objID, count) && 
		!TryRemoveKeyItem(Lara, objID, count) && 
		!TryRemoveConsumable(Lara, objID, count) && 
		!TryRemoveMiscItem(Lara, objID))
		{
			// item isn't any of the above; do nothing
		}
}

void CollectCarriedItems(ITEM_INFO* item) 
{
	short pickupNumber = item->carriedItem;
	while (pickupNumber != NO_ITEM)
	{
		ITEM_INFO* pickup = &g_Level.Items[pickupNumber];

		AddDisplayPickup(pickup->objectNumber);
		KillItem(pickupNumber);

		pickupNumber = pickup->carriedItem;
	}
	item->carriedItem = NO_ITEM;
}

void DoPickup(ITEM_INFO* character)
{
	if (getThisItemPlease == NO_ITEM)
		return;

	auto lara = (LaraInfo*&)character->data;

	short pickupitem = getThisItemPlease;
	ITEM_INFO* item = &g_Level.Items[pickupitem];
	short oldXrot = item->pos.xRot;
	short oldYrot = item->pos.yRot;
	short oldZrot = item->pos.zRot;

	if (item->objectNumber == ID_BURNING_TORCH_ITEM)
	{
		AddDisplayPickup(ID_BURNING_TORCH_ITEM);
		GetFlameTorch();
		lara->litTorch = (item->itemFlags[3] & 1);

		KillItem(pickupitem);
		item->pos.xRot = oldXrot;
		item->pos.yRot = oldYrot;
		item->pos.zRot = oldZrot;
		getThisItemPlease = NO_ITEM;
		return;
	}
	else if (item->objectNumber == ID_FLARE_ITEM)
	{
		if (character->currentAnimState == LA_UNDERWATER_PICKUP_FLARE)
		{
			lara->requestGunType = WEAPON_FLARE;
			lara->gunType = WEAPON_FLARE;
			InitialiseNewWeapon(character);
			lara->gunStatus = LG_SPECIAL;
			lara->flareAge = (int)(item->data) & 0x7FFF;
			DrawFlareMeshes(character);
			KillItem(pickupitem);

			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			getThisItemPlease = NO_ITEM;
			return;
		}
		else if (character->currentAnimState == LS_PICKUP_FLARE)
		{
			lara->requestGunType = WEAPON_FLARE;
			lara->gunType = WEAPON_FLARE;
			InitialiseNewWeapon(character);
			lara->gunStatus = LG_SPECIAL;
			lara->flareAge = (short)(item->data) & 0x7FFF;
			KillItem(pickupitem);
			getThisItemPlease = NO_ITEM;
			return;
		}
	}
	else
	{
		if (character->animNumber == LA_UNDERWATER_PICKUP) //dirty but what can I do, it uses the same state
		{
			AddDisplayPickup(item->objectNumber);
			if (!(item->triggerFlags & 0xC0))
			{
				KillItem(pickupitem);
			}
			else
			{
				item->itemFlags[3] = 1;
				item->flags |= 0x20;
				item->status = ITEM_INVISIBLE;
			}
			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			getThisItemPlease = NO_ITEM;
			return;
		}
		else
		{
			if (character->animNumber == LA_CROWBAR_PRY_WALL_SLOW)
			{
				AddDisplayPickup(ID_CROWBAR_ITEM);
				Lara.Crowbar = true;
				KillItem(pickupitem);
			}
			else if (character->currentAnimState == LS_PICKUP || character->currentAnimState == LS_PICKUP_FROM_CHEST || character->currentAnimState == LS_HOLE)
			{
				AddDisplayPickup(item->objectNumber);
				if (item->triggerFlags & 0x100)
				{
					for (int i = 0; i < g_Level.NumItems; i++)
					{
						if (g_Level.Items[i].objectNumber == item->objectNumber)
							KillItem(i);
					}
				}
				if (item->triggerFlags & 0xC0)
				{
					item->itemFlags[3] = 1;
					item->flags |= 0x20;
					item->status = ITEM_INVISIBLE;
				}
				item->pos.xRot = oldXrot;
				item->pos.yRot = oldYrot;
				item->pos.zRot = oldZrot;
				KillItem(pickupitem);
				getThisItemPlease = NO_ITEM;
				return;
			}
		}
	}

	getThisItemPlease = NO_ITEM;
}

void PickupCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	short oldXrot = item->pos.xRot;
	short oldYrot = item->pos.yRot;
	short oldZrot = item->pos.zRot;

	if (item->status == ITEM_INVISIBLE)
		return;

	short triggerFlags = item->triggerFlags & 0x3F;
	if (triggerFlags == 5 || triggerFlags == 10)
		return;

	auto lara = (LaraInfo*&)l->data;

	if (item->objectNumber == ID_FLARE_ITEM && lara->gunType == WEAPON_FLARE)
		return;

	item->pos.yRot = l->pos.yRot;
	item->pos.zRot = 0;

	if (lara->waterStatus && lara->waterStatus != LW_WADE)
	{
		if (lara->waterStatus == LW_UNDERWATER)
		{
			item->pos.xRot = -ANGLE(25);

			if (TrInput & IN_ACTION && 
				item->objectNumber != ID_BURNING_TORCH_ITEM && 
				l->currentAnimState == LS_UNDERWATER_STOP && 
				!lara->gunStatus && 
				TestLaraPosition(&PickUpBoundsUW, item, l) || lara->isMoving && lara->interactedItem == itemNum)
			{
				if (TestLaraPosition(&PickUpBoundsUW, item, l))
				{
					if (MoveLaraPosition(&PickUpPositionUW, item, l))
					{
						if (item->objectNumber == ID_FLARE_ITEM)
						{
							getThisItemPlease = itemNum;
							l->animNumber = LA_UNDERWATER_PICKUP_FLARE;
							l->currentAnimState = LS_PICKUP_FLARE;
							l->fallspeed = 0;
						}
						else
						{
							getThisItemPlease = itemNum;
							l->animNumber = LA_UNDERWATER_PICKUP;
							l->currentAnimState = LS_PICKUP;
						}
						l->goalAnimState = LS_UNDERWATER_STOP;
						l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
						lara->isMoving = false;
						lara->gunStatus = LG_HANDS_BUSY;
					}
					lara->interactedItem = itemNum;
				}
				else
				{
					if (lara->isMoving)
					{
						if (lara->interactedItem == itemNum)
						{
							getThisItemPlease = itemNum;
							lara->isMoving = false;
							lara->gunStatus = LG_NO_ARMS;
						}
					}
				}

				item->pos.xRot = oldXrot;
				item->pos.yRot = oldYrot;
				item->pos.zRot = oldZrot;
				return;
			}
		}

		item->pos.xRot = oldXrot;
		item->pos.yRot = oldYrot;
		item->pos.zRot = oldZrot;
		return;
	}
	
	if (!(TrInput & IN_ACTION) && 
		(g_Gui.GetInventoryItemChosen() == NO_ITEM || triggerFlags != 2) || 
		BinocularRange ||
		(l->currentAnimState != LS_STOP || l->animNumber != LA_STAND_IDLE || lara->gunStatus) &&
		(l->currentAnimState != LS_CROUCH_IDLE || l->animNumber != LA_CROUCH_IDLE || lara->gunStatus) &&
		(l->currentAnimState != LS_CRAWL_IDLE || l->animNumber != LA_CRAWL_IDLE))
	{
		if (!lara->isMoving)
		{
			if (lara->interactedItem == itemNum)
			{
				if (l->currentAnimState != LS_PICKUP && l->currentAnimState != LS_HOLE)
				{
					{
						item->pos.xRot = oldXrot;
						item->pos.yRot = oldYrot;
						item->pos.zRot = oldZrot;
						return;
					}
				}
				else
				{
					item->pos.xRot = oldXrot;
					item->pos.yRot = oldYrot;
					item->pos.zRot = oldZrot;
					return;
				}
			}
		}

		if (lara->interactedItem != itemNum)
		{
			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			return;
		}
	}
	
	int flag = 0;
	BOUNDING_BOX* plinth = NULL;
	item->pos.xRot = 0;
	switch (triggerFlags)
	{
	case 1: // Pickup from wall hole
		if (lara->isDucked || !TestLaraPosition(&HiddenPickUpBounds, item, l))
		{
			if (lara->isMoving)
			{
				if (lara->interactedItem == itemNum)
				{
					lara->isMoving = false;
					lara->gunStatus = LG_NO_ARMS;
				}
			}

			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			return;
		}
		else if (MoveLaraPosition(&HiddenPickUpPosition, item, l))
		{
			getThisItemPlease = itemNum;
			l->animNumber = LA_HOLESWITCH_ACTIVATE;
			l->currentAnimState = LS_HOLE;
			flag = 1;
		}
		lara->interactedItem = itemNum;
		break;

	case 2: // Pickup with crowbar
		item->pos.yRot = oldYrot;
		if (lara->isDucked || !TestLaraPosition(&CrowbarPickUpBounds, item, l))
		{
			if (!lara->isMoving)
			{
				item->pos.xRot = oldXrot;
				item->pos.yRot = oldYrot;
				item->pos.zRot = oldZrot;
				return;
			}

			if (lara->interactedItem == itemNum)
			{
				lara->isMoving = false;
				lara->gunStatus = LG_NO_ARMS;
			}

			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			return;
		}
		if (!lara->isMoving)
		{
			if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
			{
				if (g_Gui.IsObjectInInventory(ID_CROWBAR_ITEM))
					g_Gui.SetEnterInventory(ID_CROWBAR_ITEM);

				item->pos.xRot = oldXrot;
				item->pos.yRot = oldYrot;
				item->pos.zRot = oldZrot;
				return;
			}

			if (g_Gui.GetInventoryItemChosen() != ID_CROWBAR_ITEM)
			{
				item->pos.xRot = oldXrot;
				item->pos.yRot = oldYrot;
				item->pos.zRot = oldZrot;
				return;
			}

			g_Gui.SetInventoryItemChosen(NO_ITEM);
		}
		if (MoveLaraPosition(&CrowbarPickUpPosition, item, l))
		{
			getThisItemPlease = itemNum;
			l->animNumber = LA_CROWBAR_PRY_WALL_FAST;
			l->currentAnimState = LS_PICKUP;
			item->status = ITEM_ACTIVE;
			AddActiveItem(itemNum);
			AnimateItem(item);
			flag = 1;
		}

		lara->interactedItem = itemNum;
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

		PlinthPickUpBounds.boundingBox.X1 = plinth->X1;
		PlinthPickUpBounds.boundingBox.X2 = plinth->X2;
		PlinthPickUpBounds.boundingBox.Y2 = l->pos.yPos - item->pos.yPos + 100;
		PlinthPickUpBounds.boundingBox.Z2 = plinth->Z2 + 320;
		PlinthPickUpPosition.z = -200 - plinth->Z2;

		if (TestLaraPosition(&PlinthPickUpBounds, item, l) && !lara->isDucked)
		{
			if (item->pos.yPos == l->pos.yPos)
				PlinthPickUpPosition.y = 0;
			else
				PlinthPickUpPosition.y = l->pos.yPos - item->pos.yPos;
			if (MoveLaraPosition(&PlinthPickUpPosition, item, l))
			{
				if (triggerFlags == 3 || triggerFlags == 7)
				{
					getThisItemPlease = itemNum;
					l->animNumber = LA_PICKUP_PEDESTAL_HIGH;
					l->currentAnimState = LS_PICKUP;
				}
				else
				{
					getThisItemPlease = itemNum;
					l->animNumber = LA_PICKUP_PEDESTAL_LOW;
					l->currentAnimState = LS_PICKUP;
				}
				flag = 1;
			}
			lara->interactedItem = itemNum;
			break;
		}

		if (!lara->isMoving)
		{
			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			return;
		}
		
		if (lara->interactedItem == itemNum)
		{
			lara->isMoving = false;
			lara->gunStatus = LG_NO_ARMS;
		}
		
		item->pos.xRot = oldXrot;
		item->pos.yRot = oldYrot;
		item->pos.zRot = oldZrot;
		return;

	case 9: // Pickup object and conver it to crowbar (like submarine level)
		item->pos.yRot = oldYrot;
		if (!TestLaraPosition(&JobyCrowPickUpBounds, item, l))
		{
			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			return;
		}
		if (MoveLaraPosition(&JobyCrowPickUpPosition, item, l))
		{
			getThisItemPlease = itemNum;
			l->animNumber = LA_CROWBAR_PRY_WALL_SLOW;
			l->currentAnimState = LS_PICKUP;
			item->status = ITEM_ACTIVE;
			AddActiveItem(itemNum);
			flag = 1;
		}
		lara->interactedItem = itemNum;
		break;

	default:
		if (!TestLaraPosition(&PickUpBounds, item, l))
		{
			if (!lara->isMoving)
			{
				item->pos.xRot = oldXrot;
				item->pos.yRot = oldYrot;
				item->pos.zRot = oldZrot;
				return;
			}
			
			if (lara->interactedItem == itemNum)
			{
				lara->isMoving = false;
				lara->gunStatus = LG_NO_ARMS;
			}

			item->pos.xRot = oldXrot;
			item->pos.yRot = oldYrot;
			item->pos.zRot = oldZrot;
			return;
		}

		PickUpPosition.y = l->pos.yPos - item->pos.yPos;

		if (l->currentAnimState == LS_CROUCH_IDLE)
		{
			if (item->objectNumber == ID_BURNING_TORCH_ITEM)
				break;
			AlignLaraPosition(&PickUpPosition, item, l);
			if (item->objectNumber == ID_FLARE_ITEM)
			{
				getThisItemPlease = itemNum;
				l->animNumber = LA_CROUCH_PICKUP_FLARE;
				l->currentAnimState = LS_PICKUP_FLARE;
				flag = 1;
				lara->interactedItem = itemNum;
				break;
			}
			getThisItemPlease = itemNum;
			l->goalAnimState = LS_PICKUP;
		}
		else
		{
			if (l->currentAnimState == LS_CRAWL_IDLE)
			{
				if (item->objectNumber == ID_BURNING_TORCH_ITEM)
					break;

				AlignLaraPosition(&PickUpPosition, item, l);

				if (item->objectNumber == ID_FLARE_ITEM)
				{
					l->goalAnimState = LS_CROUCH_IDLE;
					lara->interactedItem = itemNum;
				}
				else
				{
					getThisItemPlease = itemNum;
					l->goalAnimState = LS_PICKUP;
				}
				break;
			}
			else
			{
				if (!MoveLaraPosition(&PickUpPosition, item, l))
				{
					lara->interactedItem = itemNum;
					break;
				}

				getThisItemPlease = itemNum;

				if (item->objectNumber == ID_FLARE_ITEM)
				{
					l->animNumber = LA_PICKUP;
					l->currentAnimState = LS_PICKUP_FLARE;
					flag = 1;
					lara->interactedItem = itemNum;
					break;
				}
				else
				{
					// HACK: because of MoveLaraPosition(), we can't properly dispatch. Must be fixed later.
					l->animNumber = LA_PICKUP;
					l->currentAnimState = LS_PICKUP;
				}
			}
		}
		flag = 1;
		lara->interactedItem = itemNum;
	}

	if (flag)
	{
		lara->headYrot = 0;
		lara->headXrot = 0;
		lara->torsoYrot = 0;
		lara->torsoXrot = 0;
		l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
		lara->isMoving = false;
		lara->gunStatus = LG_HANDS_BUSY;
	}

	item->pos.xRot = oldXrot;
	item->pos.yRot = oldYrot;
	item->pos.zRot = oldZrot;
}

void RegeneratePickups()
{
	for (int i = 0; i < NumRPickups; i++)
	{
		ITEM_INFO* item = &g_Level.Items[RPickups[i]];

		if (item->status == ITEM_INVISIBLE)
		{
			short ammo = 0;
			if (item->objectNumber == ID_CROSSBOW_AMMO1_ITEM)
				ammo = Lara.Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO1];
			else if (item->objectNumber == ID_CROSSBOW_AMMO2_ITEM)
				ammo = Lara.Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO2];
			else if (item->objectNumber == ID_CROSSBOW_AMMO3_ITEM)
				ammo = Lara.Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO3];
			else if(item->objectNumber == ID_GRENADE_AMMO1_ITEM)
				ammo = Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO1];
			else if (item->objectNumber == ID_GRENADE_AMMO2_ITEM)
				ammo = Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO2];
			else if (item->objectNumber == ID_GRENADE_AMMO3_ITEM)
				ammo = Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO3];
			else if (item->objectNumber == ID_HK_AMMO_ITEM)
				ammo = Lara.Weapons[WEAPON_HK].Ammo[WEAPON_AMMO1];
			else if (item->objectNumber == ID_UZI_AMMO_ITEM)
				ammo = Lara.Weapons[WEAPON_UZI].Ammo[WEAPON_AMMO1];
			else if (item->objectNumber == ID_HARPOON_AMMO_ITEM)
				ammo = Lara.Weapons[WEAPON_HARPOON_GUN].Ammo[WEAPON_AMMO1];
			else if (item->objectNumber == ID_ROCKET_LAUNCHER_AMMO_ITEM)
				ammo = Lara.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[WEAPON_AMMO1];
			else if (item->objectNumber == ID_REVOLVER_AMMO_ITEM)
				ammo = Lara.Weapons[WEAPON_REVOLVER].Ammo[WEAPON_AMMO1];
			else if (item->objectNumber == ID_SHOTGUN_AMMO1_ITEM)
				ammo = Lara.Weapons[WEAPON_SHOTGUN].Ammo[WEAPON_AMMO1];
			else if (item->objectNumber == ID_SHOTGUN_AMMO1_ITEM)
				ammo = Lara.Weapons[WEAPON_SHOTGUN].Ammo[WEAPON_AMMO2];

			if (ammo == 0)
			{
				item->status = ITEM_NOT_ACTIVE;
			}
		}
	}
}

void PickupControl(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	short roomNumber;
	short triggerFlags = item->triggerFlags & 0x3F;
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

BOUNDING_BOX* FindPlinth(ITEM_INFO* item)
{
	ROOM_INFO* room = &g_Level.Rooms[item->roomNumber];
	
	int found = -1;
	for (int i = 0; i < room->mesh.size(); i++)
	{
		MESH_INFO* mesh = &room->mesh[i];
		if (mesh->flags & StaticMeshFlags::SM_VISIBLE)
		{
			if (item->pos.xPos == mesh->pos.xPos && item->pos.zPos == mesh->pos.zPos)
			{
				BOUNDING_BOX* frame = (BOUNDING_BOX*)GetBestFrame(item);
				STATIC_INFO* s = &StaticObjects[mesh->staticNumber];
				if (frame->X1 <= s->collisionBox.X2 
					&& frame->X2 >= s->collisionBox.X1 
					&& frame->Z1 <= s->collisionBox.Z2 
					&& frame->Z2 >= s->collisionBox.Z1 
					&& (s->collisionBox.X1 || s->collisionBox.X2))
				{
					found = mesh->staticNumber;
					break;
				}
			}
		}
	}

	if (found != -1)
		return &StaticObjects[found].collisionBox;

	if (room->itemNumber == NO_ITEM)
		return NULL;

	short itemNumber = room->itemNumber;
	for (itemNumber = room->itemNumber; itemNumber != NO_ITEM; itemNumber = g_Level.Items[itemNumber].nextItem)
	{
		ITEM_INFO* current = &g_Level.Items[itemNumber];
		OBJECT_INFO* obj = &Objects[current->objectNumber];

		if (!obj->isPickup
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
		return (BOUNDING_BOX*)GetBestFrame(&g_Level.Items[itemNumber]);
}

void InitialisePickup(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	BOUNDING_BOX* bounds = GetBoundsAccurate(item);
	short triggerFlags = item->triggerFlags & 0x3F;
	if (triggerFlags == 5)
	{
		item->itemFlags[0] = item->pos.yPos - bounds->Y2;
		item->status = ITEM_INVISIBLE;
	}
	else
	{
		if (triggerFlags == 0 || triggerFlags == 3 || triggerFlags == 4 || triggerFlags == 7 || triggerFlags == 8 || triggerFlags == 11)
			item->pos.yPos -= bounds->Y2;
		
		if ((item->triggerFlags & 0x80) != 0)
		{
			RPickups[NumRPickups] = itemNumber;
			NumRPickups++;
		}
		
		if (item->triggerFlags & 0x100)
			item->meshBits = 0;
		
		if (item->status == ITEM_INVISIBLE)
			item->flags |= 0x20;
	}
}

void InitialiseSearchObject(short itemNumber)
{
	ITEM_INFO* item, *item2;
	short itemNumber2;

	item = &g_Level.Items[itemNumber];
	if (item->objectNumber == ID_SEARCH_OBJECT1)
	{
		item->swapMeshFlags = -1;
		item->meshBits = 7;
	}
	else if (item->objectNumber == ID_SEARCH_OBJECT2)
	{
		item->meshBits = 2;
	}
	else if (item->objectNumber == ID_SEARCH_OBJECT4)
	{
		item->itemFlags[1] = -1;
		item->meshBits = 9;
		
		for (itemNumber2 = 0; itemNumber2 < g_Level.NumItems; ++itemNumber2)
		{
			item2 = &g_Level.Items[itemNumber2];

			if (item2->objectNumber == ID_EXPLOSION)
			{
				if (item->pos.xPos == item2->pos.xPos && item->pos.yPos == item2->pos.yPos && item->pos.zPos == item2->pos.zPos)
				{
					item->itemFlags[1] = itemNumber2;
					break;
				}
			}
			else if (Objects[item2->objectNumber].isPickup
				&&  item->pos.xPos == item2->pos.xPos
				&&  item->pos.yPos == item2->pos.yPos
				&&  item->pos.zPos == item2->pos.zPos)
			{
				item->itemFlags[1] = itemNumber2;
				break;
			}

		}
		AddActiveItem(itemNumber);
		item->flags |= IFLAG_ACTIVATION_MASK;
		item->status = ITEM_ACTIVE;
	}
}

void SearchObjectCollision(short itemNumber, ITEM_INFO* laraitem, COLL_INFO* laracoll)
{
	ITEM_INFO* item;
	int objNumber;
	BOUNDING_BOX* bounds;

	item = &g_Level.Items[itemNumber];
	objNumber = (item->objectNumber - ID_SEARCH_OBJECT1) / 2;

	if (TrInput & IN_ACTION
		&& laraitem->currentAnimState == LS_STOP
		&& laraitem->animNumber == LA_STAND_IDLE 
		&& Lara.gunStatus == LG_NO_ARMS 
		&& (item->status == ITEM_NOT_ACTIVE 
			&& item->objectNumber != ID_SEARCH_OBJECT4 || !item->itemFlags[0])
		|| Lara.isMoving && Lara.interactedItem == itemNumber)
	{
		bounds = GetBoundsAccurate(item);
		if (item->objectNumber != ID_SEARCH_OBJECT1)
		{
			SOBounds.boundingBox.X1 = bounds->X1 - 128;
			SOBounds.boundingBox.X2 = bounds->X2 + 128;
		}
		else
		{
			SOBounds.boundingBox.X1 = bounds->X1 + 64;
			SOBounds.boundingBox.X2 = bounds->X2 - 64;
		}
		SOBounds.boundingBox.Z1 = bounds->Z1 - 200;
		SOBounds.boundingBox.Z2 = bounds->Z2 + 200;
		SOPos.z = bounds->Z1 - SearchOffsets[objNumber];

		if (TestLaraPosition(&SOBounds, item, laraitem))
		{
			if (MoveLaraPosition(&SOPos, item, laraitem))
			{
				laraitem->currentAnimState = LS_MISC_CONTROL;
				laraitem->animNumber = SearchAnims[objNumber];
				laraitem->frameNumber = g_Level.Anims[laraitem->animNumber].frameBase;
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;

				if (item->objectNumber == ID_SEARCH_OBJECT4)
				{
					item->itemFlags[0] = 1;
				}
				else
				{
					AddActiveItem(itemNumber);
					item->status = ITEM_ACTIVE;
				}

				item->animNumber = Objects[item->objectNumber].animIndex + 1;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				AnimateItem(item);
			}
			else
			{
				Lara.interactedItem = itemNumber;
			}
		}
		else if (Lara.isMoving && Lara.interactedItem ==  itemNumber)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		}
	}
	else if (laraitem->currentAnimState != LS_MISC_CONTROL)
	{
		ObjectCollision(itemNumber, laraitem, laracoll);
	}
}

void SearchObjectControl(short itemNumber)
{
	ITEM_INFO* item, *item2;
	int objNumber;
	short frameNumber;

	item = &g_Level.Items[itemNumber];
	objNumber = (item->objectNumber - ID_SEARCH_OBJECT1) / 2;

	if (item->objectNumber != ID_SEARCH_OBJECT4 || item->itemFlags[0] == 1)
		AnimateItem(item);

	frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;
	if (item->objectNumber == ID_SEARCH_OBJECT1)
	{
		if (frameNumber > 0)
		{
			item->swapMeshFlags = 0;
			item->meshBits = -1;
		}
		else
		{
			item->swapMeshFlags = -1;
			item->meshBits = 7;
		}
	}
	else if (item->objectNumber == ID_SEARCH_OBJECT2)
	{
		if (frameNumber == 18)
			item->meshBits = 1;
		else if (frameNumber == 172)
			item->meshBits = 2;
	}
	else if (item->objectNumber == ID_SEARCH_OBJECT4)
	{
		item->meshBits = FlipStats[0] != 0 ? 48 : 9;

		if (frameNumber >= 45 && frameNumber <= 131)
			item->meshBits |= FlipStats[0] != 0 ? 4 : 2;
			
		if (item->itemFlags[1] != -1)
		{
			item2 = &g_Level.Items[item->itemFlags[1]];
			if (Objects[item2->objectNumber].isPickup)
			{
				if (FlipStats[0])
					item2->status = ITEM_NOT_ACTIVE;
				else
					item2->status = ITEM_INVISIBLE;
			}
		}
	}

	if (frameNumber == SearchCollectFrames[objNumber])
	{
		if (item->objectNumber == ID_SEARCH_OBJECT4)
		{
			if (item->itemFlags[1] != -1)
			{
				item2 = &g_Level.Items[item->itemFlags[1]];
				if (Objects[item2->objectNumber].isPickup)
				{
					AddDisplayPickup(item2->objectNumber);
					KillItem(item->itemFlags[1]);
				}
				else
				{
					AddActiveItem(item->itemFlags[1]);
					item2->flags |= IFLAG_ACTIVATION_MASK;
					item2->status = ITEM_ACTIVE;
					LaraItem->hitPoints = 640;
				}
				item->itemFlags[1] = -1;
			}
		}
		else
		{
			CollectCarriedItems(item);
		}
	}

	
	if (item->status == ITEM_DEACTIVATED)
	{
		if (item->objectNumber == ID_SEARCH_OBJECT4)
		{
			item->itemFlags[0] = 0;
			item->status = ITEM_ACTIVE;
		}
		else
		{
			RemoveActiveItem(itemNumber);
			item->status = ITEM_NOT_ACTIVE;
		}
	}
}

int UseSpecialItem(ITEM_INFO* item)
{
	int flag = 0;
	int use = g_Gui.GetInventoryItemChosen();

	if (item->animNumber == LA_STAND_IDLE && Lara.gunStatus == LG_NO_ARMS && use != NO_ITEM)
	{
		if ((use >= ID_WATERSKIN1_EMPTY) && (use <= ID_WATERSKIN2_5))
		{
			item->itemFlags[2] = ID_LARA_WATER_MESH;
			flag = 1;
		}
		else if (use == ID_CLOCKWORK_BEETLE)
		{
			flag = 4;
			item->animNumber = LA_MECHANICAL_BEETLE_USE;
			UseClockworkBeetle(1);
		}

		if (flag == 1)
		{
			if (use != ID_WATERSKIN1_3 && use != ID_WATERSKIN2_5 && (LaraItem->pos.yPos > Lara.waterSurfaceDist))
			{
				if (use < ID_WATERSKIN1_3)
					Lara.small_waterskin = 4;
				else
					Lara.big_waterskin = 6;

				flag = 1;
			}
			else if (use != ID_WATERSKIN1_EMPTY && use != ID_WATERSKIN2_EMPTY)
			{
				if (use <= ID_WATERSKIN1_3)
				{
					item->itemFlags[3] = Lara.small_waterskin - 1;
					Lara.small_waterskin = 1;
				}
				else
				{
					item->itemFlags[3] = Lara.big_waterskin - 1;
					Lara.big_waterskin = 1;
				}

				flag = 2;
			}
			else
				return 0;
		}

		if (flag)
		{
			if (flag == 1)
				item->animNumber = LA_WATERSKIN_FILL;
			else if (flag == 2)
				item->animNumber = LA_WATERSKIN_POUR_LOW;

			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_MISC_CONTROL;
			item->currentAnimState = LS_MISC_CONTROL;
			Lara.gunStatus = LG_HANDS_BUSY;

			g_Gui.SetInventoryItemChosen(NO_ITEM);

			return 1;
		}
	}

	return 0;
}
