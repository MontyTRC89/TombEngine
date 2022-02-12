#include "framework.h"
#include "Objects/TR5/Object/tr5_pushableblock.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/animation.h"
#include "Game/items.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/control/flipeffect.h"
#include "Game/control/box.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Sound/sound.h"
#include "Specific/setup.h"
#include "Objects/TR5/Object/tr5_pushableblock_info.h"

PHD_VECTOR PushableBlockPos = { 0, 0, 0 };
static OBJECT_COLLISION_BOUNDS PushableBlockBounds = 
{ 0, 0, -64, 0, 0, 0, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), ANGLE(-10), ANGLE(10) };

int DoPushPull = 0;

void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber)
{
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	if (floor->Box == NO_BOX)
		return;
	g_Level.Boxes[floor->Box].flags &= (~BLOCKED);
	short height = g_Level.Boxes[floor->Box].height;
	short baseRoomNumber = roomNumber;
	
	floor = GetFloor(x + 1024, y, z, &roomNumber);
	if (floor->Box != NO_BOX)
	{
		if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
			ClearMovableBlockSplitters(x + WALL_SIZE, y, z, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x - 1024, y, z, &roomNumber);
	if (floor->Box != NO_BOX)
	{
		if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
			ClearMovableBlockSplitters(x - WALL_SIZE, y, z, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z + 1024, &roomNumber);
	if (floor->Box != NO_BOX)
	{
		if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
			ClearMovableBlockSplitters(x, y, z + WALL_SIZE, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z - 1024, &roomNumber);
	if (floor->Box != NO_BOX)
	{
		if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
			ClearMovableBlockSplitters(x, y, z - WALL_SIZE, roomNumber);
	}
}

void InitialisePushableBlock(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	item->ItemFlags[1] = NO_ITEM; // need to use itemFlags[1] to hold linked index for now
	
	// allocate new pushable info
	item->Data = PUSHABLE_INFO();
	PUSHABLE_INFO* pushable = item->Data;
	
	pushable->stackLimit = 3; // LUA
	pushable->gravity = 8; // LUA
	pushable->weight = 100; // LUA
	pushable->moveX = item->Position.xPos;
	pushable->moveZ = item->Position.zPos;

	// read flags from OCB
	int OCB = item->TriggerFlags;

	pushable->canFall = OCB & 0x20;
	pushable->disablePull = OCB & 0x80;
	pushable->disablePush = OCB & 0x100;
	pushable->disableW = pushable->disableE = OCB & 0x200;
	pushable->disableN = pushable->disableS = OCB & 0x400;
	
	pushable->climb = 0; // maybe there will be better way to handle this than OCBs?
	/*
	pushable->climb |= (OCB & 0x800) ? CLIMB_WEST : 0;
	pushable->climb |= (OCB & 0x1000) ? CLIMB_NORTH : 0;
	pushable->climb |= (OCB & 0x2000) ? CLIMB_EAST : 0;
	pushable->climb |= (OCB & 0x4000) ? CLIMB_SOUTH : 0;
	*/
	pushable->hasFloorCeiling = false;

	int height;
	if (OCB & 0x40 && (OCB & 0x1F) >= 2)
	{
		pushable->hasFloorCeiling = true;
		TEN::Floordata::AddBridge(itemNum);
		height = (OCB & 0x1F) * CLICK(1);
	}
	else
		height = -GetBoundsAccurate(item)->Y1;

	pushable->height = height;

	pushable->loopSound = SFX_TR4_PUSHABLE_SOUND; // LUA
	pushable->stopSound = SFX_TR4_PUSH_BLOCK_END; // LUA
	pushable->fallSound = SFX_TR4_BOULDER_FALL; // LUA

	FindStack(itemNum); // check for stack formation when pushables are initialised
}

void PushableBlockControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	Lara.interactedItem = itemNumber;

	PHD_VECTOR pos;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;

	short quadrant = (unsigned short)(LaraItem->Position.yRot + ANGLE(45)) / ANGLE(90);

	int x, z;
	FLOOR_INFO* floor;
	ROOM_INFO* r;
	PUSHABLE_INFO* pushable = item->Data;
	int blockHeight = GetStackHeight(item);

	// do sound effects, it works for now
	if (DoPushPull > 0)
	{
		SoundEffect(pushable->loopSound, &item->Position, 2);
	}
	else if (DoPushPull < 0)
	{
		DoPushPull = 0;
		SoundEffect(pushable->stopSound, &item->Position, 2);
	}

	// control block falling
	if (item->Airborne)
	{
		auto floorHeight = GetCollisionResult(item->Position.xPos, item->Position.yPos + 10, item->Position.zPos, item->RoomNumber).Position.Floor;

		if (item->Position.yPos < floorHeight - item->VerticalVelocity)
		{
			if (item->VerticalVelocity + pushable->gravity < 128)
				item->VerticalVelocity += pushable->gravity;
			else
				item->VerticalVelocity++;
			item->Position.yPos += item->VerticalVelocity;

			MoveStackY(itemNumber, item->VerticalVelocity);
		}
		else
		{
			item->Airborne = false;
			int relY = floorHeight - item->Position.yPos;
			item->Position.yPos = floorHeight;
			if (item->VerticalVelocity >= 96)
				FloorShake(item);
			item->VerticalVelocity = 0;
			SoundEffect(pushable->fallSound, &item->Position, 2);

			MoveStackY(itemNumber, relY);
			AddBridgeStack(itemNumber);

			if (FindStack(itemNumber) == NO_ITEM) // if fallen on some existing pushables, don't test triggers
			{
				TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
			}
			
			RemoveActiveItem(itemNumber);
			item->Status = ITEM_NOT_ACTIVE;

			if (pushable->hasFloorCeiling)
			{
				//AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
				AdjustStopperFlag(item, item->ItemFlags[0] + 0x8000, false);
			}
		}

		return;
	}

	int displaceBox = GetBoundsAccurate(LaraItem)->Z2 - 80; // move pushable based on bbox->Z2 of Lara

	switch (LaraItem->AnimNumber)
	{
	case LA_PUSHABLE_PUSH:

		if (LaraItem->FrameNumber == g_Level.Anims[LaraItem->AnimNumber].frameBase)
		{
			RemoveFromStack(itemNumber);
			RemoveBridgeStack(itemNumber);
		}

		switch (quadrant) 
		{
		case 0:
			z = pushable->moveZ + displaceBox;
			if (abs(item->Position.zPos - z) < CLICK(2) && item->Position.zPos < z)
				item->Position.zPos = z;
			break;

		case 1:
			x = pushable->moveX + displaceBox;
			if (abs(item->Position.xPos - x) < CLICK(2) && item->Position.xPos < x)
				item->Position.xPos = x;
			break;

		case 2:
			z = pushable->moveZ - displaceBox;
			if (abs(item->Position.zPos - z) < CLICK(2) && item->Position.zPos > z)
				item->Position.zPos = z;
			break;

		case 3:
			x = pushable->moveX - displaceBox;
			if (abs(item->Position.xPos - x) < CLICK(2) && item->Position.xPos > x)
				item->Position.xPos = x;
			break;

		default:
			break;
		}

		MoveStackXZ(itemNumber);


		if (LaraItem->FrameNumber == g_Level.Anims[LaraItem->AnimNumber].frameEnd - 1)
		{
			if (pushable->canFall) // check if pushable is about to fall
			{
				auto floorHeight = GetCollisionResult(item->Position.xPos, item->Position.yPos + 10, item->Position.zPos, item->RoomNumber).Position.Floor;
				if (floorHeight > item->Position.yPos)
				{
					item->Position.xPos = item->Position.xPos & 0xFFFFFE00 | 0x200;
					item->Position.zPos = item->Position.zPos & 0xFFFFFE00 | 0x200;
					MoveStackXZ(itemNumber);
					//SoundEffect(pushable->stopSound, &item->pos, 2);
					DoPushPull = 0;
					LaraItem->TargetState = LS_IDLE;

					item->Airborne = true; // do fall
					return;
				}
			}


			if (TrInput & IN_ACTION)
			{
				if (!TestBlockPush(item, blockHeight, quadrant))
				{
					LaraItem->TargetState = LS_IDLE;
				}
				else
				{
					item->Position.xPos = pushable->moveX = item->Position.xPos & 0xFFFFFE00 | 0x200;
					item->Position.zPos = pushable->moveZ = item->Position.zPos & 0xFFFFFE00 | 0x200;
					TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
				}
			}
			else
			{
				LaraItem->TargetState = LS_IDLE;
			}
		}
		break;

	case LA_PUSHABLE_PULL:

		if (LaraItem->FrameNumber == g_Level.Anims[LaraItem->AnimNumber].frameBase)
		{
			RemoveFromStack(itemNumber);
			RemoveBridgeStack(itemNumber);
		}

		switch (quadrant)
		{
		case NORTH:
			z = pushable->moveZ + displaceBox;
			if (abs(item->Position.zPos - z) < CLICK(2) && item->Position.zPos > z)
				item->Position.zPos = z;
			break;

		case EAST:
			x = pushable->moveX + displaceBox;
			if (abs(item->Position.xPos - x) < CLICK(2) && item->Position.xPos > x)
				item->Position.xPos = x;
			break;

		case SOUTH:
			z = pushable->moveZ - displaceBox;
			if (abs(item->Position.zPos - z) < CLICK(2) && item->Position.zPos < z)
				item->Position.zPos = z;
			break;

		case WEST:
			x = pushable->moveX - displaceBox;
			if (abs(item->Position.xPos - x) < CLICK(2) && item->Position.xPos < x)
				item->Position.xPos = x;
			break;

		default:
			break;
		}

		MoveStackXZ(itemNumber);

		if (LaraItem->FrameNumber == g_Level.Anims[LaraItem->AnimNumber].frameEnd - 1)
		{
			if (TrInput & IN_ACTION)
			{
				if (!TestBlockPull(item, blockHeight, quadrant))
				{
					LaraItem->TargetState = LS_IDLE;
				}
				else
				{
					item->Position.xPos = pushable->moveX = item->Position.xPos & 0xFFFFFE00 | 0x200;
					item->Position.zPos = pushable->moveZ = item->Position.zPos & 0xFFFFFE00 | 0x200;
					TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
				}
			}
			else
			{
				LaraItem->TargetState = LS_IDLE;
			}
		}
		break;

	case LA_PUSHABLE_PUSH_TO_STAND:
	case LA_PUSHABLE_PULL_TO_STAND:
		if (LaraItem->FrameNumber == g_Level.Anims[LA_PUSHABLE_PUSH_TO_STAND].frameBase
			|| LaraItem->FrameNumber == g_Level.Anims[LA_PUSHABLE_PULL_TO_STAND].frameBase)
		{
			item->Position.xPos = item->Position.xPos & 0xFFFFFE00 | 0x200;
			item->Position.zPos = item->Position.zPos & 0xFFFFFE00 | 0x200;

			MoveStackXZ(itemNumber);
			FindStack(itemNumber);
			AddBridgeStack(itemNumber);

			TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
		}

		if (LaraItem->FrameNumber == g_Level.Anims[LaraItem->AnimNumber].frameEnd)
		{
			RemoveActiveItem(itemNumber);
			item->Status = ITEM_NOT_ACTIVE;

			if (pushable->hasFloorCeiling)
			{
				//AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
				AdjustStopperFlag(item, item->ItemFlags[0] + 0x8000, false);
			}
		}

		break;
	}
}

void PushableBlockCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	short roomNumber = item->RoomNumber;
	FLOOR_INFO* floor = GetFloor(item->Position.xPos, item->Position.yPos - 256, item->Position.zPos, &roomNumber);
	PUSHABLE_INFO* pushable = item->Data;

	int blockHeight = GetStackHeight(item);
	
	if ((!(TrInput & IN_ACTION)
		|| l->ActiveState != LS_IDLE
		|| l->AnimNumber != LA_STAND_IDLE
		|| l->Airborne
		|| Lara.Control.HandStatus != HandStatus::Free
		|| item->Status == ITEM_INVISIBLE
		|| item->TriggerFlags < 0)
		&& (!Lara.Control.IsMoving || Lara.interactedItem != itemNum))
	{
		if ((l->ActiveState != LS_PUSHABLE_GRAB
			|| (l->FrameNumber != g_Level.Anims[LA_PUSHABLE_GRAB].frameBase + 19)
			|| Lara.NextCornerPos.xPos != itemNum))
		{
			if (!pushable->hasFloorCeiling)
				ObjectCollision(itemNum, l, coll);
			return;
		}

		short quadrant = (unsigned short)(LaraItem->Position.yRot + ANGLE(45)) / ANGLE(90);

		bool quadrantDisabled = false;
		switch (quadrant)
		{
		case NORTH:
			quadrantDisabled = pushable->disableN;
			break;
		case EAST:
			quadrantDisabled = pushable->disableE;
			break;
		case SOUTH:
			quadrantDisabled = pushable->disableS;
			break;
		case WEST:
			quadrantDisabled = pushable->disableW;
			break;
		}

		if (quadrantDisabled)
			return;

		if (!CheckStackLimit(item))
			return;

		if (!TestBlockMovable(item, blockHeight))
			return;

		if (TrInput & IN_FORWARD)
		{
			if (!TestBlockPush(item, blockHeight, quadrant) || pushable->disablePush)
				return;
			l->TargetState = LS_PUSHABLE_PUSH;
		}
		else if (TrInput & IN_BACK)
		{
			if (!TestBlockPull(item, blockHeight, quadrant) || pushable->disablePull)
				return;
			l->TargetState = LS_PUSHABLE_PULL;
		}
		else
		{
			return;
		}

		item->Status = ITEM_ACTIVE;
		AddActiveItem(itemNum);
		ResetLaraFlex(l);
		
		pushable->moveX = item->Position.xPos;
		pushable->moveZ = item->Position.zPos;

		if (pushable->hasFloorCeiling)
		{
			//AlterFloorHeight(item, ((item->triggerFlags - 64) * 256));
			AdjustStopperFlag(item, item->ItemFlags[0], false);
		}
	}
	else
	{
		BOUNDING_BOX* bounds = GetBoundsAccurate(item);

		PushableBlockBounds.boundingBox.X1 = (bounds->X1 / 2) - 100;
		PushableBlockBounds.boundingBox.X2 = (bounds->X2 / 2) + 100;
		PushableBlockBounds.boundingBox.Z1 = bounds->Z1 - 200;
		PushableBlockBounds.boundingBox.Z2 = 0;

		short rot = item->Position.yRot;
		item->Position.yRot = (l->Position.yRot + ANGLE(45)) & 0xC000;

		if (TestLaraPosition(&PushableBlockBounds, item, l))
		{
			unsigned short quadrant = (unsigned short)((item->Position.yRot / 0x4000) + ((rot + 0x2000) / 0x4000));
			if (quadrant & 1)
				PushableBlockPos.z = bounds->X1 - 35;
			else
				PushableBlockPos.z = bounds->Z1 - 35;

			if (pushable->hasFloorCeiling)
			{					
				// For now don't use auto-align function because it can collide with climb up moves of Lara

				LaraItem->Position.xRot = item->Position.xRot;
				LaraItem->Position.yRot = item->Position.yRot;
				LaraItem->Position.zRot = item->Position.zRot;

				l->AnimNumber = LA_PUSHABLE_GRAB;
				l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
				l->ActiveState = LS_PUSHABLE_GRAB;
				l->TargetState = LS_PUSHABLE_GRAB;
				Lara.Control.IsMoving = false;
				Lara.Control.HandStatus = HandStatus::Busy;
				Lara.NextCornerPos.xPos = itemNum;
				item->Position.yRot = rot;
			}
			else
			{
				if (MoveLaraPosition(&PushableBlockPos, item, l))
				{
					l->AnimNumber = LA_PUSHABLE_GRAB;
					l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
					l->ActiveState = LS_PUSHABLE_GRAB;
					l->TargetState = LS_PUSHABLE_GRAB;
					Lara.Control.IsMoving = false;
					Lara.Control.HandStatus = HandStatus::Busy;
					Lara.NextCornerPos.xPos = itemNum;
					item->Position.yRot = rot;
				}
				else
				{
					Lara.interactedItem = itemNum;
					item->Position.yRot = rot;
				}
			}
		}
		else
		{
			if (Lara.Control.IsMoving && Lara.interactedItem == itemNum)
			{
				Lara.Control.IsMoving = false;
				Lara.Control.HandStatus = HandStatus::Free;
			}
			item->Position.yRot = rot;
		}
	}
}

void PushLoop(ITEM_INFO* item) // Do Flipeffect 18 in anims
{
	DoPushPull = 1;
}

void PushEnd(ITEM_INFO* item) // Do Flipeffect 19 in anims
{
	if (DoPushPull == 1)
	{
		DoPushPull = -1;
	}
}

bool TestBlockMovable(ITEM_INFO* item, int blokhite)
{
	short roomNumber = item->RoomNumber;
	FLOOR_INFO* floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);

	if (floor->IsWall(floor->SectorPlane(item->Position.xPos, item->Position.zPos)))
		return false;

	if (floor->FloorHeight(item->Position.xPos, item->Position.zPos) != item->Position.yPos)
		return false;

	return true;
}

