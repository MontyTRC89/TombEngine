#include "framework.h"
#include "Objects/Generic/puzzles_keys.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/control/control.h"
#include "Game/control/trigger.h"
#include "Game/Gui.h"
#include "Game/Hud/Hud.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/pickup/pickup.h"
#include "Game/Setup.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Entities::Switches;
using namespace TEN::Gui;
using namespace TEN::Hud;
using namespace TEN::Input;

short PuzzleItem;

enum class PuzzleType
{
	Normal, 
	Specfic, 
	Cutscene, 
	AnimAfter
};

ObjectCollisionBounds PuzzleBounds =
{
	GameBoundingBox(
		0, 0,
		-CLICK(1), CLICK(1),
		0, 0),
	std::pair(
		EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
		EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f)))
};

const auto KeyHolePosition = Vector3i(0, 0, 312);
const ObjectCollisionBounds KeyHoleBounds =
{
	GameBoundingBox(
		-CLICK(1), CLICK(1),
		0, 0,
		0, 412),
	std::pair(
		EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
		EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f)))
};

// Puzzles
void InitializePuzzleHole(short itemNumber)
{
	auto& receptacleItem = g_Level.Items[itemNumber];
	receptacleItem.ItemFlags[5] = (int)ReusableReceptacleState::Empty;
}

void InitializePuzzleDone(short itemNumber)
{
	auto& receptacleItem = g_Level.Items[itemNumber];
	const auto& anim = GetAnimData(receptacleItem);

	receptacleItem.Animation.RequiredState = NO_STATE;
	receptacleItem.Animation.FrameNumber = anim.frameBase + anim.frameEnd;
}

void PuzzleHoleCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto& receptacleItem = g_Level.Items[itemNumber];
	auto& player = GetLaraInfo(*laraItem);

	// Start level with correct object when loading game.
	if (receptacleItem.ItemFlags[5] == (int)ReusableReceptacleState::Done)
	{
		receptacleItem.ObjectNumber += GAME_OBJECT_ID{ ID_PUZZLE_DONE1 - ID_PUZZLE_HOLE1 };
		SetAnimation(receptacleItem, 0);
		receptacleItem.ResetModelToDefault();
		return;
	}

	auto puzzleType = PuzzleType::Normal;

	if (receptacleItem.TriggerFlags >= 0)
	{
		if (receptacleItem.TriggerFlags <= 1024)
		{
			if (receptacleItem.TriggerFlags &&
				receptacleItem.TriggerFlags != 999 &&
				receptacleItem.TriggerFlags != 998)
			{
				puzzleType = PuzzleType::AnimAfter;
			}
		}
		else
		{
			puzzleType = PuzzleType::Cutscene;
		}
	}
	else
	{
		puzzleType = PuzzleType::Specfic;
	}

	if (((IsHeld(In::Action) || g_Gui.GetInventoryItemChosen() != NO_ITEM) &&
		laraItem->Animation.ActiveState == LS_IDLE &&
		laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
		player.Control.HandStatus == HandStatus::Free &&
		player.Control.Look.OpticRange == 0) ||
		(player.Control.IsMoving &&
			player.Context.InteractedItem == itemNumber))
	{
		short prevYOrient = receptacleItem.Pose.Orientation.y;

		auto bounds = GameBoundingBox(&receptacleItem);
		PuzzleBounds.BoundingBox.X1 = bounds.X1 - BLOCK(0.25f);
		PuzzleBounds.BoundingBox.X2 = bounds.X2 + BLOCK(0.25f);
		PuzzleBounds.BoundingBox.Z1 = bounds.Z1 - BLOCK(0.25f);
		PuzzleBounds.BoundingBox.Z2 = bounds.Z2 + BLOCK(0.25f);

		if (TestLaraPosition(PuzzleBounds, &receptacleItem, laraItem))
		{
			if (!player.Control.IsMoving)
			{
				if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
				{
					if (g_Gui.IsObjectInInventory(receptacleItem.ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1)))
						g_Gui.SetEnterInventory(receptacleItem.ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1));

					receptacleItem.Pose.Orientation.y = prevYOrient;
					return;
				}

				if (g_Gui.GetInventoryItemChosen() != receptacleItem.ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1))
				{
					receptacleItem.Pose.Orientation.y = prevYOrient;
					return;
				}
			}

			if (puzzleType != PuzzleType::Cutscene)
			{
				auto pos = Vector3i(0, 0, bounds.Z1 - 100);
				if (!MoveLaraPosition(pos, &receptacleItem, laraItem))
				{
					player.Context.InteractedItem = itemNumber;
					g_Gui.SetInventoryItemChosen(NO_ITEM);
					receptacleItem.Pose.Orientation.y = prevYOrient;
					return;
				}
			}

			RemoveObjectFromInventory(GAME_OBJECT_ID(receptacleItem.ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1)), 1);

			if (puzzleType == PuzzleType::Specfic)
			{
				laraItem->Animation.ActiveState = LS_MISC_CONTROL;
				laraItem->Animation.AnimNumber = -receptacleItem.TriggerFlags;

				if (laraItem->Animation.AnimNumber != LA_TRIDENT_SET)
					PuzzleDone(&receptacleItem, itemNumber);
			}
			else
			{
				laraItem->Animation.AnimNumber = LA_USE_PUZZLE;
				laraItem->Animation.ActiveState = LS_INSERT_PUZZLE;
				receptacleItem.ItemFlags[0] = 1;
			}

			g_Gui.SetInventoryItemChosen(NO_ITEM);
			ResetPlayerFlex(laraItem);
			laraItem->Animation.FrameNumber = GetAnimData(laraItem).frameBase;
			player.Control.IsMoving = false;
			player.Control.HandStatus = HandStatus::Busy;
			player.Context.InteractedItem = itemNumber;
			receptacleItem.Pose.Orientation.y = prevYOrient;
			receptacleItem.Flags |= TRIGGERED;
			return;
		}

		if (player.Control.IsMoving)
		{
			if (player.Context.InteractedItem == itemNumber)
			{
				player.Control.IsMoving = false;
				player.Control.HandStatus = HandStatus::Free;
			}
		}

		receptacleItem.Pose.Orientation.y = prevYOrient;
	}
	else
	{
		if (!player.Control.IsMoving && player.Context.InteractedItem == itemNumber || player.Context.InteractedItem != itemNumber)
		{
			if (player.Context.InteractedItem == itemNumber)
			{
				if (laraItem->Animation.ActiveState != LS_MISC_CONTROL)
				{
					if (puzzleType != PuzzleType::Cutscene)
						ObjectCollision(itemNumber, laraItem, coll);

					return;
				}
			}

			if (laraItem->Animation.ActiveState == LS_MISC_CONTROL)
				return;

			if (puzzleType != PuzzleType::Cutscene)
				ObjectCollision(itemNumber, laraItem, coll);

			return;
		}
	}
}

void PuzzleDoneCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	if ((g_Level.Items[itemNumber].TriggerFlags - 998) > 1)
		ObjectCollision(itemNumber, laraItem, coll);

	auto& receptacleItem = g_Level.Items[itemNumber];
	auto& player = GetLaraInfo(*laraItem);

	// NOTE: Only execute code below if Triggertype is switch trigger.
	short* triggerIndexPtr = GetTriggerIndex(&receptacleItem);
	if (triggerIndexPtr == nullptr)
		return;

	int triggerType = (*(triggerIndexPtr++) >> 8) & TRIGGER_BITS;
	if (triggerType != TRIGGER_TYPES::SWITCH)
		return;

	AnimateItem(&receptacleItem);

	// Start level with correct object when loading game.
	if (receptacleItem.ItemFlags[5] == (int)ReusableReceptacleState::Empty)
	{
		receptacleItem.ObjectNumber = GAME_OBJECT_ID(receptacleItem.ObjectNumber - (ID_PUZZLE_DONE1 - ID_PUZZLE_HOLE1));
		SetAnimation(receptacleItem, 0);
		receptacleItem.ResetModelToDefault();
		return;
	}

	// Activate triggers when startig level for first time.
	if (receptacleItem.ItemFlags[5] == (int)ReusableReceptacleState::None)
	{
		receptacleItem.ItemFlags[1] = true;
		TestTriggers(receptacleItem.Pose.Position.x, receptacleItem.Pose.Position.y, receptacleItem.Pose.Position.z, receptacleItem.RoomNumber, false, 0);
		receptacleItem.ItemFlags[5] = (int)ReusableReceptacleState::Done;
	}

	auto puzzleType = PuzzleType::Normal;

	if ((IsHeld(In::Action) &&
		laraItem->Animation.ActiveState == LS_IDLE &&
		laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
		player.Control.HandStatus == HandStatus::Free &&
		player.Control.Look.OpticRange == 0) ||
		(player.Control.IsMoving &&
			player.Context.InteractedItem == itemNumber))
	{
		short prevYOrient = receptacleItem.Pose.Orientation.y;

		auto bounds = GameBoundingBox(&receptacleItem);
		PuzzleBounds.BoundingBox.X1 = bounds.X1 - BLOCK(0.25f);
		PuzzleBounds.BoundingBox.X2 = bounds.X2 + BLOCK(0.25f);
		PuzzleBounds.BoundingBox.Z1 = bounds.Z1 - BLOCK(0.25f);
		PuzzleBounds.BoundingBox.Z2 = bounds.Z2 + BLOCK(0.25f);

		if (TestLaraPosition(PuzzleBounds, &receptacleItem, laraItem))
		{
			auto pos = Vector3i(0, 0, bounds.Z1 - 100);
			if (!MoveLaraPosition(pos, &receptacleItem, laraItem))
			{
				player.Context.InteractedItem = itemNumber;
				g_Gui.SetInventoryItemChosen(NO_ITEM);
				receptacleItem.Pose.Orientation.y = prevYOrient;
				return;
			}

			laraItem->Animation.AnimNumber = LA_REMOVE_PUZZLE;
			laraItem->Animation.ActiveState = LS_REMOVE_PUZZLE;
			receptacleItem.ItemFlags[0] = 1;

			ResetPlayerFlex(laraItem);
			laraItem->Animation.FrameNumber = GetAnimData(*laraItem, laraItem->Animation.AnimNumber).frameBase;
			player.Control.IsMoving = false;
			player.Control.HandStatus = HandStatus::Busy;
			player.Context.InteractedItem = itemNumber;
			receptacleItem.Pose.Orientation.y = prevYOrient;
			receptacleItem.Flags |= TRIGGERED;
			return;
		}

		if (player.Control.IsMoving)
		{
			if (player.Context.InteractedItem == itemNumber)
			{
				player.Control.IsMoving = false;
				player.Control.HandStatus = HandStatus::Free;
			}
		}

		receptacleItem.Pose.Orientation.y = prevYOrient;
	}
	else
	{
		if ((!player.Control.IsMoving && player.Context.InteractedItem == itemNumber) ||
			player.Context.InteractedItem != itemNumber)
		{
			if (laraItem->Animation.ActiveState == LS_MISC_CONTROL)
				return;

			if (puzzleType != PuzzleType::Cutscene)
				ObjectCollision(itemNumber, laraItem, coll);

			return;
		}
	}
}

