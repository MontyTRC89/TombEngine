#include "framework.h"
#include "Game/pickup/pickup.h"

#include "pickuputil.h"
#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/debris.h"
#include "Game/Gui.h"
#include "Game/Hud/Hud.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_two_guns.h"
#include "Game/pickup/pickup_ammo.h"
#include "Game/pickup/pickup_consumable.h"
#include "Game/pickup/pickup_key_items.h"
#include "Game/pickup/pickup_misc_items.h"
#include "Game/pickup/pickup_weapon.h"
#include "Game/room.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Objects/TR4/Object/tr4_clockwork_beetle.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Math/Math.h"
#include "Specific/setup.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"

using namespace TEN::Entities::Generic;
using namespace TEN::Hud;
using namespace TEN::Input;

static auto PickUpPosition = Vector3i(0, 0, -100);
const ObjectCollisionBounds PickUpBounds =
{
	GameBoundingBox(
		-CLICK(1), CLICK(1),
		-200, 200,
		-CLICK(1), CLICK(1)),
	std::pair(
		EulerAngles(ANGLE(-45.0f), 0, ANGLE(-45.0f)),
		EulerAngles(ANGLE(45.0f), 0, ANGLE(45.0f)))
};

static auto HiddenPickUpPosition = Vector3i(0, 0, -690);
const ObjectCollisionBounds HiddenPickUpBounds =
{
	GameBoundingBox(
		-CLICK(1), CLICK(1),
		-100, 100,
		-800, -CLICK(1)),
	std::pair(
		EulerAngles(ANGLE(-45.0f), ANGLE(-30.0f), ANGLE(-45.0f)),
		EulerAngles(ANGLE(45.0f), ANGLE(30.0f), ANGLE(45.0f)))
};

static auto CrowbarPickUpPosition = Vector3i(0, 0, 215);
const ObjectCollisionBounds CrowbarPickUpBounds =
{
	GameBoundingBox(
		-CLICK(1), CLICK(1),
		-100, 100,
		200, CLICK(2)	),
	std::pair(
		EulerAngles(ANGLE(-45.0f), ANGLE(-30.0f), ANGLE(-45.0f)),
		EulerAngles(ANGLE(45.0f), ANGLE(30.0f), ANGLE(45.0f)))
};

static auto JobyCrowPickUpPosition = Vector3i(-224, 0, 240);
const ObjectCollisionBounds JobyCrowPickUpBounds =
{
	GameBoundingBox(
		-CLICK(2), 0,
		-100, 100,
		0, CLICK(2)),
	std::pair(
		EulerAngles(ANGLE(-45.0f), ANGLE(-30.0f), ANGLE(-45.0f)),
		EulerAngles(ANGLE(45.0f), ANGLE(30.0f), ANGLE(45.0f)))
};

static auto PlinthPickUpPosition = Vector3i(0, 0, -460);
ObjectCollisionBounds PlinthPickUpBounds =
{
	GameBoundingBox(
		-CLICK(1), CLICK(1),
		-640, 640,
		-511, 0),
	std::pair(
		EulerAngles(ANGLE(-45.0f), ANGLE(-30.0f), ANGLE(-45.0f)),
		EulerAngles(ANGLE(45.0f), ANGLE(30.0f), ANGLE(45.0f)))
};

static auto PickUpPositionUW = Vector3i(0, -200, -350);
const ObjectCollisionBounds PickUpBoundsUW =
{
	GameBoundingBox(
		-CLICK(2), CLICK(2),
		-CLICK(2), CLICK(2),
		-CLICK(2), CLICK(2)),
	std::pair(
		EulerAngles(ANGLE(-45.0f), ANGLE(-45.0f), ANGLE(-45.0f)),
		EulerAngles(ANGLE(45.0f), ANGLE(45.0f), ANGLE(45.0f)))
};

static auto SOPos = Vector3i::Zero;
ObjectCollisionBounds SOBounds =
{
	GameBoundingBox::Zero,
	std::pair(
		EulerAngles(ANGLE(-45.0f), ANGLE(-30.0f), ANGLE(-45.0f)),
		EulerAngles(ANGLE(45.0f), ANGLE(30.0f), ANGLE(45.0f)))
};

short SearchCollectFrames[4] = { 180, 100, 153, 83 };
short SearchAnims[4] = { LA_LOOT_CABINET, LA_LOOT_DRAWER, LA_LOOT_SHELF, LA_LOOT_CHEST };
short SearchOffsets[4] = { 160, 96, 160, 112 };

const ObjectCollisionBounds MSBounds =
{
	GameBoundingBox::Zero,
	std::pair(
		EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
		EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f)))
};

int NumRPickups;
short RPickups[16];
Vector3i OldPickupPos;