bool TestBlockPush(ITEM_INFO* item, int blockhite, unsigned short quadrant)
{
	int x = item->Position.xPos;
	int y = item->Position.yPos;
	int z = item->Position.zPos;
	
	switch (quadrant)
	{
	case NORTH:
		z += WALL_SIZE;
		break;

	case EAST:
		x += WALL_SIZE;
		break;

	case SOUTH:
		z -= WALL_SIZE;
		break;

	case WEST:
		x -= WALL_SIZE;
		break;
	}

	auto collResult = GetCollisionResult(x, y - blockhite, z, item->RoomNumber);

	ROOM_INFO* r = &g_Level.Rooms[collResult.RoomNumber];
	if (GetSector(r, x - r->x, z - r->z)->Stopper)
		return false;

	if (collResult.Position.FloorSlope || collResult.Position.DiagonalStep ||
		collResult.Block->FloorSlope(0) != Vector2::Zero || collResult.Block->FloorSlope(1) != Vector2::Zero)
		return false;

	if (((PUSHABLE_INFO*)item->Data)->canFall)
	{
		if (collResult.Position.Floor < y)
			return false;
	}
	else
	{
		if (collResult.Position.Floor != y)
			return false;
	}

	int ceiling = y - blockhite + 100;

	if (GetCollisionResult(x, ceiling, z, item->RoomNumber).Position.Ceiling > ceiling)
		return false;

	int oldX = item->Position.xPos;
	int oldZ = item->Position.zPos;
	item->Position.xPos = x;
	item->Position.zPos = z;
	GetCollidedObjects(item, 256, true, &CollidedItems[0], nullptr, 1);
	item->Position.xPos = oldX;
	item->Position.zPos = oldZ;

	int i = 0;
	while (CollidedItems[i] != NULL)
	{
		if (Objects[CollidedItems[i]->ObjectNumber].floor == NULL)
			return false;
		else
		{
			const auto& object = Objects[CollidedItems[i]->ObjectNumber];
			int collidedIndex = CollidedItems[i] - g_Level.Items.data(); // index of CollidedItems[i]

			int xCol = CollidedItems[i]->Position.xPos;
			int yCol = CollidedItems[i]->Position.yPos;
			int zCol = CollidedItems[i]->Position.zPos;

			// check if floor function returns nullopt
			if (object.floor(collidedIndex, xCol, yCol, zCol) == std::nullopt)
				return false;
		}

		i++;
	}

	return true;
}

