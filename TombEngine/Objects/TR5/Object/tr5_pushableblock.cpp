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

using namespace TEN::Input;

static auto PushableBlockPos = Vector3i::Zero;
InteractBounds PushableBlockBounds = 
{
	GameBoundingBox(
		0, 0,
		-CLICK(0.25f), 0,
		0, 0
	),
	std::pair(
		EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
		EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f))
	)
};

int DoPushPull = 0;

void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber)
{
	FloorInfo* floor = GetFloor(x, y, z, &roomNumber);
	if (floor->Box == NO_BOX)
		return;

	g_Level.Boxes[floor->Box].flags &= (~BLOCKED);
	short height = g_Level.Boxes[floor->Box].height;
	short baseRoomNumber = roomNumber;
	
	floor = GetFloor(x + 1024, y, z, &roomNumber);
	if (floor->Box != NO_BOX)
	{
		if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
			ClearMovableBlockSplitters(x + SECTOR(1), y, z, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x - 1024, y, z, &roomNumber);
	if (floor->Box != NO_BOX)
	{
		if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
			ClearMovableBlockSplitters(x - SECTOR(1), y, z, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z + 1024, &roomNumber);
	if (floor->Box != NO_BOX)
	{
		if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
			ClearMovableBlockSplitters(x, y, z + SECTOR(1), roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z - 1024, &roomNumber);
	if (floor->Box != NO_BOX)
	{
		if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
			ClearMovableBlockSplitters(x, y, z - SECTOR(1), roomNumber);
	}
}

void InitialisePushableBlock(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	item->ItemFlags[1] = NO_ITEM; // need to use itemFlags[1] to hold linked index for now
	
	// allocate new pushable info
	item->Data = PushableInfo();
	auto* info = (PushableInfo*)item->Data;
	
	info->stackLimit = 3; // LUA
	info->gravity = 8; // LUA
	info->weight = 100; // LUA
	info->moveX = item->Pose.Position.x;
	info->moveZ = item->Pose.Position.z;

	// read flags from OCB
	int OCB = item->TriggerFlags;

	info->canFall = OCB & 0x20;
	info->disablePull = OCB & 0x80;
	info->disablePush = OCB & 0x100;
	info->disableW = info->disableE = OCB & 0x200;
	info->disableN = info->disableS = OCB & 0x400;
	
	info->climb = 0; // maybe there will be better way to handle this than OCBs?
	/*
	pushable->climb |= (OCB & 0x800) ? CLIMB_WEST : 0;
	pushable->climb |= (OCB & 0x1000) ? CLIMB_NORTH : 0;
	pushable->climb |= (OCB & 0x2000) ? CLIMB_EAST : 0;
	pushable->climb |= (OCB & 0x4000) ? CLIMB_SOUTH : 0;
	*/
	info->hasFloorCeiling = false;

	int height;
	if (OCB & 0x40 && (OCB & 0x1F) >= 2)
	{
		info->hasFloorCeiling = true;
		TEN::Floordata::AddBridge(itemNumber);
		height = (OCB & 0x1F) * CLICK(1);
	}
	else
		height = -GameBoundingBox(item).Y1;

	info->height = height;

	info->loopSound = SFX_TR4_PUSHABLE_SOUND; // LUA
	info->stopSound = SFX_TR4_PUSH_BLOCK_END; // LUA
	info->fallSound = SFX_TR4_BOULDER_FALL; // LUA

	FindStack(itemNumber); // check for stack formation when pushables are initialised
}

void PushableBlockControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* info = (PushableInfo*)item->Data;

	Lara.InteractedItem = itemNumber;

	Vector3i pos = { 0, 0, 0 };

	short quadrant = (unsigned short)(LaraItem->Pose.Orientation.y + ANGLE(45.0f)) / ANGLE(90.0f);

	int x, z;
	int blockHeight = GetStackHeight(item);

	// do sound effects, it works for now
	if (DoPushPull > 0)
		SoundEffect(info->loopSound, &item->Pose, SoundEnvironment::Always);
	else if (DoPushPull < 0)
	{
		DoPushPull = 0;
		SoundEffect(info->stopSound, &item->Pose, SoundEnvironment::Always);
	}

	// control block falling
	if (item->Animation.IsAirborne)
	{
		int floorHeight = GetCollision(item->Pose.Position.x, item->Pose.Position.y + 10, item->Pose.Position.z, item->RoomNumber).Position.Floor;

		if (item->Pose.Position.y < (floorHeight - item->Animation.Velocity.y))
		{
			if ((item->Animation.Velocity.y + info->gravity) < 128)
				item->Animation.Velocity.y += info->gravity;
			else
				item->Animation.Velocity.y++;
			item->Pose.Position.y += item->Animation.Velocity.y;

			MoveStackY(itemNumber, item->Animation.Velocity.y);
		}
		else
		{
			item->Animation.IsAirborne = false;
			int relY = floorHeight - item->Pose.Position.y;
			item->Pose.Position.y = floorHeight;

			if (item->Animation.Velocity.y >= 96)
				FloorShake(item);

			item->Animation.Velocity.y = 0;
			SoundEffect(info->fallSound, &item->Pose, SoundEnvironment::Always);

			MoveStackY(itemNumber, relY);
			AddBridgeStack(itemNumber);

			if (FindStack(itemNumber) == NO_ITEM) // if fallen on some existing pushables, don't test triggers
				TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
			
			RemoveActiveItem(itemNumber);
			item->Status = ITEM_NOT_ACTIVE;

			if (info->hasFloorCeiling)
			{
				//AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
				AdjustStopperFlag(item, item->ItemFlags[0] + 0x8000, false);
			}
		}

		return;
	}

	int displaceBox = GameBoundingBox(LaraItem).Z2 - 80; // move pushable based on bbox->Z2 of Lara

	switch (LaraItem->Animation.AnimNumber)
	{
	case LA_PUSHABLE_PUSH:

		if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase)
		{
			RemoveFromStack(itemNumber);
			RemoveBridgeStack(itemNumber);
		}

		switch (quadrant) 
		{
		case 0:
			z = info->moveZ + displaceBox;

			if (abs(item->Pose.Position.z - z) < CLICK(2) && item->Pose.Position.z < z)
				item->Pose.Position.z = z;

			break;

		case 1:
			x = info->moveX + displaceBox;

			if (abs(item->Pose.Position.x - x) < CLICK(2) && item->Pose.Position.x < x)
				item->Pose.Position.x = x;

			break;

		case 2:
			z = info->moveZ - displaceBox;

			if (abs(item->Pose.Position.z - z) < CLICK(2) && item->Pose.Position.z > z)
				item->Pose.Position.z = z;

			break;

		case 3:
			x = info->moveX - displaceBox;

			if (abs(item->Pose.Position.x - x) < CLICK(2) && item->Pose.Position.x > x)
				item->Pose.Position.x = x;

			break;

		default:
			break;
		}

		MoveStackXZ(itemNumber);


		if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd - 1)
		{
			if (info->canFall) // check if pushable is about to fall
			{
				int floorHeight = GetCollision(item->Pose.Position.x, item->Pose.Position.y + 10, item->Pose.Position.z, item->RoomNumber).Position.Floor;
				if (floorHeight > item->Pose.Position.y)
				{
					item->Pose.Position.x = item->Pose.Position.x & 0xFFFFFE00 | 0x200;
					item->Pose.Position.z = item->Pose.Position.z & 0xFFFFFE00 | 0x200;
					MoveStackXZ(itemNumber);
					//SoundEffect(pushable->stopSound, &item->pos, SoundEnvironment::Always);
					DoPushPull = 0;
					LaraItem->Animation.TargetState = LS_IDLE;

					item->Animation.IsAirborne = true; // do fall
					return;
				}
			}


			if (TrInput & IN_ACTION)
			{
				if (!TestBlockPush(item, blockHeight, quadrant))
					LaraItem->Animation.TargetState = LS_IDLE;
				else
				{
					item->Pose.Position.x = info->moveX = item->Pose.Position.x & 0xFFFFFE00 | 0x200;
					item->Pose.Position.z = info->moveZ = item->Pose.Position.z & 0xFFFFFE00 | 0x200;
					TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
				}
			}
			else
				LaraItem->Animation.TargetState = LS_IDLE;
		}

		break;

	case LA_PUSHABLE_PULL:

		if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase)
		{
			RemoveFromStack(itemNumber);
			RemoveBridgeStack(itemNumber);
		}

		switch (quadrant)
		{
		case NORTH:
			z = info->moveZ + displaceBox;

			if (abs(item->Pose.Position.z - z) < CLICK(2) && item->Pose.Position.z > z)
				item->Pose.Position.z = z;

			break;

		case EAST:
			x = info->moveX + displaceBox;

			if (abs(item->Pose.Position.x - x) < CLICK(2) && item->Pose.Position.x > x)
				item->Pose.Position.x = x;

			break;

		case SOUTH:
			z = info->moveZ - displaceBox;

			if (abs(item->Pose.Position.z - z) < CLICK(2) && item->Pose.Position.z < z)
				item->Pose.Position.z = z;

			break;

		case WEST:
			x = info->moveX - displaceBox;

			if (abs(item->Pose.Position.x - x) < CLICK(2) && item->Pose.Position.x < x)
				item->Pose.Position.x = x;

			break;

		default:
			break;
		}

		MoveStackXZ(itemNumber);

		if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd - 1)
		{
			if (TrInput & IN_ACTION)
			{
				if (!TestBlockPull(item, blockHeight, quadrant))
					LaraItem->Animation.TargetState = LS_IDLE;
				else
				{
					item->Pose.Position.x = info->moveX = item->Pose.Position.x & 0xFFFFFE00 | 0x200;
					item->Pose.Position.z = info->moveZ = item->Pose.Position.z & 0xFFFFFE00 | 0x200;
					TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
				}
			}
			else
				LaraItem->Animation.TargetState = LS_IDLE;
		}

		break;

	case LA_PUSHABLE_PUSH_TO_STAND:
	case LA_PUSHABLE_PULL_TO_STAND:
		if (LaraItem->Animation.FrameNumber == g_Level.Anims[LA_PUSHABLE_PUSH_TO_STAND].frameBase ||
			LaraItem->Animation.FrameNumber == g_Level.Anims[LA_PUSHABLE_PULL_TO_STAND].frameBase)
		{
			item->Pose.Position.x = item->Pose.Position.x & 0xFFFFFE00 | 0x200;
			item->Pose.Position.z = item->Pose.Position.z & 0xFFFFFE00 | 0x200;

			MoveStackXZ(itemNumber);
			FindStack(itemNumber);
			AddBridgeStack(itemNumber);

			TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
		}

		if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd)
		{
			RemoveActiveItem(itemNumber);
			item->Status = ITEM_NOT_ACTIVE;

			if (info->hasFloorCeiling)
			{
				//AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
				AdjustStopperFlag(item, item->ItemFlags[0] + 0x8000, false);
			}
		}

		break;
	}
}

void PushableBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* pushableItem = &g_Level.Items[itemNumber];
	auto* pushableInfo = (PushableInfo*)pushableItem->Data;

	int blockHeight = GetStackHeight(pushableItem);
	
	if ((!(TrInput & IN_ACTION) ||
		laraItem->Animation.ActiveState != LS_IDLE ||
		laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
		laraItem->Animation.IsAirborne ||
		laraInfo->Control.HandStatus != HandStatus::Free ||
		pushableItem->Status == ITEM_INVISIBLE ||
		pushableItem->TriggerFlags < 0) &&
		(!laraInfo->Control.IsMoving || laraInfo->InteractedItem != itemNumber))
	{
		if ((laraItem->Animation.ActiveState != LS_PUSHABLE_GRAB ||
			(laraItem->Animation.FrameNumber != g_Level.Anims[LA_PUSHABLE_GRAB].frameBase + 19) ||
			laraInfo->NextCornerPos.Position.x != itemNumber))
		{
			if (!pushableInfo->hasFloorCeiling)
				ObjectCollision(itemNumber, laraItem, coll);

			return;
		}

		short quadrant = (unsigned short)(LaraItem->Pose.Orientation.y + ANGLE(45.0f)) / ANGLE(90.0f);

		bool quadrantDisabled = false;
		switch (quadrant)
		{
		case NORTH:
			quadrantDisabled = pushableInfo->disableN;
			break;
		case EAST:
			quadrantDisabled = pushableInfo->disableE;
			break;
		case SOUTH:
			quadrantDisabled = pushableInfo->disableS;
			break;
		case WEST:
			quadrantDisabled = pushableInfo->disableW;
			break;
		}

		if (quadrantDisabled)
			return;

		if (!CheckStackLimit(pushableItem))
			return;

		if (!TestBlockMovable(pushableItem, blockHeight))
			return;

		if (TrInput & IN_FORWARD)
		{
			if (!TestBlockPush(pushableItem, blockHeight, quadrant) || pushableInfo->disablePush)
				return;

			laraItem->Animation.TargetState = LS_PUSHABLE_PUSH;
		}
		else if (TrInput & IN_BACK)
		{
			if (!TestBlockPull(pushableItem, blockHeight, quadrant) || pushableInfo->disablePull)
				return;

			laraItem->Animation.TargetState = LS_PUSHABLE_PULL;
		}
		else
			return;

		pushableItem->Status = ITEM_ACTIVE;
		AddActiveItem(itemNumber);
		ResetLaraFlex(laraItem);
		
		pushableInfo->moveX = pushableItem->Pose.Position.x;
		pushableInfo->moveZ = pushableItem->Pose.Position.z;

		if (pushableInfo->hasFloorCeiling)
		{
			//AlterFloorHeight(item, ((item->triggerFlags - 64) * 256));
			AdjustStopperFlag(pushableItem, pushableItem->ItemFlags[0], false);
		}
	}
	else
	{
		auto bounds = GameBoundingBox(pushableItem);
		PushableBlockBounds.BoundingBox.X1 = (bounds.X1 / 2) - 100;
		PushableBlockBounds.BoundingBox.X2 = (bounds.X2 / 2) + 100;
		PushableBlockBounds.BoundingBox.Z1 = bounds.Z1 - 200;
		PushableBlockBounds.BoundingBox.Z2 = 0;

		short rot = pushableItem->Pose.Orientation.y;
		pushableItem->Pose.Orientation.y = (laraItem->Pose.Orientation.y + ANGLE(45.0f)) & 0xC000;

		if (TestPlayerEntityInteract(PushableBlockBounds, pushableItem, laraItem))
		{
			unsigned short quadrant = (unsigned short)((pushableItem->Pose.Orientation.y / 0x4000) + ((rot + 0x2000) / 0x4000));
			if (quadrant & 1)
				PushableBlockPos.z = bounds.X1 - 35;
			else
				PushableBlockPos.z = bounds.Z1 - 35;

			if (pushableInfo->hasFloorCeiling)
			{					
				// For now don't use auto-align function because it can collide with climb up moves of Lara

				laraItem->Pose.Orientation.x = pushableItem->Pose.Orientation.x;
				laraItem->Pose.Orientation.y = pushableItem->Pose.Orientation.y;
				laraItem->Pose.Orientation.z = pushableItem->Pose.Orientation.z;

				laraItem->Animation.AnimNumber = LA_PUSHABLE_GRAB;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.ActiveState = LS_PUSHABLE_GRAB;
				laraItem->Animation.TargetState = LS_PUSHABLE_GRAB;
				laraInfo->Control.IsMoving = false;
				laraInfo->Control.HandStatus = HandStatus::Busy;
				laraInfo->NextCornerPos.Position.x = itemNumber;
				pushableItem->Pose.Orientation.y = rot;
			}
			else
			{
				if (AlignPlayerToEntity(pushableItem, laraItem, PushableBlockPos))
				{
					laraItem->Animation.AnimNumber = LA_PUSHABLE_GRAB;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					laraItem->Animation.ActiveState = LS_PUSHABLE_GRAB;
					laraItem->Animation.TargetState = LS_PUSHABLE_GRAB;
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;
					laraInfo->NextCornerPos.Position.x = itemNumber;
					pushableItem->Pose.Orientation.y = rot;
				}
				else
				{
					laraInfo->InteractedItem = itemNumber;
					pushableItem->Pose.Orientation.y = rot;
				}
			}
		}
		else
		{
			if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
			{
				laraInfo->Control.IsMoving = false;
				laraInfo->Control.HandStatus = HandStatus::Free;
			}

			pushableItem->Pose.Orientation.y = rot;
		}
	}
}

