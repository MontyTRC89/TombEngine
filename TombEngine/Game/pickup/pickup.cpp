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

using namespace TEN::Input;
using namespace TEN::Entities::Generic;

// TODO: Adjust these bounds when crawl surface alignment is implemented. @Sezz 2021.11.04
static Vector3Int PickUpPosition(0, 0, -100);
OBJECT_COLLISION_BOUNDS PickUpBounds = 
{
	-256, 256,
	-200, 200,
	-256, 256,
	-ANGLE(10.0f), ANGLE(10.0f),
	0, 0,
	-ANGLE(2.0f), ANGLE(2.0f)
};

static Vector3Int HiddenPickUpPosition(0, 0, -690);
OBJECT_COLLISION_BOUNDS HiddenPickUpBounds =
{
	-256, 256,
	-100, 100,
	-800, -256,
	ANGLE(-10.0f), ANGLE(10.0f),
	ANGLE(-30.0f), ANGLE(30.0f),
	0, 0
};

static Vector3Int CrowbarPickUpPosition(0, 0, 215);
OBJECT_COLLISION_BOUNDS CrowbarPickUpBounds =
{
	-256, 256,
	-100, 100,
	200, 512,
	ANGLE(-10.0f), ANGLE(10.0f),
	ANGLE(-30.0f), ANGLE(30.0f),
	0, 0
};

static Vector3Int JobyCrowPickUpPosition(-224, 0, 240);
OBJECT_COLLISION_BOUNDS JobyCrowPickUpBounds =
{
	-512, 0,
	-100, 100,
	0, 512,
	ANGLE(-10.0f), ANGLE(10.0f),
	ANGLE(-30.0f), ANGLE(30.0f),
	0, 0
};

static Vector3Int PlinthPickUpPosition(0, 0, -460);
OBJECT_COLLISION_BOUNDS PlinthPickUpBounds =
{
	-256, 256,
	-640, 640,
	-511, 0,
	ANGLE(-10.0f), ANGLE(10.0f),
	ANGLE(-30.0f), ANGLE(30.0f),
	0, 0
};

static Vector3Int PickUpPositionUW(0, -200, -350);
OBJECT_COLLISION_BOUNDS PickUpBoundsUW =
{
	-512, 512,
	-512, 512,
	-512, 512,
	ANGLE(-45.0f),  ANGLE(45.0f),
	ANGLE(-45.0f),  ANGLE(45.0f),
	ANGLE(-45.0f),  ANGLE(45.0f)
};

static Vector3Int SOPos(0, 0, 0);
OBJECT_COLLISION_BOUNDS SOBounds =
{
	0, 0,
	0, 0,
	0, 0,
	ANGLE(-10.0f), ANGLE(10.0f),
	ANGLE(-30.0f), ANGLE(30.0f),
	ANGLE(-10.0f), ANGLE(10.0f)
};

short SearchCollectFrames[4] = { 180, 100, 153, 83 };
short SearchAnims[4] = { LA_LOOT_CABINET, LA_LOOT_DRAWER, LA_LOOT_SHELF, LA_LOOT_CHEST };
short SearchOffsets[4] = { 160, 96, 160, 112 };

OBJECT_COLLISION_BOUNDS MSBounds =
{
	0, 0,
	0, 0,
	0, 0,
	ANGLE(-10.0f), ANGLE(10.0f),
	ANGLE(-30.0f), ANGLE(30.0f),
	ANGLE(-10.0f), ANGLE(10.0f)
};

int NumRPickups;
short RPickups[16];
short getThisItemPlease = NO_ITEM;
Vector3Int OldPickupPos;

void PickedUpObject(GAME_OBJECT_ID objectID, int count)
{
	// see if the items fit into one of these easy groups
	if (!TryAddingWeapon(Lara, objectID, count) &&
		!TryAddingAmmo(Lara, objectID, count) &&
		!TryAddingKeyItem(Lara, objectID, count) &&
		!TryAddingConsumable(Lara, objectID, count) &&
		!TryAddMiscItem(Lara, objectID))
	{
		// item isn't any of the above; do nothing
	}
}

