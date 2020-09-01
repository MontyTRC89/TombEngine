#include "framework.h"
#include "collide.h"
#include "input.h"
#include "lara.h"
#include "inventory.h"
#include "switch.h"
#include "pickup.h"
#include "draw.h"
#include "puzzles_keys.h"
/*vars*/
short puzzleItem;
/*bounds*/
OBJECT_COLLISION_BOUNDS PuzzleBounds =
{
	0x0000, 0x0000, 0xFF00, 0x0100, 0x0000, 0x0000, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
OBJECT_COLLISION_BOUNDS KeyHoleBounds =
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0x0000, 0x019C, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
static PHD_VECTOR KeyHolePosition(0, 0, 312);

/*puzzles*/
void PuzzleHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
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

	if (!((TrInput & IN_ACTION || g_Inventory.GetSelectedObject() != NO_ITEM)
		&& !BinocularRange
		&& !Lara.gunStatus
		&& l->currentAnimState == LS_STOP
		&& l->animNumber == LA_STAND_IDLE
		&& GetKeyTrigger(&g_Level.Items[itemNum])))
	{
		if (!Lara.isMoving && (short)Lara.generalPtr == itemNum || (short)Lara.generalPtr != itemNum)
		{
			if ((short)Lara.generalPtr == itemNum)
			{
				if (l->currentAnimState != LS_MISC_CONTROL)
				{
					if (flag != 2)
						ObjectCollision(itemNum, l, coll);
					return;
				}
			}
			if (l->currentAnimState == LS_MISC_CONTROL)
				return;

			if (flag != 2)
				ObjectCollision(itemNum, l, coll);
			return;
		}
	}

	short oldYrot = item->pos.yRot;
	BOUNDING_BOX* bounds = GetBoundsAccurate(item);

	PuzzleBounds.boundingBox.X1 = bounds->X1 - 256;
	PuzzleBounds.boundingBox.X2 = bounds->X2 + 256;
	PuzzleBounds.boundingBox.Z1 = bounds->Z1 - 256;
	PuzzleBounds.boundingBox.Z2 = bounds->Z2 + 256;

	if (item->triggerFlags == 1058)
	{
		PuzzleBounds.boundingBox.X1 = bounds->X1 - 256 - 300;
		PuzzleBounds.boundingBox.X2 = bounds->X2 + 256 + 300;
		PuzzleBounds.boundingBox.Z1 = bounds->Z1 - 256 - 300;
		PuzzleBounds.boundingBox.Z2 = bounds->Z2 + 256 + 300;
		item->pos.yRot = l->pos.yRot;
	}

	if (TestLaraPosition(&PuzzleBounds, item, l))
	{
		PHD_VECTOR pos;
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;

		if (!Lara.isMoving)
		{
			if (g_Inventory.GetSelectedObject() == NO_ITEM)
			{
				if (g_Inventory.IsObjectPresentInInventory(item->objectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1)))
					g_Inventory.SetEnterObject(item->objectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1));
				item->pos.yRot = oldYrot;
				return;
			}
			if (g_Inventory.GetSelectedObject() != item->objectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1))
			{
				item->pos.yRot = oldYrot;
				return;
			}
		}

		pos.z = bounds->Z1 - 100;
		if (flag != 2 || item->triggerFlags == 1036)
		{
			if (!MoveLaraPosition(&pos, item, l))
			{
				Lara.generalPtr = (void*)itemNum;
				g_Inventory.SetSelectedObject(NO_ITEM);
				item->pos.yRot = oldYrot;
				return;
			}
		}

		RemoveObjectFromInventory(item->objectNumber - (ID_PUZZLE_HOLE1 - ID_PUZZLE_ITEM1), 1);

		if (flag == 1)
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
		Lara.generalPtr = (void*)itemNum;
		g_Inventory.SetSelectedObject(NO_ITEM);
		item->pos.yRot = oldYrot;
		return;
	}

	if (Lara.isMoving)
	{
		if ((short)Lara.generalPtr == itemNum)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		}
	}

	item->pos.yRot = oldYrot;
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
	item->objectNumber += (ID_PUZZLE_DONE1 - ID_PUZZLE_HOLE1);
	item->animNumber = Objects[item->objectNumber].animIndex;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->requiredAnimState = 0;
	item->goalAnimState = g_Level.Anims[item->animNumber].currentAnimState;
	item->currentAnimState = g_Level.Anims[item->animNumber].currentAnimState;

	AddActiveItem(itemNum);

	item->flags |= IFLAG_ACTIVATION_MASK;
	item->status = ITEM_ACTIVE;
}

void do_puzzle()
{
	puzzleItem = (short)Lara.generalPtr;
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

	if ((!(TrInput & IN_ACTION) && g_Inventory.GetSelectedObject() == NO_ITEM
		|| BinocularRange
		|| Lara.gunStatus
		|| l->currentAnimState != LS_STOP
		|| l->animNumber != LA_STAND_IDLE)
		&& (!Lara.isMoving || (short)Lara.generalPtr != itemNum))
	{
		if (item->objectNumber < ID_KEY_HOLE6)
			ObjectCollision(itemNum, l, coll);
	}
	else
	{
		if (TestLaraPosition(&KeyHoleBounds, item, l))
		{
			if (!Lara.isMoving)
			{
				if (item->status != ITEM_NOT_ACTIVE)
					return;
				if (g_Inventory.GetSelectedObject() == NO_ITEM)
				{
					if (g_Inventory.IsObjectPresentInInventory(item->objectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1)))
						g_Inventory.SetEnterObject(item->objectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1));
					return;
				}
				if (g_Inventory.GetSelectedObject() != item->objectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1))
					return;
			}

			if (MoveLaraPosition(&KeyHolePosition, item, l))
			{
				if (item->objectNumber == ID_KEY_HOLE8)
					l->animNumber = LA_KEYCARD_USE;
				else
				{
					RemoveObjectFromInventory(item->objectNumber - (ID_KEY_HOLE1 - ID_KEY_ITEM1), 1);
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
					g_Inventory.SetSelectedObject(NO_ITEM);
					return;
				}
			}
			else
			{
				Lara.generalPtr = (void*)itemNum;
			}

			g_Inventory.SetSelectedObject(NO_ITEM);
			return;
		}

		if (Lara.isMoving && (short)Lara.generalPtr == itemNum)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		}
	}

	return;
}
int KeyTrigger(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	int oldkey;

	if ((item->status != ITEM_ACTIVE || Lara.gunStatus == LG_HANDS_BUSY) && (!KeyTriggerActive || Lara.gunStatus != LG_HANDS_BUSY))
		return -1;

	oldkey = KeyTriggerActive;

	if (!oldkey)
		item->status = ITEM_DEACTIVATED;

	KeyTriggerActive = false;

	return oldkey;
}