void PushLoop(ItemInfo* item) // Do Flipeffect 18 in anims
{
	DoPushPull = 1;
}

void PushEnd(ItemInfo* item) // Do Flipeffect 19 in anims
{
	if (DoPushPull == 1)
		DoPushPull = -1;
}

bool TestBlockMovable(ItemInfo* item, int blokhite)
{
	short roomNumber = item->RoomNumber;
	FloorInfo* floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);

	if (floor->IsWall(floor->SectorPlane(item->Pose.Position.x, item->Pose.Position.z)))
		return false;

	if (floor->FloorHeight(item->Pose.Position.x, item->Pose.Position.z) != item->Pose.Position.y)
		return false;

	return true;
}

bool TestBlockPush(ItemInfo* item, int blockHeight, unsigned short quadrant)
{
	auto* info = (PushableInfo*)item->Data;

	int x = item->Pose.Position.x;
	int y = item->Pose.Position.y;
	int z = item->Pose.Position.z;
	
	switch (quadrant)
	{
	case NORTH:
		z += SECTOR(1);
		break;

	case EAST:
		x += SECTOR(1);
		break;

	case SOUTH:
		z -= SECTOR(1);
		break;

	case WEST:
		x -= SECTOR(1);
		break;
	}

	auto probe = GetCollision(x, y - blockHeight, z, item->RoomNumber);

	auto* room = &g_Level.Rooms[probe.RoomNumber];
	if (GetSector(room, x - room->x, z - room->z)->Stopper)
		return false;

	if (probe.Position.FloorSlope || probe.Position.DiagonalStep ||
		probe.Block->FloorSlope(0) != Vector2::Zero || probe.Block->FloorSlope(1) != Vector2::Zero)
		return false;

	if (info->canFall)
	{
		if (probe.Position.Floor < y)
			return false;
	}
	else
	{
		if (probe.Position.Floor != y)
			return false;
	}

	int ceiling = y - blockHeight + 100;

	if (GetCollision(x, ceiling, z, item->RoomNumber).Position.Ceiling > ceiling)
		return false;

	int oldX = item->Pose.Position.x;
	int oldZ = item->Pose.Position.z;
	item->Pose.Position.x = x;
	item->Pose.Position.z = z;
	GetCollidedObjects(item, 256, true, &CollidedItems[0], nullptr, 1);
	item->Pose.Position.x = oldX;
	item->Pose.Position.z = oldZ;

	int i = 0;
	while (CollidedItems[i] != NULL)
	{
		if (Objects[CollidedItems[i]->ObjectNumber].floor == NULL)
			return false;
		else
		{
			const auto& object = Objects[CollidedItems[i]->ObjectNumber];
			int collidedIndex = CollidedItems[i] - g_Level.Items.data(); // index of CollidedItems[i]

			int xCol = CollidedItems[i]->Pose.Position.x;
			int yCol = CollidedItems[i]->Pose.Position.y;
			int zCol = CollidedItems[i]->Pose.Position.z;

			// check if floor function returns nullopt
			if (object.floor(collidedIndex, xCol, yCol, zCol) == std::nullopt)
				return false;
		}

		i++;
	}

	return true;
}

