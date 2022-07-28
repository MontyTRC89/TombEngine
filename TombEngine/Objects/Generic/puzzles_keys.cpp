#include "framework.h"
#include "Objects/Generic/puzzles_keys.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/control/control.h"
#include "Game/gui.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/pickup/pickup.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Input;
using namespace TEN::Entities::Switches;

short PuzzleItem;

enum class PuzzleType
{
	Normal, 
	Specfic, 
	Cutscene, 
	AnimAfter 
};

OBJECT_COLLISION_BOUNDS PuzzleBounds =
{
	0, 0,
	-256, 256,
	0, 0,
	-ANGLE(10.0f), ANGLE(10.0f),
	-ANGLE(30.0f), ANGLE(30.0f),
	-ANGLE(10.0f), ANGLE(10.0f)
};

static Vector3Int KeyHolePosition(0, 0, 312);
OBJECT_COLLISION_BOUNDS KeyHoleBounds =
{
	-256, 256,
	0, 0,
	0, 412,
	-ANGLE(10.0f), ANGLE(10.0f),
	-ANGLE(30.0f), ANGLE(30.0f),
	-ANGLE(10.0f), ANGLE(10.0f)
};

// Puzzles
void PuzzleHoleCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* receptableItem = &g_Level.Items[itemNumber];

	auto puzzleType = PuzzleType::Normal;
	
	if (receptableItem->TriggerFlags >= 0)
	{
		if (receptableItem->TriggerFlags <= 1024)
		{
			if (receptableItem->TriggerFlags &&
				receptableItem->TriggerFlags != 999 &&
				receptableItem->TriggerFlags != 998)
			{
				puzzleType = PuzzleType::AnimAfter;
			}
		}
		else
			puzzleType = PuzzleType::Cutscene;
	}
	else
		puzzleType = PuzzleType::Specfic;

	if (((TrInput & IN_ACTION || g_Gui.GetInventoryItemChosen() != NO_ITEM) &&
		laraItem->Animation.ActiveState == LS_IDLE &&
		laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
		laraInfo->Control.HandStatus == HandStatus::Free &&
		!BinocularRange &&
		GetKeyTrigger(&g_Level.Items[itemNumber])) ||
		(laraInfo->Control.IsMoving &&
			laraInfo->InteractedItem == itemNumber))
	{
		short oldYrot = receptableItem->Pose.Orientation.y;

		auto* bounds = GetBoundsAccurate(receptableItem);
		PuzzleBounds.boundingBox.X1 = bounds->X1 - CLICK(1);
		PuzzleBounds.boundingBox.X2 = bounds->X2 + CLICK(1);
		PuzzleBounds.boundingBox.Z1 = bounds->Z1 - CLICK(1);;
		PuzzleBounds.boundingBox.Z2 = bounds->Z2 + CLICK(1);;

		if (TestLaraPosition(&PuzzleBounds, receptableItem, laraItem))
		{
			if (!laraInfo->Control.IsMoving)
			{
				if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
				{
					if (g_Gui.IsObjectInInventory(receptableItem->ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1)))
						g_Gui.SetEnterInventory(receptableItem->ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1));

					receptableItem->Pose.Orientation.y = oldYrot;
					return;
				}

				if (g_Gui.GetInventoryItemChosen() != receptableItem->ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1))
				{
					receptableItem->Pose.Orientation.y = oldYrot;
					return;
				}
			}

			if (puzzleType != PuzzleType::Cutscene)
			{
				Vector3Int pos = { 0, 0, bounds->Z1 - 100 };
				if (!MoveLaraPosition(&pos, receptableItem, laraItem))
				{
					laraInfo->InteractedItem = itemNumber;
					g_Gui.SetInventoryItemChosen(NO_ITEM);
					receptableItem->Pose.Orientation.y = oldYrot;
					return;
				}
			}

			RemoveObjectFromInventory(static_cast<GAME_OBJECT_ID>(receptableItem->ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1)), 1);

			if (puzzleType == PuzzleType::Specfic)
			{
				laraItem->Animation.ActiveState = LS_MISC_CONTROL;
				laraItem->Animation.AnimNumber = -receptableItem->TriggerFlags;

				if (laraItem->Animation.AnimNumber != LA_TRIDENT_SET)
					PuzzleDone(receptableItem, itemNumber);
			}
			else
			{
				laraItem->Animation.AnimNumber = LA_USE_PUZZLE;
				laraItem->Animation.ActiveState = LS_INSERT_PUZZLE;
				receptableItem->ItemFlags[0] = 1;
			}

			g_Gui.SetInventoryItemChosen(NO_ITEM);
			ResetLaraFlex(laraItem);
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraInfo->Control.IsMoving = false;
			laraInfo->Control.HandStatus = HandStatus::Busy;
			laraInfo->InteractedItem = itemNumber;
			receptableItem->Pose.Orientation.y = oldYrot;
			receptableItem->Flags |= TRIGGERED;
			return;
		}

		if (laraInfo->Control.IsMoving)
		{
			if (laraInfo->InteractedItem == itemNumber)
			{
				laraInfo->Control.IsMoving = false;
				laraInfo->Control.HandStatus = HandStatus::Free;
			}
		}

		receptableItem->Pose.Orientation.y = oldYrot;
	}
	else
	{
		if (!laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber || laraInfo->InteractedItem != itemNumber)
		{
			if (laraInfo->InteractedItem == itemNumber)
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
}

void PuzzleDone(ItemInfo* item, short itemNumber)
{
	item->ObjectNumber += GAME_OBJECT_ID{ ID_PUZZLE_DONE1 - ID_PUZZLE_HOLE1 };
	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.ActiveState = g_Level.Anims[item->Animation.AnimNumber].ActiveState;
	item->Animation.TargetState = g_Level.Anims[item->Animation.AnimNumber].ActiveState;
	item->Animation.RequiredState = 0;

	AddActiveItem(itemNumber);

	item->Flags |= IFLAG_ACTIVATION_MASK;
	item->Status = ITEM_ACTIVE;
}

void DoPuzzle()
{
	PuzzleItem = Lara.InteractedItem;
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
			flag = 2;
	}
	else
		flag = 1;

	if (LaraItem->Animation.ActiveState == LS_INSERT_PUZZLE)
	{
		if (item->ItemFlags[0])
		{
			if (flag == 3)
				LaraItem->ItemFlags[0] = item->TriggerFlags;
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
}

// Keys
void KeyHoleCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* keyHoleItem = &g_Level.Items[itemNumber];

	if (g_Level.Items[itemNumber].TriggerFlags == 1 &&
		keyHoleItem->ObjectNumber == ID_KEY_HOLE8)
	{
		if (keyHoleItem->ItemFlags[3])
		{
			keyHoleItem->ItemFlags[3]--;
			if (!keyHoleItem->ItemFlags[3])
				keyHoleItem->MeshBits = 2;
		}
	}

	bool actionReady = (TrInput & IN_ACTION || g_Gui.GetInventoryItemChosen() != NO_ITEM);

	bool laraAvailable = !BinocularRange &&
						 laraItem->Animation.ActiveState == LS_IDLE &&
						 laraItem->Animation.AnimNumber == LA_STAND_IDLE;

	bool actionActive = laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber;

	if (actionActive || (actionReady && laraAvailable))
	{
		if (TestLaraPosition(&KeyHoleBounds, keyHoleItem, laraItem))
		{
			if (!laraInfo->Control.IsMoving) //TROYE INVENTORY FIX ME
			{
				if (keyHoleItem->Status != ITEM_NOT_ACTIVE)
					return;

				if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
				{
					if (g_Gui.IsObjectInInventory(keyHoleItem->ObjectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1)))
						g_Gui.SetEnterInventory(keyHoleItem->ObjectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1));

					return;
				}

				if (g_Gui.GetInventoryItemChosen() != keyHoleItem->ObjectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1))
					return;

				laraInfo->InteractedItem = itemNumber;
			}

			if (laraInfo->InteractedItem != itemNumber)
				return;

			if (MoveLaraPosition(&KeyHolePosition, keyHoleItem, laraItem))
			{
				if (keyHoleItem->ObjectNumber == ID_KEY_HOLE8)
					laraItem->Animation.AnimNumber = LA_KEYCARD_USE;
				else
				{
					RemoveObjectFromInventory(static_cast<GAME_OBJECT_ID>(keyHoleItem->ObjectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1)), 1);
					laraItem->Animation.AnimNumber = LA_USE_KEY;
				}

				laraItem->Animation.ActiveState = LS_INSERT_KEY;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraInfo->Control.IsMoving = false;
				ResetLaraFlex(laraItem);
				laraInfo->Control.HandStatus = HandStatus::Busy;
				keyHoleItem->Flags |= TRIGGERED;
				keyHoleItem->Status = ITEM_ACTIVE;

				if (keyHoleItem->TriggerFlags == 1 && keyHoleItem->ObjectNumber == ID_KEY_HOLE8)
				{
					keyHoleItem->ItemFlags[3] = 92;
					g_Gui.SetInventoryItemChosen(NO_ITEM);
					return;
				}
			}

			g_Gui.SetInventoryItemChosen(NO_ITEM);
			return;
		}

		if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
		{
			laraInfo->Control.IsMoving = false;
			laraInfo->Control.HandStatus = HandStatus::Free;
		}
	}
	else
	{
		if (keyHoleItem->ObjectNumber < ID_KEY_HOLE6)
			ObjectCollision(itemNumber, laraItem, coll);
	}

	return;
}
