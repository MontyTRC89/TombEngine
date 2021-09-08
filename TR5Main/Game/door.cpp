#include "framework.h"
#include "door.h"
#include "items.h"
#include "lot.h"
#include "objects.h"
#include "Lara.h"
#ifdef NEW_INV
#include "newinv2.h"
#else
#include "inventory.h"
#endif
#include "draw.h"
#include "sphere.h"
#include "misc.h"
#include "box.h"
#include "level.h"
#include "input.h"
#include "Sound\sound.h"
#include "trmath.h"
#include "cog_switch.h"
#include "generic_switch.h"
#include "pickup\pickup.h"
#include "fullblock_switch.h"

using namespace TEN::Entities::Switches;

PHD_VECTOR DoubleDoorPos(0, 0, 220);
PHD_VECTOR PullDoorPos(-201, 0, 322);
PHD_VECTOR PushDoorPos(201, 0, -702);
PHD_VECTOR KickDoorPos(0, 0, -917);
PHD_VECTOR UnderwaterDoorPos(-251, -540, -46);
PHD_VECTOR CrowbarDoorPos(-412, 0, 256);

OBJECT_COLLISION_BOUNDS PushPullKickDoorBounds =
{
	0xFE80, 0x0180, 0x0000, 0x0000, 0xFC00, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554, 0xF8E4, 0x071C
};

OBJECT_COLLISION_BOUNDS UnderwaterDoorBounds =
{
	0xFF00, 0x0100, 0xFC00, 0x0000, 0xFC00, 0x0000, 0xC720, 0x38E0, 0xC720, 0x38E0, 0xC720, 0x38E0
};

OBJECT_COLLISION_BOUNDS CrowbarDoorBounds =
{
	0xFE00, 0x0200, 0xFC00, 0x0000, 0x0000, 0x0200, 0xC720, 0x38E0, 0xC720, 0x38E0, 0xC720, 0x38E0
};

ITEM_INFO* ClosedDoors[32];
byte LiftDoor;
int DontUnlockBox;

#ifndef NEW_INV
extern Inventory g_Inventory;
#endif

void SequenceDoorControl(short itemNumber) 
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	DOOR_DATA* door = (DOOR_DATA*)item->data;

	if (CurrentSequence == 3)
	{
		if (SequenceResults[Sequences[0]][Sequences[1]][Sequences[2]] == item->triggerFlags)
		{
			if (item->currentAnimState == 1)
				item->goalAnimState = 1;
			else
				item->goalAnimState = 0;

			TestTriggers(item, true, NULL);
		}

		CurrentSequence = 4;
	}	

	if (item->currentAnimState == item->goalAnimState)
	{
		if (item->currentAnimState == 1)
		{
			if (!door->opened)
			{
				OpenThatDoor(&door->d1, door);
				OpenThatDoor(&door->d2, door);
				OpenThatDoor(&door->d1flip, door);
				OpenThatDoor(&door->d2flip, door);
				door->opened = true;
				item->flags |= 0x3E;
			}
		}
		else
		{
			if (door->opened)
			{
				ShutThatDoor(&door->d1, door);
				ShutThatDoor(&door->d2, door);
				ShutThatDoor(&door->d1flip, door);
				ShutThatDoor(&door->d2flip, door);
				door->opened = false;
				item->flags &= 0xC1;
			}
		}
	}

	AnimateItem(item);
}

void UnderwaterDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	
	if (TrInput & IN_ACTION
	&&  l->currentAnimState == LS_UNDERWATER_STOP
	&&  !(item->status && item->gravityStatus)
	&&  Lara.waterStatus == LW_UNDERWATER
	&&  !Lara.gunStatus
	||  Lara.isMoving && Lara.interactedItem == itemNum)
	{
		l->pos.yRot ^= ANGLE(180.0f);
		
		if (TestLaraPosition(&UnderwaterDoorBounds, item, l))
		{
			if (MoveLaraPosition(&UnderwaterDoorPos, item, l))
			{
				l->animNumber = LA_UNDERWATER_DOOR_OPEN;
				l->frameNumber = GF(LA_UNDERWATER_DOOR_OPEN, 0);
				l->currentAnimState = LS_MISC_CONTROL;
				l->fallspeed = 0;
				item->status = ITEM_ACTIVE;
				AddActiveItem(itemNum);
				item->goalAnimState = LS_RUN_FORWARD;
				AnimateItem(item);
				Lara.isMoving = false;
				Lara.gunStatus = LG_HANDS_BUSY;
			}
			else
			{
				Lara.interactedItem = itemNum;
			}
			l->pos.yRot ^= ANGLE(180);
		}
		else
		{
			if (Lara.isMoving && Lara.interactedItem == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
			l->pos.yRot ^= ANGLE(180);
		}
	}
	else if (item->status == ITEM_ACTIVE)
	{
		ObjectCollision(itemNum, l, coll);
	}
}

void DoubleDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (TrInput & IN_ACTION
	&&  l->currentAnimState == LS_STOP
	&&  l->animNumber == LA_STAND_IDLE
	&&  !(item->status && item->gravityStatus)
	&&  !(l->hitStatus)
	&&  !Lara.gunStatus
	||  Lara.isMoving && Lara.interactedItem == itemNum)
	{
		item->pos.yRot ^= ANGLE(180);
		if (TestLaraPosition(&PushPullKickDoorBounds, item, l))
		{
			if (MoveLaraPosition(&DoubleDoorPos, item, l))
			{
				l->animNumber = LA_DOUBLEDOOR_OPEN_PUSH;
				l->frameNumber = GF(LA_DOUBLEDOOR_OPEN_PUSH, 0);
				l->currentAnimState = LS_DOUBLEDOOR_PUSH;
				
				AddActiveItem(itemNum);

				item->status = ITEM_ACTIVE;
				Lara.isMoving = false;
				Lara.gunStatus = LG_HANDS_BUSY;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
			}
			else
			{
				Lara.interactedItem = itemNum;
			}
			item->pos.yRot ^= ANGLE(180);
		}
		else
		{
			if (Lara.isMoving && Lara.interactedItem == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
			item->pos.yRot ^= ANGLE(180);
		}
	}
}

void PushPullKickDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	if (TrInput & IN_ACTION
	&&  l->currentAnimState == LS_STOP
	&&  l->animNumber == LA_STAND_IDLE
	&&  item->status != ITEM_ACTIVE
	&&  !(l->hitStatus)
	&&  !Lara.gunStatus
	||  Lara.isMoving && Lara.interactedItem == itemNum)
	{
		bool applyRot = false;

		if (l->roomNumber == item->roomNumber)
		{
			item->pos.yRot ^= ANGLE(180);
			applyRot = true;
		}

		if (!TestLaraPosition(&PushPullKickDoorBounds, item, l))
		{
			if (Lara.isMoving && Lara.interactedItem == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
			if (applyRot)
				item->pos.yRot ^= ANGLE(180);
			return;
		}

		if (applyRot)
		{
			if (!MoveLaraPosition(&PullDoorPos, item, l))
			{
				Lara.interactedItem = itemNum;
				item->pos.yRot ^= ANGLE(180);
				return;
			}

			l->animNumber = LA_DOOR_OPEN_PULL;
			l->frameNumber = GF(LA_DOOR_OPEN_PULL, 0);
			item->goalAnimState = 3;

			AddActiveItem(itemNum);

			item->status = ITEM_ACTIVE;
			l->currentAnimState = LS_MISC_CONTROL;
			l->goalAnimState = LS_STOP;
			Lara.isMoving = false;
			Lara.gunStatus = LG_HANDS_BUSY;

			if (applyRot)
				item->pos.yRot ^= ANGLE(180);
			return;
		}

		if (item->objectNumber >= ID_KICK_DOOR1)
		{
			if (MoveLaraPosition(&KickDoorPos, item, l))
			{
				l->animNumber = LA_DOOR_OPEN_KICK;
				l->frameNumber = GF(LA_DOOR_OPEN_KICK, 0);
				item->goalAnimState = 2;

				AddActiveItem(itemNum);

				item->status = ITEM_ACTIVE;
				l->currentAnimState = LS_MISC_CONTROL;
				l->goalAnimState = LS_STOP;
				Lara.isMoving = false;
				Lara.gunStatus = LG_HANDS_BUSY;

				if (applyRot)
					item->pos.yRot ^= ANGLE(180);
				return;
			}
		}
		else if (item->objectNumber == ID_PUSHPULL_DOOR1 || item->objectNumber == ID_PUSHPULL_DOOR2)
		{
			if (MoveLaraPosition(&PushDoorPos, item, l))
			{
				l->animNumber = LA_DOOR_OPEN_PUSH;
				l->frameNumber = GF(LA_DOOR_OPEN_PUSH, 0);
				item->goalAnimState = 2;

				AddActiveItem(itemNum);

				item->status = ITEM_ACTIVE;
				l->currentAnimState = LS_MISC_CONTROL;
				l->goalAnimState = LS_STOP;
				Lara.isMoving = false;
				Lara.gunStatus = LG_HANDS_BUSY;

				if (applyRot)
					item->pos.yRot ^= ANGLE(180);
			}
			return;
		}

		Lara.interactedItem = itemNum;
		return;
	}

	if (!item->currentAnimState)
		DoorCollision(itemNum, l, coll);
}