bool TestBlockPull(ItemInfo* item, int blockHeight, short quadrant)
{
	int xadd = 0;
	int zadd = 0;

	switch (quadrant)
	{
	case NORTH:
		zadd = -SECTOR(1);
		break;

	case EAST:
		xadd = -SECTOR(1);
		break;

	case SOUTH:
		zadd = SECTOR(1);
		break;

	case WEST:
		xadd = SECTOR(1);
		break;
	}

	int x = item->Pose.Position.x + xadd;
	int y = item->Pose.Position.y;
	int z = item->Pose.Position.z + zadd;

	short roomNumber = item->RoomNumber;
	auto* room = &g_Level.Rooms[roomNumber];
	if (GetSector(room, x - room->x, z - room->z)->Stopper)
		return false;

	auto probe = GetCollision(x, y - blockHeight, z, item->RoomNumber);

	if (probe.Position.Floor != y)
		return false;

	if (probe.Position.FloorSlope || probe.Position.DiagonalStep ||
		probe.Block->FloorSlope(0) != Vector2::Zero || probe.Block->FloorSlope(1) != Vector2::Zero)
		return false;

	int ceiling = y - blockHeight + 100;

	if (GetCollision(x, ceiling, z, item->RoomNumber).Position.Ceiling > ceiling)
		return false;

	int oldX = item->Pose.Position.x;
	int oldZ = item->Pose.Position.z;
	item->Pose.Position.x = x;
	item->Pose.Position.z = z;
	GetCollidedObjects(item, 256, true, &CollidedItems[0], 0, 1);
	item->Pose.Position.x = oldX;
	item->Pose.Position.z = oldZ;

	int i = 0;
	while (CollidedItems[i] != NULL)
	{
		if (Objects[CollidedItems[i]->ObjectNumber].floor == NULL)
			return false;
		else
		{
			const auto& object = Objects[CollidedItems[i]->ObjectNumber];
			int collidedIndex = CollidedItems[i] - g_Level.Items.data();

			int xCol = CollidedItems[i]->Pose.Position.x;
			int yCol = CollidedItems[i]->Pose.Position.y;
			int zCol = CollidedItems[i]->Pose.Position.z;

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

	x = LaraItem->Pose.Position.x + xadd + xAddLara;
	z = LaraItem->Pose.Position.z + zadd + zAddLara;

	roomNumber = LaraItem->RoomNumber;

	probe = GetCollision(x, y - LARA_HEIGHT, z, LaraItem->RoomNumber);

	room = &g_Level.Rooms[roomNumber];
	if (GetSector(room, x - room->x, z - room->z)->Stopper)
		return false;

	if (probe.Position.Floor != y)
		return false;

	if (probe.Block->CeilingHeight(x, z) > y - LARA_HEIGHT)
		return false;

	oldX = LaraItem->Pose.Position.x;
	oldZ = LaraItem->Pose.Position.z;
	LaraItem->Pose.Position.x = x;
	LaraItem->Pose.Position.z = z;
	GetCollidedObjects(LaraItem, 256, true, &CollidedItems[0], 0, 1);
	LaraItem->Pose.Position.x = oldX;
	LaraItem->Pose.Position.z = oldZ;

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
				int xCol = CollidedItems[i]->Pose.Position.x;
				int yCol = CollidedItems[i]->Pose.Position.y;
				int zCol = CollidedItems[i]->Pose.Position.z;

				if (object.floor(collidedIndex, xCol, yCol, zCol) == std::nullopt)
					return false;
			}
		}

		i++;
	}

	return true;
}