bool SetInventoryCount(GAME_OBJECT_ID objectID, int count)
{
	if (!TryModifyWeapon(Lara, objectID, count, ModificationType::Set) &&
		!TryModifyingAmmo(Lara, objectID, count, ModificationType::Set) &&
		!TryModifyingKeyItem(Lara, objectID, count, ModificationType::Set) &&
		!TryModifyingConsumable(Lara, objectID, count, ModificationType::Set) &&
		!TryModifyMiscCount(Lara, objectID, count, ModificationType::Set))
	{
		return false;
	}

	return true;
}

void PickedUpObject(GAME_OBJECT_ID objectID, std::optional<int> count)
{
	// See if the items fit into one of these easy groups.
	if (!TryAddingWeapon(Lara, objectID) &&
		!TryAddingAmmo(Lara, objectID, count) &&
		!TryAddingKeyItem(Lara, objectID, count) &&
		!TryAddingConsumable(Lara, objectID, count) &&
		!TryAddMiscItem(Lara, objectID))
	{
		// Item isn't any of the above; do nothing.
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

void RemoveObjectFromInventory(GAME_OBJECT_ID objectID, std::optional<int> count)
{
	// See if the items fit into one of these easy groups.
	if (!TryRemovingWeapon(Lara, objectID) && 
		!TryRemovingAmmo(Lara, objectID, count) && 
		!TryRemovingKeyItem(Lara, objectID, count) && 
		!TryRemovingConsumable(Lara, objectID, count) && 
		!TryRemoveMiscItem(Lara, objectID))
		{
			// Item isn't any of the above; do nothing.
		}
}

void CollectCarriedItems(ItemInfo* item) 
{
	short pickupNumber = item->CarriedItem;
	while (pickupNumber != NO_ITEM)
	{
		auto* pickupItem = &g_Level.Items[pickupNumber];

		g_Hud.PickupSummary.AddDisplayPickup(pickupItem->ObjectNumber, pickupItem->Pose.Position.ToVector3());
		KillItem(pickupNumber);

		pickupNumber = pickupItem->CarriedItem;
	}

	item->CarriedItem = NO_ITEM;
}

void CollectMultiplePickups(int itemNumber)
{
	auto* firstItem = &g_Level.Items[itemNumber];
	GetCollidedObjects(firstItem, LARA_RADIUS, true, CollidedItems, CollidedMeshes, true);

	unsigned int count = 0;
	for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
	{
		auto* currentItem = CollidedItems[i];

		if (currentItem == nullptr)
			currentItem = firstItem;

		if (!Objects[currentItem->ObjectNumber].isPickup)
			continue;

		// HACK: Exclude flares and torches from pickup batches.
		bool hasFlareOrTorch = false;
		if (currentItem->ObjectNumber == ID_FLARE_ITEM ||
			currentItem->ObjectNumber == ID_BURNING_TORCH_ITEM)
		{
			if (count > 0)
				continue;

			hasFlareOrTorch = true;
		}

		count++;

		g_Hud.PickupSummary.AddDisplayPickup(currentItem->ObjectNumber, currentItem->Pose.Position.ToVector3());
		if (currentItem->TriggerFlags & (1 << 8))
		{
			for (int i = 0; i < g_Level.NumItems; i++)
			{
				if (g_Level.Items[i].ObjectNumber == currentItem->ObjectNumber)
					KillItem(i);
			}
		}

		if (currentItem->TriggerFlags & 0xC0)
		{
			currentItem->Status = ITEM_INVISIBLE;
			currentItem->Flags |= TRIGGERED;
			currentItem->ItemFlags[3] = 1;
		}

		//currentItem->Pose.Orientation = prevOrient;
		KillItem(currentItem->Index);

		if (hasFlareOrTorch)
			break;

		if (currentItem == firstItem)
			break;
	}
}

void DoPickup(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	if (lara->InteractedItem == NO_ITEM)
		return;

	short pickupItemNumber = lara->InteractedItem;
	auto* pickupItem = &g_Level.Items[pickupItemNumber];

	if (!Objects[pickupItem->ObjectNumber].isPickup)
		return;

	auto prevOrient = pickupItem->Pose.Orientation;

	if (pickupItem->ObjectNumber == ID_BURNING_TORCH_ITEM)
	{
		g_Hud.PickupSummary.AddDisplayPickup(ID_BURNING_TORCH_ITEM, pickupItem->Pose.Position.ToVector3());
		GetFlameTorch();
		lara->Torch.IsLit = (pickupItem->ItemFlags[3] & 1);

		KillItem(pickupItemNumber);
		pickupItem->Pose.Orientation = prevOrient;
		lara->InteractedItem = NO_ITEM;
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
			lara->Flare.Life = int(pickupItem->Data) & 0x7FFF;
			DrawFlareMeshes(laraItem);
			KillItem(pickupItemNumber);

			pickupItem->Pose.Orientation = prevOrient;
			lara->InteractedItem = NO_ITEM;
			return;
		}
		else if (laraItem->Animation.ActiveState == LS_PICKUP_FLARE)
		{
			lara->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			lara->Control.Weapon.GunType = LaraWeaponType::Flare;
			InitialiseNewWeapon(laraItem);
			lara->Control.HandStatus = HandStatus::Special;
			lara->Flare.Life = int(pickupItem->Data) & 0x7FFF;
			KillItem(pickupItemNumber);
			lara->InteractedItem = NO_ITEM;
			return;
		}
	}
	else
	{
		// Dirty, but it uses the same state.
		if (laraItem->Animation.AnimNumber == LA_UNDERWATER_PICKUP)
		{
			if (g_GameFlow->IsMassPickupEnabled())
			{
				CollectMultiplePickups(lara->InteractedItem);
				lara->InteractedItem = NO_ITEM;
				return;
			}

			g_Hud.PickupSummary.AddDisplayPickup(pickupItem->ObjectNumber, pickupItem->Pose.Position.ToVector3());
			if (!(pickupItem->TriggerFlags & 0xC0))
			{
				KillItem(pickupItemNumber);
			}
			else
			{
				pickupItem->Status = ITEM_INVISIBLE;
				pickupItem->Flags |= TRIGGERED;
				pickupItem->ItemFlags[3] = 1;
			}

			pickupItem->Pose.Orientation = prevOrient;
			lara->InteractedItem = NO_ITEM;
			return;
		}
		else
		{
			if (laraItem->Animation.AnimNumber == LA_CROWBAR_PRY_WALL_SLOW)
			{
				g_Hud.PickupSummary.AddDisplayPickup(ID_CROWBAR_ITEM, pickupItem->Pose.Position.ToVector3());
				lara->Inventory.HasCrowbar = true;
				KillItem(pickupItemNumber);
			}
			else if (laraItem->Animation.ActiveState == LS_PICKUP ||
					 laraItem->Animation.ActiveState == LS_PICKUP_FROM_CHEST ||
					 laraItem->Animation.ActiveState == LS_HOLE)
			{
				if (g_GameFlow->IsMassPickupEnabled())
				{
					CollectMultiplePickups(lara->InteractedItem);
					lara->InteractedItem = NO_ITEM;
					return;
				}

				g_Hud.PickupSummary.AddDisplayPickup(pickupItem->ObjectNumber, pickupItem->Pose.Position.ToVector3());
				if (pickupItem->TriggerFlags & (1 << 8))
				{
					for (int i = 0; i < g_Level.NumItems; i++)
					{
						if (g_Level.Items[i].ObjectNumber == pickupItem->ObjectNumber)
							KillItem(i);
					}
				}

				if (pickupItem->TriggerFlags & 0xC0)
				{
					pickupItem->Status = ITEM_INVISIBLE;
					pickupItem->Flags |= TRIGGERED;
					pickupItem->ItemFlags[3] = 1;
				}

				pickupItem->Pose.Orientation = prevOrient;
				KillItem(pickupItemNumber);
				lara->InteractedItem = NO_ITEM;
				return;
			}
		}
	}

	lara->InteractedItem = NO_ITEM;
}

void PickupCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];

	auto prevOrient = item->Pose.Orientation;

	if (item->Status == ITEM_INVISIBLE)
		return;

	short triggerFlags = item->TriggerFlags & 0x3F;
	if (triggerFlags == 5 || triggerFlags == 10)
		return;

	auto lara = GetLaraInfo(laraItem);

	if (item->ObjectNumber == ID_FLARE_ITEM && lara->Control.Weapon.GunType == LaraWeaponType::Flare)
		return;

	item->Pose.Orientation.y = laraItem->Pose.Orientation.y;
	item->Pose.Orientation.z = 0.0f;

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
				TestLaraPosition(PickUpBoundsUW, item, laraItem) || lara->Control.IsMoving && lara->InteractedItem == itemNumber)
			{
				if (TestLaraPosition(PickUpBoundsUW, item, laraItem))
				{
					if (MoveLaraPosition(PickUpPositionUW, item, laraItem))
					{
						if (item->ObjectNumber == ID_FLARE_ITEM)
						{
							laraItem->Animation.AnimNumber = LA_UNDERWATER_PICKUP_FLARE;
							laraItem->Animation.ActiveState = LS_PICKUP_FLARE;
							laraItem->Animation.Velocity.y = 0;
						}
						else
						{
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
							lara->Control.IsMoving = false;
							lara->Control.HandStatus = HandStatus::Free;
						}
					}
				}

				item->Pose.Orientation = prevOrient;
				return;
			}
		}

		item->Pose.Orientation = prevOrient;
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
					item->Pose.Orientation = prevOrient;
					return;
				}
				else
				{
					item->Pose.Orientation = prevOrient;
					return;
				}
			}
		}

		if (lara->InteractedItem != itemNumber)
		{
			item->Pose.Orientation = prevOrient;
			return;
		}
	}
	
	bool flag = false;
	GameBoundingBox* plinth = nullptr;
	item->Pose.Orientation.x = 0;
	switch (triggerFlags)
	{
	// Pick up from hole in wall.
	case 1:
		if (lara->Control.IsLow || !TestLaraPosition(HiddenPickUpBounds, item, laraItem))
		{
			if (lara->Control.IsMoving)
			{
				if (lara->InteractedItem == itemNumber)
				{
					lara->Control.IsMoving = false;
					lara->Control.HandStatus = HandStatus::Free;
				}
			}

			item->Pose.Orientation = prevOrient;
			return;
		}
		else if (MoveLaraPosition(HiddenPickUpPosition, item, laraItem))
		{
			laraItem->Animation.AnimNumber = LA_HOLESWITCH_ACTIVATE;
			laraItem->Animation.ActiveState = LS_HOLE;
			flag = true;
		}

		lara->InteractedItem = itemNumber;
		break;

	// Pick up with crowbar.
	case 2:
		item->Pose.Orientation.y = prevOrient.y;

		if (lara->Control.IsLow || !TestLaraPosition(CrowbarPickUpBounds, item, laraItem))
		{
			if (!lara->Control.IsMoving)
			{
				item->Pose.Orientation = prevOrient;
				return;
			}

			if (lara->InteractedItem == itemNumber)
			{
				lara->Control.IsMoving = false;
				lara->Control.HandStatus = HandStatus::Free;
			}

			item->Pose.Orientation = prevOrient;
			return;
		}
		if (!lara->Control.IsMoving)
		{
			if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
			{
				if (g_Gui.IsObjectInInventory(ID_CROWBAR_ITEM))
					g_Gui.SetEnterInventory(ID_CROWBAR_ITEM);

				item->Pose.Orientation = prevOrient;
				return;
			}

			if (g_Gui.GetInventoryItemChosen() != ID_CROWBAR_ITEM)
			{
				item->Pose.Orientation = prevOrient;
				return;
			}

			g_Gui.SetInventoryItemChosen(NO_ITEM);
		}

		if (MoveLaraPosition(CrowbarPickUpPosition, item, laraItem))
		{
			laraItem->Animation.AnimNumber = LA_CROWBAR_PRY_WALL_FAST;
			laraItem->Animation.ActiveState = LS_PICKUP;
			item->Status = ITEM_ACTIVE;
			AddActiveItem(itemNumber);
			AnimateItem(item);
			flag = true;
		}

		lara->InteractedItem = itemNumber;
		break;

	// Pick up from plinth.
	case 3:
	case 4:
	case 7:
	case 8:
		plinth = FindPlinth(item);

		if (!plinth)
		{
			item->Pose.Orientation = prevOrient;
			return;
		}

		PlinthPickUpBounds.BoundingBox.X1 = plinth->X1;
		PlinthPickUpBounds.BoundingBox.X2 = plinth->X2;
		PlinthPickUpBounds.BoundingBox.Y2 = laraItem->Pose.Position.y - item->Pose.Position.y + 100;
		PlinthPickUpBounds.BoundingBox.Z2 = plinth->Z2 + 320;
		PlinthPickUpPosition.z = -200 - plinth->Z2;

		// HACK: Until we refactor a way plinth collision is detected, this must be here
		// to prevent false positives with two stacked plinths -- Lwmte, 16.06.22
		if (abs(laraItem->Pose.Position.y - item->Pose.Position.y) > CLICK(4))
			break;

		// Also prevent picking up from plinth if item is below Lara
		if (laraItem->Pose.Position.y < item->Pose.Position.y)
			break;

		if (TestLaraPosition(PlinthPickUpBounds, item, laraItem) && !lara->Control.IsLow)
		{
			if (item->Pose.Position.y == laraItem->Pose.Position.y)
				PlinthPickUpPosition.y = 0;
			else
				PlinthPickUpPosition.y = laraItem->Pose.Position.y - item->Pose.Position.y;

			if (MoveLaraPosition(PlinthPickUpPosition, item, laraItem))
			{
				if (triggerFlags == 3 || triggerFlags == 7)
				{
					SetAnimation(laraItem, LA_PICKUP_PEDESTAL_HIGH);
				}
				else
				{
					SetAnimation(laraItem, LA_PICKUP_PEDESTAL_LOW);
				}

				flag = true;
			}

			lara->InteractedItem = itemNumber;
			break;
		}

		if (!lara->Control.IsMoving)
		{
			item->Pose.Orientation = prevOrient;
			return;
		}
		
		if (lara->InteractedItem == itemNumber)
		{
			lara->Control.IsMoving = false;
			lara->Control.HandStatus = HandStatus::Free;
		}

		item->Pose.Orientation = prevOrient;
		return;

	// Pick up and convert object to a crowbar. Used in TR5 submarine level.
	case 9:
		item->Pose.Orientation.y = prevOrient.y;

		if (!TestLaraPosition(JobyCrowPickUpBounds, item, laraItem))
		{
			item->Pose.Orientation = prevOrient;
			return;
		}

		if (MoveLaraPosition(JobyCrowPickUpPosition, item, laraItem))
		{
			SetAnimation(laraItem, LA_CROWBAR_PRY_WALL_SLOW);
			item->Status = ITEM_ACTIVE;
			AddActiveItem(itemNumber);
			flag = true;
		}

		lara->InteractedItem = itemNumber;
		break;

	// Pick up from ground.
	default:
		if (!TestLaraPosition(PickUpBounds, item, laraItem))
		{
			if (!lara->Control.IsMoving)
			{
				item->Pose.Orientation = prevOrient;
				return;
			}
			
			if (lara->InteractedItem == itemNumber)
			{
				lara->Control.IsMoving = false;
				lara->Control.HandStatus = HandStatus::Free;
			}

			item->Pose.Orientation = prevOrient;
			return;
		}

		PickUpPosition.y = laraItem->Pose.Position.y - item->Pose.Position.y;

		if (laraItem->Animation.ActiveState == LS_CROUCH_IDLE)
		{
			if (item->ObjectNumber == ID_BURNING_TORCH_ITEM)
				break;

			if (!AlignLaraPosition(PickUpPosition, item, laraItem))
				break;

			if (item->ObjectNumber == ID_FLARE_ITEM)
			{
				laraItem->Animation.AnimNumber = LA_CROUCH_PICKUP_FLARE;
				laraItem->Animation.ActiveState = LS_PICKUP_FLARE;
				lara->InteractedItem = itemNumber;
				flag = true;
				break;
			}

			laraItem->Animation.TargetState = LS_PICKUP;
		}
		else
		{
			if (laraItem->Animation.ActiveState == LS_CRAWL_IDLE)
			{
				if (item->ObjectNumber == ID_BURNING_TORCH_ITEM)
					break;

				if (!AlignLaraPosition(PickUpPosition, item, laraItem))
					break;

				if (item->ObjectNumber == ID_FLARE_ITEM)
				{
					laraItem->Animation.TargetState = LS_CROUCH_IDLE;
				}
				else
				{
					laraItem->Animation.TargetState = LS_PICKUP;
				}
				lara->InteractedItem = itemNumber;
				break;
			}
			else
			{
				if (!MoveLaraPosition(PickUpPosition, item, laraItem))
				{
					lara->InteractedItem = itemNumber;
					break;
				}

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
					// HACK: Because of MoveLaraPosition(), we can't properly dispatch. Must be fixed later.
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

	item->Pose.Orientation = prevOrient;
}

void RegeneratePickups()
{
	auto* lara = GetLaraInfo(LaraItem);

	for (int i = 0; i < NumRPickups; i++)
	{
		auto& item = g_Level.Items[RPickups[i]];

		if (item.Status == ITEM_INVISIBLE)
		{
			short ammo = 0;
			switch (item.ObjectNumber)
			{
			case ID_CROSSBOW_AMMO1_ITEM:
				ammo = lara->Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_CROSSBOW_AMMO2_ITEM:
				ammo = lara->Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo2];
				break;

			case ID_CROSSBOW_AMMO3_ITEM:
				ammo = lara->Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo3];
				break;

			case ID_GRENADE_AMMO1_ITEM:
				ammo = lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_GRENADE_AMMO2_ITEM:
				ammo = lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo2];
				break;

			case ID_GRENADE_AMMO3_ITEM:
				ammo = lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo3];
				break;

			case ID_HK_AMMO_ITEM:
				ammo = lara->Weapons[(int)LaraWeaponType::HK].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_UZI_AMMO_ITEM:
				ammo = lara->Weapons[(int)LaraWeaponType::Uzi].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_HARPOON_AMMO_ITEM:
				ammo = lara->Weapons[(int)LaraWeaponType::HarpoonGun].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_ROCKET_LAUNCHER_AMMO_ITEM:
				ammo = lara->Weapons[(int)LaraWeaponType::RocketLauncher].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_REVOLVER_AMMO_ITEM:
				ammo = lara->Weapons[(int)LaraWeaponType::Revolver].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_SHOTGUN_AMMO1_ITEM:
				ammo = lara->Weapons[(int)LaraWeaponType::Shotgun].Ammo[(int)WeaponAmmoType::Ammo1];
				break;

			case ID_SHOTGUN_AMMO2_ITEM:
				ammo = lara->Weapons[(int)LaraWeaponType::Shotgun].Ammo[(int)WeaponAmmoType::Ammo2];
				break;
			}

			if (ammo == 0)
				item.Status = ITEM_NOT_ACTIVE;
		}
	}
}