void PushPullKickDoorControl(short itemNumber)
{
	ITEM_INFO* item;
	DOOR_DATA* door;

	item = &g_Level.Items[itemNumber];
	door = (DOOR_DATA*)item->data;

	if (!door->opened)
	{
		OpenThatDoor(&door->d1, door);
		OpenThatDoor(&door->d2, door);
		OpenThatDoor(&door->d1flip, door);
		OpenThatDoor(&door->d2flip, door);
		door->opened = true;
	} 

	AnimateItem(item);
}

void DoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (item->triggerFlags == 2
	&&  item->status == ITEM_NOT_ACTIVE && !item->gravityStatus // CHECK
	&&  ((TrInput & IN_ACTION || 
#ifdef NEW_INV
		GLOBAL_inventoryitemchosen == ID_CROWBAR_ITEM
#else
		g_Inventory.GetSelectedObject() == ID_CROWBAR_ITEM
#endif
		)
	&&  l->currentAnimState == LS_STOP
	&&  l->animNumber == LA_STAND_IDLE
	&&  !l->hitStatus
	&&  Lara.gunStatus == LG_NO_ARMS
	||  Lara.isMoving && Lara.interactedItem == itemNum))
	{
		item->pos.yRot ^= ANGLE(180);
		if (TestLaraPosition(&CrowbarDoorBounds, item, l))
		{
			if (!Lara.isMoving)
			{
#ifdef NEW_INV
				if (GLOBAL_inventoryitemchosen == NO_ITEM)
#else
				if (g_Inventory.GetSelectedObject() == NO_ITEM)
#endif
				{
#ifdef NEW_INV
					if (have_i_got_object(ID_CROWBAR_ITEM))
					{
						GLOBAL_enterinventory = ID_CROWBAR_ITEM;
						item->pos.yRot ^= ANGLE(180);
					}
#else
					if (g_Inventory.IsObjectPresentInInventory(ID_CROWBAR_ITEM))
					{
						g_Inventory.SetEnterObject(ID_CROWBAR_ITEM);
						item->pos.yRot ^= ANGLE(180);
					}
#endif
					else
					{
						if (OldPickupPos.x != l->pos.xPos || OldPickupPos.y != l->pos.yPos || OldPickupPos.z != l->pos.zPos)
						{
							OldPickupPos.x = l->pos.xPos;
							OldPickupPos.y = l->pos.yPos;
							OldPickupPos.z = l->pos.zPos;
							SayNo();
						}
						item->pos.yRot ^= ANGLE(180);
					}
					return;
				}
#ifdef NEW_INV
				if (GLOBAL_inventoryitemchosen != ID_CROWBAR_ITEM)
#else
				if (g_Inventory.GetSelectedObject() != ID_CROWBAR_ITEM)
#endif
				{
					item->pos.yRot ^= ANGLE(180);
					return;
				}
			}
#ifdef NEW_INV
			GLOBAL_inventoryitemchosen = NO_ITEM;
#else
			g_Inventory.SetSelectedObject(NO_ITEM);
#endif
			if (MoveLaraPosition(&CrowbarDoorPos, item, l))
			{
				l->animNumber = LA_DOOR_OPEN_CROWBAR;
				l->frameNumber = GF(LA_DOOR_OPEN_CROWBAR, 0);
				l->currentAnimState = LS_MISC_CONTROL;
				item->pos.yRot ^= ANGLE(180);

				AddActiveItem(itemNum);

				item->flags |= IFLAG_ACTIVATION_MASK;
				item->status = ITEM_ACTIVE;
				item->goalAnimState = LS_RUN_FORWARD;
				Lara.isMoving = 0;
				Lara.gunStatus = LG_HANDS_BUSY;

				return;
			}

			Lara.interactedItem = itemNum;
		}
		else if (Lara.isMoving && Lara.interactedItem == itemNum)
		{
			Lara.isMoving = 0;
			Lara.gunStatus = LG_NO_ARMS;
		}

		item->pos.yRot ^= ANGLE(180);
	}

	if (TestBoundsCollide(item, l, coll->radius))
	{
		if (TestCollision(item, l))
		{
			if (coll->enableBaddiePush)
			{
				if (!(item->objectNumber >= ID_LIFT_DOORS1 && item->objectNumber <= ID_LIFT_DOORS2) || item->itemFlags[0])
				{
					ItemPushItem(item, l, coll, FALSE, TRUE);
				}
			}
		}
	}
}