void MoveStackXZ(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	short probedRoomNumber = GetCollision(item).RoomNumber;
	if (probedRoomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, probedRoomNumber);

	auto* stackItem = item;
	while (stackItem->ItemFlags[1] != NO_ITEM) // move pushblock stack together with bottom pushblock
	{
		int stackIndex = stackItem->ItemFlags[1];
		stackItem = &g_Level.Items[stackIndex];

		stackItem->Pose.Position.x = item->Pose.Position.x;
		stackItem->Pose.Position.z = item->Pose.Position.z;

		probedRoomNumber = GetCollision(item).RoomNumber;
		if (probedRoomNumber != stackItem->RoomNumber)
			ItemNewRoom(stackIndex, probedRoomNumber);
	}
}

void MoveStackY(short itemNumber, int y)
{
	auto* item = &g_Level.Items[itemNumber];

	short probedRoomNumber = GetCollision(item).RoomNumber;
	if (probedRoomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, probedRoomNumber);

	while (item->ItemFlags[1] != NO_ITEM) // move pushblock stack together with bottom pushblock
	{
		int stackIndex = item->ItemFlags[1];
		item = &g_Level.Items[stackIndex];

		item->Pose.Position.y += y;

		probedRoomNumber = GetCollision(item).RoomNumber;
		if (probedRoomNumber != item->RoomNumber)
			ItemNewRoom(stackIndex, probedRoomNumber);
	}
}

