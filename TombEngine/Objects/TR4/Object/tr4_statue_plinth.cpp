#include "framework.h"
#include "tr4_statue_plinth.h"

#include "Specific/level.h"
#include "Game/Collision/Sphere.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Gui.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/Input/Input.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/items.h"
#include "Game/pickup/pickup.h"
#include "Game/Setup.h"
#include "Game/Debug/Debug.h"

using namespace TEN::Collision::Sphere;
using namespace TEN::Entities::Switches;
using namespace TEN::Gui;
using namespace TEN::Input;
using namespace TEN::Debug;

namespace TEN::Entities::TR4
{
	const auto KeyHolePosition = Vector3i(0, 0, -390);
	const ObjectCollisionBounds KeyHoleBounds =
	{
		GameBoundingBox(
			-BLOCK(0.5f), BLOCK(0.5f),
			-BLOCK(0.5f), BLOCK(0.5f),
			-BLOCK(0.5f), 0),
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f)))
	};

	void InitialiseStatuePlinth(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		// Hide the mesh 1 
		//item->MeshBits = 1;
	}

	//TODO frame hardcore or check how puzzle items do
	//TODO Switch actions - Will not work as the object will rotate to face lara and break the illusion.

	void StatuePlinthCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* keyHoleItem = &g_Level.Items[itemNumber];
		auto* player = GetLaraInfo(laraItem);

		short* triggerIndexPtr = GetTriggerIndex(keyHoleItem);
		short y_rot = keyHoleItem->Pose.Orientation.y;

		GAME_OBJECT_ID keyItem;

		//There are only 16 puzzle items. -1 is added to keep the OCB aligned with Puzzle Item number.
		if (keyHoleItem->TriggerFlags < 17 && keyHoleItem->TriggerFlags > 0)
		{
			keyItem = GAME_OBJECT_ID(keyHoleItem->TriggerFlags + ID_PUZZLE_ITEM1 - 1);
		}
		else
		{
			keyItem = ID_PUZZLE_ITEM1;
		}

		if (triggerIndexPtr == nullptr)
			return;

		short triggerType = (*(triggerIndexPtr++) >> 8) & TRIGGER_BITS;

		bool isActionReady = (IsHeld(In::Action) || g_Gui.GetInventoryItemChosen() != NO_VALUE);

		bool isPlayerAvailable = (player->Control.Look.OpticRange == 0 &&
			laraItem->Animation.ActiveState == LS_IDLE &&
			laraItem->Animation.AnimNumber == LA_STAND_IDLE);

		bool actionActive = player->Control.IsMoving && player->Context.InteractedItem == itemNumber;

		if (actionActive || (isActionReady && isPlayerAvailable))
		{	
			int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);
			keyHoleItem->DisableInterpolation = true;
			switch (quadrant)
			{
			case NORTH:
				keyHoleItem->Pose.Orientation.y = ANGLE(0.0f);
				break;

			case EAST:
				keyHoleItem->Pose.Orientation.y = ANGLE(90.0f);
				break;

			case SOUTH:
				keyHoleItem->Pose.Orientation.y = ANGLE(180.0f);
				break;

			case WEST:
				keyHoleItem->Pose.Orientation.y = ANGLE(270.0f);
				break;

			default:
				break;
			}
			
			if (TestLaraPosition(KeyHoleBounds, keyHoleItem, laraItem))
			{
				if (!player->Control.IsMoving)
				{
					if (keyHoleItem->Status != ITEM_NOT_ACTIVE && triggerType != TRIGGER_TYPES::SWITCH)
						keyHoleItem->Pose.Orientation.y = y_rot;
						return;

					if (g_Gui.GetInventoryItemChosen() == NO_VALUE)
					{
						if (g_Gui.IsObjectInInventory(keyItem))
							g_Gui.SetEnterInventory(keyItem);

						return;
					}

					if (g_Gui.GetInventoryItemChosen() != keyItem)
						return;

					player->Context.InteractedItem = itemNumber;
				}

				if (player->Context.InteractedItem != itemNumber)
					return;

				if (MoveLaraPosition(KeyHolePosition, keyHoleItem, laraItem))
				{
					keyHoleItem->Pose.Orientation.y = y_rot;

					if (triggerType = TRIGGER_TYPES::SWITCH)
						keyHoleItem->ItemFlags[1] = true;

					//int animNumber = abs(keyHoleItem->TriggerFlags);
					//if (keyHoleItem->TriggerFlags <= 0)
					//{
					//	auto objectID = GAME_OBJECT_ID(keyItem);
						RemoveObjectFromInventory(keyItem, 1);
					//}

					//if (keyHoleItem->TriggerFlags == 0)
					//{
						laraItem->Animation.AnimNumber = LA_PICKUP_PEDESTAL_HIGH;
					//}
					//else
					//{
					//	laraItem->Animation.AnimNumber = animNumber;
					//}

					laraItem->Animation.ActiveState = LS_INSERT_KEY;
					laraItem->Animation.FrameNumber = GetAnimData(laraItem).frameBase;
					player->Control.IsMoving = false;
					ResetPlayerFlex(laraItem); 32;
					player->Control.HandStatus = HandStatus::Busy;
					keyHoleItem->Flags |= TRIGGERED;
					keyHoleItem->Status = ITEM_ACTIVE;
					keyHoleItem->Model.MeshIndex[1] = Objects[keyItem].meshIndex + 0;
					keyHoleItem->Model.Mutators[1].Offset = Vector3(0.0f, -384.0f, 0.0f);
					keyHoleItem->MeshBits = 255;
					
				}

				g_Gui.SetInventoryItemChosen(NO_VALUE);
				return;
			}

			if (player->Control.IsMoving && player->Context.InteractedItem == itemNumber)
			{
				player->Control.IsMoving = false;
				player->Control.HandStatus = HandStatus::Free;
			}
		}
		else
		{
			ObjectCollision(itemNumber, laraItem, coll);

		}
		keyHoleItem->Pose.Orientation.y = y_rot;
		return;
	}
}

