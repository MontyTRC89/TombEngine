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

enum PuzzleType {
	PUZZLETYPE_NORMAL, 
	PUZZLETYPE_SPECIFIC, 
	PUZZLETYPE_CUTSCENE, 
	PUZZLETYPE_ANIM_AFTER 
};

/*vars*/
short puzzleItem;
/*bounds*/
OBJECT_COLLISION_BOUNDS PuzzleBounds =
{ 0, 0, -256, 256, 0, 0, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), ANGLE(-10), ANGLE(10) };

static PHD_VECTOR KeyHolePosition(0, 0, 312);
OBJECT_COLLISION_BOUNDS KeyHoleBounds =
{ -256, 256, 0, 0, 0, 412, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), ANGLE(-10), ANGLE(10) };

/*puzzles*/
void PuzzleHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	int flag = PUZZLETYPE_NORMAL;
	
	if (item->TriggerFlags >= 0)
	{
		if (item->TriggerFlags <= 1024)
		{
			if (item->TriggerFlags && item->TriggerFlags != 999 && item->TriggerFlags != 998)
				flag = PUZZLETYPE_ANIM_AFTER;
		}
		else
			flag = PUZZLETYPE_CUTSCENE;
	}
	else
		flag = PUZZLETYPE_SPECIFIC;

	if (((TrInput & IN_ACTION || g_Gui.GetInventoryItemChosen() != NO_ITEM)
		&& !BinocularRange
		&& Lara.Control.HandStatus == HandStatus::Free
		&& l->ActiveState == LS_IDLE
		&& l->AnimNumber == LA_STAND_IDLE
		&& GetKeyTrigger(&g_Level.Items[itemNum])) 
		|| (Lara.Control.IsMoving
			&& Lara.interactedItem == itemNum))
	{
		short oldYrot = item->Position.yRot;
		BOUNDING_BOX* bounds = GetBoundsAccurate(item);

		PuzzleBounds.boundingBox.X1 = bounds->X1 - 256;
		PuzzleBounds.boundingBox.X2 = bounds->X2 + 256;
		PuzzleBounds.boundingBox.Z1 = bounds->Z1 - 256;
		PuzzleBounds.boundingBox.Z2 = bounds->Z2 + 256;

		if (TestLaraPosition(&PuzzleBounds, item, l))
		{
			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = 0;

			if (!Lara.Control.IsMoving)
			{
				if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
				{
					if (g_Gui.IsObjectInInventory(item->ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1)))
						g_Gui.SetEnterInventory(item->ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1));

					item->Position.yRot = oldYrot;
					return;
				}

				if (g_Gui.GetInventoryItemChosen() != item->ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1))
				{
					item->Position.yRot = oldYrot;
					return;
				}
			}

			pos.z = bounds->Z1 - 100;

			if (flag != PUZZLETYPE_CUTSCENE)
			{
				if (!MoveLaraPosition(&pos, item, l))
				{
					Lara.interactedItem = itemNum;
					g_Gui.SetInventoryItemChosen(NO_ITEM);
					item->Position.yRot = oldYrot;
					return;
				}
			}

			RemoveObjectFromInventory(static_cast<GAME_OBJECT_ID>(item->ObjectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1)), 1);

			if (flag == PUZZLETYPE_SPECIFIC)
			{
				l->ActiveState = LS_MISC_CONTROL;
				l->AnimNumber = -item->TriggerFlags;
				if (l->AnimNumber != LA_TRIDENT_SET)
					PuzzleDone(item, itemNum);
			}
			else
			{
				l->AnimNumber = LA_USE_PUZZLE;
				l->ActiveState = LS_INSERT_PUZZLE;
				item->ItemFlags[0] = 1;
			}

			l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
			Lara.Control.IsMoving = false;
			ResetLaraFlex(l);
			Lara.Control.HandStatus = HandStatus::Busy;
			item->Flags |= 0x20;
			Lara.interactedItem = itemNum;
			g_Gui.SetInventoryItemChosen(NO_ITEM);
			item->Position.yRot = oldYrot;
			return;
		}

		if (Lara.Control.IsMoving)
		{
			if (Lara.interactedItem == itemNum)
			{
				Lara.Control.IsMoving = false;
				Lara.Control.HandStatus = HandStatus::Free;
			}
		}

		item->Position.yRot = oldYrot;
	}
	else
	{
		if (!Lara.Control.IsMoving && Lara.interactedItem == itemNum || Lara.interactedItem != itemNum)
		{
			if (Lara.interactedItem == itemNum)
			{
				if (l->ActiveState != LS_MISC_CONTROL)
				{
					if (flag != PUZZLETYPE_CUTSCENE)
						ObjectCollision(itemNum, l, coll);
					return;
				}
			}
			if (l->ActiveState == LS_MISC_CONTROL)
				return;

			if (flag != PUZZLETYPE_CUTSCENE)
				ObjectCollision(itemNum, l, coll);
			return;
		}
	}
}

void PuzzleDoneCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	if (g_Level.Items[itemNum].TriggerFlags - 998 > 1)
	{
		ObjectCollision(itemNum, l, coll);
	}
}

void PuzzleDone(ITEM_INFO* item, short itemNum)
{
	item->ObjectNumber += GAME_OBJECT_ID{ ID_PUZZLE_DONE1 - ID_PUZZLE_HOLE1 };
	item->AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->RequiredState = 0;
	item->TargetState = g_Level.Anims[item->AnimNumber].ActiveState;
	item->ActiveState = g_Level.Anims[item->AnimNumber].ActiveState;

	AddActiveItem(itemNum);

	item->Flags |= IFLAG_ACTIVATION_MASK;
	item->Status = ITEM_ACTIVE;
}

void DoPuzzle()
{
	puzzleItem = Lara.interactedItem;
	ITEM_INFO* item = &g_Level.Items[puzzleItem];
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

	if (LaraItem->ActiveState == LS_INSERT_PUZZLE)
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
		if (LaraItem->AnimNumber == LA_TRIDENT_SET)
		{
			PuzzleDone(item, puzzleItem);
		}
	}
}
/*keys*/
void KeyHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	if (g_Level.Items[itemNum].TriggerFlags == 1 && item->ObjectNumber == ID_KEY_HOLE8)
	{
		if (item->ItemFlags[3])
		{
			item->ItemFlags[3]--;
			if (!item->ItemFlags[3])
				item->MeshBits = 2;
		}
	}

	if (!((TrInput & IN_ACTION || g_Gui.GetInventoryItemChosen() != NO_ITEM)
		&& !BinocularRange
		&& Lara.Control.HandStatus == HandStatus::Free
		&& l->ActiveState == LS_IDLE
		&& l->AnimNumber == LA_STAND_IDLE)
		&& (!Lara.Control.IsMoving || Lara.interactedItem != itemNum))
	{
		if (item->ObjectNumber < ID_KEY_HOLE6)
			ObjectCollision(itemNum, l, coll);
	}
	else
	{
		if (TestLaraPosition(&KeyHoleBounds, item, l))
		{
			if (!Lara.Control.IsMoving)//TROYE INVENTORY FIX ME
			{
				if (item->Status != ITEM_NOT_ACTIVE)
					return;

				if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
				{
					if (g_Gui.IsObjectInInventory(item->ObjectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1)))
						g_Gui.SetEnterInventory(item->ObjectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1));

					return;
				}

				if (g_Gui.GetInventoryItemChosen() != item->ObjectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1))
					return;
			}

			if (MoveLaraPosition(&KeyHolePosition, item, l))
			{
				if (item->ObjectNumber == ID_KEY_HOLE8)
					l->AnimNumber = LA_KEYCARD_USE;
				else
				{
					RemoveObjectFromInventory(static_cast<GAME_OBJECT_ID>(item->ObjectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1)), 1);
					l->AnimNumber = LA_USE_KEY;
				}
				l->ActiveState = LS_INSERT_KEY;
				l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
				Lara.Control.IsMoving = false;
				ResetLaraFlex(l);
				Lara.Control.HandStatus = HandStatus::Busy;
				item->Flags |= 0x20;
				item->Status = ITEM_ACTIVE;

				if (item->TriggerFlags == 1 && item->ObjectNumber == ID_KEY_HOLE8)
				{
					item->ItemFlags[3] = 92;
					g_Gui.SetInventoryItemChosen(NO_ITEM);
					return;
				}
			}
			else
			{
				Lara.interactedItem = itemNum;
			}

			g_Gui.SetInventoryItemChosen(NO_ITEM);
			return;
		}

		if (Lara.Control.IsMoving && Lara.interactedItem == itemNum)
		{
			Lara.Control.IsMoving = false;
			Lara.Control.HandStatus = HandStatus::Free;
		}
	}

	return;
}