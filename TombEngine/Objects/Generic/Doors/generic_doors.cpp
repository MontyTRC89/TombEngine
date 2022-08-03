#include "framework.h"
#include "Objects/Generic/Doors/generic_doors.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/control/lot.h"
#include "Game/gui.h"
#include "Specific/Input/Input.h"
#include "Game/pickup/pickup.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/collision/sphere.h"
#include "Objects/Generic/Switches/cog_switch.h"
#include "Objects/objectslist.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Specific/trmath.h"
#include "Game/misc.h"
#include "Game/itemdata/door_data.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/itemdata/itemdata.h"

using namespace TEN::Gui;
using namespace TEN::Input;

namespace TEN::Entities::Doors
{
	Vector3Int CrowbarDoorPos(-412, 0, 256);

	OBJECT_COLLISION_BOUNDS CrowbarDoorBounds =
	{
		-512, 512, 
		-1024, 0, 
		0, 512, 
		-ANGLE(80.0f), ANGLE(80.0f),
		-ANGLE(80.0f), ANGLE(80.0f),
		-ANGLE(80.0f), ANGLE(80.0f)
	};

	void InitialiseDoor(short itemNumber)
	{
		auto* doorItem = &g_Level.Items[itemNumber];

		if (doorItem->ObjectNumber == ID_SEQUENCE_DOOR1)
			doorItem->Flags &= 0xBFFFu;

		if (doorItem->ObjectNumber == ID_LIFT_DOORS1 || doorItem->ObjectNumber == ID_LIFT_DOORS2)
			doorItem->ItemFlags[0] = 4096;

		doorItem->Data = ITEM_DATA(DOOR_DATA());
		auto* doorData = (DOOR_DATA*)doorItem->Data;

		doorData->opened = false;
		doorData->dptr1 = nullptr;
		doorData->dptr2 = nullptr;
		doorData->dptr3 = nullptr;
		doorData->dptr4 = nullptr;

		short boxNumber, twoRoom;

		int xOffset = 0;
		int zOffset = 0;

		if (doorItem->Pose.Orientation.y == 0)
			zOffset = -SECTOR(1);
		else if (doorItem->Pose.Orientation.y == ANGLE(180.0f))
			zOffset = SECTOR(1);
		else if (doorItem->Pose.Orientation.y == ANGLE(90.0f))
			xOffset = -SECTOR(1);
		else
			xOffset = SECTOR(1);

		auto* r = &g_Level.Rooms[doorItem->RoomNumber];
		doorData->d1.floor = GetSector(r, doorItem->Pose.Position.x - r->x + xOffset, doorItem->Pose.Position.z - r->z + zOffset);

		auto roomNumber = doorData->d1.floor->WallPortal;
		if (roomNumber == NO_ROOM)
			boxNumber = doorData->d1.floor->Box;
		else
		{
			auto* b = &g_Level.Rooms[roomNumber];
			boxNumber = GetSector(b, doorItem->Pose.Position.x - b->x + xOffset, doorItem->Pose.Position.z - b->z + zOffset)->Box;
		}

		doorData->d1.block = (boxNumber != NO_BOX && g_Level.Boxes[boxNumber].flags & BLOCKABLE) ? boxNumber : NO_BOX; 
		doorData->d1.data = *doorData->d1.floor;

		if (r->flippedRoom != -1)
		{
			r = &g_Level.Rooms[r->flippedRoom];
			doorData->d1flip.floor = GetSector(r, doorItem->Pose.Position.x - r->x + xOffset, doorItem->Pose.Position.z - r->z + zOffset);
				
			roomNumber = doorData->d1flip.floor->WallPortal;
			if (roomNumber == NO_ROOM)
				boxNumber = doorData->d1flip.floor->Box;
			else
			{
				auto* b = &g_Level.Rooms[roomNumber];
				boxNumber = GetSector(b, doorItem->Pose.Position.x - b->x + xOffset, doorItem->Pose.Position.z - b->z + zOffset)->Box;
			}

			doorData->d1flip.block = (boxNumber != NO_BOX && g_Level.Boxes[boxNumber].flags & BLOCKABLE) ? boxNumber : NO_BOX;
			doorData->d1flip.data = *doorData->d1flip.floor;
		}
		else
			doorData->d1flip.floor = NULL;

		twoRoom = doorData->d1.floor->WallPortal;

		ShutThatDoor(&doorData->d1, doorData);
		ShutThatDoor(&doorData->d1flip, doorData);

		if (twoRoom == NO_ROOM)
		{
			doorData->d2.floor = NULL;
			doorData->d2flip.floor = NULL;
		}
		else
		{
			r = &g_Level.Rooms[twoRoom];
			doorData->d2.floor = GetSector(r, doorItem->Pose.Position.x - r->x, doorItem->Pose.Position.z - r->z);

			roomNumber = doorData->d2.floor->WallPortal;
			if (roomNumber == NO_ROOM)
				boxNumber = doorData->d2.floor->Box;
			else
			{
				auto* b = &g_Level.Rooms[roomNumber];
				boxNumber = GetSector(b, doorItem->Pose.Position.x - b->x, doorItem->Pose.Position.z - b->z)->Box;
			}

			doorData->d2.block = (boxNumber != NO_BOX && g_Level.Boxes[boxNumber].flags & BLOCKABLE) ? boxNumber : NO_BOX;
			doorData->d2.data = *doorData->d2.floor;

			if (r->flippedRoom != -1)
			{
				r = &g_Level.Rooms[r->flippedRoom];
				doorData->d2flip.floor = GetSector(r, doorItem->Pose.Position.x - r->x, doorItem->Pose.Position.z - r->z);

				roomNumber = doorData->d2flip.floor->WallPortal;
				if (roomNumber == NO_ROOM)
					boxNumber = doorData->d2flip.floor->Box;
				else
				{
					auto* b = &g_Level.Rooms[roomNumber];
					boxNumber = GetSector(b, doorItem->Pose.Position.x - b->x, doorItem->Pose.Position.z - b->z)->Box;
				}

				doorData->d2flip.block = (boxNumber != NO_BOX && g_Level.Boxes[boxNumber].flags & BLOCKABLE) ? boxNumber : NO_BOX; 
				doorData->d2flip.data = *doorData->d2flip.floor;
			}
			else
				doorData->d2flip.floor = NULL;

			ShutThatDoor(&doorData->d2, doorData);
			ShutThatDoor(&doorData->d2flip, doorData);

			roomNumber = doorItem->RoomNumber;
			ItemNewRoom(itemNumber, twoRoom);
			doorItem->RoomNumber = roomNumber;
			doorItem->InDrawRoom = true;
		}
	}