bool TestBlockPull(ITEM_INFO* item, int blockhite, short quadrant)
{
	int xadd = 0;
	int zadd = 0;

	switch (quadrant)
	{
	case NORTH:
		zadd = -WALL_SIZE;
		break;

	case EAST:
		xadd = -WALL_SIZE;
		break;

	case SOUTH:
		zadd = WALL_SIZE;
		break;

	case WEST:
		xadd = WALL_SIZE;
		break;
	}

	int x = item->Position.xPos + xadd;
	int y = item->Position.yPos;
	int z = item->Position.zPos + zadd;

	short roomNum = item->RoomNumber;
	ROOM_INFO* r = &g_Level.Rooms[roomNum];
	if (GetSector(r, x - r->x, z - r->z)->Stopper)
		return false;

	auto collResult = GetCollisionResult(x, y - blockhite, z, item->RoomNumber);

	if (collResult.Position.Floor != y)
		return false;

	if (collResult.Position.FloorSlope || collResult.Position.DiagonalStep ||
		collResult.Block->FloorSlope(0) != Vector2::Zero || collResult.Block->FloorSlope(1) != Vector2::Zero)
		return false;

	int ceiling = y - blockhite + 100;

	if (GetCollisionResult(x, ceiling, z, item->RoomNumber).Position.Ceiling > ceiling)
		return false;

	int oldX = item->Position.xPos;
	int oldZ = item->Position.zPos;
	item->Position.xPos = x;
	item->Position.zPos = z;
	GetCollidedObjects(item, 256, true, &CollidedItems[0], 0, 1);
	item->Position.xPos = oldX;
	item->Position.zPos = oldZ;

	int i = 0;
	while (CollidedItems[i] != NULL)
	{
		if (Objects[CollidedItems[i]->ObjectNumber].floor == NULL)
			return false;
		else
		{
			const auto& object = Objects[CollidedItems[i]->ObjectNumber];
			int collidedIndex = CollidedItems[i] - g_Level.Items.data();

			int xCol = CollidedItems[i]->Position.xPos;
			int yCol = CollidedItems[i]->Position.yPos;
			int zCol = CollidedItems[i]->Position.zPos;

			if (object.floor(collidedIndex, xCol, yCol, zCol) == std::nullopt)
				return false;
		}

		i++;
	}

	int xAddLara = 0, zAddLara = 0;
	switch (quadrant)
	{
	case NORTH:
		zAddLara = GetBestFrame(LaraItem)->offsetZ;
		break;
	case EAST:
		xAddLara = GetBestFrame(LaraItem)->offsetZ;
		break;
	case SOUTH:
		zAddLara = -GetBestFrame(LaraItem)->offsetZ;
		break;
	case WEST:
		xAddLara = -GetBestFrame(LaraItem)->offsetZ;
		break;
	}

	x = LaraItem->Position.xPos + xadd + xAddLara;
	z = LaraItem->Position.zPos + zadd + zAddLara;

	roomNum = LaraItem->RoomNumber;

	collResult = GetCollisionResult(x, y - LARA_HEIGHT, z, LaraItem->RoomNumber);

	r = &g_Level.Rooms[roomNum];
	if (GetSector(r, x - r->x, z - r->z)->Stopper)
		return false;

	if (collResult.Position.Floor != y)
		return false;

	if (collResult.Block->CeilingHeight(x, z) > y - LARA_HEIGHT)
		return false;

	oldX = LaraItem->Position.xPos;
	oldZ = LaraItem->Position.zPos;
	LaraItem->Position.xPos = x;
	LaraItem->Position.zPos = z;
	GetCollidedObjects(LaraItem, 256, true, &CollidedItems[0], 0, 1);
	LaraItem->Position.xPos = oldX;
	LaraItem->Position.zPos = oldZ;

	i = 0;
	while (CollidedItems[i] != NULL)
	{
		if (CollidedItems[i] != item) // if collided item is not pushblock in which lara embedded
		{
			if (Objects[CollidedItems[i]->ObjectNumber].floor == NULL)
				return false;
			else
			{
				const auto& object = Objects[CollidedItems[i]->ObjectNumber];
				int collidedIndex = CollidedItems[i] - g_Level.Items.data();
				int xCol = CollidedItems[i]->Position.xPos;
				int yCol = CollidedItems[i]->Position.yPos;
				int zCol = CollidedItems[i]->Position.zPos;

				if (object.floor(collidedIndex, xCol, yCol, zCol) == std::nullopt)
					return false;
			}
		}

		i++;
	}

	return true;
}