void DropPickups(ItemInfo* item)
{
	ItemInfo* pickup = nullptr;

	auto bounds = GameBoundingBox(item);
	auto extents = bounds.GetExtents();
	auto origin = Geometry::TranslatePoint(item->Pose.Position.ToVector3(), item->Pose.Orientation, bounds.GetCenter());
	auto yPos = GetCollision(item).Position.Floor;

	origin.y = yPos; // Initialize drop origin Y point as floor height at centerpoint, in case all corner tests fail.

	// Also collect objects which are around.
	bool collidedWithObjects = GetCollidedObjects(item, extents.Length(), true, CollidedItems, CollidedMeshes, true);

	short startAngle = ANGLE(Random::GenerateInt(0, 3) * 90); // Randomize start corner.

	// Iterate through 4 corners and find best-fitting position, which is not inside a wall, not on a slope
	// and also does not significantly differ in height to an object centerpoint height.
	// If all corner tests will fail, a pickup will be spawned at bounding box centerpoint, as it does in tomb4.

	for (int corner = 0; corner < 4; corner++)
	{
		auto angle = item->Pose.Orientation;
		angle.y += startAngle + corner * ANGLE(90);

		// At first, do an inside-wall test at an extended extent point to make sure player can correctly align.
		auto candidatePos = Geometry::TranslatePoint(origin, angle, extents * 1.2f);
		candidatePos.y = yPos;
		auto collPoint = GetCollision(candidatePos.x, candidatePos.y, candidatePos.z, item->RoomNumber);

		// If position is inside a wall or on a slope, don't use it.
		if (collPoint.Position.Floor == NO_HEIGHT || collPoint.Position.FloorSlope)
			continue;

		// Remember floor position for a tested point.
		int candidateYPos = collPoint.Position.Floor;

		// Now repeat the same test for original extent point to make sure it's also valid.
		candidatePos = Geometry::TranslatePoint(origin, angle, extents);
		candidatePos.y = yPos;
		collPoint = GetCollision(candidatePos.x, candidatePos.y, candidatePos.z, item->RoomNumber);

		// If position is inside a wall or on a slope, don't use it.
		if (collPoint.Position.Floor == NO_HEIGHT || collPoint.Position.FloorSlope)
			continue;

		// If position is not in the same room, don't use it.
		if (collPoint.RoomNumber != item->RoomNumber)
			continue;

		// Setup a dummy sphere with 1-click diameter for item and static mesh collision tests.
		auto sphere = BoundingSphere(candidatePos, CLICK(0.5f));
		bool collidedWithObject = false;

		// Iterate through all found items and statics around, and determine if dummy sphere
		// intersects any of those. If so, try other corner.

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			auto* currentItem = CollidedItems[i];
			if (!currentItem)
				break;

			if (GameBoundingBox(currentItem).ToBoundingOrientedBox(currentItem->Pose).Intersects(sphere))
			{
				collidedWithObject = true;
				break;
			}
		}

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			auto* currentMesh = CollidedMeshes[i];
			if (!currentMesh)
				break;

			if (StaticObjects[currentMesh->staticNumber].collisionBox.ToBoundingOrientedBox(currentMesh->pos).Intersects(sphere))
			{
				collidedWithObject = true;
				break;
			}
		}

		if (collidedWithObject)
			continue;

		// Finally, do height difference tests. If difference is more than one and a half click,
		// most likely it's hanging in the air or submerged, so bypass the corner.
		if (abs(collPoint.Position.Floor - yPos) > CLICK(1.5f))
			continue;

		// If height difference between extent points is more than one click, it means it landed
		// on a step, so let's search for other position.
		if (abs(collPoint.Position.Floor - candidateYPos) >= CLICK(1.0f))
			continue;

		origin = candidatePos;
		origin.y = collPoint.Position.Floor;
		break;
	}

	for (short pickupNumber = item->CarriedItem; pickupNumber != NO_ITEM; pickupNumber = pickup->CarriedItem)
	{
		pickup = &g_Level.Items[pickupNumber];
		pickup->Pose.Position = origin;
		pickup->Pose.Position.y -= GameBoundingBox(pickup).Y2;

		pickup->Pose.Orientation.y = ANGLE(Random::GenerateInt(0, 359)); // Randomize pickup rotation.

		// HACK: Pickup is not moved to a right room at this moment, it will only update next game loop.
		// Therefore, we need to temporarily inject actual room number, so AlignEntityToSurface succeeds.

		pickup->RoomNumber = item->RoomNumber;
		AlignEntityToSurface(pickup, Vector2(Objects[pickup->ObjectNumber].radius));
		pickup->RoomNumber = -1;
		pickup->Flags |= 32;

		ItemNewRoom(pickupNumber, item->RoomNumber);
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
		item->Animation.Velocity.y += 6.0f;
		item->Pose.Position.y += item->Animation.Velocity.y;
		
		roomNumber = item->RoomNumber;
		GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);

		if (item->Pose.Position.y > item->ItemFlags[0])
		{
			item->Pose.Position.y = item->ItemFlags[0];
			if (item->Animation.Velocity.y <= 64.0f)
				item->TriggerFlags &= 0xC0;
			else
				item->Animation.Velocity.y = -item->Animation.Velocity.y / 4;
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
		//sub_401014(itemNumber);
		break;
	}
}