void DoorControl(short itemNumber) 
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	DOOR_DATA* door = (DOOR_DATA*)item->data;
	
	if (item->triggerFlags == 1)
	{
		if (item->itemFlags[0])
		{
			BOUNDING_BOX* bounds = GetBoundsAccurate(item);
			--item->itemFlags[0];
			item->pos.yPos -= TEN::Entities::Switches::COG_DOOR_SPEED;
			int y = bounds->Y1 + item->itemFlags[2] - STEP_SIZE;
			if (item->pos.yPos < y)
			{
				item->pos.yPos = y;
				item->itemFlags[0] = 0;
			}
			if (!door->opened)
			{
				OpenThatDoor(&door->d1, door);
				OpenThatDoor(&door->d2, door);
				OpenThatDoor(&door->d1flip, door);
				OpenThatDoor(&door->d2flip, door);
				door->opened = true;
			}
		}
		else
		{
			if (item->pos.yPos < item->itemFlags[2])
				item->pos.yPos += 2;
			if (item->pos.yPos >= item->itemFlags[2])
			{
				item->pos.yPos = item->itemFlags[2];
				if (door->opened)
				{
					ShutThatDoor(&door->d1, door);
					ShutThatDoor(&door->d2, door);
					ShutThatDoor(&door->d1flip, door);
					ShutThatDoor(&door->d2flip, door);
					door->opened = false;
				}
			}
		}
	}

	if (item->objectNumber < ID_LIFT_DOORS1 || item->objectNumber > ID_LIFT_DOORS2)
	{
		if (TriggerActive(item))
		{
			if (!item->currentAnimState)
			{
				item->goalAnimState = 1;
				AnimateItem(item);
				return;
			}
			if (!door->opened)
			{
				OpenThatDoor(&door->d1, door);
				OpenThatDoor(&door->d2, door);
				OpenThatDoor(&door->d1flip, door);
				OpenThatDoor(&door->d2flip, door);
				door->opened = true;
			}
		}
		else
		{
			item->status = ITEM_ACTIVE;

			if (item->currentAnimState == 1)
			{
				item->goalAnimState = 0;
				AnimateItem(item);
				return;
			}

			if (door->opened)
			{
				ShutThatDoor(&door->d1, door);
				ShutThatDoor(&door->d2, door);
				ShutThatDoor(&door->d1flip, door);
				ShutThatDoor(&door->d2flip, door);
				door->opened = false;
			}
		}	
		AnimateItem(item);
		return;
	}

	if (!TriggerActive(item))
	{
		if (item->itemFlags[0] >= SECTOR(4))
		{
			if (door->opened)
			{
				ShutThatDoor(&door->d1, door);
				ShutThatDoor(&door->d2, door);
				ShutThatDoor(&door->d1flip, door);
				ShutThatDoor(&door->d2flip, door);
				door->opened = false;
			}
		}
		else
		{
			if (!item->itemFlags[0])
				SoundEffect(SFX_TR5_LIFT_DOORS, &item->pos, 0);
			item->itemFlags[0] += STEP_SIZE;
		}
	}
	else
	{
		if (item->itemFlags[0] > 0)
		{
			if (item->itemFlags[0] == SECTOR(4))
				SoundEffect(SFX_TR5_LIFT_DOORS, &item->pos, 0);
			item->itemFlags[0] -= STEP_SIZE;
		}
		if (!door->opened)
		{
			DontUnlockBox = true;
			OpenThatDoor(&door->d1, door);
			OpenThatDoor(&door->d2, door);
			OpenThatDoor(&door->d1flip, door);
			OpenThatDoor(&door->d2flip, door);
			DontUnlockBox = false;
			door->opened = true;
		}
	}
}

void OpenThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd) 
{
	FLOOR_INFO* floor = doorPos->floor;

	if (floor != NULL)
	{
		memcpy(doorPos->floor, &doorPos->data, sizeof(FLOOR_INFO));
		
		short boxIndex = doorPos->block;
		if (boxIndex != NO_BOX)
		{
			if (!DontUnlockBox)
				g_Level.Boxes[boxIndex].flags &= ~BLOCKED;

			for (int i = 0; i < NUM_SLOTS; i++)
			{
				BaddieSlots[i].LOT.targetBox = NO_BOX;
			}
		}
	}

	if (dd->dptr1)
	{
		short n = dd->dn1 < 0 ? -1 : 1;
		if (dd->dn1 & 1)
		{
			dd->dptr1[0] = n;
		}
		else if (dd->dn1 & 2)
		{
			dd->dptr1[1] = n;
		}
		else
		{
			dd->dptr1[2] = n;
		}
		
		n = dd->dn3 < 0 ? -1 : 1;
		if (dd->dn3 & 1)
		{
			dd->dptr3[0] = n;
		}
		else if (dd->dn3 & 2)
		{
			dd->dptr3[1] = n;
		}
		else
		{
			dd->dptr3[2] = n;
		}
		
		if (dd->dptr2)
		{
			n = dd->dn2 < 0 ? -1 : 1;
			if (dd->dn2 & 1)
			{
				dd->dptr2[0] = n;
			}
			else if (dd->dn2 & 2)
			{
				dd->dptr2[1] = n;
			}
			else
			{
				dd->dptr2[2] = n;
			}
		}
		
		if (dd->dptr4)
		{
			n = dd->dn4 < 0 ? -1 : 1;
			if (dd->dn4 & 1)
			{
				dd->dptr4[0] = n;
			}
			else if (dd->dn4 & 2)
			{
				dd->dptr4[1] = n;
			}
			else
			{
				dd->dptr4[2] = n;
			}
		}
	}
}

void ShutThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd) 
{
	FLOOR_INFO* floor = doorPos->floor;

	if (floor)
	{
		floor->box = NO_BOX;
		floor->ceiling = -127;
		floor->floor = -127;
		floor->index = 0;
		floor->skyRoom = NO_ROOM;
		floor->pitRoom = NO_ROOM;

		short boxIndex = doorPos->block;
		if (boxIndex != NO_BOX)
		{
			g_Level.Boxes[boxIndex].flags |= BLOCKED;
			for (int i = 0; i < NUM_SLOTS; i++)
			{
				BaddieSlots[i].LOT.targetBox = NO_BOX;
			}
		}
	}

	if (dd->dptr1)
	{
		dd->dptr1[0] = 0;
		dd->dptr1[1] = 0;
		dd->dptr1[2] = 0;

		dd->dptr3[0] = 0;
		dd->dptr3[1] = 0;
		dd->dptr3[2] = 0;

		if (dd->dptr2)
		{
			dd->dptr2[0] = 0;
			dd->dptr2[1] = 0;
			dd->dptr2[2] = 0;
		}
		
		if (dd->dptr4)
		{
			dd->dptr4[0] = 0;
			dd->dptr4[1] = 0;
			dd->dptr4[2] = 0;
		}
	}
}