void MoveStackXZ(short itemNum)
{
	auto item = &g_Level.Items[itemNum];

	short newRoomNumber = item->RoomNumber;
	GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &newRoomNumber);
	if (newRoomNumber != item->RoomNumber)
		ItemNewRoom(itemNum, newRoomNumber);

	auto stackItem = item;
	while (stackItem->ItemFlags[1] != NO_ITEM) // move pushblock stack together with bottom pushblock
	{
		int stackIndex = stackItem->ItemFlags[1];
		stackItem = &g_Level.Items[stackIndex];

		stackItem->Position.xPos = item->Position.xPos;
		stackItem->Position.zPos = item->Position.zPos;

		newRoomNumber = stackItem->RoomNumber;
		GetFloor(stackItem->Position.xPos, stackItem->Position.yPos, stackItem->Position.zPos, &newRoomNumber);
		if (newRoomNumber != stackItem->RoomNumber)
			ItemNewRoom(stackIndex, newRoomNumber);
	}
}

void MoveStackY(short itemNum, int y)
{
	auto item = &g_Level.Items[itemNum];

	short newRoomNumber = item->RoomNumber;
	GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &newRoomNumber);
	if (newRoomNumber != item->RoomNumber)
		ItemNewRoom(itemNum, newRoomNumber);

	while (item->ItemFlags[1] != NO_ITEM) // move pushblock stack together with bottom pushblock
	{
		int stackIndex = item->ItemFlags[1];
		item = &g_Level.Items[stackIndex];

		item->Position.yPos += y;

		short newRoomNumber = item->RoomNumber;
		GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &newRoomNumber);
		if (newRoomNumber != item->RoomNumber)
			ItemNewRoom(stackIndex, newRoomNumber);
	}
}