void AddBridgeStack(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* info = (PushableInfo*)item->Data;

	if (info->hasFloorCeiling)
		TEN::Floordata::AddBridge(itemNumber);

	int stackIndex = g_Level.Items[itemNumber].ItemFlags[1];
	while (stackIndex != NO_ITEM)
	{
		auto* stackItem = &g_Level.Items[stackIndex];

		if (info->hasFloorCeiling)
			TEN::Floordata::AddBridge(stackIndex);

		stackIndex = g_Level.Items[stackIndex].ItemFlags[1];
	}
}

void RemoveBridgeStack(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* info = (PushableInfo*)item->Data;

	if (info->hasFloorCeiling)
		TEN::Floordata::RemoveBridge(itemNumber);

	int stackIndex = g_Level.Items[itemNumber].ItemFlags[1];
	while (stackIndex != NO_ITEM)
	{
		auto* stackItem = &g_Level.Items[stackIndex];

		if (info->hasFloorCeiling)
			TEN::Floordata::RemoveBridge(stackIndex);

		stackIndex = g_Level.Items[stackIndex].ItemFlags[1];
	}
}

void RemoveFromStack(short itemNumber) // unlink pushable from stack if linked
{
	for (int i = 0; i < g_Level.NumItems; i++)
	{
		if (i == itemNumber)
			continue;

		auto* itemBelow = &g_Level.Items[i];

		int objectNumber = itemBelow->ObjectNumber;
		if (objectNumber >= ID_PUSHABLE_OBJECT1 && objectNumber <= ID_PUSHABLE_OBJECT10)
		{
			if (itemBelow->ItemFlags[1] == itemNumber)
				itemBelow->ItemFlags[1] = NO_ITEM;
		}
	}
}