	void DoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* doorItem = &g_Level.Items[itemNumber];

		if (doorItem->TriggerFlags == 2 &&
			doorItem->Status == ITEM_NOT_ACTIVE && !doorItem->Animation.IsAirborne && // CHECK
			((TrInput & IN_ACTION || g_Gui.GetInventoryItemChosen() == ID_CROWBAR_ITEM) &&
				laraItem->Animation.ActiveState == LS_IDLE &&
				laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
				!laraItem->HitStatus &&
				laraInfo->Control.HandStatus == HandStatus::Free ||
				laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber))
		{
			doorItem->Pose.Orientation.y ^= ANGLE(180.0f);
			if (TestLaraPosition(&CrowbarDoorBounds, doorItem, laraItem))
			{
				if (!laraInfo->Control.IsMoving)
				{
					if (g_Gui.GetInventoryItemChosen() == NO_ITEM)
					{
						if (g_Gui.IsObjectInInventory(ID_CROWBAR_ITEM))
						{
							g_Gui.SetEnterInventory(ID_CROWBAR_ITEM);
							doorItem->Pose.Orientation.y ^= ANGLE(180.0f);
						}
						else
						{
							if (OldPickupPos.x != laraItem->Pose.Position.x || OldPickupPos.y != laraItem->Pose.Position.y || OldPickupPos.z != laraItem->Pose.Position.z)
							{
								OldPickupPos.x = laraItem->Pose.Position.x;
								OldPickupPos.y = laraItem->Pose.Position.y;
								OldPickupPos.z = laraItem->Pose.Position.z;
								SayNo();
							}

							doorItem->Pose.Orientation.y ^= ANGLE(180.0f);
						}

						return;
					}

					if (g_Gui.GetInventoryItemChosen() != ID_CROWBAR_ITEM)
					{
						doorItem->Pose.Orientation.y ^= ANGLE(180.0f);
						return;
					}
				}

				g_Gui.SetInventoryItemChosen(NO_ITEM);

				if (MoveLaraPosition(&CrowbarDoorPos, doorItem, laraItem))
				{
					SetAnimation(laraItem, LA_DOOR_OPEN_CROWBAR);
					doorItem->Pose.Orientation.y ^= ANGLE(180.0f);

					AddActiveItem(itemNumber);

					laraInfo->Control.IsMoving = 0;
					laraInfo->Control.HandStatus = HandStatus::Busy;
					doorItem->Flags |= IFLAG_ACTIVATION_MASK;
					doorItem->Status = ITEM_ACTIVE;
					doorItem->Animation.TargetState = LS_RUN_FORWARD;
					return;
				}

				laraInfo->InteractedItem = itemNumber;
			}
			else if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
			{
				laraInfo->Control.IsMoving = 0;
				laraInfo->Control.HandStatus = HandStatus::Free;
			}

			doorItem->Pose.Orientation.y ^= ANGLE(180.0f);
		}