int GetInventoryCount(GAME_OBJECT_ID objectID)
{
	auto boolResult = HasWeapon(Lara, objectID);
	if (boolResult.has_value())
		return int{ boolResult.value() };

	auto intResult = GetAmmoCount(Lara, objectID);
	if (intResult.has_value())
		return intResult.value();

	intResult = GetKeyItemCount(Lara, objectID);
	if (intResult.has_value())
		return intResult.value();

	intResult = GetConsumableCount(Lara, objectID);
	if (intResult.has_value())
		return intResult.value();

	boolResult = HasMiscItem(Lara, objectID);
	if (boolResult.has_value())
		return int{ boolResult.value() };

	return 0;
}

void RemoveObjectFromInventory(GAME_OBJECT_ID objectID, int count)
{
	// see if the items fit into one of these easy groups
	if (!TryRemovingWeapon(Lara, objectID, count) && 
		!TryRemovingAmmo(Lara, objectID, count) && 
		!TryRemovingKeyItem(Lara, objectID, count) && 
		!TryRemovingConsumable(Lara, objectID, count) && 
		!TryRemoveMiscItem(Lara, objectID))
		{
			// item isn't any of the above; do nothing
		}
}

void CollectCarriedItems(ItemInfo* item) 
{
	short pickupNumber = item->CarriedItem;
	while (pickupNumber != NO_ITEM)
	{
		auto* pickupItem = &g_Level.Items[pickupNumber];

		AddDisplayPickup(pickupItem->ObjectNumber);
		KillItem(pickupNumber);

		pickupNumber = pickupItem->CarriedItem;
	}

	item->CarriedItem = NO_ITEM;
}

void DoPickup(ItemInfo* laraItem)
{
	if (getThisItemPlease == NO_ITEM)
		return;

	auto* lara = GetLaraInfo(laraItem);

	short pickupItemNumber = getThisItemPlease;
	auto* pickupItem = &g_Level.Items[pickupItemNumber];

	auto oldOrient = pickupItem->Pose.Orientation;

	if (pickupItem->ObjectNumber == ID_BURNING_TORCH_ITEM)
	{
		AddDisplayPickup(ID_BURNING_TORCH_ITEM);
		GetFlameTorch();
		lara->Torch.IsLit = (pickupItem->ItemFlags[3] & 1);

		KillItem(pickupItemNumber);
		pickupItem->Pose.Orientation = oldOrient;
		getThisItemPlease = NO_ITEM;
		return;
	}
	else if (pickupItem->ObjectNumber == ID_FLARE_ITEM)
	{
		if (laraItem->Animation.ActiveState == LA_UNDERWATER_PICKUP_FLARE)
		{
			lara->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			lara->Control.Weapon.GunType = LaraWeaponType::Flare;
			InitialiseNewWeapon(laraItem);
			lara->Control.HandStatus = HandStatus::Special;
			lara->Flare.Life = (int)(pickupItem->Data) & 0x7FFF;
			DrawFlareMeshes(laraItem);
			KillItem(pickupItemNumber);

			pickupItem->Pose.Orientation = oldOrient;
			getThisItemPlease = NO_ITEM;
			return;
		}
		else if (laraItem->Animation.ActiveState == LS_PICKUP_FLARE)
		{
			lara->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			lara->Control.Weapon.GunType = LaraWeaponType::Flare;
			InitialiseNewWeapon(laraItem);
			lara->Control.HandStatus = HandStatus::Special;
			lara->Flare.Life = (int)(pickupItem->Data) & 0x7FFF;
			KillItem(pickupItemNumber);
			getThisItemPlease = NO_ITEM;
			return;
		}
	}
	else
	{
		if (laraItem->Animation.AnimNumber == LA_UNDERWATER_PICKUP) //dirty but what can I do, it uses the same state
		{
			AddDisplayPickup(pickupItem->ObjectNumber);
			if (!(pickupItem->TriggerFlags & 0xC0))
				KillItem(pickupItemNumber);
			else
			{
				pickupItem->ItemFlags[3] = 1;
				pickupItem->Flags |= TRIGGERED;
				pickupItem->Status = ITEM_INVISIBLE;
			}

			pickupItem->Pose.Orientation = oldOrient;
			getThisItemPlease = NO_ITEM;
			return;
		}
		else
		{
			if (laraItem->Animation.AnimNumber == LA_CROWBAR_PRY_WALL_SLOW)
			{
				AddDisplayPickup(ID_CROWBAR_ITEM);
				Lara.Inventory.HasCrowbar = true;
				KillItem(pickupItemNumber);
			}
			else if (laraItem->Animation.ActiveState == LS_PICKUP || laraItem->Animation.ActiveState == LS_PICKUP_FROM_CHEST || laraItem->Animation.ActiveState == LS_HOLE)
			{
				AddDisplayPickup(pickupItem->ObjectNumber);
				if (pickupItem->TriggerFlags & 0x100)
				{
					for (int i = 0; i < g_Level.NumItems; i++)
					{
						if (g_Level.Items[i].ObjectNumber == pickupItem->ObjectNumber)
							KillItem(i);
					}
				}

				if (pickupItem->TriggerFlags & 0xC0)
				{
					pickupItem->ItemFlags[3] = 1;
					pickupItem->Flags |= TRIGGERED;
					pickupItem->Status = ITEM_INVISIBLE;
				}

				pickupItem->Pose.Orientation = oldOrient;
				KillItem(pickupItemNumber);
				getThisItemPlease = NO_ITEM;
				return;
			}
		}
	}

	getThisItemPlease = NO_ITEM;
}

void PickupCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];

	auto oldOrient = item->Pose.Orientation;

	if (item->Status == ITEM_INVISIBLE)
		return;

	short triggerFlags = item->TriggerFlags & 0x3F;
	if (triggerFlags == 5 || triggerFlags == 10)
		return;

	auto lara = GetLaraInfo(laraItem);

	if (item->ObjectNumber == ID_FLARE_ITEM && lara->Control.Weapon.GunType == LaraWeaponType::Flare)
		return;

	item->Pose.Orientation.y = laraItem->Pose.Orientation.y;
	item->Pose.Orientation.z = 0;

	if (lara->Control.WaterStatus != WaterStatus::Dry &&
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		if (lara->Control.WaterStatus == WaterStatus::Underwater)
		{
			item->Pose.Orientation.x = -ANGLE(25.0f);

			if (TrInput & IN_ACTION && 
				item->ObjectNumber != ID_BURNING_TORCH_ITEM && 
				laraItem->Animation.ActiveState == LS_UNDERWATER_IDLE && 
				lara->Control.HandStatus == HandStatus::Free &&
				TestLaraPosition(&PickUpBoundsUW, item, laraItem) || lara->Control.IsMoving && lara->InteractedItem == itemNumber)
			{
				if (TestLaraPosition(&PickUpBoundsUW, item, laraItem))
				{
					if (MoveLaraPosition(&PickUpPositionUW, item, laraItem))
					{
						if (item->ObjectNumber == ID_FLARE_ITEM)
						{
							getThisItemPlease = itemNumber;
							laraItem->Animation.AnimNumber = LA_UNDERWATER_PICKUP_FLARE;
							laraItem->Animation.ActiveState = LS_PICKUP_FLARE;
							laraItem->Animation.VerticalVelocity = 0;
						}
						else
						{
							getThisItemPlease = itemNumber;
							laraItem->Animation.AnimNumber = LA_UNDERWATER_PICKUP;
							laraItem->Animation.ActiveState = LS_PICKUP;
						}

						laraItem->Animation.TargetState = LS_UNDERWATER_IDLE;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
						lara->Control.IsMoving = false;
						lara->Control.HandStatus = HandStatus::Busy;
					}

					lara->InteractedItem = itemNumber;
				}
				else
				{
					if (lara->Control.IsMoving)
					{
						if (lara->InteractedItem == itemNumber)
						{
							getThisItemPlease = itemNumber;
							Lara.Control.IsMoving = false;
							Lara.Control.HandStatus = HandStatus::Free;
						}
					}
				}

				item->Pose.Orientation = oldOrient;
				return;
			}
		}

		item->Pose.Orientation = oldOrient;
		return;
	}
	
	if (!(TrInput & IN_ACTION) && 
		(g_Gui.GetInventoryItemChosen() == NO_ITEM || triggerFlags != 2) || 
		BinocularRange ||
		(laraItem->Animation.ActiveState != LS_IDLE || laraItem->Animation.AnimNumber != LA_STAND_IDLE || lara->Control.HandStatus != HandStatus::Free) &&
		(laraItem->Animation.ActiveState != LS_CROUCH_IDLE || laraItem->Animation.AnimNumber != LA_CROUCH_IDLE || lara->Control.HandStatus != HandStatus::Free) &&
		(laraItem->Animation.ActiveState != LS_CRAWL_IDLE || laraItem->Animation.AnimNumber != LA_CRAWL_IDLE))
	{
		if (!lara->Control.IsMoving)
		{
			if (lara->InteractedItem == itemNumber)
			{
				if (laraItem->Animation.ActiveState != LS_PICKUP && laraItem->Animation.ActiveState != LS_HOLE)
				{
					item->Pose.Orientation = oldOrient;
					return;
				}
				else
				{
					item->Pose.Orientation = oldOrient;
					return;
				}
			}
		}

		if (lara->InteractedItem != itemNumber)
		{
			item->Pose.Orientation = oldOrient;
			return;
		}
	}
	
	bool flag = false;
	BOUNDING_BOX* plinth = NULL;
	item->Pose.Orientation.x = 0;
	switch (triggerFlags)
	{
	case 1: // Pickup from wall hole
		if (lara->Control.IsLow || !TestLaraPosition(&HiddenPickUpBounds, item, laraItem))
		{
			if (lara->Control.IsMoving)
			{
				if (lara->InteractedItem == itemNumber)
				{
					Lara.Control.IsMoving = false;
					Lara.Control.HandStatus = HandStatus::Free;
				}
			}

			item->Pose.Orientation = oldOrient;
			return;
		}
		else if (MoveLaraPosition(&HiddenPickUpPosition, item, laraItem))
		{
			getThisItemPlease = itemNumber;
			laraItem->Animation.AnimNumber = LA_HOLESWITCH_ACTIVATE;
			laraItem->Animation.ActiveState = LS_HOLE;
			flag = true;
		}

		lara->InteractedItem = itemNumber;
		break;

	case 2: // Pickup with crowbar
		item->Pose.Orientation.y = oldOrient.y;
		if (lara->Control.IsLow || !TestLaraPosition(&CrowbarPickUpBounds, item, laraItem))
		{
			if (!lara->Control.IsMoving)
			{
				item->Pose.Orientation = oldOrient;
				return;
			}

			if (lara->InteractedItem == itemNumber)
			{
				Lara.Control.IsMoving = false;
				Lara.Control.HandStatus = HandStatus::Free;
			}

			item->Pose.Orientation = oldOrient;
			return;
		}
		if (!lara->Control.IsMoving)
		{
			if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
			{
				if (g_Gui.IsObjectInInventory(ID_CROWBAR_ITEM))
					g_Gui.SetEnterInventory(ID_CROWBAR_ITEM);

				item->Pose.Orientation = oldOrient;
				return;
			}

			if (g_Gui.GetInventoryItemChosen() != ID_CROWBAR_ITEM)
			{
				item->Pose.Orientation = oldOrient;
				return;
			}

			g_Gui.SetInventoryItemChosen(NO_ITEM);
		}
		if (MoveLaraPosition(&CrowbarPickUpPosition, item, laraItem))
		{
			getThisItemPlease = itemNumber;
			laraItem->Animation.AnimNumber = LA_CROWBAR_PRY_WALL_FAST;
			laraItem->Animation.ActiveState = LS_PICKUP;
			item->Status = ITEM_ACTIVE;
			AddActiveItem(itemNumber);
			AnimateItem(item);
			flag = true;
		}

		lara->InteractedItem = itemNumber;
		break;

	case 3:
	case 4:
	case 7:
	case 8:
		plinth = FindPlinth(item);

		if (!plinth)
		{
			item->Pose.Orientation = oldOrient;
			return;
		}

		PlinthPickUpBounds.boundingBox.X1 = plinth->X1;
		PlinthPickUpBounds.boundingBox.X2 = plinth->X2;
		PlinthPickUpBounds.boundingBox.Y2 = laraItem->Pose.Position.y - item->Pose.Position.y + 100;
		PlinthPickUpBounds.boundingBox.Z2 = plinth->Z2 + 320;
		PlinthPickUpPosition.z = -200 - plinth->Z2;

		// HACK: Until we refactor a way plinth collision is detected, this must be here
		// to prevent false positives with two stacked plinths -- Lwmte, 16.06.22
		if (abs(laraItem->Pose.Position.y - item->Pose.Position.y) > CLICK(4))
			break;

		if (TestLaraPosition(&PlinthPickUpBounds, item, laraItem) && !lara->Control.IsLow)
		{
			if (item->Pose.Position.y == laraItem->Pose.Position.y)
				PlinthPickUpPosition.y = 0;
			else
				PlinthPickUpPosition.y = laraItem->Pose.Position.y - item->Pose.Position.y;
			if (MoveLaraPosition(&PlinthPickUpPosition, item, laraItem))
			{
				if (triggerFlags == 3 || triggerFlags == 7)
				{
					getThisItemPlease = itemNumber;
					laraItem->Animation.AnimNumber = LA_PICKUP_PEDESTAL_HIGH;
					laraItem->Animation.ActiveState = LS_PICKUP;
				}
				else
				{
					getThisItemPlease = itemNumber;
					laraItem->Animation.AnimNumber = LA_PICKUP_PEDESTAL_LOW;
					laraItem->Animation.ActiveState = LS_PICKUP;
				}

				flag = true;
			}

			lara->InteractedItem = itemNumber;
			break;
		}

		if (!lara->Control.IsMoving)
		{
			item->Pose.Orientation = oldOrient;
			return;
		}
		
		if (lara->InteractedItem == itemNumber)
		{
			Lara.Control.IsMoving = false;
			Lara.Control.HandStatus = HandStatus::Free;
		}

		item->Pose.Orientation = oldOrient;
		return;

	case 9: // Pickup object and conver it to crowbar (like submarine level)
		item->Pose.Orientation.y = oldOrient.y;

		if (!TestLaraPosition(&JobyCrowPickUpBounds, item, laraItem))
		{
			item->Pose.Orientation = oldOrient;
			return;
		}

		if (MoveLaraPosition(&JobyCrowPickUpPosition, item, laraItem))
		{
			getThisItemPlease = itemNumber;
			laraItem->Animation.AnimNumber = LA_CROWBAR_PRY_WALL_SLOW;
			laraItem->Animation.ActiveState = LS_PICKUP;
			item->Status = ITEM_ACTIVE;
			AddActiveItem(itemNumber);
			flag = true;
		}

		lara->InteractedItem = itemNumber;
		break;

	default:
		if (!TestLaraPosition(&PickUpBounds, item, laraItem))
		{
			if (!lara->Control.IsMoving)
			{
				item->Pose.Orientation = oldOrient;
				return;
			}
			
			if (lara->InteractedItem == itemNumber)
			{
				Lara.Control.IsMoving = false;
				Lara.Control.HandStatus = HandStatus::Free;
			}

			item->Pose.Orientation = oldOrient;
			return;
		}

		PickUpPosition.y = laraItem->Pose.Position.y - item->Pose.Position.y;

		if (laraItem->Animation.ActiveState == LS_CROUCH_IDLE)
		{
			if (item->ObjectNumber == ID_BURNING_TORCH_ITEM)
				break;

			if (!AlignLaraPosition(&PickUpPosition, item, laraItem))
				break;

			if (item->ObjectNumber == ID_FLARE_ITEM)
			{
				getThisItemPlease = itemNumber;
				laraItem->Animation.AnimNumber = LA_CROUCH_PICKUP_FLARE;
				laraItem->Animation.ActiveState = LS_PICKUP_FLARE;
				lara->InteractedItem = itemNumber;
				flag = true;
				break;
			}

			getThisItemPlease = itemNumber;
			laraItem->Animation.TargetState = LS_PICKUP;
		}
		else
		{
			if (laraItem->Animation.ActiveState == LS_CRAWL_IDLE)
			{
				if (item->ObjectNumber == ID_BURNING_TORCH_ITEM)
					break;

				if (!AlignLaraPosition(&PickUpPosition, item, laraItem))
					break;

				if (item->ObjectNumber == ID_FLARE_ITEM)
				{
					laraItem->Animation.TargetState = LS_CROUCH_IDLE;
					lara->InteractedItem = itemNumber;
				}
				else
				{
					getThisItemPlease = itemNumber;
					laraItem->Animation.TargetState = LS_PICKUP;
				}

				break;
			}
			else
			{
				if (!MoveLaraPosition(&PickUpPosition, item, laraItem))
				{
					lara->InteractedItem = itemNumber;
					break;
				}

				getThisItemPlease = itemNumber;

				if (item->ObjectNumber == ID_FLARE_ITEM)
				{
					laraItem->Animation.AnimNumber = LA_PICKUP;
					laraItem->Animation.ActiveState = LS_PICKUP_FLARE;
					lara->InteractedItem = itemNumber;
					flag = true;
					break;
				}
				else
				{
					// HACK: because of MoveLaraPosition(), we can't properly dispatch. Must be fixed later.
					laraItem->Animation.AnimNumber = LA_PICKUP;
					laraItem->Animation.ActiveState = LS_PICKUP;
				}
			}
		}

		lara->InteractedItem = itemNumber;
		flag = true;
	}

	if (flag)
	{
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		ResetLaraFlex(laraItem);
		lara->Control.IsMoving = false;
		lara->Control.HandStatus = HandStatus::Busy;
	}

	item->Pose.Orientation = oldOrient;
}