int FindStack(short itemNumber)
{
	int stackTop = NO_ITEM; // index of heighest (Position.y) pushable in stack
	int stackYmin = CLICK(256); // set starting height

	//Check for pushable directly below current one in same sector
	for (int i = 0; i < g_Level.NumItems; i++)
	{
		if (i == itemNumber)
			continue;

		auto* itemBelow = &g_Level.Items[i];

		int objectNumber = itemBelow->ObjectNumber;
		if (objectNumber >= ID_PUSHABLE_OBJECT1 && objectNumber <= ID_PUSHABLE_OBJECT10)
		{
			auto* item = &g_Level.Items[itemNumber];
			int x = item->Pose.Position.x;
			int y = item->Pose.Position.y;
			int z = item->Pose.Position.z;

			if (itemBelow->Pose.Position.x == x && itemBelow->Pose.Position.z == z)
			{
				int belowY = itemBelow->Pose.Position.y;
				if (belowY > y && belowY < stackYmin)
				{
					// set heighest pushable so far as top of stack
					stackTop = i;
					stackYmin = itemBelow->Pose.Position.y;
				}
			}
		}
	}

	if (stackTop != NO_ITEM)
		g_Level.Items[stackTop].ItemFlags[1] = itemNumber;

	return stackTop;
}

int GetStackHeight(ItemInfo* item)
{
	auto* info = (PushableInfo*)item->Data;

	int height = info->height;

	auto* stackItem = item;
	while (stackItem->ItemFlags[1] != NO_ITEM)
	{
		stackItem = &g_Level.Items[stackItem->ItemFlags[1]];
		height += info->height;
	}

	return height;
}

bool CheckStackLimit(ItemInfo* item)
{
	auto* info = (PushableInfo*)item->Data;

	int limit = info->stackLimit;
	
	int count = 1;
	auto* stackItem = item;
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
	const auto& pushable = (PushableInfo&)item.Data;
	
	if (item.Status != ITEM_INVISIBLE && pushable.hasFloorCeiling)
	{
		const auto height = item.Pose.Position.y - (item.TriggerFlags & 0x1F) * CLICK(1);
		return std::optional{height};
	}
	return std::nullopt;
}

std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	const auto& pushable = (PushableInfo&)item.Data;

	if (item.Status != ITEM_INVISIBLE && pushable.hasFloorCeiling)
		return std::optional{item.Pose.Position.y};

	return std::nullopt;
}

int PushableBlockFloorBorder(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];
	const auto height = item.Pose.Position.y - (item.TriggerFlags & 0x1F) * CLICK(1);
	return height;
}

int PushableBlockCeilingBorder(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];
	return item.Pose.Position.y;
}
