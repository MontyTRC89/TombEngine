#include "framework.h"
#include "Game/pickup/pickup.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/debris.h"
#include "Game/gui.h"
#include "Game/items.h"
#include "Game/health.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_two_guns.h"
#include "Game/pickup/pickup_ammo.h"
#include "Game/pickup/pickup_key_items.h"
#include "Game/pickup/pickup_weapon.h"
#include "Game/pickup/pickup_consumable.h"
#include "Game/pickup/pickup_misc_items.h"
#include "Game/room.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Objects/TR4/Object/tr4_clockwork_beetle.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Specific/phd_global.h"
#include "Specific/setup.h"

using namespace TEN::Entities::Generic;

static PHD_VECTOR PickUpPosition(0, 0, -100);
OBJECT_COLLISION_BOUNDS PickUpBounds = 
{ -256, 256, -200, 200, -256, 256, -ANGLE(10.0f), ANGLE(10.0f), 0, 0, -ANGLE(2.0f), ANGLE(2.0f) }; // TODO: Adjust these bounds when crawl surface alignment is implemented. @Sezz 2021.11.04

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
	short pickupNumber = item->CarriedItem;
	while (pickupNumber != NO_ITEM)
	{
		ITEM_INFO* pickup = &g_Level.Items[pickupNumber];

		AddDisplayPickup(pickup->ObjectNumber);
		KillItem(pickupNumber);

		pickupNumber = pickup->CarriedItem;
	}
	item->CarriedItem = NO_ITEM;
}