GameBoundingBox* FindPlinth(ItemInfo* item)
{
	auto* room = &g_Level.Rooms[item->RoomNumber];
	
	for (int i = 0; i < room->mesh.size(); i++)
	{
		auto* mesh = &room->mesh[i];

		if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
			continue;

		if (item->Pose.Position.x != mesh->pos.Position.x || item->Pose.Position.z != mesh->pos.Position.z)
			continue;

		auto* frame = (GameBoundingBox*)GetBestFrame(item);
		auto& bBox = GetBoundsAccurate(*mesh, false);

		if (frame->X1 <= bBox.X2 && frame->X2 >= bBox.X1 &&
			frame->Z1 <= bBox.Z2 && frame->Z2 >= bBox.Z1 &&
			(bBox.X1 || bBox.X2))
		{
			return &bBox;
		}
	}

	if (room->itemNumber == NO_ITEM)
		return nullptr;

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
		return nullptr;
	else
		return (GameBoundingBox*)GetBestFrame(&g_Level.Items[itemNumber]);
}

void InitialisePickup(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	auto bounds = GameBoundingBox(item);

	short triggerFlags = item->TriggerFlags & 0x3F;
	if (triggerFlags == 5)
	{
		item->ItemFlags[0] = item->Pose.Position.y - bounds.Y2;
		item->Status = ITEM_INVISIBLE;
	}
	else
	{
		if (triggerFlags == 0 ||
			triggerFlags == 3 ||
			triggerFlags == 4 ||
			triggerFlags == 7 ||
			triggerFlags == 8 ||
			triggerFlags == 11)
		{
			item->Pose.Position.y -= bounds.Y2;

			if (triggerFlags == 0)
			{
				// Automatically align pickups to the floor surface.
				auto pointColl = GetCollision(item);
				int bridgeItemNumber = pointColl.Block->GetInsideBridgeItemNumber(
					item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z,
					true, true);

				if (bridgeItemNumber != NO_ITEM)
				{
					// If pickup is within bridge item, most likely it means it is
					// below pushable or raising block, so ignore its collision.
					pointColl.Block->RemoveBridge(bridgeItemNumber);
					pointColl = GetCollision(item);
					pointColl.Block->AddBridge(bridgeItemNumber);
				}

				item->Pose.Position.y = pointColl.Position.Floor - bounds.Y2;
				AlignEntityToSurface(item, Vector2(Objects[item->ObjectNumber].radius));
			}
		}
		
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
		item->SetMeshSwapFlags(ALL_JOINT_BITS);
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
				if (item->Pose.Position == item2->Pose.Position)
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
		item->Status = ITEM_ACTIVE;
		item->Flags |= IFLAG_ACTIVATION_MASK;
	}
}

void SearchObjectCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* lara = GetLaraInfo(laraItem);

	int objectNumber = (item->ObjectNumber - ID_SEARCH_OBJECT1);

	if ((TrInput & IN_ACTION &&
		laraItem->Animation.ActiveState == LS_IDLE &&
		laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
		lara->Control.HandStatus == HandStatus::Free &&
		((item->Status == ITEM_NOT_ACTIVE && item->ObjectNumber != ID_SEARCH_OBJECT4) || !item->ItemFlags[0])) ||
		(lara->Control.IsMoving && lara->InteractedItem == itemNumber))
	{
		auto bounds = GameBoundingBox(item);
		if (item->ObjectNumber != ID_SEARCH_OBJECT1)
		{
			SOBounds.BoundingBox.X1 = bounds.X1 - CLICK(0.5f);
			SOBounds.BoundingBox.X2 = bounds.X2 + CLICK(0.5f);
		}
		else
		{
			SOBounds.BoundingBox.X1 = bounds.X1 + CLICK(0.25f);
			SOBounds.BoundingBox.X2 = bounds.X2 - CLICK(0.25f);
		}

		SOBounds.BoundingBox.Z1 = bounds.Z1 - 200;
		SOBounds.BoundingBox.Z2 = bounds.Z2 + 200;
		SOPos.z = bounds.Z1 - SearchOffsets[objectNumber];

		if (TestLaraPosition(SOBounds, item, laraItem))
		{
			if (MoveLaraPosition(SOPos, item, laraItem))
			{
				ResetLaraFlex(laraItem);
				laraItem->Animation.AnimNumber = SearchAnims[objectNumber];
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.ActiveState = LS_MISC_CONTROL;
				lara->Control.IsMoving = false;
				lara->Control.HandStatus = HandStatus::Busy;

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
				lara->InteractedItem = itemNumber;
		}
		else if (lara->Control.IsMoving && lara->InteractedItem ==  itemNumber)
		{
			lara->Control.IsMoving = false;
			lara->Control.HandStatus = HandStatus::Free;
		}
	}
	else if (laraItem->Animation.ActiveState != LS_MISC_CONTROL)
		ObjectCollision(itemNumber, laraItem, coll);
}

void SearchObjectControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	int objectNumber = (item->ObjectNumber - ID_SEARCH_OBJECT1);

	if (item->ObjectNumber != ID_SEARCH_OBJECT4 || item->ItemFlags[0] == 1)
		AnimateItem(item);

	int frameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
	if (item->ObjectNumber == ID_SEARCH_OBJECT1)
	{
		if (frameNumber > 0)
		{
			item->SetMeshSwapFlags(NO_JOINT_BITS);
			item->MeshBits = ALL_JOINT_BITS;
		}
		else
		{
			item->SetMeshSwapFlags(ALL_JOINT_BITS);
			item->MeshBits = 7;
		}
	}
	else if (item->ObjectNumber == ID_SEARCH_OBJECT2)
	{
		if (frameNumber == 18)
		{
			item->MeshBits = 1;
		}
		else if (frameNumber == 172)
		{
			item->MeshBits = 2;
		}
	}
	else if (item->ObjectNumber == ID_SEARCH_OBJECT4)
	{
		item->MeshBits = FlipStats[0] != 0 ? 48 : 9;

		if (frameNumber >= 45 && frameNumber <= 131)
			item->MeshBits |= FlipStats[0] != 0 ? 4 : 2;
			
		if (item->ItemFlags[1] != -1)
		{
			auto* item2 = &g_Level.Items[item->ItemFlags[1]];

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
				auto* item2 = &g_Level.Items[item->ItemFlags[1]];

				if (Objects[item2->ObjectNumber].isPickup)
				{
					g_Hud.PickupSummary.AddDisplayPickup(item2->ObjectNumber, item2->Pose.Position.ToVector3());
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
			item->Status = ITEM_ACTIVE;
			item->ItemFlags[0] = 0;
		}
		else
		{
			RemoveActiveItem(itemNumber);
			item->Status = ITEM_NOT_ACTIVE;
		}
	}
}