void InitialiseDoor(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->objectNumber == ID_SEQUENCE_DOOR1)
		item->flags &= 0xBFFFu;

	if (item->objectNumber == ID_LIFT_DOORS1 || item->objectNumber == ID_LIFT_DOORS2)
		item->itemFlags[0] = 4096;

	DOOR_DATA * door = game_malloc<DOOR_DATA>();

	item->data = door;
	door->opened = false;
	door->dptr1 = NULL;
	door->dptr2 = NULL;
	door->dptr3 = NULL;
	door->dptr4 = NULL;

	int dz, dx;
	ROOM_INFO* r;
	ROOM_INFO* b;
	short boxNumber, twoRoom, roomNumber;

	dz = dx = 0;

	if (item->pos.yRot == 0)
		dz--;
	else if (item->pos.yRot == -0x8000)
		dz++;
	else if (item->pos.yRot == 0x4000)
		dx--;
	else
		dx++;

	r = &g_Level.Rooms[item->roomNumber];

	door->d1.floor = &r->floor[(item->pos.zPos - r->z) / SECTOR(1) + dz + ((item->pos.xPos - r->x) / SECTOR(1) + dx) * r->xSize];
	roomNumber = GetDoor(door->d1.floor);
	if (roomNumber == NO_ROOM)
		boxNumber = door->d1.floor->box;
	else
	{
		b = &g_Level.Rooms[roomNumber];
		boxNumber = b->floor[(item->pos.zPos - b->z) / SECTOR(1) + dz + ((item->pos.xPos - b->x) / SECTOR(1) + dx) * b->xSize].box;
	}
	door->d1.block = (boxNumber != NO_BOX && g_Level.Boxes[boxNumber].flags & BLOCKABLE) ? boxNumber : NO_BOX;

	memcpy(&door->d1.data, door->d1.floor, sizeof(FLOOR_INFO));

	if (r->flippedRoom != -1)
	{
		r = &g_Level.Rooms[r->flippedRoom];

		door->d1flip.floor = &r->floor[(item->pos.zPos - r->z) / SECTOR(1) + dz + ((item->pos.xPos - r->x) / SECTOR(1) + dx) * r->xSize];
		roomNumber = GetDoor(door->d1flip.floor);
		if (roomNumber == NO_ROOM)
			boxNumber = door->d1flip.floor->box;
		else
		{
			b = &g_Level.Rooms[roomNumber];
			boxNumber = b->floor[(item->pos.zPos - b->z) / SECTOR(1) + dz + ((item->pos.xPos - b->x) / SECTOR(1) + dx) * b->xSize].box;
		}
		door->d1flip.block = (boxNumber != NO_BOX && g_Level.Boxes[boxNumber].flags & BLOCKABLE) ? boxNumber : NO_BOX;

		memcpy(&door->d1flip.data, door->d1flip.floor, sizeof(FLOOR_INFO));
	}
	else
		door->d1flip.floor = NULL;

	twoRoom = GetDoor(door->d1.floor);

	ShutThatDoor(&door->d1, door);
	ShutThatDoor(&door->d1flip, door);

	if (twoRoom == NO_ROOM)
	{
		door->d2.floor = NULL;
		door->d2flip.floor = NULL;
	}
	else
	{
		r = &g_Level.Rooms[twoRoom];

		door->d2.floor = &r->floor[(item->pos.zPos - r->z) / SECTOR(1) + (item->pos.xPos - r->x) / SECTOR(1) * r->xSize];
		roomNumber = GetDoor(door->d2.floor);
		if (roomNumber == NO_ROOM)
			boxNumber = door->d2.floor->box;
		else
		{
			b = &g_Level.Rooms[roomNumber];
			boxNumber = b->floor[(item->pos.zPos - b->z) / SECTOR(1) + (item->pos.xPos - b->x) / SECTOR(1) * b->xSize].box;
		}
		door->d2.block = (boxNumber != NO_BOX && g_Level.Boxes[boxNumber].flags & BLOCKABLE) ? boxNumber : NO_BOX;

		memcpy(&door->d2.data, door->d2.floor, sizeof(FLOOR_INFO));

		if (r->flippedRoom != -1)
		{
			r = &g_Level.Rooms[r->flippedRoom];

			door->d2flip.floor = &r->floor[(item->pos.zPos - r->z) / SECTOR(1) + (item->pos.xPos - r->x) / SECTOR(1) * r->xSize];
			roomNumber = GetDoor(door->d2flip.floor);
			if (roomNumber == NO_ROOM)
				boxNumber = door->d2flip.floor->box;
			else
			{
				b = &g_Level.Rooms[roomNumber];
				boxNumber = b->floor[(item->pos.zPos - b->z) / SECTOR(1) + (item->pos.xPos - b->x) / SECTOR(1) * b->xSize].box;
			}
			door->d2flip.block = (boxNumber != NO_BOX && g_Level.Boxes[boxNumber].flags & BLOCKABLE) ? boxNumber : NO_BOX;

			memcpy(&door->d2flip.data, door->d2flip.floor, sizeof(FLOOR_INFO));
		}
		else
			door->d2flip.floor = NULL;

		ShutThatDoor(&door->d2, door);
		ShutThatDoor(&door->d2flip, door);

		roomNumber = item->roomNumber;
		ItemNewRoom(itemNumber, twoRoom);
		item->roomNumber = roomNumber;
		item->inDrawRoom = true;
	}
}