/*auto* item = &g_Level.Items[itemNumber];
		auto* player = GetLaraInfo(laraItem);
		short y_rot;

		bool isActionReady = (IsHeld(In::Action) || g_Gui.GetInventoryItemChosen() != NO_VALUE);

		bool isPlayerAvailable = (player->Control.Look.OpticRange == 0 &&
			laraItem->Animation.ActiveState == LS_IDLE &&
			laraItem->Animation.AnimNumber == LA_STAND_IDLE);

		bool actionActive = player->Control.IsMoving && player->Context.InteractedItem == itemNumber;

		bool itemStatus = (item->Status == ITEM_NOT_ACTIVE);

		if (actionActive || (isActionReady && isPlayerAvailable && itemStatus))
		{
			if (!item->ItemFlags[1])
			{
				y_rot = item->Pose.Orientation.y;
				item->Pose.Orientation.y = laraItem->Pose.Orientation.y;

				if (TestLaraPosition(StatuePlinthBounds, item, laraItem))
				{
					if (g_Gui.GetInventoryItemChosen() == NO_VALUE)
					{
						if (g_Gui.IsObjectInInventory(ID_PUZZLE_ITEM5))

							g_Gui.SetEnterInventory(ID_PUZZLE_ITEM5);
						item->ItemFlags[1] = 1;

						return;
					}
				}

				item->Pose.Orientation.y = y_rot;
				return;
			}

			if (g_Gui.IsObjectInInventory(ID_PUZZLE_ITEM5))
			{
				laraItem->Animation.AnimNumber = LA_PICKUP_PEDESTAL_HIGH;
				laraItem->Animation.ActiveState = LS_INSERT_KEY;
				laraItem->Animation.FrameNumber = GetAnimData(laraItem).frameBase;
				player->Control.IsMoving = false;
				ResetPlayerFlex(laraItem);
				player->Control.HandStatus = HandStatus::Busy;
				g_Gui.SetInventoryItemChosen(NO_VALUE);
				return;
			}
		}
		else if (item->ItemFlags[1])
		{
			if (g_Gui.IsObjectInInventory(ID_PUZZLE_ITEM5))
			{
				laraItem->Animation.AnimNumber = LA_PICKUP_PEDESTAL_HIGH;
				laraItem->Animation.ActiveState = LS_INSERT_KEY;
				laraItem->Animation.FrameNumber = GetAnimData(laraItem).frameBase;
				player->Control.IsMoving = false;
				ResetPlayerFlex(laraItem);
				player->Control.HandStatus = HandStatus::Busy;
				g_Gui.SetInventoryItemChosen(NO_VALUE);
				return;
			}
		}

		if (laraItem->Animation.AnimNumber == LA_PICKUP_PEDESTAL_HIGH && laraItem->Animation.FrameNumber == GetAnimData(laraItem).frameBase + 45)
		{
			item->MeshBits = 255;
			item->Flags |= TRIGGERED;
			item->Status = ITEM_ACTIVE;

			auto objectID = GAME_OBJECT_ID(ID_PUZZLE_ITEM5);
			RemoveObjectFromInventory(objectID, 1);
		}
		else
		{
			item->ItemFlags[1] = 0;
			ObjectCollision(itemNumber, laraItem, coll);
		}
	}*/


