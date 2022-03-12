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

using namespace TEN::Entities::Switches;

short puzzleItem;

enum PuzzleType
{
	PUZZLETYPE_NORMAL, 
	PUZZLETYPE_SPECIFIC, 
	PUZZLETYPE_CUTSCENE, 
	PUZZLETYPE_ANIM_AFTER 
};

OBJECT_COLLISION_BOUNDS PuzzleBounds =
{
	0, 0,
	-256, 256,
	0, 0,
	ANGLE(-10.0f), ANGLE(10.0f),
	ANGLE(-30.0f), ANGLE(30.0f),
	ANGLE(-10.0f), ANGLE(10.0f)
};

static PHD_VECTOR KeyHolePosition(0, 0, 312);
OBJECT_COLLISION_BOUNDS KeyHoleBounds =
{
	-256, 256,
	0, 0,
	0, 412,
	ANGLE(-10.0f), ANGLE(10.0f),
	ANGLE(-30.0f), ANGLE(30.0f),
	ANGLE(-10.0f), ANGLE(10.0f)
};

// Puzzles
void PuzzleHoleCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* receptableItem = &g_Level.Items[itemNumber];

	int flag = PUZZLETYPE_NORMAL;
	
	if (receptableItem->TriggerFlags >= 0)
	{
		if (receptableItem->TriggerFlags <= 1024)
		{
			if (receptableItem->TriggerFlags &&
				receptableItem->TriggerFlags != 999 &&
				receptableItem->TriggerFlags != 998)
			{
				flag = PUZZLETYPE_ANIM_AFTER;
			}
		}
		else
			flag = PUZZLETYPE_CUTSCENE;
	}
	else
		flag = PUZZLETYPE_SPECIFIC;

	if (((TrInput & IN_ACTION || g_Gui.GetInventoryItemChosen() != NO_ITEM) &&
		!BinocularRange &&
		laraItem->Animation.ActiveState == LS_IDLE &&
		laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
		laraInfo->Control.HandStatus == HandStatus::Free &&
		GetKeyTrigger(&g_Level.Items[itemNumber])) ||
		(laraInfo->Control.IsMoving &&
			laraInfo->InteractedItem == itemNumber))
	{
		short oldYrot = receptableItem->Position.yRot;

		auto* bounds = GetBoundsAccurate(receptableItem);
		PuzzleBounds.boundingBox.X1 = bounds->X1 - 256;
		PuzzleBounds.boundingBox.X2 = bounds->X2 + 256;
		PuzzleBounds.boundingBox.Z1 = bounds->Z1 - 256;
		PuzzleBounds.boundingBox.Z2 = bounds->Z2 + 256;

		if (TestLaraPosition(&PuzzleBounds, receptableItem, laraItem))
		{
			if (!laraInfo->Control.IsMoving)
			{
				if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
				{
					if (g_Gui.IsObjectInInventory(receptableItem->ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1)))
						g_Gui.SetEnterInventory(receptableItem->ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1));

					receptableItem->Position.yRot = oldYrot;
					return;
				}

				if (g_Gui.GetInventoryItemChosen() != receptableItem->ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1))
				{
					receptableItem->Position.yRot = oldYrot;
					return;
				}
			}

			if (flag != PUZZLETYPE_CUTSCENE)
			{
				PHD_VECTOR pos = { 0, 0, bounds->Z1 - 100 };
				if (!MoveLaraPosition(&pos, receptableItem, laraItem))
				{
					laraInfo->InteractedItem = itemNumber;
					g_Gui.SetInventoryItemChosen(NO_ITEM);
					receptableItem->Position.yRot = oldYrot;
					return;
				}
			}

			RemoveObjectFromInventory(static_cast<GAME_OBJECT_ID>(receptableItem->ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1)), 1);

			if (flag == PUZZLETYPE_SPECIFIC)
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

			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraInfo->Control.IsMoving = false;
			ResetLaraFlex(laraItem);
			laraInfo->Control.HandStatus = HandStatus::Busy;
			receptableItem->Flags |= 0x20;
			laraInfo->InteractedItem = itemNumber;
			g_Gui.SetInventoryItemChosen(NO_ITEM);
			receptableItem->Position.yRot = oldYrot;
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

		receptableItem->Position.yRot = oldYrot;
	}
	else
	{
		if (!laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber || laraInfo->InteractedItem != itemNumber)
		{
			if (laraInfo->InteractedItem == itemNumber)
			{
				if (laraItem->Animation.ActiveState != LS_MISC_CONTROL)
				{
					if (flag != PUZZLETYPE_CUTSCENE)
						ObjectCollision(itemNumber, laraItem, coll);
					return;
				}
			}

			if (laraItem->Animation.ActiveState == LS_MISC_CONTROL)
				return;

			if (flag != PUZZLETYPE_CUTSCENE)
				ObjectCollision(itemNumber, laraItem, coll);

			return;
		}
	}
}

void PuzzleDoneCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	if (g_Level.Items[itemNumber].TriggerFlags - 998 > 1)
		ObjectCollision(itemNumber, laraItem, coll);
}

void PuzzleDone(ITEM_INFO* item, short itemNumber)
{
	item->ObjectNumber += GAME_OBJECT_ID{ ID_PUZZLE_DONE1 - ID_PUZZLE_HOLE1 };
	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.RequiredState = 0;
	item->Animation.TargetState = g_Level.Anims[item->Animation.AnimNumber].ActiveState;
	item->Animation.ActiveState = g_Level.Anims[item->Animation.AnimNumber].ActiveState;

	AddActiveItem(itemNumber);

	item->Flags |= IFLAG_ACTIVATION_MASK;
	item->Status = ITEM_ACTIVE;
}

void DoPuzzle()
{
	puzzleItem = Lara.InteractedItem;
	auto* item = &g_Level.Items[puzzleItem];

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
				PuzzleDone(item, puzzleItem);
				item->ItemFlags[0] = 0;
			}
		}
		if (LaraItem->Animation.AnimNumber == LA_TRIDENT_SET)
			PuzzleDone(item, puzzleItem);
	}
}

// Keys
void KeyHoleCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
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

	if (!((TrInput & IN_ACTION || g_Gui.GetInventoryItemChosen() != NO_ITEM) &&
		!BinocularRange &&
		laraItem->Animation.ActiveState == LS_IDLE &&
		laraItem->Animation.AnimNumber == LA_STAND_IDLE) &&
		laraInfo->Control.HandStatus == HandStatus::Free &&
		(!laraInfo->Control.IsMoving || laraInfo->InteractedItem != itemNumber))
	{
		if (keyHoleItem->ObjectNumber < ID_KEY_HOLE6)
			ObjectCollision(itemNumber, laraItem, coll);
	}
	else
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
			}

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
				keyHoleItem->Flags |= 0x20;
				keyHoleItem->Status = ITEM_ACTIVE;

				if (keyHoleItem->TriggerFlags == 1 && keyHoleItem->ObjectNumber == ID_KEY_HOLE8)
				{
					keyHoleItem->ItemFlags[3] = 92;
					g_Gui.SetInventoryItemChosen(NO_ITEM);
					return;
				}
			}
			else
				laraInfo->InteractedItem = itemNumber;

			g_Gui.SetInventoryItemChosen(NO_ITEM);
			return;
		}

		if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
		{
			laraInfo->Control.IsMoving = false;
			laraInfo->Control.HandStatus = HandStatus::Free;
		}
	}

	return;
}