void AddBridgeStack(short itemNum)
{
	auto item = &g_Level.Items[itemNum];
	PUSHABLE_INFO* pushable = item->Data;
	if (pushable->hasFloorCeiling)
		TEN::Floordata::AddBridge(itemNum);

	int stackIndex = g_Level.Items[itemNum].ItemFlags[1];
	while (stackIndex != NO_ITEM)
	{
		auto stackItem = &g_Level.Items[stackIndex];

		if (pushable->hasFloorCeiling)
			TEN::Floordata::AddBridge(stackIndex);

		stackIndex = g_Level.Items[stackIndex].ItemFlags[1];
	}
}

void RemoveBridgeStack(short itemNum)
{
	auto item = &g_Level.Items[itemNum];
	PUSHABLE_INFO* pushable = item->Data;

	if (pushable->hasFloorCeiling)
		TEN::Floordata::RemoveBridge(itemNum);

	int stackIndex = g_Level.Items[itemNum].ItemFlags[1];
	while (stackIndex != NO_ITEM)
	{
		auto stackItem = &g_Level.Items[stackIndex];

		if (pushable->hasFloorCeiling)
			TEN::Floordata::RemoveBridge(stackIndex);

		stackIndex = g_Level.Items[stackIndex].ItemFlags[1];
	}
}

