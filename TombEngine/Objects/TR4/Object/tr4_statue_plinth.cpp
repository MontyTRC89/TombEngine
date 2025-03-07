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

using namespace TEN::Collision::Sphere;
using namespace TEN::Entities::Switches;
using namespace TEN::Gui;
using namespace TEN::Input;
using namespace TEN::Debug;

namespace TEN::Entities::TR4
{
	constexpr auto PLACE_PLINTHITEM_FRAME = 45;

	const auto KeyHolePosition = Vector3i(0, 0, -390);
	const ObjectCollisionBounds KeyHoleBounds =
	{
		GameBoundingBox(
			-CLICK(1), CLICK(1),
			-BLOCK(0.5f), 0,
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

	void StatuePlinthCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* keyHoleItem = &g_Level.Items[itemNumber];
		auto* player = GetLaraInfo(laraItem);

		short* triggerIndexPtr = GetTriggerIndex(keyHoleItem);
		short y_rot;

		if (!keyHoleItem->ItemFlags[0])
			keyHoleItem->ItemFlags[0] = PLACE_PLINTHITEM_FRAME;

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

		if (isActionReady && isPlayerAvailable && !keyHoleItem->ItemFlags[3])
		{
			if (!keyHoleItem->ItemFlags[1])
			{
				y_rot = keyHoleItem->Pose.Orientation.y;

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
					if (g_Gui.IsObjectInInventory(keyItem))
					{
						g_Gui.SetEnterInventory(keyItem);
						keyHoleItem->ItemFlags[1] = 1;
					}
				}

				keyHoleItem->Pose.Orientation.y = y_rot;
				return;
			}

			if (g_Gui.GetInventoryItemChosen() == keyItem)
			{
				laraItem->Animation.AnimNumber = LA_PICKUP_PEDESTAL_HIGH;
				laraItem->Animation.FrameNumber = GetAnimData(LA_PICKUP_PEDESTAL_HIGH).frameBase;
				laraItem->Animation.ActiveState = LS_INSERT_KEY;
				ResetPlayerFlex(laraItem);
				player->Control.HandStatus = HandStatus::Busy;
				g_Gui.SetInventoryItemChosen(NO_VALUE);
				keyHoleItem->ItemFlags[2] = 1;
				return;
			}
		}

		if (laraItem->Animation.AnimNumber == LA_PICKUP_PEDESTAL_HIGH && laraItem->Animation.FrameNumber == GetAnimData(LA_PICKUP_PEDESTAL_HIGH).frameBase + keyHoleItem->ItemFlags[0] && keyHoleItem->ItemFlags[2])
		{
			TestTriggers(keyHoleItem, true, keyHoleItem->Flags & 0x3E00);
			keyHoleItem->Flags |= TRIGGERED;
			keyHoleItem->Status = ITEM_ACTIVE;
			keyHoleItem->MeshBits = 255;
			//TODO: Allow meshswap of puzzle items.
			//keyHoleItem->Model.MeshIndex[1] = Objects[keyItem].meshIndex + 0;
			//keyHoleItem->Model.Mutators[0].Offset = Vector3(0.0f, -896.0f, 0.0f);
			keyHoleItem->ItemFlags[3] = 1;
			RemoveObjectFromInventory(keyItem, 1);
		}
		else
		{
			keyHoleItem->ItemFlags[1] = 0;
			ObjectCollision(itemNumber, laraItem, coll);
		}
	}
}
		