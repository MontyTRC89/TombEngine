#include "framework.h"
#include "door.h"
#include "items.h"
#include "lot.h"
#include "objects.h"
#include "Lara.h"
#include "inventory.h"
#include "draw.h"
#include "sphere.h"
#include "switch.h"
#include "misc.h"
#include "box.h"
#include "level.h"
#include "input.h"
#include "sound.h"
#include "trmath.h"

PHD_VECTOR DoubleDoorPos(0, 0, 220);
PHD_VECTOR PullDoorPos(-201, 0, 322);
PHD_VECTOR PushDoorPos(201, 0, -702);
PHD_VECTOR KickDoorPos(0, 0, -917);
PHD_VECTOR UnderwaterDoorPos(-251, -540, -46);
PHD_VECTOR CrowbarDoorPos(-412, 0, 256);

static short PushPullKickDoorBounds[12] =
{
	0xFE80, 0x0180, 0x0000, 0x0000, 0xFC00, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554, 0xF8E4, 0x071C
};

static short UnderwaterDoorBounds[12] =
{
	0xFF00, 0x0100, 0xFC00, 0x0000, 0xFC00, 0x0000, 0xC720, 0x38E0, 0xC720, 0x38E0, 0xC720, 0x38E0
};

static short CrowbarDoorBounds[12] =
{
	0xFE00, 0x0200, 0xFC00, 0x0000, 0x0000, 0x0200, 0xC720, 0x38E0, 0xC720, 0x38E0, 0xC720, 0x38E0
};

ITEM_INFO* ClosedDoors[32];
byte LiftDoor;
int DontUnlockBox;

extern byte SequenceUsed[6];
extern byte SequenceResults[3][3][3];
extern byte Sequences[3];
extern byte CurrentSequence;
extern PHD_VECTOR OldPickupPos;
extern Inventory g_Inventory;

void SequenceDoorControl(short itemNumber) 
{
	ITEM_INFO* item = &Items[itemNumber];
	DOOR_DATA* door = (DOOR_DATA*)item->data;

	if (CurrentSequence == 3)
	{
		if (SequenceResults[Sequences[0]][Sequences[1]][Sequences[2]] == item->triggerFlags)
		{
			if (item->currentAnimState == 1)
				item->goalAnimState = 1;
			else
				item->goalAnimState = 0;

			TestTriggersAtXYZ(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, TRUE, 0);
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
				door->opened = TRUE;
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
				door->opened = FALSE;
				item->flags &= 0xC1;
			}
		}
	}

	AnimateItem(item);
}

void UnderwaterDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &Items[itemNum];
	
	if (TrInput & IN_ACTION
	&&  l->currentAnimState == STATE_LARA_UNDERWATER_STOP
	&&  !(item->status && item->gravityStatus)
	&&  Lara.waterStatus == LW_UNDERWATER
	&&  !Lara.gunStatus
	||  Lara.isMoving && Lara.generalPtr == (void*)itemNum)
	{
		l->pos.yRot ^= ANGLE(180.0f);
		
		if (TestLaraPosition(UnderwaterDoorBounds, item, l))
		{
			if (MoveLaraPosition(&UnderwaterDoorPos, item, l))
			{
				l->animNumber = ANIMATION_LARA_UNDERWATER_DOOR_OPEN;
				l->frameNumber = GF(ANIMATION_LARA_UNDERWATER_DOOR_OPEN, 0);
				l->currentAnimState = STATE_LARA_MISC_CONTROL;
				l->fallspeed = 0;
				item->status = ITEM_ACTIVE;
				AddActiveItem(itemNum);
				item->goalAnimState = STATE_LARA_RUN_FORWARD;
				AnimateItem(item);
				Lara.isMoving = false;
				Lara.gunStatus = LG_HANDS_BUSY;
			}
			else
			{
				Lara.generalPtr = (void*)itemNum;
			}
			l->pos.yRot ^= ANGLE(180);
		}
		else
		{
			if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
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
	ITEM_INFO* item = &Items[itemNum];

	if (TrInput & IN_ACTION
	&&  l->currentAnimState == STATE_LARA_STOP
	&&  l->animNumber == ANIMATION_LARA_STAY_IDLE
	&&  !(item->status && item->gravityStatus)
	&&  !(l->hitStatus)
	&&  !Lara.gunStatus
	||  Lara.isMoving && Lara.generalPtr == (void*)itemNum)
	{
		item->pos.yRot ^= ANGLE(180);
		if (TestLaraPosition(PushPullKickDoorBounds, item, l))
		{
			if (MoveLaraPosition(&DoubleDoorPos, item, l))
			{
				l->animNumber = ANIMATION_LARA_DOUBLEDOORS_PUSH;
				l->frameNumber = GF(ANIMATION_LARA_DOUBLEDOORS_PUSH, 0);
				l->currentAnimState = STATE_LARA_DOUBLEDOORS_PUSH;
				
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
				Lara.generalPtr = (void*)itemNum;
			}
			item->pos.yRot ^= ANGLE(180);
		}
		else
		{
			if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
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
	ITEM_INFO* item = &Items[itemNum];
	if (TrInput & IN_ACTION
	&&  l->currentAnimState == STATE_LARA_STOP
	&&  l->animNumber == ANIMATION_LARA_STAY_IDLE
	&&  item->status != ITEM_ACTIVE
	&&  !(l->hitStatus)
	&&  !Lara.gunStatus
	||  Lara.isMoving && Lara.generalPtr == (void*)itemNum)
	{
		bool applyRot = false;

		if (l->roomNumber == item->roomNumber)
		{
			item->pos.yRot ^= ANGLE(180);
			applyRot = true;
		}

		if (!TestLaraPosition(PushPullKickDoorBounds, item, l))
		{
			if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
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
				Lara.generalPtr = (void*)itemNum;
				item->pos.yRot ^= ANGLE(180);
				return;
			}

			l->animNumber = ANIMATION_LARA_DOOR_OPEN_BACK;
			l->frameNumber = GF(ANIMATION_LARA_DOOR_OPEN_BACK, 0);
			item->goalAnimState = 3;

			AddActiveItem(itemNum);

			item->status = ITEM_ACTIVE;
			l->currentAnimState = STATE_LARA_MISC_CONTROL;
			l->goalAnimState = STATE_LARA_STOP;
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
				l->animNumber = ANIMATION_LARA_DOOR_KICK;
				l->frameNumber = GF(ANIMATION_LARA_DOOR_KICK, 0);
				item->goalAnimState = 2;

				AddActiveItem(itemNum);

				item->status = ITEM_ACTIVE;
				l->currentAnimState = STATE_LARA_MISC_CONTROL;
				l->goalAnimState = STATE_LARA_STOP;
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
				l->animNumber = ANIMATION_LARA_DOOR_OPEN_FORWARD;
				l->frameNumber = GF(ANIMATION_LARA_DOOR_OPEN_FORWARD, 0);
				item->goalAnimState = 2;

				AddActiveItem(itemNum);

				item->status = ITEM_ACTIVE;
				l->currentAnimState = STATE_LARA_MISC_CONTROL;
				l->goalAnimState = STATE_LARA_STOP;
				Lara.isMoving = false;
				Lara.gunStatus = LG_HANDS_BUSY;

				if (applyRot)
					item->pos.yRot ^= ANGLE(180);
			}
			return;
		}

		Lara.generalPtr = (void*)itemNum;
		return;
	}

	if (!item->currentAnimState)
		DoorCollision(itemNum, l, coll);
}

void PushPullKickDoorControl(short itemNumber)
{
	ITEM_INFO* item;
	DOOR_DATA* door;

	item = &Items[itemNumber];
	door = (DOOR_DATA*)item->data;

	if (!door->opened)
	{
		OpenThatDoor(&door->d1, door);
		OpenThatDoor(&door->d2, door);
		OpenThatDoor(&door->d1flip, door);
		OpenThatDoor(&door->d2flip, door);
		door->opened = TRUE;
	} 

	AnimateItem(item);
}

void DoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->triggerFlags == 2
	&&  item->status == ITEM_NOT_ACTIVE && !item->gravityStatus // CHECK
	&&  ((TrInput & IN_ACTION || g_Inventory.GetSelectedObject() == ID_CROWBAR_ITEM)
	&&  l->currentAnimState == STATE_LARA_STOP
	&&  l->animNumber == ANIMATION_LARA_STAY_IDLE
	&&  !l->hitStatus
	&&  Lara.gunStatus == LG_NO_ARMS
	||  Lara.isMoving && Lara.generalPtr == (void*)itemNum))
	{
		item->pos.yRot ^= ANGLE(180);
		if (TestLaraPosition(CrowbarDoorBounds, item, l))
		{
			if (!Lara.isMoving)
			{
				if (g_Inventory.GetSelectedObject() == NO_ITEM)
				{
					if (g_Inventory.IsObjectPresentInInventory(ID_CROWBAR_ITEM))
					{
						g_Inventory.SetEnterObject(ID_CROWBAR_ITEM);
						item->pos.yRot ^= ANGLE(180);
					}
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
				if (g_Inventory.GetSelectedObject() != ID_CROWBAR_ITEM)
				{
					item->pos.yRot ^= ANGLE(180);
					return;
				}
			}

			g_Inventory.SetSelectedObject(NO_ITEM);
			if (MoveLaraPosition(&CrowbarDoorPos, item, l))
			{
				l->animNumber = ANIMATION_LARA_DOOR_OPEN_CROWBAR;
				l->frameNumber = GF(ANIMATION_LARA_DOOR_OPEN_CROWBAR, 0);
				l->currentAnimState = STATE_LARA_MISC_CONTROL;
				item->pos.yRot ^= ANGLE(180);

				AddActiveItem(itemNum);

				item->flags |= IFLAG_ACTIVATION_MASK;
				item->status = ITEM_ACTIVE;
				item->goalAnimState = STATE_LARA_RUN_FORWARD;
				Lara.isMoving = 0;
				Lara.gunStatus = LG_HANDS_BUSY;

				return;
			}

			Lara.generalPtr = (void*)itemNum;
		}
		else if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
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
					ItemPushLara(item, l, coll, FALSE, TRUE);
				}
			}
		}
	}
}