void DoPickup(ITEM_INFO* character)
{
	if (getThisItemPlease == NO_ITEM)
		return;

	auto lara = (LaraInfo*&)character->Data;

	short pickupitem = getThisItemPlease;
	ITEM_INFO* item = &g_Level.Items[pickupitem];
	short oldXrot = item->Position.xRot;
	short oldYrot = item->Position.yRot;
	short oldZrot = item->Position.zRot;

	if (item->ObjectNumber == ID_BURNING_TORCH_ITEM)
	{
		AddDisplayPickup(ID_BURNING_TORCH_ITEM);
		GetFlameTorch();
		lara->litTorch = (item->ItemFlags[3] & 1);

		KillItem(pickupitem);
		item->Position.xRot = oldXrot;
		item->Position.yRot = oldYrot;
		item->Position.zRot = oldZrot;
		getThisItemPlease = NO_ITEM;
		return;
	}
	else if (item->ObjectNumber == ID_FLARE_ITEM)
	{
		if (character->ActiveState == LA_UNDERWATER_PICKUP_FLARE)
		{
			lara->requestGunType = WEAPON_FLARE;
			lara->gunType = WEAPON_FLARE;
			InitialiseNewWeapon(character);
			lara->gunStatus = LG_SPECIAL;
			lara->Flare.FlareAge = (int)(item->Data) & 0x7FFF;
			DrawFlareMeshes(character);
			KillItem(pickupitem);

			item->Position.xRot = oldXrot;
			item->Position.yRot = oldYrot;
			item->Position.zRot = oldZrot;
			getThisItemPlease = NO_ITEM;
			return;
		}
		else if (character->ActiveState == LS_PICKUP_FLARE)
		{
			lara->requestGunType = WEAPON_FLARE;
			lara->gunType = WEAPON_FLARE;
			InitialiseNewWeapon(character);
			lara->gunStatus = LG_SPECIAL;
			lara->Flare.FlareAge = (short)(item->Data) & 0x7FFF;
			KillItem(pickupitem);
			getThisItemPlease = NO_ITEM;
			return;
		}
	}
	else
	{
		if (character->AnimNumber == LA_UNDERWATER_PICKUP) //dirty but what can I do, it uses the same state
		{
			AddDisplayPickup(item->ObjectNumber);
			if (!(item->TriggerFlags & 0xC0))
			{
				KillItem(pickupitem);
			}
			else
			{
				item->ItemFlags[3] = 1;
				item->Flags |= 0x20;
				item->Status = ITEM_INVISIBLE;
			}
			item->Position.xRot = oldXrot;
			item->Position.yRot = oldYrot;
			item->Position.zRot = oldZrot;
			getThisItemPlease = NO_ITEM;
			return;
		}
		else
		{
			if (character->AnimNumber == LA_CROWBAR_PRY_WALL_SLOW)
			{
				AddDisplayPickup(ID_CROWBAR_ITEM);
				Lara.Crowbar = true;
				KillItem(pickupitem);
			}
			else if (character->ActiveState == LS_PICKUP || character->ActiveState == LS_PICKUP_FROM_CHEST || character->ActiveState == LS_HOLE)
			{
				AddDisplayPickup(item->ObjectNumber);
				if (item->TriggerFlags & 0x100)
				{
					for (int i = 0; i < g_Level.NumItems; i++)
					{
						if (g_Level.Items[i].ObjectNumber == item->ObjectNumber)
							KillItem(i);
					}
				}
				if (item->TriggerFlags & 0xC0)
				{
					item->ItemFlags[3] = 1;
					item->Flags |= 0x20;
					item->Status = ITEM_INVISIBLE;
				}
				item->Position.xRot = oldXrot;
				item->Position.yRot = oldYrot;
				item->Position.zRot = oldZrot;
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
	short oldXrot = item->Position.xRot;
	short oldYrot = item->Position.yRot;
	short oldZrot = item->Position.zRot;

	if (item->Status == ITEM_INVISIBLE)
		return;

	short triggerFlags = item->TriggerFlags & 0x3F;
	if (triggerFlags == 5 || triggerFlags == 10)
		return;

	auto lara = (LaraInfo*&)l->Data;

	if (item->ObjectNumber == ID_FLARE_ITEM && lara->gunType == WEAPON_FLARE)
		return;

	item->Position.yRot = l->Position.yRot;
	item->Position.zRot = 0;

	if (lara->waterStatus && lara->waterStatus != LW_WADE)
	{
		if (lara->waterStatus == LW_UNDERWATER)
		{
			item->Position.xRot = -ANGLE(25);

			if (TrInput & IN_ACTION && 
				item->ObjectNumber != ID_BURNING_TORCH_ITEM && 
				l->ActiveState == LS_UNDERWATER_STOP && 
				!lara->gunStatus && 
				TestLaraPosition(&PickUpBoundsUW, item, l) || lara->isMoving && lara->interactedItem == itemNum)
			{
				if (TestLaraPosition(&PickUpBoundsUW, item, l))
				{
					if (MoveLaraPosition(&PickUpPositionUW, item, l))
					{
						if (item->ObjectNumber == ID_FLARE_ITEM)
						{
							getThisItemPlease = itemNum;
							l->AnimNumber = LA_UNDERWATER_PICKUP_FLARE;
							l->ActiveState = LS_PICKUP_FLARE;
							l->VerticalVelocity = 0;
						}
						else
						{
							getThisItemPlease = itemNum;
							l->AnimNumber = LA_UNDERWATER_PICKUP;
							l->ActiveState = LS_PICKUP;
						}
						l->TargetState = LS_UNDERWATER_STOP;
						l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
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
							Lara.isMoving = false;
							Lara.gunStatus = LG_HANDS_FREE;
						}
					}
				}

				item->Position.xRot = oldXrot;
				item->Position.yRot = oldYrot;
				item->Position.zRot = oldZrot;
				return;
			}
		}

		item->Position.xRot = oldXrot;
		item->Position.yRot = oldYrot;
		item->Position.zRot = oldZrot;
		return;
	}
	
	if (!(TrInput & IN_ACTION) && 
		(g_Gui.GetInventoryItemChosen() == NO_ITEM || triggerFlags != 2) || 
		BinocularRange ||
		(l->ActiveState != LS_IDLE || l->AnimNumber != LA_STAND_IDLE || lara->gunStatus) &&
		(l->ActiveState != LS_CROUCH_IDLE || l->AnimNumber != LA_CROUCH_IDLE || lara->gunStatus) &&
		(l->ActiveState != LS_CRAWL_IDLE || l->AnimNumber != LA_CRAWL_IDLE))
	{
		if (!lara->isMoving)
		{
			if (lara->interactedItem == itemNum)
			{
				if (l->ActiveState != LS_PICKUP && l->ActiveState != LS_HOLE)
				{
					{
						item->Position.xRot = oldXrot;
						item->Position.yRot = oldYrot;
						item->Position.zRot = oldZrot;
						return;
					}
				}
				else
				{
					item->Position.xRot = oldXrot;
					item->Position.yRot = oldYrot;
					item->Position.zRot = oldZrot;
					return;
				}
			}
		}

		if (lara->interactedItem != itemNum)
		{
			item->Position.xRot = oldXrot;
			item->Position.yRot = oldYrot;
			item->Position.zRot = oldZrot;
			return;
		}
	}
	
	int flag = 0;
	BOUNDING_BOX* plinth = NULL;
	item->Position.xRot = 0;
	switch (triggerFlags)
	{
	case 1: // Pickup from wall hole
		if (lara->isLow || !TestLaraPosition(&HiddenPickUpBounds, item, l))
		{
			if (lara->isMoving)
			{
				if (lara->interactedItem == itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_FREE;
				}
			}

			item->Position.xRot = oldXrot;
			item->Position.yRot = oldYrot;
			item->Position.zRot = oldZrot;
			return;
		}
		else if (MoveLaraPosition(&HiddenPickUpPosition, item, l))
		{
			getThisItemPlease = itemNum;
			l->AnimNumber = LA_HOLESWITCH_ACTIVATE;
			l->ActiveState = LS_HOLE;
			flag = 1;
		}
		lara->interactedItem = itemNum;
		break;

	case 2: // Pickup with crowbar
		item->Position.yRot = oldYrot;
		if (lara->isLow || !TestLaraPosition(&CrowbarPickUpBounds, item, l))
		{
			if (!lara->isMoving)
			{
				item->Position.xRot = oldXrot;
				item->Position.yRot = oldYrot;
				item->Position.zRot = oldZrot;
				return;
			}

			if (lara->interactedItem == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_HANDS_FREE;
			}

			item->Position.xRot = oldXrot;
			item->Position.yRot = oldYrot;
			item->Position.zRot = oldZrot;
			return;
		}
		if (!lara->isMoving)
		{
			if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
			{
				if (g_Gui.IsObjectInInventory(ID_CROWBAR_ITEM))
					g_Gui.SetEnterInventory(ID_CROWBAR_ITEM);

				item->Position.xRot = oldXrot;
				item->Position.yRot = oldYrot;
				item->Position.zRot = oldZrot;
				return;
			}

			if (g_Gui.GetInventoryItemChosen() != ID_CROWBAR_ITEM)
			{
				item->Position.xRot = oldXrot;
				item->Position.yRot = oldYrot;
				item->Position.zRot = oldZrot;
				return;
			}

			g_Gui.SetInventoryItemChosen(NO_ITEM);
		}
		if (MoveLaraPosition(&CrowbarPickUpPosition, item, l))
		{
			getThisItemPlease = itemNum;
			l->AnimNumber = LA_CROWBAR_PRY_WALL_FAST;
			l->ActiveState = LS_PICKUP;
			item->Status = ITEM_ACTIVE;
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
			item->Position.xRot = oldXrot;
			item->Position.yRot = oldYrot;
			item->Position.zRot = oldZrot;
			return;
		}

		PlinthPickUpBounds.boundingBox.X1 = plinth->X1;
		PlinthPickUpBounds.boundingBox.X2 = plinth->X2;
		PlinthPickUpBounds.boundingBox.Y2 = l->Position.yPos - item->Position.yPos + 100;
		PlinthPickUpBounds.boundingBox.Z2 = plinth->Z2 + 320;
		PlinthPickUpPosition.z = -200 - plinth->Z2;

		if (TestLaraPosition(&PlinthPickUpBounds, item, l) && !lara->isLow)
		{
			if (item->Position.yPos == l->Position.yPos)
				PlinthPickUpPosition.y = 0;
			else
				PlinthPickUpPosition.y = l->Position.yPos - item->Position.yPos;
			if (MoveLaraPosition(&PlinthPickUpPosition, item, l))
			{
				if (triggerFlags == 3 || triggerFlags == 7)
				{
					getThisItemPlease = itemNum;
					l->AnimNumber = LA_PICKUP_PEDESTAL_HIGH;
					l->ActiveState = LS_PICKUP;
				}
				else
				{
					getThisItemPlease = itemNum;
					l->AnimNumber = LA_PICKUP_PEDESTAL_LOW;
					l->ActiveState = LS_PICKUP;
				}
				flag = 1;
			}
			lara->interactedItem = itemNum;
			break;
		}

		if (!lara->isMoving)
		{
			item->Position.xRot = oldXrot;
			item->Position.yRot = oldYrot;
			item->Position.zRot = oldZrot;
			return;
		}
		
		if (lara->interactedItem == itemNum)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_HANDS_FREE;
		}
		
		item->Position.xRot = oldXrot;
		item->Position.yRot = oldYrot;
		item->Position.zRot = oldZrot;
		return;

	case 9: // Pickup object and conver it to crowbar (like submarine level)
		item->Position.yRot = oldYrot;
		if (!TestLaraPosition(&JobyCrowPickUpBounds, item, l))
		{
			item->Position.xRot = oldXrot;
			item->Position.yRot = oldYrot;
			item->Position.zRot = oldZrot;
			return;
		}
		if (MoveLaraPosition(&JobyCrowPickUpPosition, item, l))
		{
			getThisItemPlease = itemNum;
			l->AnimNumber = LA_CROWBAR_PRY_WALL_SLOW;
			l->ActiveState = LS_PICKUP;
			item->Status = ITEM_ACTIVE;
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
				item->Position.xRot = oldXrot;
				item->Position.yRot = oldYrot;
				item->Position.zRot = oldZrot;
				return;
			}
			
			if (lara->interactedItem == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_HANDS_FREE;
			}

			item->Position.xRot = oldXrot;
			item->Position.yRot = oldYrot;
			item->Position.zRot = oldZrot;
			return;
		}

		PickUpPosition.y = l->Position.yPos - item->Position.yPos;

		if (l->ActiveState == LS_CROUCH_IDLE)
		{
			if (item->ObjectNumber == ID_BURNING_TORCH_ITEM)
				break;
			AlignLaraPosition(&PickUpPosition, item, l);
			if (item->ObjectNumber == ID_FLARE_ITEM)
			{
				getThisItemPlease = itemNum;
				l->AnimNumber = LA_CROUCH_PICKUP_FLARE;
				l->ActiveState = LS_PICKUP_FLARE;
				flag = 1;
				lara->interactedItem = itemNum;
				break;
			}
			getThisItemPlease = itemNum;
			l->TargetState = LS_PICKUP;
		}
		else
		{
			if (l->ActiveState == LS_CRAWL_IDLE)
			{
				if (item->ObjectNumber == ID_BURNING_TORCH_ITEM)
					break;

				AlignLaraPosition(&PickUpPosition, item, l);

				if (item->ObjectNumber == ID_FLARE_ITEM)
				{
					l->TargetState = LS_CROUCH_IDLE;
					lara->interactedItem = itemNum;
				}
				else
				{
					getThisItemPlease = itemNum;
					l->TargetState = LS_PICKUP;
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

				if (item->ObjectNumber == ID_FLARE_ITEM)
				{
					l->AnimNumber = LA_PICKUP;
					l->ActiveState = LS_PICKUP_FLARE;
					flag = 1;
					lara->interactedItem = itemNum;
					break;
				}
				else
				{
					// HACK: because of MoveLaraPosition(), we can't properly dispatch. Must be fixed later.
					l->AnimNumber = LA_PICKUP;
					l->ActiveState = LS_PICKUP;
				}
			}
		}
		flag = 1;
		lara->interactedItem = itemNum;
	}

	if (flag)
	{
		ResetLaraFlex(l);
		l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
		lara->isMoving = false;
		lara->gunStatus = LG_HANDS_BUSY;
	}

	item->Position.xRot = oldXrot;
	item->Position.yRot = oldYrot;
	item->Position.zRot = oldZrot;
}

void RegeneratePickups()
{
	for (int i = 0; i < NumRPickups; i++)
	{
		ITEM_INFO* item = &g_Level.Items[RPickups[i]];

		if (item->Status == ITEM_INVISIBLE)
		{
			short ammo = 0;
			if (item->ObjectNumber == ID_CROSSBOW_AMMO1_ITEM)
				ammo = Lara.Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO1];
			else if (item->ObjectNumber == ID_CROSSBOW_AMMO2_ITEM)
				ammo = Lara.Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO2];
			else if (item->ObjectNumber == ID_CROSSBOW_AMMO3_ITEM)
				ammo = Lara.Weapons[WEAPON_CROSSBOW].Ammo[WEAPON_AMMO3];
			else if(item->ObjectNumber == ID_GRENADE_AMMO1_ITEM)
				ammo = Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO1];
			else if (item->ObjectNumber == ID_GRENADE_AMMO2_ITEM)
				ammo = Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO2];
			else if (item->ObjectNumber == ID_GRENADE_AMMO3_ITEM)
				ammo = Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO3];
			else if (item->ObjectNumber == ID_HK_AMMO_ITEM)
				ammo = Lara.Weapons[WEAPON_HK].Ammo[WEAPON_AMMO1];
			else if (item->ObjectNumber == ID_UZI_AMMO_ITEM)
				ammo = Lara.Weapons[WEAPON_UZI].Ammo[WEAPON_AMMO1];
			else if (item->ObjectNumber == ID_HARPOON_AMMO_ITEM)
				ammo = Lara.Weapons[WEAPON_HARPOON_GUN].Ammo[WEAPON_AMMO1];
			else if (item->ObjectNumber == ID_ROCKET_LAUNCHER_AMMO_ITEM)
				ammo = Lara.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[WEAPON_AMMO1];
			else if (item->ObjectNumber == ID_REVOLVER_AMMO_ITEM)
				ammo = Lara.Weapons[WEAPON_REVOLVER].Ammo[WEAPON_AMMO1];
			else if (item->ObjectNumber == ID_SHOTGUN_AMMO1_ITEM)
				ammo = Lara.Weapons[WEAPON_SHOTGUN].Ammo[WEAPON_AMMO1];
			else if (item->ObjectNumber == ID_SHOTGUN_AMMO1_ITEM)
				ammo = Lara.Weapons[WEAPON_SHOTGUN].Ammo[WEAPON_AMMO2];

			if (ammo == 0)
			{
				item->Status = ITEM_NOT_ACTIVE;
			}
		}
	}
}