void PuzzleDone(ItemInfo* item, short itemNumber)
{
	short* triggerIndexPtr = GetTriggerIndex(item);
	short triggerType = (triggerIndexPtr != nullptr) ? (*(triggerIndexPtr++) >> 8) & TRIGGER_BITS : TRIGGER_TYPES::TRIGGER;

	if (triggerType == TRIGGER_TYPES::SWITCH)
	{
		item->ItemFlags[1] = true;

		item->ObjectNumber += GAME_OBJECT_ID{ ID_PUZZLE_DONE1 - ID_PUZZLE_HOLE1 };
		item->ItemFlags[5] = (int)ReusableReceptacleState::Done;
		SetAnimation(item, 0);
		item->ResetModelToDefault();	
	}
	else
	{
		item->ObjectNumber += GAME_OBJECT_ID{ ID_PUZZLE_DONE1 - ID_PUZZLE_HOLE1 };
		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
		item->Animation.FrameNumber = GetAnimData(item).frameBase;
		item->Animation.ActiveState = GetAnimData(item).ActiveState;
		item->Animation.TargetState = GetAnimData(item).ActiveState;
		item->Animation.RequiredState = NO_STATE;
		item->ResetModelToDefault();

		AddActiveItem(itemNumber);

		item->Flags |= IFLAG_ACTIVATION_MASK;
		item->Status = ITEM_ACTIVE;
	}
}

void PuzzleHole(ItemInfo* item, short itemNumber)
{
	// Display pickup object. TODO: Get offset.
	auto objectID = GAME_OBJECT_ID(item->ObjectNumber - (ID_PUZZLE_DONE1 - ID_PUZZLE_ITEM1));
	g_Hud.PickupSummary.AddDisplayPickup(objectID, item->Pose.Position.ToVector3());

	item->ItemFlags[1] = true;

	item->ObjectNumber = GAME_OBJECT_ID(item->ObjectNumber - (ID_PUZZLE_DONE1 - ID_PUZZLE_HOLE1));
	item->ItemFlags[5] = (int)ReusableReceptacleState::Empty;
	SetAnimation(item, 0);
	item->ResetModelToDefault();
}