/*		auto* keyHoleItem = &g_Level.Items[itemNumber];
		auto* player = GetLaraInfo(laraItem);

		short* triggerIndexPtr = GetTriggerIndex(keyHoleItem);

		if (triggerIndexPtr == nullptr)
			return;

		short triggerType = (*(triggerIndexPtr++) >> 8) & TRIGGER_BITS;

		bool isActionReady = (IsHeld(In::Action) || g_Gui.GetInventoryItemChosen() != NO_VALUE);

		bool isPlayerAvailable = (player->Control.Look.OpticRange == 0 &&
			laraItem->Animation.ActiveState == LS_IDLE &&
			laraItem->Animation.AnimNumber == LA_STAND_IDLE);

		bool actionActive = player->Control.IsMoving && player->Context.InteractedItem == itemNumber;

		bool inPosition = false;

		if (actionActive || (isActionReady && isPlayerAvailable))
		{
			if (TestLaraPosition(PlinthBoundsA, keyHoleItem, laraItem))
			{
				inPosition = true;
			}
			else if (TestLaraPosition(PlinthBoundsB, keyHoleItem, laraItem))
			{
				inPosition = true;
			}
			else if (TestLaraPosition(PlinthBoundsC, keyHoleItem, laraItem))
			{
				inPosition = true;
			}
			else if (TestLaraPosition(PlinthBoundsD, keyHoleItem, laraItem))
			{
				inPosition = true;
			}
			else
			{
				inPosition = false;
			}

			if (inPosition)
			{
				TENLog("TestLara complete", LogLevel::Error, LogConfig::All, false);
				if (!player->Control.IsMoving)
				{
					if (keyHoleItem->Status != ITEM_NOT_ACTIVE && triggerType != TRIGGER_TYPES::SWITCH)
						return;

					if (g_Gui.GetInventoryItemChosen() == NO_VALUE)
					{
						if (g_Gui.IsObjectInInventory(ID_PUZZLE_ITEM5))
							g_Gui.SetEnterInventory(ID_PUZZLE_ITEM5);

						return;
					}

					if (g_Gui.GetInventoryItemChosen() != ID_PUZZLE_ITEM5)
						return;

					player->Context.InteractedItem = itemNumber;
				}

				if (player->Context.InteractedItem != itemNumber)
					return;

				if (MoveLaraPosition(KeyHolePosition, keyHoleItem, laraItem))
				{
					//if (triggerType = TRIGGER_TYPES::SWITCH)
					//	keyHoleItem->ItemFlags[1] = true;

					//int animNumber = abs(keyHoleItem->TriggerFlags);
					if (keyHoleItem->TriggerFlags <= 0)
					{
						auto objectID = GAME_OBJECT_ID(ID_PUZZLE_ITEM5);
						RemoveObjectFromInventory(objectID, 1);
					}

					//if (keyHoleItem->TriggerFlags == 0)
					//{
					laraItem->Animation.AnimNumber = LA_PICKUP_PEDESTAL_HIGH;
					//}
					//else
					//{
					//	laraItem->Animation.AnimNumber = animNumber;
					//}

					laraItem->Animation.ActiveState = LS_INSERT_KEY;
					laraItem->Animation.FrameNumber = GetAnimData(laraItem).frameBase;
					player->Control.IsMoving = false;
					ResetPlayerFlex(laraItem);
					player->Control.HandStatus = HandStatus::Busy;
					keyHoleItem->Flags |= TRIGGERED;
					keyHoleItem->Status = ITEM_ACTIVE;
					keyHoleItem->MeshBits = 255;

				}

				if (laraItem->Animation.AnimNumber == LA_PICKUP_PEDESTAL_HIGH && laraItem->Animation.FrameNumber == 45)
				{
					TENLog("TestFrame complete", LogLevel::Error, LogConfig::All, false);

				}

				g_Gui.SetInventoryItemChosen(NO_VALUE);
				return;
			}

			if (player->Control.IsMoving && player->Context.InteractedItem == itemNumber)
			{
				player->Control.IsMoving = false;
				player->Control.HandStatus = HandStatus::Free;
			}
		}
		else
		{
			if (keyHoleItem->ObjectNumber < ID_KEY_HOLE6)
				ObjectCollision(itemNumber, laraItem, coll);
		}
		
		return;
	}
}

/*

{
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	short* bounds;
	short room_number, y_rot;

	item = &items[item_number];

	if (input & IN_ACTION && l->current_anim_state == AS_STOP && l->anim_number == ANIM_BREATH && !l->gravity_status &&
		lara.gun_status == LG_NO_ARMS && !item->trigger_flags && !item->item_flags[0])
	{
		if (!item->item_flags[1])
		{
			bounds = GetBoundsAccurate(item);
			StatuePlinthBounds[0] = bounds[0];
			StatuePlinthBounds[1] = bounds[1];
			StatuePlinthBounds[4] = bounds[4] - 200;
			StatuePlinthBounds[5] = bounds[4] + 200;
			y_rot = item->pos.y_rot;
			item->pos.y_rot = l->pos.y_rot;

			if (TestLaraPosition(StatuePlinthBounds, item, l))
			{
				if (have_i_got_object(PUZZLE_ITEM5))
				{
					GLOBAL_enterinventory = PUZZLE_ITEM5;
					item->item_flags[1] = 1;
				}
			}

			item->pos.y_rot = y_rot;
			return;
		}

		if (GLOBAL_inventoryitemchosen == PUZZLE_ITEM5)
		{
			l->anim_number = ANIM_PLINTHHI;
			l->frame_number = anims[ANIM_PLINTHHI].frame_base;
			l->current_anim_state = AS_CONTROLLED;
			lara.gun_status = LG_HANDS_BUSY;
			GLOBAL_inventoryitemchosen = NO_ITEM;
			return;
		}
	}
	else if (item->item_flags[1])
	{
		if (GLOBAL_inventoryitemchosen == PUZZLE_ITEM5)
		{
			l->anim_number = ANIM_PLINTHHI;
			l->frame_number = anims[ANIM_PLINTHHI].frame_base;
			l->current_anim_state = AS_CONTROLLED;
			lara.gun_status = LG_HANDS_BUSY;
			GLOBAL_inventoryitemchosen = NO_ITEM;
			return;
		}
	}

	if (l->anim_number == ANIM_PLINTHHI && l->frame_number == anims[ANIM_PLINTHHI].frame_base + 45)
	{
		room_number = item->room_number;
		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
		GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
		TestTriggers(trigger_index, 1, item->flags & 0x3E00);
		item->mesh_bits = 255;
		item->item_flags[0] = 1;
		lara.puzzleitems[4]--;
	}
	else
	{
		item->item_flags[1] = 0;
		ObjectCollision(item_number, l, coll);
	}
}*/