void PickupControl(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	short roomNumber;
	short triggerFlags = item->TriggerFlags & 0x3F;
	switch (triggerFlags)
	{
	case 5:
		item->VerticalVelocity += 6;
		item->Position.yPos += item->VerticalVelocity;
		
		roomNumber = item->RoomNumber;
		GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);

		if (item->Position.yPos > item->ItemFlags[0])
		{
			item->Position.yPos = item->ItemFlags[0];
			if (item->VerticalVelocity <= 64)
				item->TriggerFlags &= 0xC0;
			else
				item->VerticalVelocity = -item->VerticalVelocity >> 2;
		}

		if (item->RoomNumber != roomNumber)
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
	ROOM_INFO* room = &g_Level.Rooms[item->RoomNumber];
	
	int found = -1;
	for (int i = 0; i < room->mesh.size(); i++)
	{
		MESH_INFO* mesh = &room->mesh[i];
		if (mesh->flags & StaticMeshFlags::SM_VISIBLE)
		{
			if (item->Position.xPos == mesh->pos.xPos && item->Position.zPos == mesh->pos.zPos)
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
	for (itemNumber = room->itemNumber; itemNumber != NO_ITEM; itemNumber = g_Level.Items[itemNumber].NextItem)
	{
		ITEM_INFO* current = &g_Level.Items[itemNumber];
		OBJECT_INFO* obj = &Objects[current->ObjectNumber];

		if (!obj->isPickup
			&& item->Position.xPos == current->Position.xPos
			&& item->Position.yPos <= current->Position.yPos
			&& item->Position.zPos == current->Position.zPos
			&& (current->ObjectNumber != ID_HIGH_OBJECT1 || current->ItemFlags[0] == 5))
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
	short triggerFlags = item->TriggerFlags & 0x3F;
	if (triggerFlags == 5)
	{
		item->ItemFlags[0] = item->Position.yPos - bounds->Y2;
		item->Status = ITEM_INVISIBLE;
	}
	else
	{
		if (triggerFlags == 0 || triggerFlags == 3 || triggerFlags == 4 || triggerFlags == 7 || triggerFlags == 8 || triggerFlags == 11)
			item->Position.yPos -= bounds->Y2;
		
		if ((item->TriggerFlags & 0x80) != 0)
		{
			RPickups[NumRPickups] = itemNumber;
			NumRPickups++;
		}
		
		if (item->TriggerFlags & 0x100)
			item->MeshBits = 0;
		
		if (item->Status == ITEM_INVISIBLE)
			item->Flags |= 0x20;
	}
}

void InitialiseSearchObject(short itemNumber)
{
	ITEM_INFO* item, *item2;
	short itemNumber2;

	item = &g_Level.Items[itemNumber];
	if (item->ObjectNumber == ID_SEARCH_OBJECT1)
	{
		item->SwapMeshFlags = -1;
		item->MeshBits = 7;
	}
	else if (item->ObjectNumber == ID_SEARCH_OBJECT2)
	{
		item->MeshBits = 2;
	}
	else if (item->ObjectNumber == ID_SEARCH_OBJECT4)
	{
		item->ItemFlags[1] = -1;
		item->MeshBits = 9;
		
		for (itemNumber2 = 0; itemNumber2 < g_Level.NumItems; ++itemNumber2)
		{
			item2 = &g_Level.Items[itemNumber2];

			if (item2->ObjectNumber == ID_EXPLOSION)
			{
				if (item->Position.xPos == item2->Position.xPos && item->Position.yPos == item2->Position.yPos && item->Position.zPos == item2->Position.zPos)
				{
					item->ItemFlags[1] = itemNumber2;
					break;
				}
			}
			else if (Objects[item2->ObjectNumber].isPickup
				&&  item->Position.xPos == item2->Position.xPos
				&&  item->Position.yPos == item2->Position.yPos
				&&  item->Position.zPos == item2->Position.zPos)
			{
				item->ItemFlags[1] = itemNumber2;
				break;
			}

		}
		AddActiveItem(itemNumber);
		item->Flags |= IFLAG_ACTIVATION_MASK;
		item->Status = ITEM_ACTIVE;
	}
}

void SearchObjectCollision(short itemNumber, ITEM_INFO* laraitem, COLL_INFO* laracoll)
{
	ITEM_INFO* item;
	int objNumber;
	BOUNDING_BOX* bounds;

	item = &g_Level.Items[itemNumber];
	objNumber = (item->ObjectNumber - ID_SEARCH_OBJECT1) / 2;

	if (TrInput & IN_ACTION
		&& laraitem->ActiveState == LS_IDLE
		&& laraitem->AnimNumber == LA_STAND_IDLE 
		&& Lara.gunStatus == LG_HANDS_FREE 
		&& (item->Status == ITEM_NOT_ACTIVE 
			&& item->ObjectNumber != ID_SEARCH_OBJECT4 || !item->ItemFlags[0])
		|| Lara.isMoving && Lara.interactedItem == itemNumber)
	{
		bounds = GetBoundsAccurate(item);
		if (item->ObjectNumber != ID_SEARCH_OBJECT1)
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
				laraitem->ActiveState = LS_MISC_CONTROL;
				laraitem->AnimNumber = SearchAnims[objNumber];
				laraitem->FrameNumber = g_Level.Anims[laraitem->AnimNumber].frameBase;
				Lara.isMoving = false;
				ResetLaraFlex(laraitem);
				Lara.gunStatus = LG_HANDS_BUSY;

				if (item->ObjectNumber == ID_SEARCH_OBJECT4)
				{
					item->ItemFlags[0] = 1;
				}
				else
				{
					AddActiveItem(itemNumber);
					item->Status = ITEM_ACTIVE;
				}

				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
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
			Lara.gunStatus = LG_HANDS_FREE;
		}
	}
	else if (laraitem->ActiveState != LS_MISC_CONTROL)
	{
		ObjectCollision(itemNumber, laraitem, laracoll);
	}
}

void SearchObjectControl(short itemNumber)
{
	ITEM_INFO* item, *item2;
	int objNumber;
	int frameNumber;

	item = &g_Level.Items[itemNumber];
	objNumber = (item->ObjectNumber - ID_SEARCH_OBJECT1) / 2;

	if (item->ObjectNumber != ID_SEARCH_OBJECT4 || item->ItemFlags[0] == 1)
		AnimateItem(item);

	frameNumber = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;
	if (item->ObjectNumber == ID_SEARCH_OBJECT1)
	{
		if (frameNumber > 0)
		{
			item->SwapMeshFlags = 0;
			item->MeshBits = -1;
		}
		else
		{
			item->SwapMeshFlags = -1;
			item->MeshBits = 7;
		}
	}
	else if (item->ObjectNumber == ID_SEARCH_OBJECT2)
	{
		if (frameNumber == 18)
			item->MeshBits = 1;
		else if (frameNumber == 172)
			item->MeshBits = 2;
	}
	else if (item->ObjectNumber == ID_SEARCH_OBJECT4)
	{
		item->MeshBits = FlipStats[0] != 0 ? 48 : 9;

		if (frameNumber >= 45 && frameNumber <= 131)
			item->MeshBits |= FlipStats[0] != 0 ? 4 : 2;
			
		if (item->ItemFlags[1] != -1)
		{
			item2 = &g_Level.Items[item->ItemFlags[1]];
			if (Objects[item2->ObjectNumber].isPickup)
			{
				if (FlipStats[0])
					item2->Status = ITEM_NOT_ACTIVE;
				else
					item2->Status = ITEM_INVISIBLE;
			}
		}
	}

	if (frameNumber == SearchCollectFrames[objNumber])
	{
		if (item->ObjectNumber == ID_SEARCH_OBJECT4)
		{
			if (item->ItemFlags[1] != -1)
			{
				item2 = &g_Level.Items[item->ItemFlags[1]];
				if (Objects[item2->ObjectNumber].isPickup)
				{
					AddDisplayPickup(item2->ObjectNumber);
					KillItem(item->ItemFlags[1]);
				}
				else
				{
					AddActiveItem(item->ItemFlags[1]);
					item2->Flags |= IFLAG_ACTIVATION_MASK;
					item2->Status = ITEM_ACTIVE;
					LaraItem->HitPoints = 640;
				}
				item->ItemFlags[1] = -1;
			}
		}
		else
		{
			CollectCarriedItems(item);
		}
	}

	
	if (item->Status == ITEM_DEACTIVATED)
	{
		if (item->ObjectNumber == ID_SEARCH_OBJECT4)
		{
			item->ItemFlags[0] = 0;
			item->Status = ITEM_ACTIVE;
		}
		else
		{
			RemoveActiveItem(itemNumber);
			item->Status = ITEM_NOT_ACTIVE;
		}
	}
}

int UseSpecialItem(ITEM_INFO* item)
{
	int flag = 0;
	int use = g_Gui.GetInventoryItemChosen();

	if (item->AnimNumber == LA_STAND_IDLE && Lara.gunStatus == LG_HANDS_FREE && use != NO_ITEM)
	{
		if ((use >= ID_WATERSKIN1_EMPTY) && (use <= ID_WATERSKIN2_5))
		{
			item->ItemFlags[2] = ID_LARA_WATER_MESH;
			flag = 1;
		}
		else if (use == ID_CLOCKWORK_BEETLE)
		{
			flag = 4;
			item->AnimNumber = LA_MECHANICAL_BEETLE_USE;
			UseClockworkBeetle(1);
		}

		if (flag == 1)
		{
			if (use != ID_WATERSKIN1_3 && use != ID_WATERSKIN2_5 && (LaraItem->Position.yPos > Lara.waterSurfaceDist))
			{
				if (use < ID_WATERSKIN1_3)
					Lara.smallWaterskin = 4;
				else
					Lara.bigWaterskin = 6;

				flag = 1;
			}
			else if (use != ID_WATERSKIN1_EMPTY && use != ID_WATERSKIN2_EMPTY)
			{
				if (use <= ID_WATERSKIN1_3)
				{
					item->ItemFlags[3] = Lara.smallWaterskin - 1;
					Lara.smallWaterskin = 1;
				}
				else
				{
					item->ItemFlags[3] = Lara.bigWaterskin - 1;
					Lara.bigWaterskin = 1;
				}

				flag = 2;
			}
			else
				return 0;
		}

		if (flag)
		{
			if (flag == 1)
				item->AnimNumber = LA_WATERSKIN_FILL;
			else if (flag == 2)
				item->AnimNumber = LA_WATERSKIN_POUR_LOW;

			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->TargetState = LS_MISC_CONTROL;
			item->ActiveState = LS_MISC_CONTROL;
			Lara.gunStatus = LG_HANDS_BUSY;

			g_Gui.SetInventoryItemChosen(NO_ITEM);

			return 1;
		}
	}

	return 0;
}
