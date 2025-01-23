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
		item->MeshBits = 1;
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
				//TENLog("TestLara complete", LogLevel::Error, LogConfig::All, false);
				if (!player->Control.IsMoving)
				{
					if (keyHoleItem->Status != ITEM_NOT_ACTIVE && triggerType != TRIGGER_TYPES::SWITCH)
					{
						keyHoleItem->Pose.Orientation.y = y_rot;
						return;
					}

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
					ResetPlayerFlex(laraItem);
					player->Control.HandStatus = HandStatus::Busy;
					keyHoleItem->Flags |= TRIGGERED;
					keyHoleItem->Status = ITEM_ACTIVE;
					keyHoleItem->MeshBits = 255;
				}

				g_Gui.SetInventoryItemChosen(NO_VALUE);
				return;
			}
			//laraItem->Animation.AnimNumber == LA_PICKUP_PEDESTAL_HIGH && laraItem->Animation.FrameNumber == GetAnimData(laraItem).frameBase + 45
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