void DoorControl(short itemNumber) 
{
	ITEM_INFO* item = &Items[itemNumber];
	DOOR_DATA* door = (DOOR_DATA*)item->data;
	
	if (item->triggerFlags == 1)
	{
		if (item->itemFlags[0])
		{
			short* bounds = GetBoundsAccurate(item);
			--item->itemFlags[0];
			item->pos.yPos -= 12;
			int y = bounds[2] + item->itemFlags[2] - STEP_SIZE;
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
				door->opened = TRUE;
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
					door->opened = FALSE;
				}
			}
		}
		return;
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
				door->opened = TRUE;
			}
			/*if (item->frameNumber == Anims[item->animNumber].frameEnd)
			{
				if (gfCurrentLevel == 11)
				{
					v9 = item->object_number;
					if (v9 != 302 && v9 != 304)
					{
						LOBYTE(v5) = AnimateItem((int)item);
						return v5;
					}
				LABEL_40:
					v10 = item->_bf15ea;
					LOBYTE(v10) = v10 | 6;
					item->_bf15ea = v10;
					LOBYTE(v5) = AnimateItem((int)item);
					return v5;
				}
				if (gfCurrentLevel >= 0xCu && gfCurrentLevel <= 0xEu && item->object_number == 300)
					goto LABEL_40;
			}*/
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
				door->opened = FALSE;
			}
		}
		else
		{
			if (!item->itemFlags[0])
				SoundEffect(SFX_LIFT_DOORS, &item->pos, 0);
			item->itemFlags[0] += STEP_SIZE;
		}
	}
	else
	{
		if (item->itemFlags[0] > 0)
		{
			if (item->itemFlags[0] == SECTOR(4))
				SoundEffect(SFX_LIFT_DOORS, &item->pos, 0);
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
				Boxes[boxIndex].overlapIndex &= ~BLOCKED;

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
			Boxes[boxIndex].overlapIndex |= BLOCKED;
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
	ITEM_INFO* item = &Items[itemNumber];

	if (item->objectNumber == ID_SEQUENCE_DOOR1)
		item->flags &= 0xBFFFu;

	if (item->objectNumber == ID_LIFT_DOORS1 || item->objectNumber == ID_LIFT_DOORS2)
		item->itemFlags[0] = 4096;

	DOOR_DATA * door = (DOOR_DATA*)game_malloc(sizeof(DOOR_DATA));

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

	r = &Rooms[item->roomNumber];

	door->d1.floor = &r->floor[(((item->pos.zPos - r->z) >> WALL_SHIFT) + dz) + (((item->pos.xPos - r->x) >> WALL_SHIFT) + dx) * r->xSize];
	roomNumber = GetDoor(door->d1.floor);
	if (roomNumber == NO_ROOM)
		boxNumber = door->d1.floor->box;
	else
	{
		b = &Rooms[roomNumber];
		boxNumber = b->floor[(((item->pos.zPos - b->z) >> WALL_SHIFT) + dz) + (((item->pos.xPos - b->x) >> WALL_SHIFT) + dx) * b->xSize].box;
	}
	door->d1.block = (Boxes[boxNumber].overlapIndex & BLOCKABLE) ? boxNumber : NO_BOX;

	memcpy(&door->d1.data, door->d1.floor, sizeof(FLOOR_INFO));

	if (r->flippedRoom != -1)
	{
		r = &Rooms[r->flippedRoom];

		door->d1flip.floor = &r->floor[(((item->pos.zPos - r->z) >> WALL_SHIFT) + dz) + (((item->pos.xPos - r->x) >> WALL_SHIFT) + dx) * r->xSize];
		roomNumber = GetDoor(door->d1flip.floor);
		if (roomNumber == NO_ROOM)
			boxNumber = door->d1flip.floor->box;
		else
		{
			b = &Rooms[roomNumber];
			boxNumber = b->floor[(((item->pos.zPos - b->z) >> WALL_SHIFT) + dz) + (((item->pos.xPos - b->x) >> WALL_SHIFT) + dx) * b->xSize].box;
		}
		door->d1flip.block = (Boxes[boxNumber].overlapIndex & BLOCKABLE) ? boxNumber : NO_BOX;

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
		r = &Rooms[twoRoom];

		door->d2.floor = &r->floor[((item->pos.zPos - r->z) >> WALL_SHIFT) + ((item->pos.xPos - r->x) >> WALL_SHIFT) * r->xSize];
		roomNumber = GetDoor(door->d2.floor);
		if (roomNumber == NO_ROOM)
			boxNumber = door->d2.floor->box;
		else
		{
			b = &Rooms[roomNumber];
			boxNumber = b->floor[((item->pos.zPos - b->z) >> WALL_SHIFT) + ((item->pos.xPos - b->x) >> WALL_SHIFT) * b->xSize].box;
		}
		door->d2.block = (Boxes[boxNumber].overlapIndex & BLOCKABLE) ? boxNumber : NO_BOX;

		memcpy(&door->d2.data, door->d2.floor, sizeof(FLOOR_INFO));

		if (r->flippedRoom != -1)
		{
			r = &Rooms[r->flippedRoom];

			door->d2flip.floor = &r->floor[((item->pos.zPos - r->z) >> WALL_SHIFT) + ((item->pos.xPos - r->x) >> WALL_SHIFT) * r->xSize];
			roomNumber = GetDoor(door->d2flip.floor);
			if (roomNumber == NO_ROOM)
				boxNumber = door->d2flip.floor->box;
			else
			{
				b = &Rooms[roomNumber];
				boxNumber = b->floor[((item->pos.zPos - b->z) >> WALL_SHIFT) + ((item->pos.xPos - b->x) >> WALL_SHIFT) * b->xSize].box;
			}
			door->d2flip.block = (Boxes[boxNumber].overlapIndex & BLOCKABLE) ? boxNumber : NO_BOX;

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

	/*if (twoRoom != NO_ROOM && item->objectNumber >= ID_CLOSED_DOOR1 && item->objectNumber <= ID_LIFT_DOORS2)
	{
		FillDoorPointers(door, item, twoRoom, dz, dx);
		
		door->dptr1[0] = 0;
		door->dptr1[1] = 0;
		door->dptr1[2] = 0;

		door->dptr3[0] = 0;
		door->dptr3[1] = 0;
		door->dptr3[2] = 0;

		if (Rooms[item->roomNumber].flippedRoom != -1)
		{
			//if (!door->dptr2)
			//	MEMORY[1] = 1;
			
			door->dptr2[0] = 0;
			door->dptr2[1] = 0;
			door->dptr2[2] = 0;
		}

		if (Rooms[item->drawRoom].flippedRoom != -1)
		{
			//if (!door->dptr4)
			//	MEMORY[1] = 1;

			door->dptr4[0] = 0;
			door->dptr4[1] = 0;
			door->dptr4[2] = 0;
		}

		door->item = item;

		AssignClosedDoor(item);
	}*/
}

void InitialiseClosedDoors()
{
	ZeroMemory(ClosedDoors, 32 * sizeof(ITEM_INFO*));
}

void FillDoorPointers(DOOR_DATA* doorData, ITEM_INFO* item, short roomNumber, int dz, int dx)
{
	int absX = (dx << WALL_SHIFT) + item->pos.xPos;
	int absZ = (dz << WALL_SHIFT) + item->pos.zPos;

	dx <<= WALL_SHIFT;
	dz <<= WALL_SHIFT;

	ROOM_INFO* r = &Rooms[item->roomNumber];
	GetClosedDoorNormal(r, &doorData->dptr1, &doorData->dn1, dz, dx, absX, absZ);

	if (r->flippedRoom != -1)
		GetClosedDoorNormal(&Rooms[r->flippedRoom], &doorData->dptr2, &doorData->dn2, dz, dx, absX, absZ);
	
	r = &Rooms[roomNumber];
	GetClosedDoorNormal(r, &doorData->dptr3, &doorData->dn3, dz, dx, absX, absZ);

	if (r->flippedRoom != -1)
		GetClosedDoorNormal(&Rooms[r->flippedRoom], &doorData->dptr4, &doorData->dn4, dz, dx, absX, absZ);
}

void GetClosedDoorNormal(ROOM_INFO* room, short** dptr, byte* n, int z, int x, int absX, int absZ)
{
	*dptr = NULL;

	if (room->door)
	{
		if (room->door > 0)
		{
			int numDoors = *(room->door);
			short* door = &room->door[1];

			int halfX = x >> 1;
			int halfZ = z >> 1;

			for (int i = 0; i < numDoors; i++)
			{
				int x1 = halfX + room->x + (door[4] + 128) & 0xFFFFFF00;
				int x2 = halfX + room->x + (door[10] + 128) & 0xFFFFFF00;

				if (x1 > x2)
				{
					int temp = x1;
					x1 = x2;
					x2 = temp;
				}

				int z1 = halfZ + room->z + (door[6] + 128) & 0xFFFFFF00;
				int z2 = halfZ + room->z + (door[12] + 128) & 0xFFFFFF00;

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

				door += 16;
			}
		}
	}
}

void ProcessClosedDoors()
{
	for (int i = 0; i < 32; i++)
	{
		ITEM_INFO* item = ClosedDoors[i];
		
		if (item == NULL)
			break;

		short roomNumber = item->roomNumber;
		if (!Rooms[roomNumber].boundActive && !Rooms[item->drawRoom].boundActive)
			continue;

		if (Rooms[item->drawRoom].boundActive)
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
	}
}

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
	ITEM_INFO* item = &Items[itemNumber];

	item->meshBits = 1;
	item->pos.yPos -= 1024;
}

void SteelDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (item->itemFlags[0] != 3)
	{
		if (TestBoundsCollide(item, l, coll->radius))
		{
			if (TestCollision(item, l))
			{
				if (coll->enableBaddiePush)
					ItemPushLara(item, l, coll, 0, 1);
			}
		}
	}
}