		if (TestBoundsCollide(doorItem, laraItem, coll->Setup.Radius))
		{
			if (TestCollision(doorItem, laraItem))
			{
				if (coll->Setup.EnableObjectPush)
				{
					if (!(doorItem->ObjectNumber >= ID_LIFT_DOORS1 &&
						doorItem->ObjectNumber <= ID_LIFT_DOORS2) || doorItem->ItemFlags[0])
					{
						ItemPushItem(doorItem, laraItem, coll, FALSE, TRUE);
					}
				}
			}
		}
	}

	void DoorControl(short itemNumber)
	{
		auto* doorItem = &g_Level.Items[itemNumber];
		auto* doorData = (DOOR_DATA*)doorItem->Data;

		// Doors with OCB = 1 are raisable with cog switchs
		if (doorItem->TriggerFlags == 1)
		{
			if (doorItem->ItemFlags[0])
			{
				auto* bounds = GetBoundsAccurate(doorItem);
			
				doorItem->ItemFlags[0]--;
				doorItem->Pose.Position.y -= TEN::Entities::Switches::COG_DOOR_SPEED;
				
				int y = bounds->Y1 + doorItem->ItemFlags[2] - STEP_SIZE;
				if (doorItem->Pose.Position.y < y)
				{
					doorItem->Pose.Position.y = y;
					doorItem->ItemFlags[0] = 0;
				}
				if (!doorData->opened)
				{
					OpenThatDoor(&doorData->d1, doorData);
					OpenThatDoor(&doorData->d2, doorData);
					OpenThatDoor(&doorData->d1flip, doorData);
					OpenThatDoor(&doorData->d2flip, doorData);
					doorData->opened = true;
				}
			}
			else
			{
				if (doorItem->Pose.Position.y < doorItem->StartPose.Position.y)
					doorItem->Pose.Position.y += 4;
				if (doorItem->Pose.Position.y >= doorItem->StartPose.Position.y)
				{
					doorItem->Pose.Position.y = doorItem->StartPose.Position.y;
					if (doorData->opened)
					{
						ShutThatDoor(&doorData->d1, doorData);
						ShutThatDoor(&doorData->d2, doorData);
						ShutThatDoor(&doorData->d1flip, doorData);
						ShutThatDoor(&doorData->d2flip, doorData);
						doorData->opened = false;
					}
				}
			}

			return;
		}

		if (doorItem->ObjectNumber < ID_LIFT_DOORS1 || doorItem->ObjectNumber > ID_LIFT_DOORS2)
		{
			if (TriggerActive(doorItem))
			{
				if (doorItem->Animation.ActiveState == 0)
					doorItem->Animation.TargetState = 1;
				else if (!doorData->opened)
				{
					OpenThatDoor(&doorData->d1, doorData);
					OpenThatDoor(&doorData->d2, doorData);
					OpenThatDoor(&doorData->d1flip, doorData);
					OpenThatDoor(&doorData->d2flip, doorData);
					doorData->opened = true;
				}
			}
			else
			{
				doorItem->Status = ITEM_ACTIVE;

				if (doorItem->Animation.ActiveState == 1)
					doorItem->Animation.TargetState = 0;
				else if (doorData->opened)
				{
					ShutThatDoor(&doorData->d1, doorData);
					ShutThatDoor(&doorData->d2, doorData);
					ShutThatDoor(&doorData->d1flip, doorData);
					ShutThatDoor(&doorData->d2flip, doorData);
					doorData->opened = false;
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
						SoundEffect(SFX_TR5_LIFT_DOORS, &item->pos);
					item->itemFlags[0] += STEP_SIZE;
				}
			}
			else
			{
				if (item->itemFlags[0] > 0)
				{
					if (item->itemFlags[0] == SECTOR(4))
						SoundEffect(SFX_TR5_LIFT_DOORS, &item->pos);
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

		AnimateItem(doorItem);
	}

	void OpenThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd)
	{
		FloorInfo* floor = doorPos->floor;

		if (floor != NULL)
		{
			*doorPos->floor = doorPos->data;

			short boxIndex = doorPos->block;
			if (boxIndex != NO_BOX)
			{
				g_Level.Boxes[boxIndex].flags &= ~BLOCKED;

				for (int i = 0; i < ActiveCreatures.size(); i++)
				{
					ActiveCreatures[i]->LOT.TargetBox = NO_BOX;
				}
			}
		}
	}

	void ShutThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd)
	{
		FloorInfo* floor = doorPos->floor;

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
					ActiveCreatures[i]->LOT.TargetBox = NO_BOX;
			}
		}
	}
}
