#include "framework.h"
#include "collide.h"
#include "input.h"
#include "level.h"
#include "setup.h"
#include "lara.h"
#include "gui.h"
#include "pickup.h"
#include "animation.h"
#include "control/control.h"
#include "puzzles_keys.h"
#include "generic_switch.h"
#include "camera.h"
#include "items.h"

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
	
	if (item->triggerFlags >= 0)
	{
		if (item->triggerFlags <= 1024)
		{
			if (item->triggerFlags && item->triggerFlags != 999 && item->triggerFlags != 998)
				flag = PUZZLETYPE_ANIM_AFTER;
		}
		else
			flag = PUZZLETYPE_CUTSCENE;
	}
	else
		flag = PUZZLETYPE_SPECIFIC;

	if (((TrInput & IN_ACTION || g_Gui.GetInventoryItemChosen() != NO_ITEM)
		&& !BinocularRange
		&& !Lara.gunStatus
		&& l->currentAnimState == LS_STOP
		&& l->animNumber == LA_STAND_IDLE
		&& GetKeyTrigger(&g_Level.Items[itemNum])) 
		|| (Lara.isMoving
			&& Lara.interactedItem == itemNum))
	{
		short oldYrot = item->pos.yRot;
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

			if (!Lara.isMoving)
			{
				if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
				{
					if (g_Gui.IsObjectInInventory(item->objectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1)))
						g_Gui.SetEnterInventory(item->objectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1));

					item->pos.yRot = oldYrot;
					return;
				}

				if (g_Gui.GetInventoryItemChosen() != item->objectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1))
				{
					item->pos.yRot = oldYrot;
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
					item->pos.yRot = oldYrot;
					return;
				}
			}

			RemoveObjectFromInventory(static_cast<GAME_OBJECT_ID>(item->objectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1)), 1);

			if (flag == PUZZLETYPE_SPECIFIC)
			{
				l->currentAnimState = LS_MISC_CONTROL;
				l->animNumber = -item->triggerFlags;
				if (l->animNumber != LA_TRIDENT_SET)
					PuzzleDone(item, itemNum);
			}
			else
			{
				l->animNumber = LA_USE_PUZZLE;
				l->currentAnimState = LS_INSERT_PUZZLE;
				item->itemFlags[0] = 1;
			}

			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			Lara.isMoving = false;
			Lara.headYrot = 0;
			Lara.headXrot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			Lara.gunStatus = LG_HANDS_BUSY;
			item->flags |= 0x20;
			Lara.interactedItem = itemNum;
			g_Gui.SetInventoryItemChosen(NO_ITEM);
			item->pos.yRot = oldYrot;
			return;
		}

		if (Lara.isMoving)
		{
			if (Lara.interactedItem == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
		}

		item->pos.yRot = oldYrot;
	}
	else
	{
		if (!Lara.isMoving && Lara.interactedItem == itemNum || Lara.interactedItem != itemNum)
		{
			if (Lara.interactedItem == itemNum)
			{
				if (l->currentAnimState != LS_MISC_CONTROL)
				{
					if (flag != PUZZLETYPE_CUTSCENE)
						ObjectCollision(itemNum, l, coll);
					return;
				}
			}
			if (l->currentAnimState == LS_MISC_CONTROL)
				return;

			if (flag != PUZZLETYPE_CUTSCENE)
				ObjectCollision(itemNum, l, coll);
			return;
		}
	}
}

void PuzzleDoneCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	if (g_Level.Items[itemNum].triggerFlags - 998 > 1)
	{
		ObjectCollision(itemNum, l, coll);
	}
}

void PuzzleDone(ITEM_INFO* item, short itemNum)
{
	item->objectNumber += GAME_OBJECT_ID{ ID_PUZZLE_DONE1 - ID_PUZZLE_HOLE1 };
	item->animNumber = Objects[item->objectNumber].animIndex;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->requiredAnimState = 0;
	item->goalAnimState = g_Level.Anims[item->animNumber].currentAnimState;
	item->currentAnimState = g_Level.Anims[item->animNumber].currentAnimState;

	AddActiveItem(itemNum);

	item->flags |= IFLAG_ACTIVATION_MASK;
	item->status = ITEM_ACTIVE;
}

void DoPuzzle()
{
	puzzleItem = Lara.interactedItem;
	ITEM_INFO* item = &g_Level.Items[puzzleItem];
	int flag = 0;

	if (item->triggerFlags >= 0)
	{
		if (item->triggerFlags <= 1024)
		{
			if (item->triggerFlags && item->triggerFlags != 999 && item->triggerFlags != 998)
				flag = 3;
		}
		else
			flag = 2;
	}
	else
		flag = 1;

	if (LaraItem->currentAnimState == LS_INSERT_PUZZLE)
	{
		if (item->itemFlags[0])
		{
			if (flag == 3)
				LaraItem->itemFlags[0] = item->triggerFlags;
			else
			{
				LaraItem->itemFlags[0] = 0;
				PuzzleDone(item, puzzleItem);
				item->itemFlags[0] = 0;
			}
		}
		if (LaraItem->animNumber == LA_TRIDENT_SET)
		{
			PuzzleDone(item, puzzleItem);
		}
	}
}
/*keys*/
void KeyHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	if (g_Level.Items[itemNum].triggerFlags == 1 && item->objectNumber == ID_KEY_HOLE8)
	{
		if (item->itemFlags[3])
		{
			item->itemFlags[3]--;
			if (!item->itemFlags[3])
				item->meshBits = 2;
		}
	}

	if (!((TrInput & IN_ACTION || g_Gui.GetInventoryItemChosen() != NO_ITEM)
		&& !BinocularRange
		&& !Lara.gunStatus
		&& l->currentAnimState == LS_STOP
		&& l->animNumber == LA_STAND_IDLE)
		&& (!Lara.isMoving || Lara.interactedItem != itemNum))
	{
		if (item->objectNumber < ID_KEY_HOLE6)
			ObjectCollision(itemNum, l, coll);
	}
	else
	{
		if (TestLaraPosition(&KeyHoleBounds, item, l))
		{
			if (!Lara.isMoving)//TROYE INVENTORY FIX ME
			{
				if (item->status != ITEM_NOT_ACTIVE)
					return;

				if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
				{
					if (g_Gui.IsObjectInInventory(item->objectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1)))
						g_Gui.SetEnterInventory(item->objectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1));

					return;
				}

				if (g_Gui.GetInventoryItemChosen() != item->objectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1))
					return;
			}

			if (MoveLaraPosition(&KeyHolePosition, item, l))
			{
				if (item->objectNumber == ID_KEY_HOLE8)
					l->animNumber = LA_KEYCARD_USE;
				else
				{
					RemoveObjectFromInventory(static_cast<GAME_OBJECT_ID>(item->objectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1)), 1);
					l->animNumber = LA_USE_KEY;
				}
				l->currentAnimState = LS_INSERT_KEY;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
				item->flags |= 0x20;
				item->status = ITEM_ACTIVE;

				if (item->triggerFlags == 1 && item->objectNumber == ID_KEY_HOLE8)
				{
					item->itemFlags[3] = 92;
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

		if (Lara.isMoving && Lara.interactedItem == itemNum)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		}
	}

	return;
}