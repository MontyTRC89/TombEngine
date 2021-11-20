#include "framework.h"
#include "generic_doors.h"
#include "level.h"
#include "control/control.h"
#include "control/box.h"
#include "items.h"
#include "control/lot.h"
#include "gui.h"
#include "input.h"
#include "pickup.h"
#include "sound.h"
#include "animation.h"
#include "sphere.h"
#include "cog_switch.h"
#include "objectslist.h"
#include "lara_struct.h"
#include "lara.h"
#include "trmath.h"
#include "misc.h"
#include "itemdata/door_data.h"
#include "collide.h"
#include "itemdata/itemdata.h"

namespace TEN::Entities::Doors
{
	PHD_VECTOR CrowbarDoorPos(-412, 0, 256);

	OBJECT_COLLISION_BOUNDS CrowbarDoorBounds =
	{
		-512, 512, 
		-1024, 0, 
		0, 512, 
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80)
	};

	void InitialiseDoor(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->objectNumber == ID_SEQUENCE_DOOR1)
			item->flags &= 0xBFFFu;

		if (item->objectNumber == ID_LIFT_DOORS1 || item->objectNumber == ID_LIFT_DOORS2)
			item->itemFlags[0] = 4096;

		item->data = ITEM_DATA(DOOR_DATA());
		DOOR_DATA* door = item->data;

		door->opened = false;
		door->dptr1 = nullptr;
		door->dptr2 = nullptr;
		door->dptr3 = nullptr;
		door->dptr4 = nullptr;

		short boxNumber, twoRoom;

		int xOffset = 0;
		int zOffset = 0;

		if (item->pos.yRot == 0)
			zOffset = -WALL_SIZE;
		else if (item->pos.yRot == ANGLE(180))
			zOffset = WALL_SIZE;
		else if (item->pos.yRot == ANGLE(90))
			xOffset = -WALL_SIZE;
		else
			xOffset = WALL_SIZE;

		auto r = &g_Level.Rooms[item->roomNumber];
		door->d1.floor = GetSector(r, item->pos.xPos - r->x + xOffset, item->pos.zPos - r->z + zOffset);

		auto roomNumber = door->d1.floor->WallPortal;
		if (roomNumber == NO_ROOM)
			boxNumber = door->d1.floor->Box;
		else
		{
			auto b = &g_Level.Rooms[roomNumber];
			boxNumber = GetSector(b, item->pos.xPos - b->x + xOffset, item->pos.zPos - b->z + zOffset)->Box;
		}

		door->d1.block = (boxNumber != NO_BOX && g_Level.Boxes[boxNumber].flags & BLOCKABLE) ? boxNumber : NO_BOX; 
		door->d1.data = *door->d1.floor;

		if (r->flippedRoom != -1)
		{
			r = &g_Level.Rooms[r->flippedRoom];
			door->d1flip.floor = GetSector(r, item->pos.xPos - r->x + xOffset, item->pos.zPos - r->z + zOffset);
				
			roomNumber = door->d1flip.floor->WallPortal;
			if (roomNumber == NO_ROOM)
				boxNumber = door->d1flip.floor->Box;
			else
			{
				auto b = &g_Level.Rooms[roomNumber];
				boxNumber = GetSector(b, item->pos.xPos - b->x + xOffset, item->pos.zPos - b->z + zOffset)->Box;
			}

			door->d1flip.block = (boxNumber != NO_BOX && g_Level.Boxes[boxNumber].flags & BLOCKABLE) ? boxNumber : NO_BOX;
			door->d1flip.data = *door->d1flip.floor;
		}
		else
			door->d1flip.floor = NULL;

		twoRoom = door->d1.floor->WallPortal;

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
			door->d2.floor = GetSector(r, item->pos.xPos - r->x, item->pos.zPos - r->z);

			roomNumber = door->d2.floor->WallPortal;
			if (roomNumber == NO_ROOM)
				boxNumber = door->d2.floor->Box;
			else
			{
				auto b = &g_Level.Rooms[roomNumber];
				boxNumber = GetSector(b, item->pos.xPos - b->x, item->pos.zPos - b->z)->Box;
			}

			door->d2.block = (boxNumber != NO_BOX && g_Level.Boxes[boxNumber].flags & BLOCKABLE) ? boxNumber : NO_BOX;
			door->d2.data = *door->d2.floor;

			if (r->flippedRoom != -1)
			{
				r = &g_Level.Rooms[r->flippedRoom];
				door->d2flip.floor = GetSector(r, item->pos.xPos - r->x, item->pos.zPos - r->z);

				roomNumber = door->d2flip.floor->WallPortal;
				if (roomNumber == NO_ROOM)
					boxNumber = door->d2flip.floor->Box;
				else
				{
					auto b = &g_Level.Rooms[roomNumber];
					boxNumber = GetSector(b, item->pos.xPos - b->x, item->pos.zPos - b->z)->Box;
				}

				door->d2flip.block = (boxNumber != NO_BOX && g_Level.Boxes[boxNumber].flags & BLOCKABLE) ? boxNumber : NO_BOX; 
				door->d2flip.data = *door->d2flip.floor;
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

	void DoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (item->triggerFlags == 2
			&& item->status == ITEM_NOT_ACTIVE && !item->gravityStatus // CHECK
			&& ((TrInput & IN_ACTION || g_Gui.GetInventoryItemChosen() == ID_CROWBAR_ITEM)
				&& l->currentAnimState == LS_STOP
				&& l->animNumber == LA_STAND_IDLE
				&& !l->hitStatus
				&& Lara.gunStatus == LG_NO_ARMS
				|| Lara.isMoving && Lara.interactedItem == itemNum))
		{
			item->pos.yRot ^= ANGLE(180);
			if (TestLaraPosition(&CrowbarDoorBounds, item, l))
			{
				if (!Lara.isMoving)
				{
					if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
					{
						if (g_Gui.IsObjectInInventory(ID_CROWBAR_ITEM))
						{
							g_Gui.SetEnterInventory(ID_CROWBAR_ITEM);
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

					if (g_Gui.GetInventoryItemChosen() != ID_CROWBAR_ITEM)
					{
						item->pos.yRot ^= ANGLE(180);
						return;
					}
				}

				g_Gui.SetInventoryItemChosen(NO_ITEM);

				if (MoveLaraPosition(&CrowbarDoorPos, item, l))
				{
					SetAnimation(l, LA_DOOR_OPEN_CROWBAR);
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

		if (TestBoundsCollide(item, l, coll->Setup.Radius))
		{
			if (TestCollision(item, l))
			{
				if (coll->Setup.EnableObjectPush)
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

		// Doors with OCB = 1 are raisable with cog switchs
		if (item->triggerFlags == 1)
		{
			if (item->itemFlags[0])
			{
				BOUNDING_BOX* bounds = GetBoundsAccurate(item);
			
				item->itemFlags[0]--;
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
				if (item->pos.yPos < item->startPos.yPos)
					item->pos.yPos += 4;
				if (item->pos.yPos >= item->startPos.yPos)
				{
					item->pos.yPos = item->startPos.yPos;
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

			return;
		}

		if (item->objectNumber < ID_LIFT_DOORS1 || item->objectNumber > ID_LIFT_DOORS2)
		{
			if (TriggerActive(item))
			{
				if (item->currentAnimState == 0)
				{
					item->goalAnimState = 1;
				}
				else if (!door->opened)
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
				}
				else if (door->opened)
				{
					ShutThatDoor(&door->d1, door);
					ShutThatDoor(&door->d2, door);
					ShutThatDoor(&door->d1flip, door);
					ShutThatDoor(&door->d2flip, door);
					door->opened = false;
				}
			}
		}
		else
		{
			// TR5 lift doors
			/*if (!TriggerActive(item))
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
			}*/
		}

		AnimateItem(item);
	}

	void OpenThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd)
	{
		FLOOR_INFO* floor = doorPos->floor;

		if (floor != NULL)
		{
			*doorPos->floor = doorPos->data;

			short boxIndex = doorPos->block;
			if (boxIndex != NO_BOX)
			{
				g_Level.Boxes[boxIndex].flags &= ~BLOCKED;

				for (int i = 0; i < ActiveCreatures.size(); i++)
				{
					ActiveCreatures[i]->LOT.targetBox = NO_BOX;
				}
			}
		}
	}

	void ShutThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd)
	{
		FLOOR_INFO* floor = doorPos->floor;

		if (floor)
		{
			floor->Box = NO_BOX;
			floor->TriggerIndex = 0;

			// FIXME: HACK!!!!!!!
			// We should find a better way of dealing with doors using new floordata.

			floor->WallPortal = -1;
			floor->FloorCollision.Portals[0]   = NO_ROOM;
			floor->FloorCollision.Portals[1]   = NO_ROOM;
			floor->CeilingCollision.Portals[0] = NO_ROOM;
			floor->CeilingCollision.Portals[1] = NO_ROOM;
			floor->FloorCollision.Planes[0]    = WALL_PLANE;
			floor->FloorCollision.Planes[1]    = WALL_PLANE;
			floor->CeilingCollision.Planes[0]  = WALL_PLANE;
			floor->CeilingCollision.Planes[1]  = WALL_PLANE;

			short boxIndex = doorPos->block;
			if (boxIndex != NO_BOX)
			{
				g_Level.Boxes[boxIndex].flags |= BLOCKED;

				for (int i = 0; i < ActiveCreatures.size(); i++)
				{
					ActiveCreatures[i]->LOT.targetBox = NO_BOX;
				}
			}
		}
	}
}