void RemoveFromStack(short itemNum) // unlink pushable from stack if linked
{
	for (int i = 0; i < g_Level.NumItems; i++)
	{
		if (i == itemNum)
			continue;

		auto belowItem = &g_Level.Items[i];

		int objectNum = belowItem->ObjectNumber;
		if (objectNum >= ID_PUSHABLE_OBJECT1 && objectNum <= ID_PUSHABLE_OBJECT10)
		{
			if (belowItem->ItemFlags[1] == itemNum)
				belowItem->ItemFlags[1] = NO_ITEM;
		}
	}
}

int FindStack(short itemNum)
{
	int stackTop = NO_ITEM; // index of heighest (yPos) pushable in stack
	int stackYmin = CLICK(256); // set starting height

	//Check for pushable directly below current one in same sector
	for (int i = 0; i < g_Level.NumItems; i++)
	{
		if (i == itemNum)
			continue;

		auto belowItem = &g_Level.Items[i];

		int objectNum = belowItem->ObjectNumber;
		if (objectNum >= ID_PUSHABLE_OBJECT1 && objectNum <= ID_PUSHABLE_OBJECT10)
		{
			auto item = &g_Level.Items[itemNum];
			int x = item->Position.xPos;
			int y = item->Position.yPos;
			int z = item->Position.zPos;

			if (belowItem->Position.xPos == x && belowItem->Position.zPos == z)
			{
				int belowY = belowItem->Position.yPos;
				if (belowY > y && belowY < stackYmin)
				{
					// set heighest pushable so far as top of stack
					stackTop = i;
					stackYmin = belowItem->Position.yPos;
				}
			}
		}
	}

	if (stackTop != NO_ITEM)
		g_Level.Items[stackTop].ItemFlags[1] = itemNum;

	return stackTop;
}