bool UseSpecialItem(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	int flag = 0;
	int itemIDToUse = g_Gui.GetInventoryItemChosen();

	if (itemIDToUse != NO_ITEM &&
		laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
		lara->Control.HandStatus == HandStatus::Free)
	{
		if (itemIDToUse >= ID_WATERSKIN1_EMPTY && itemIDToUse <= ID_WATERSKIN2_5)
		{
			laraItem->ItemFlags[2] = ID_LARA_WATER_MESH;
			flag = 1;
		}
		else if (itemIDToUse == ID_CLOCKWORK_BEETLE)
		{
			flag = 4;
			laraItem->Animation.AnimNumber = LA_MECHANICAL_BEETLE_USE;
			UseClockworkBeetle(1);
		}

		if (flag == 1)
		{
			if (itemIDToUse != ID_WATERSKIN1_3 && itemIDToUse != ID_WATERSKIN2_5 && (lara->WaterSurfaceDist < -SHALLOW_WATER_DEPTH))
			{
				if (itemIDToUse < ID_WATERSKIN1_3)
					lara->Inventory.SmallWaterskin = 4;
				else
					lara->Inventory.BigWaterskin = 6;

				flag = 1;
			}
			else if (itemIDToUse != ID_WATERSKIN1_EMPTY && itemIDToUse != ID_WATERSKIN2_EMPTY)
			{
				if (itemIDToUse <= ID_WATERSKIN1_3)
				{
					laraItem->ItemFlags[3] = lara->Inventory.SmallWaterskin - 1;
					lara->Inventory.SmallWaterskin = 1;
				}
				else
				{
					laraItem->ItemFlags[3] = lara->Inventory.BigWaterskin - 1;
					lara->Inventory.BigWaterskin = 1;
				}

				flag = 2;
			}
			else
				return false;
		}

		if (flag)
		{
			if (flag == 1)
				SetAnimation(laraItem, LA_WATERSKIN_FILL);
			else if (flag == 2)
				SetAnimation(laraItem, LA_WATERSKIN_POUR_LOW);

			lara->Control.HandStatus = HandStatus::Busy;
			g_Gui.SetInventoryItemChosen(NO_ITEM);
			return true;
		}
	}

	return false;
}