void DoPuzzle()
{
	PuzzleItem = Lara.Context.InteractedItem;
	auto* item = &g_Level.Items[PuzzleItem];

	int flag = 0;

	if (item->TriggerFlags >= 0)
	{
		if (item->TriggerFlags <= 1024)
		{
			if (item->TriggerFlags && item->TriggerFlags != 999 && item->TriggerFlags != 998)
				flag = 3;
		}
		else
		{
			flag = 2;
		}
	}
	else
	{
		flag = 1;
	}

	if (LaraItem->Animation.ActiveState == LS_INSERT_PUZZLE)
	{
		if (item->ItemFlags[0])
		{
			if (flag == 3)
			{
				LaraItem->ItemFlags[0] = item->TriggerFlags;
			}
			else
			{
				LaraItem->ItemFlags[0] = 0;
				PuzzleDone(item, PuzzleItem);
				item->ItemFlags[0] = 0;
			}
		}

		if (LaraItem->Animation.AnimNumber == LA_TRIDENT_SET)
			PuzzleDone(item, PuzzleItem);
	}

	if (LaraItem->Animation.ActiveState == LS_REMOVE_PUZZLE)
	{
		if (item->ItemFlags[0])
		{
			if (flag == 3)
			{
				LaraItem->ItemFlags[0] = item->TriggerFlags;
			}
			else
			{
				LaraItem->ItemFlags[0] = 0;
				PuzzleHole(item, PuzzleItem);
				item->ItemFlags[0] = 0;
			}
		}
	}
}

// Keys
void KeyHoleCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* keyHoleItem = &g_Level.Items[itemNumber];
	auto* player = GetLaraInfo(laraItem);

	short* triggerIndexPtr = GetTriggerIndex(keyHoleItem);

	if (triggerIndexPtr == nullptr)
		return;

	short triggerType = (*(triggerIndexPtr++) >> 8) & TRIGGER_BITS;

	bool isActionReady = (IsHeld(In::Action) || g_Gui.GetInventoryItemChosen() != NO_ITEM);

	bool isPlayerAvailable = (player->Control.Look.OpticRange == 0 &&
							 laraItem->Animation.ActiveState == LS_IDLE &&
							 laraItem->Animation.AnimNumber == LA_STAND_IDLE);

	bool actionActive = player->Control.IsMoving && player->Context.InteractedItem == itemNumber;

	if (actionActive || (isActionReady && isPlayerAvailable))
	{
		if (TestLaraPosition(KeyHoleBounds, keyHoleItem, laraItem))
		{
			if (!player->Control.IsMoving)
			{
				if (keyHoleItem->Status != ITEM_NOT_ACTIVE && triggerType != TRIGGER_TYPES::SWITCH)
					return;

				if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
				{
					if (g_Gui.IsObjectInInventory(keyHoleItem->ObjectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1)))
						g_Gui.SetEnterInventory(keyHoleItem->ObjectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1));

					return;
				}

				if (g_Gui.GetInventoryItemChosen() != keyHoleItem->ObjectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1))
					return;

				player->Context.InteractedItem = itemNumber;
			}

			if (player->Context.InteractedItem != itemNumber)
				return;

			if (MoveLaraPosition(KeyHolePosition, keyHoleItem, laraItem))
			{
				if (triggerType != TRIGGER_TYPES::SWITCH)
				{
					RemoveObjectFromInventory(GAME_OBJECT_ID(keyHoleItem->ObjectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1)), 1);
				}
				else
				{
					keyHoleItem->ItemFlags[1] = true;
				}

				if (keyHoleItem->TriggerFlags == 0)
				{
					laraItem->Animation.AnimNumber = LA_USE_KEY;
				}
				else
				{
					laraItem->Animation.AnimNumber = keyHoleItem->TriggerFlags;
				}
				
				laraItem->Animation.ActiveState = LS_INSERT_KEY;
				laraItem->Animation.FrameNumber = GetAnimData(laraItem).frameBase;
				player->Control.IsMoving = false;
				ResetPlayerFlex(laraItem);
				player->Control.HandStatus = HandStatus::Busy;
				keyHoleItem->Flags |= TRIGGERED;
				keyHoleItem->Status = ITEM_ACTIVE;
			}

			g_Gui.SetInventoryItemChosen(NO_ITEM);
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