int GetStackHeight(ITEM_INFO* item)
{
	PUSHABLE_INFO* pushable = item->Data;
	int height = pushable->height;

	auto stackItem = item;
	while (stackItem->ItemFlags[1] != NO_ITEM)
	{
		stackItem = &g_Level.Items[stackItem->ItemFlags[1]];
		height += pushable->height;
	}

	return height;
}

bool CheckStackLimit(ITEM_INFO* item)
{
	PUSHABLE_INFO* pushable = item->Data;

	int limit = pushable->stackLimit;
	
	int count = 1;
	auto stackItem = item;
	while (stackItem->ItemFlags[1] != NO_ITEM)
	{
		stackItem = &g_Level.Items[stackItem->ItemFlags[1]];
		count++;

		if (count > limit)
			return false;
	}

	return true;
}

std::optional<int> PushableBlockFloor(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	const auto& pushable = (PUSHABLE_INFO&)item.Data;
	
	if (item.Status != ITEM_INVISIBLE && pushable.hasFloorCeiling)
	{
		const auto height = item.Position.yPos - (item.TriggerFlags & 0x1F) * CLICK(1);
		return std::optional{height};
	}
	return std::nullopt;
}

std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	const auto& pushable = (PUSHABLE_INFO&)item.Data;

	if (item.Status != ITEM_INVISIBLE && pushable.hasFloorCeiling)
		return std::optional{item.Position.yPos};
	return std::nullopt;
}

int PushableBlockFloorBorder(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];
	const auto height = item.Position.yPos - (item.TriggerFlags & 0x1F) * CLICK(1);
	return height;
}

int PushableBlockCeilingBorder(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];
	return item.Position.yPos;
}