void RegeneratePickups()
{
	for (int i = 0; i < NumRPickups; i++)
	{
		auto* item = &g_Level.Items[RPickups[i]];

		if (item->Status == ITEM_INVISIBLE)
		{
			short ammo = 0;
			switch (item->ObjectNumber)
			{
			case ID_CROSSBOW_AMMO1_ITEM:
				ammo = Lara.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_CROSSBOW_AMMO2_ITEM:
				ammo = Lara.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo2];
				break;

			case ID_CROSSBOW_AMMO3_ITEM:
				ammo = Lara.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo3];
				break;

			case ID_GRENADE_AMMO1_ITEM:
				ammo = Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_GRENADE_AMMO2_ITEM:
				ammo = Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo2];
				break;

			case ID_GRENADE_AMMO3_ITEM:
				ammo = Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo3];
				break;

			case ID_HK_AMMO_ITEM:
				ammo = Lara.Weapons[(int)LaraWeaponType::HK].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_UZI_AMMO_ITEM:
				ammo = Lara.Weapons[(int)LaraWeaponType::Uzi].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_HARPOON_AMMO_ITEM:
				ammo = Lara.Weapons[(int)LaraWeaponType::HarpoonGun].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_ROCKET_LAUNCHER_AMMO_ITEM:
				ammo = Lara.Weapons[(int)LaraWeaponType::RocketLauncher].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_REVOLVER_AMMO_ITEM:
				ammo = Lara.Weapons[(int)LaraWeaponType::Revolver].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_SHOTGUN_AMMO1_ITEM:
				ammo = Lara.Weapons[(int)LaraWeaponType::Shotgun].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_SHOTGUN_AMMO2_ITEM:
				ammo = Lara.Weapons[(int)LaraWeaponType::Shotgun].Ammo[(int)WeaponAmmoType::Ammo2];
				break;
			}

			if (ammo == 0)
				item->Status = ITEM_NOT_ACTIVE;
		}
	}
}

void PickupControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	short roomNumber;
	short triggerFlags = item->TriggerFlags & 0x3F;

	switch (triggerFlags)
	{
	case 5:
		item->Animation.VerticalVelocity += 6;
		item->Pose.Position.y += item->Animation.VerticalVelocity;
		
		roomNumber = item->RoomNumber;
		GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);

		if (item->Pose.Position.y > item->ItemFlags[0])
		{
			item->Pose.Position.y = item->ItemFlags[0];
			if (item->Animation.VerticalVelocity <= 64)
				item->TriggerFlags &= 0xC0;
			else
				item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity >> 2;
		}

		if (item->RoomNumber != roomNumber)
			ItemNewRoom(itemNumber, roomNumber);

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

BOUNDING_BOX* FindPlinth(ItemInfo* item)
{
	auto* room = &g_Level.Rooms[item->RoomNumber];
	
	int found = -1;
	for (int i = 0; i < room->mesh.size(); i++)
	{
		auto* mesh = &room->mesh[i];

		if (mesh->flags & StaticMeshFlags::SM_VISIBLE)
		{
			if (item->Pose.Position.x == mesh->pos.Position.x && item->Pose.Position.z == mesh->pos.Position.z)
			{
				auto* frame = (BOUNDING_BOX*)GetBestFrame(item);
				auto* staticInfo = &StaticObjects[mesh->staticNumber];

				if (frame->X1 <= staticInfo->collisionBox.X2 &&
					frame->X2 >= staticInfo->collisionBox.X1 &&
					frame->Z1 <= staticInfo->collisionBox.Z2 &&
					frame->Z2 >= staticInfo->collisionBox.Z1 &&
					(staticInfo->collisionBox.X1 || staticInfo->collisionBox.X2))
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
		auto* currentItem = &g_Level.Items[itemNumber];
		auto* object = &Objects[currentItem->ObjectNumber];

		if (!object->isPickup &&
			item->Pose.Position.x == currentItem->Pose.Position.x &&
			item->Pose.Position.y <= currentItem->Pose.Position.y &&
			item->Pose.Position.z == currentItem->Pose.Position.z &&
			(currentItem->ObjectNumber != ID_HIGH_OBJECT1 || currentItem->ItemFlags[0] == 5))
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
	auto* item = &g_Level.Items[itemNumber];
	auto* bounds = GetBoundsAccurate(item);

	short triggerFlags = item->TriggerFlags & 0x3F;
	if (triggerFlags == 5)
	{
		item->ItemFlags[0] = item->Pose.Position.y - bounds->Y2;
		item->Status = ITEM_INVISIBLE;
	}
	else
	{
		if (triggerFlags == 0 || triggerFlags == 3 || triggerFlags == 4 || triggerFlags == 7 || triggerFlags == 8 || triggerFlags == 11)
			item->Pose.Position.y -= bounds->Y2;
		
		if ((item->TriggerFlags & 0x80) != 0)
		{
			RPickups[NumRPickups] = itemNumber;
			NumRPickups++;
		}
		
		if (item->TriggerFlags & 0x100)
			item->MeshBits = 0;
		
		if (item->Status == ITEM_INVISIBLE)
			item->Flags |= TRIGGERED;
	}
}

void InitialiseSearchObject(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	if (item->ObjectNumber == ID_SEARCH_OBJECT1)
	{
		item->MeshSwapBits = ALL_JOINT_BITS;
		item->MeshBits = 7;
	}
	else if (item->ObjectNumber == ID_SEARCH_OBJECT2)
		item->MeshBits = 2;
	else if (item->ObjectNumber == ID_SEARCH_OBJECT4)
	{
		item->ItemFlags[1] = -1;
		item->MeshBits = 9;
		
		for (short itemNumber2 = 0; itemNumber2 < g_Level.NumItems; ++itemNumber2)
		{
			auto* item2 = &g_Level.Items[itemNumber2];

			if (item2->ObjectNumber == ID_EXPLOSION)
			{
				if (item->Pose.Position.x == item2->Pose.Position.x && item->Pose.Position.y == item2->Pose.Position.y && item->Pose.Position.z == item2->Pose.Position.z)
				{
					item->ItemFlags[1] = itemNumber2;
					break;
				}
			}
			else if (Objects[item2->ObjectNumber].isPickup &&
				item->Pose.Position == item2->Pose.Position)
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

void SearchObjectCollision(short itemNumber, ItemInfo* laraitem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];
	int objectNumber = (item->ObjectNumber - ID_SEARCH_OBJECT1) / 2;

	if (TrInput & IN_ACTION &&
		laraitem->Animation.ActiveState == LS_IDLE &&
		laraitem->Animation.AnimNumber == LA_STAND_IDLE &&
		Lara.Control.HandStatus == HandStatus::Free &&
		(item->Status == ITEM_NOT_ACTIVE &&
			item->ObjectNumber != ID_SEARCH_OBJECT4 || !item->ItemFlags[0]) ||
		Lara.Control.IsMoving && Lara.InteractedItem == itemNumber)
	{
		auto* bounds = GetBoundsAccurate(item);
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
		SOPos.z = bounds->Z1 - SearchOffsets[objectNumber];

		if (TestLaraPosition(&SOBounds, item, laraitem))
		{
			if (MoveLaraPosition(&SOPos, item, laraitem))
			{
				ResetLaraFlex(laraitem);
				laraitem->Animation.AnimNumber = SearchAnims[objectNumber];
				laraitem->Animation.FrameNumber = g_Level.Anims[laraitem->Animation.AnimNumber].frameBase;
				laraitem->Animation.ActiveState = LS_MISC_CONTROL;
				Lara.Control.IsMoving = false;
				Lara.Control.HandStatus = HandStatus::Busy;

				if (item->ObjectNumber == ID_SEARCH_OBJECT4)
					item->ItemFlags[0] = 1;
				else
				{
					AddActiveItem(itemNumber);
					item->Status = ITEM_ACTIVE;
				}

				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				AnimateItem(item);
			}
			else
				Lara.InteractedItem = itemNumber;
		}
		else if (Lara.Control.IsMoving && Lara.InteractedItem ==  itemNumber)
		{
			Lara.Control.IsMoving = false;
			Lara.Control.HandStatus = HandStatus::Free;
		}
	}
	else if (laraitem->Animation.ActiveState != LS_MISC_CONTROL)
		ObjectCollision(itemNumber, laraitem, coll);
}

void SearchObjectControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	int objectNumber = (item->ObjectNumber - ID_SEARCH_OBJECT1) / 2;

	if (item->ObjectNumber != ID_SEARCH_OBJECT4 || item->ItemFlags[0] == 1)
		AnimateItem(item);

	ItemInfo* item2;

	int frameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
	if (item->ObjectNumber == ID_SEARCH_OBJECT1)
	{
		if (frameNumber > 0)
		{
			item->MeshSwapBits = NO_JOINT_BITS;
			item->MeshBits = ALL_JOINT_BITS;
		}
		else
		{
			item->MeshSwapBits = ALL_JOINT_BITS;
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

	if (frameNumber == SearchCollectFrames[objectNumber])
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
			CollectCarriedItems(item);
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

bool UseSpecialItem(ItemInfo* item)
{
	int flag = 0;
	int use = g_Gui.GetInventoryItemChosen();

	if (item->Animation.AnimNumber == LA_STAND_IDLE && Lara.Control.HandStatus == HandStatus::Free && use != NO_ITEM)
	{
		if ((use >= ID_WATERSKIN1_EMPTY) && (use <= ID_WATERSKIN2_5))
		{
			item->ItemFlags[2] = ID_LARA_WATER_MESH;
			flag = 1;
		}
		else if (use == ID_CLOCKWORK_BEETLE)
		{
			flag = 4;
			item->Animation.AnimNumber = LA_MECHANICAL_BEETLE_USE;
			UseClockworkBeetle(1);
		}

		if (flag == 1)
		{
			if (use != ID_WATERSKIN1_3 && use != ID_WATERSKIN2_5 && (Lara.WaterSurfaceDist < -SHALLOW_WATER_START_LEVEL))
			{
				if (use < ID_WATERSKIN1_3)
					Lara.Inventory.SmallWaterskin = 4;
				else
					Lara.Inventory.BigWaterskin = 6;

				flag = 1;
			}
			else if (use != ID_WATERSKIN1_EMPTY && use != ID_WATERSKIN2_EMPTY)
			{
				if (use <= ID_WATERSKIN1_3)
				{
					item->ItemFlags[3] = Lara.Inventory.SmallWaterskin - 1;
					Lara.Inventory.SmallWaterskin = 1;
				}
				else
				{
					item->ItemFlags[3] = Lara.Inventory.BigWaterskin - 1;
					Lara.Inventory.BigWaterskin = 1;
				}

				flag = 2;
			}
			else
				return false;
		}

		if (flag)
		{
			if (flag == 1)
				item->Animation.AnimNumber = LA_WATERSKIN_FILL;
			else if (flag == 2)
				item->Animation.AnimNumber = LA_WATERSKIN_POUR_LOW;

			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = LS_MISC_CONTROL;
			item->Animation.TargetState = LS_MISC_CONTROL;
			Lara.Control.HandStatus = HandStatus::Busy;

			g_Gui.SetInventoryItemChosen(NO_ITEM);

			return true;
		}
	}

	return false;
}