void InitialiseClosedDoors()
{
	ZeroMemory(ClosedDoors, 32 * sizeof(ITEM_INFO*));
}

void FillDoorPointers(DOOR_DATA* doorData, ITEM_INFO* item, short roomNumber, int dz, int dx)
{
	int absX = dx * SECTOR(1) + item->pos.xPos;
	int absZ = dz * SECTOR(1) + item->pos.zPos;

	dx *= SECTOR(1);
	dz *= SECTOR(1);

	ROOM_INFO* r = &g_Level.Rooms[item->roomNumber];
	GetClosedDoorNormal(r, &doorData->dptr1, &doorData->dn1, dz, dx, absX, absZ);

	if (r->flippedRoom != -1)
		GetClosedDoorNormal(&g_Level.Rooms[r->flippedRoom], &doorData->dptr2, &doorData->dn2, dz, dx, absX, absZ);
	
	r = &g_Level.Rooms[roomNumber];
	GetClosedDoorNormal(r, &doorData->dptr3, &doorData->dn3, dz, dx, absX, absZ);

	if (r->flippedRoom != -1)
		GetClosedDoorNormal(&g_Level.Rooms[r->flippedRoom], &doorData->dptr4, &doorData->dn4, dz, dx, absX, absZ);
}

void GetClosedDoorNormal(ROOM_INFO* room, short** dptr, byte* n, int z, int x, int absX, int absZ)
{
	/**dptr = NULL;

		int halfX = x >> 1;
		int halfZ = z >> 1;

		for (int i = 0; i < room->doors.size(); i++)
		{
			ROOM_DOOR door = room->doors[i];

			int x1 = halfX + room->x + ((int)door.vertices[0].x + 128) & 0xFFFFFF00;
			int x2 = halfX + room->x + ((int)door.vertices[2].x + 128) & 0xFFFFFF00;

			if (x1 > x2)
			{
				int temp = x1;
				x1 = x2;
				x2 = temp;
			}

			int z1 = halfZ + room->z + ((int)door.vertices[0].z + 128) & 0xFFFFFF00;
			int z2 = halfZ + room->z + ((int)door.vertices[2].z + 128) & 0xFFFFFF00;

			if (z1 > z2)
			{
				int temp = z1;
				z1 = z2;
				z2 = temp;
			}

			if (absX >= x1 && absX <= x2 && absZ >= z1 && absZ <= z2)
			{
				*dptr = &door[1];

				if (door[1])
				{
					*n = (byte)door[1] & 0x81 | 1;
				}
				else if (door[2])
				{
					*n = (byte)door[2] & 0x82 | 2;
				}
				else
				{
					*n = (byte)door[3] & 0x84 | 4;
				}
			}
		}*/
}

void ProcessClosedDoors()
{
	/*for (int i = 0; i < 32; i++)
	{
		ITEM_INFO* item = ClosedDoors[i];
		
		if (item == NULL)
			break;

		short roomNumber = item->roomNumber;
		if (!g_Level.Rooms[roomNumber].boundActive && !g_Level.Rooms[item->drawRoom].boundActive)
			continue;

		if (g_Level.Rooms[item->drawRoom].boundActive)
		{
			if (!(item->inDrawRoom))
			{
				ItemNewRoom(item - Items, item->drawRoom);
				item->roomNumber = roomNumber;
				item->inDrawRoom = true;
			}
		}
		else if (item->inDrawRoom)
		{
			item->roomNumber = item->drawRoom;
			ItemNewRoom(item - Items, roomNumber);
			item->inDrawRoom = false;
		}
	}*/
}
// keeping these cocmments for now in case they're actually needed?

void AssignClosedDoor(ITEM_INFO* item)
{
	for (int i = 0; i < 32; i++)
	{
		if (ClosedDoors[i] == NULL)
		{
			ClosedDoors[i] = item;
			return;
		}
	}
}

void InitialiseSteelDoor(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->meshBits = 1;
	item->pos.yPos -= 1024;
}

void SteelDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	
	if (item->itemFlags[0] != 3)
	{
		if (TestBoundsCollide(item, l, coll->radius))
		{
			if (TestCollision(item, l))
			{
				if (coll->enableBaddiePush)
					ItemPushItem(item, l, coll, 0, 1);
			}
		}
	}
}
