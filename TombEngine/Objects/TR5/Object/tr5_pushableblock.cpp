#include "framework.h"
#include "Objects/TR5/Object/tr5_pushableblock.h"

#include "Game/animation.h"
#include "Game/items.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/control/box.h"
#include "Game/control/flipeffect.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Floordata;
using namespace TEN::Input;

namespace TEN::Entities::Generic
{
	// TODO: Derive from anim data.
	constexpr auto PUSHABLE_BOX_OFFSET_PUSH = 1108 - BLOCK(1);
	constexpr auto PUSHABLE_BOX_OFFSET_PULL = BLOCK(1) - 944;

	static auto PushableBlockPos = Vector3i::Zero;
	ObjectCollisionBounds PushableBlockBounds = 
	{
		GameBoundingBox(
			0, 0,
			-CLICK(0.25f), 0,
			0, 0),
		std::pair(
			EulerAngles(ANGLE(-10.0f), -LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), LARA_GRAB_THRESHOLD, ANGLE(10.0f)))
	};

	PushableInfo& GetPushableInfo(ItemInfo& item)
	{
		return (PushableInfo&)item.Data;
	}

	void InitialisePushableBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		item.Data = PushableInfo();
		auto& pushable = GetPushableInfo(item);

		item.ItemFlags[1] = NO_ITEM; // NOTE: ItemFlags[1] stores linked index.

		pushable.moveX = item.Pose.Position.x;
		pushable.moveZ = item.Pose.Position.z;

		// TODO: Attributes.
		pushable.stackLimit = 3;
		pushable.gravity = 8;
		pushable.weight = 100;
		pushable.loopSound = SFX_TR4_PUSHABLE_SOUND;
		pushable.stopSound = SFX_TR4_PUSH_BLOCK_END;
		pushable.fallSound = SFX_TR4_BOULDER_FALL;

		// Read OCB flags.
		int ocb = item.TriggerFlags;

		pushable.canFall = ocb & 0x20;
		pushable.disablePull = ocb & 0x80;
		pushable.disablePush = ocb & 0x100;
		pushable.disableW = pushable.disableE = ocb & 0x200;
		pushable.disableN = pushable.disableS = ocb & 0x400;

		// TODO: Must be a better way.
		pushable.climb = 0;
		/*
		pushable.climb |= (OCB & 0x800) ? CLIMB_WEST : 0;
		pushable.climb |= (OCB & 0x1000) ? CLIMB_NORTH : 0;
		pushable.climb |= (OCB & 0x2000) ? CLIMB_EAST : 0;
		pushable.climb |= (OCB & 0x4000) ? CLIMB_SOUTH : 0;
		*/
		pushable.hasFloorCeiling = false;

		int height;
		if (ocb & 0x40 && (ocb & 0x1F) >= 2)
		{
			pushable.hasFloorCeiling = true;
			TEN::Floordata::AddBridge(itemNumber);
			height = (ocb & 0x1F) * CLICK(1);
		}
		else
		{
			height = -GameBoundingBox(&item).Y1;
		}

		pushable.height = height;

		// Check for stack formation.
		FindStack(itemNumber);
	}

	void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber)
	{
		FloorInfo* floor = GetFloor(x, y, z, &roomNumber);
		if (floor->Box == NO_BOX)
			return;

		g_Level.Boxes[floor->Box].flags &= ~BLOCKED;
		int height = g_Level.Boxes[floor->Box].height;
		int baseRoomNumber = roomNumber;
	
		floor = GetFloor(x + BLOCK(1), y, z, &roomNumber);
		if (floor->Box != NO_BOX)
		{
			if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
				ClearMovableBlockSplitters(x + BLOCK(1), y, z, roomNumber);
		}

		roomNumber = baseRoomNumber;
		floor = GetFloor(x - BLOCK(1), y, z, &roomNumber);
		if (floor->Box != NO_BOX)
		{
			if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
				ClearMovableBlockSplitters(x - BLOCK(1), y, z, roomNumber);
		}

		roomNumber = baseRoomNumber;
		floor = GetFloor(x, y, z + BLOCK(1), &roomNumber);
		if (floor->Box != NO_BOX)
		{
			if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
				ClearMovableBlockSplitters(x, y, z + BLOCK(1), roomNumber);
		}

		roomNumber = baseRoomNumber;
		floor = GetFloor(x, y, z - BLOCK(1), &roomNumber);
		if (floor->Box != NO_BOX)
		{
			if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
				ClearMovableBlockSplitters(x, y, z - BLOCK(1), roomNumber);
		}
	}

	void PushableBlockControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(*item);

		Lara.InteractedItem = itemNumber;

		auto pos = Vector3i::Zero;

		int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);

		int x, z;
		int blockHeight = GetStackHeight(item);

		// Control block falling.
		if (item->Animation.IsAirborne)
		{
			int floorHeight = GetCollision(item->Pose.Position.x, item->Pose.Position.y + 10, item->Pose.Position.z, item->RoomNumber).Position.Floor;

			if (item->Pose.Position.y < (floorHeight - item->Animation.Velocity.y))
			{
				if ((item->Animation.Velocity.y + pushable.gravity) < 128)
					item->Animation.Velocity.y += pushable.gravity;
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
				SoundEffect(pushable.fallSound, &item->Pose, SoundEnvironment::Always);

				MoveStackY(itemNumber, relY);
				AddBridgeStack(itemNumber);

				if (FindStack(itemNumber) == NO_ITEM) // if fallen on some existing pushables, don't test triggers
					TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
			
				RemoveActiveItem(itemNumber);
				item->Status = ITEM_NOT_ACTIVE;

				if (pushable.hasFloorCeiling)
				{
					//AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
					AdjustStopperFlag(item, item->ItemFlags[0] + 0x8000, false);
				}
			}

			return;
		}

		int displaceBox = GameBoundingBox(LaraItem).Z2; // Move pushable based on bbox->Z2 of Lara
		auto prevPos = item->Pose.Position;

		switch (LaraItem->Animation.AnimNumber)
		{
		case LA_PUSHABLE_PUSH:
			displaceBox -= PUSHABLE_BOX_OFFSET_PUSH;

			if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase)
			{
				RemoveFromStack(itemNumber);
				RemoveBridgeStack(itemNumber);
			}

			switch (quadrant) 
			{
			case NORTH:
				z = pushable.moveZ + displaceBox;

				if (abs(item->Pose.Position.z - z) < BLOCK(0.5f) && item->Pose.Position.z < z)
					item->Pose.Position.z = z;
				
				break;

			case EAST:
				x = pushable.moveX + displaceBox;

				if (abs(item->Pose.Position.x - x) < BLOCK(0.5f) && item->Pose.Position.x < x)
					item->Pose.Position.x = x;

				break;

			case SOUTH:
				z = pushable.moveZ - displaceBox;

				if (abs(item->Pose.Position.z - z) < BLOCK(0.5f) && item->Pose.Position.z > z)
					item->Pose.Position.z = z;
				
				break;

			case WEST:
				x = pushable.moveX - displaceBox;

				if (abs(item->Pose.Position.x - x) < BLOCK(0.5f) && item->Pose.Position.x > x)
					item->Pose.Position.x = x;
				
				break;

			default:
				break;
			}

			MoveStackXZ(itemNumber);

			if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd - 1)
			{
				if (pushable.canFall) // check if pushable is about to fall
				{
					int floorHeight = GetCollision(item->Pose.Position.x, item->Pose.Position.y + 10, item->Pose.Position.z, item->RoomNumber).Position.Floor;
					if (floorHeight > item->Pose.Position.y)
					{
						item->Pose.Position.x = item->Pose.Position.x & 0xFFFFFE00 | 0x200;
						item->Pose.Position.z = item->Pose.Position.z & 0xFFFFFE00 | 0x200;
						MoveStackXZ(itemNumber);
						LaraItem->Animation.TargetState = LS_IDLE;

						pushable.MovementState = PushableMovementState::None;

						item->Animation.IsAirborne = true; // Do fall.
						return;
					}
				}

				if (IsHeld(In::Action))
				{
					if (!TestBlockPush(item, blockHeight, quadrant))
					{
						LaraItem->Animation.TargetState = LS_IDLE;
					}
					else
					{
						item->Pose.Position.x = pushable.moveX = item->Pose.Position.x & 0xFFFFFE00 | 0x200;
						item->Pose.Position.z = pushable.moveZ = item->Pose.Position.z & 0xFFFFFE00 | 0x200;
						TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
					}
				}
				else
				{
					LaraItem->Animation.TargetState = LS_IDLE;
				}
			}

			break;

		case LA_PUSHABLE_PULL:
			displaceBox -= PUSHABLE_BOX_OFFSET_PULL;

			if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase)
			{
				RemoveFromStack(itemNumber);
				RemoveBridgeStack(itemNumber);
			}

			switch (quadrant)
			{
			case NORTH:
				z = pushable.moveZ + displaceBox;
				if (abs(item->Pose.Position.z - z) < BLOCK(0.5f) && item->Pose.Position.z > z)
					item->Pose.Position.z = z;

				break;

			case EAST:
				x = pushable.moveX + displaceBox;
				if (abs(item->Pose.Position.x - x) < BLOCK(0.5f) && item->Pose.Position.x > x)
					item->Pose.Position.x = x;

				break;

			case SOUTH:
				z = pushable.moveZ - displaceBox;
				if (abs(item->Pose.Position.z - z) < BLOCK(0.5f) && item->Pose.Position.z < z)
					item->Pose.Position.z = z;

				break;

			case WEST:
				x = pushable.moveX - displaceBox;
				if (abs(item->Pose.Position.x - x) < BLOCK(0.5f) && item->Pose.Position.x < x)
					item->Pose.Position.x = x;

				break;

			default:
				break;
			}

			MoveStackXZ(itemNumber);

			if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd - 1)
			{
				if (IsHeld(In::Action))
				{
					if (!TestBlockPull(item, blockHeight, quadrant))
					{
						LaraItem->Animation.TargetState = LS_IDLE;
					}
					else
					{
						item->Pose.Position.x = pushable.moveX = item->Pose.Position.x & 0xFFFFFE00 | 0x200;
						item->Pose.Position.z = pushable.moveZ = item->Pose.Position.z & 0xFFFFFE00 | 0x200;
						TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
					}
				}
				else
				{
					LaraItem->Animation.TargetState = LS_IDLE;
				}
			}

			break;

		case LA_PUSHABLE_GRAB:
		case LA_PUSHABLE_RELEASE:
		case LA_PUSHABLE_PUSH_TO_STAND:
		case LA_PUSHABLE_PULL_TO_STAND:
			break;

		default:
			if (item->Status == ITEM_ACTIVE)
			{
				item->Pose.Position.x = item->Pose.Position.x & 0xFFFFFE00 | 0x200;
				item->Pose.Position.z = item->Pose.Position.z & 0xFFFFFE00 | 0x200;

				MoveStackXZ(itemNumber);
				FindStack(itemNumber);
				AddBridgeStack(itemNumber);

				TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);

				RemoveActiveItem(itemNumber);
				item->Status = ITEM_NOT_ACTIVE;

				if (pushable.hasFloorCeiling)
				{
					//AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
					AdjustStopperFlag(item, item->ItemFlags[0] + 0x8000, false);
				}
			}

			break;
		}

		// Set pushable movement state.
		float distance = Vector3i::Distance(item->Pose.Position, prevPos);
		if (distance > 0.0f)
			PushLoop(item);
		else
			PushEnd(item);

		// Do sound effects.
		if (pushable.MovementState == PushableMovementState::Moving)
		{
			SoundEffect(pushable.loopSound, &item->Pose, SoundEnvironment::Always);
		}
		else if (pushable.MovementState == PushableMovementState::Stopping)
		{
			pushable.MovementState = PushableMovementState::None;
			SoundEffect(pushable.stopSound, &item->Pose, SoundEnvironment::Always);
		}
	}

	void PushableBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* pushableItem = &g_Level.Items[itemNumber];
		auto* pushable = (PushableInfo*)pushableItem->Data;

		int blockHeight = GetStackHeight(pushableItem);
	
		if ((!IsHeld(In::Action) ||
			laraItem->Animation.ActiveState != LS_IDLE ||
			laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
			laraItem->Animation.IsAirborne ||
			lara->Control.HandStatus != HandStatus::Free ||
			pushableItem->Status == ITEM_INVISIBLE ||
			pushableItem->TriggerFlags < 0) &&
			(!lara->Control.IsMoving || lara->InteractedItem != itemNumber))
		{
			if (laraItem->Animation.ActiveState != LS_PUSHABLE_GRAB ||
				!TestLastFrame(laraItem, LA_PUSHABLE_GRAB) ||
				lara->NextCornerPos.Position.x != itemNumber)
			{
				if (!pushable->hasFloorCeiling)
					ObjectCollision(itemNumber, laraItem, coll);

				return;
			}

			short quadrant = (unsigned short)(LaraItem->Pose.Orientation.y + ANGLE(45.0f)) / ANGLE(90.0f);

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

			if (!CheckStackLimit(pushableItem))
				return;

			if (IsHeld(In::Forward))
			{
				if (!TestBlockPush(pushableItem, blockHeight, quadrant) || pushable->disablePush)
					return;

				laraItem->Animation.TargetState = LS_PUSHABLE_PUSH;
			}
			else if (IsHeld(In::Back))
			{
				if (!TestBlockPull(pushableItem, blockHeight, quadrant) || pushable->disablePull)
					return;

				laraItem->Animation.TargetState = LS_PUSHABLE_PULL;
			}
			else
			{
				return;
			}

			pushableItem->Status = ITEM_ACTIVE;
			AddActiveItem(itemNumber);
			ResetLaraFlex(laraItem);
		
			pushable->moveX = pushableItem->Pose.Position.x;
			pushable->moveZ = pushableItem->Pose.Position.z;

			if (pushable->hasFloorCeiling)
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

			if (TestLaraPosition(PushableBlockBounds, pushableItem, laraItem))
			{
				unsigned short quadrant = (unsigned short)((pushableItem->Pose.Orientation.y / 0x4000) + ((rot + 0x2000) / 0x4000));
				if (quadrant & 1)
					PushableBlockPos.z = bounds.X1 - CLICK(0.4f);
				else
					PushableBlockPos.z = bounds.Z1 - CLICK(0.4f);

				if (pushable->hasFloorCeiling)
				{					
					// For now don't use auto-align function because it can collide with climb up moves of Lara

					SetAnimation(laraItem, LA_PUSHABLE_GRAB);
					laraItem->Pose.Orientation = pushableItem->Pose.Orientation;
					lara->Control.IsMoving = false;
					lara->Control.HandStatus = HandStatus::Busy;
					lara->NextCornerPos.Position.x = itemNumber;
					pushableItem->Pose.Orientation.y = rot;
				}
				else
				{
					if (MoveLaraPosition(PushableBlockPos, pushableItem, laraItem))
					{
						SetAnimation(laraItem, LA_PUSHABLE_GRAB);
						lara->Control.IsMoving = false;
						lara->Control.HandStatus = HandStatus::Busy;
						lara->NextCornerPos.Position.x = itemNumber;
						pushableItem->Pose.Orientation.y = rot;
					}
					else
					{
						lara->InteractedItem = itemNumber;
						pushableItem->Pose.Orientation.y = rot;
					}
				}
			}
			else
			{
				if (lara->Control.IsMoving && lara->InteractedItem == itemNumber)
				{
					lara->Control.IsMoving = false;
					lara->Control.HandStatus = HandStatus::Free;
				}

				pushableItem->Pose.Orientation.y = rot;
			}
		}
	}

	void PushLoop(ItemInfo* item)
	{
		auto* pushable = (PushableInfo*)item->Data;

		pushable->MovementState = PushableMovementState::Moving;
	}

	void PushEnd(ItemInfo* item)
	{
		auto* pushable = (PushableInfo*)item->Data;

		if (pushable->MovementState == PushableMovementState::Moving)
			pushable->MovementState = PushableMovementState::Stopping;
	}

	bool TestBlockMovable(ItemInfo* item, int blockHeight)
	{
		RemoveBridge(item->Index);
		auto pointColl = GetCollision(item);
		AddBridge(item->Index);

		if (pointColl.Block->IsWall(pointColl.Block->SectorPlane(item->Pose.Position.x, item->Pose.Position.z)))
			return false;

		if (pointColl.Position.Floor != item->Pose.Position.y)
			return false;

		return true;
	}

	bool TestBlockPush(ItemInfo* item, int blockHeight, unsigned short quadrant)
	{
		if (!TestBlockMovable(item, blockHeight))
			return false;

		auto* pushable = (PushableInfo*)item->Data;

		int x = item->Pose.Position.x;
		int y = item->Pose.Position.y;
		int z = item->Pose.Position.z;
	
		switch (quadrant)
		{
		case NORTH:
			z += BLOCK(1);
			break;

		case EAST:
			x += BLOCK(1);
			break;

		case SOUTH:
			z -= BLOCK(1);
			break;

		case WEST:
			x -= BLOCK(1);
			break;
		}

		auto probe = GetCollision(x, y - blockHeight, z, item->RoomNumber);

		auto* room = &g_Level.Rooms[probe.RoomNumber];
		if (GetSector(room, x - room->x, z - room->z)->Stopper)
			return false;

		if (probe.Position.FloorSlope || probe.Position.DiagonalStep ||
			probe.Block->FloorSlope(0) != Vector2::Zero || probe.Block->FloorSlope(1) != Vector2::Zero)
			return false;

		if (pushable->canFall)
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
		GetCollidedObjects(item, BLOCK(0.25f), true, &CollidedItems[0], &CollidedMeshes[0], true);
		item->Pose.Position.x = oldX;
		item->Pose.Position.z = oldZ;

		if (CollidedMeshes[0])
			return false;

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			if (!CollidedItems[i])
				break;

			if (Objects[CollidedItems[i]->ObjectNumber].isPickup)
				continue;

			if (Objects[CollidedItems[i]->ObjectNumber].floor == nullptr)
				return false;
		
			const auto& object = Objects[CollidedItems[i]->ObjectNumber];
			int collidedIndex = CollidedItems[i] - g_Level.Items.data(); // index of CollidedItems[i]

			auto colPos = CollidedItems[i]->Pose.Position;

			// Check if floor function returns nullopt.
			if (object.floor(collidedIndex, colPos.x, colPos.y, colPos.z) == std::nullopt)
				return false;
		}

		return true;
	}

	bool TestBlockPull(ItemInfo* item, int blockHeight, short quadrant)
	{
		if (!TestBlockMovable(item, blockHeight))
			return false;

		int xadd = 0;
		int zadd = 0;

		switch (quadrant)
		{
		case NORTH:
			zadd = -BLOCK(1);
			break;

		case EAST:
			xadd = -BLOCK(1);
			break;

		case SOUTH:
			zadd = BLOCK(1);
			break;

		case WEST:
			xadd = BLOCK(1);
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
		GetCollidedObjects(item, BLOCK(0.25f), true, &CollidedItems[0], &CollidedMeshes[0], true);
		item->Pose.Position.x = oldX;
		item->Pose.Position.z = oldZ;

		if (CollidedMeshes[0])
			return false;

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			if (!CollidedItems[i])
				break;

			if (Objects[CollidedItems[i]->ObjectNumber].isPickup)
				continue;

			if (Objects[CollidedItems[i]->ObjectNumber].floor == nullptr)
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
		GetCollidedObjects(LaraItem, LARA_RADIUS, true, &CollidedItems[0], &CollidedMeshes[0], true);
		LaraItem->Pose.Position.x = oldX;
		LaraItem->Pose.Position.z = oldZ;

		if (CollidedMeshes[0])
			return false;

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			if (!CollidedItems[i])
				break;

			if (CollidedItems[i] == item) // If collided item is not pushblock in which lara embedded
				continue;

			if (Objects[CollidedItems[i]->ObjectNumber].isPickup)
				continue;

			if (Objects[CollidedItems[i]->ObjectNumber].floor == nullptr)
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
		auto* pushable = (PushableInfo*)item->Data;

		if (pushable->hasFloorCeiling)
			TEN::Floordata::AddBridge(itemNumber);

		int stackIndex = g_Level.Items[itemNumber].ItemFlags[1];
		while (stackIndex != NO_ITEM)
		{
			auto* stackItem = &g_Level.Items[stackIndex];

			if (pushable->hasFloorCeiling)
				TEN::Floordata::AddBridge(stackIndex);

			stackIndex = g_Level.Items[stackIndex].ItemFlags[1];
		}
	}

	void RemoveBridgeStack(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* pushable = (PushableInfo*)item->Data;

		if (pushable->hasFloorCeiling)
			TEN::Floordata::RemoveBridge(itemNumber);

		int stackIndex = g_Level.Items[itemNumber].ItemFlags[1];
		while (stackIndex != NO_ITEM)
		{
			auto* stackItem = &g_Level.Items[stackIndex];

			if (pushable->hasFloorCeiling)
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
		auto* pushable = (PushableInfo*)item->Data;

		int height = pushable->height;

		auto* stackItem = item;
		while (stackItem->ItemFlags[1] != NO_ITEM)
		{
			stackItem = &g_Level.Items[stackItem->ItemFlags[1]];
			height += pushable->height;
		}

		return height;
	}

	bool CheckStackLimit(ItemInfo* item)
	{
		auto* pushable = (PushableInfo*)item->Data;

		int limit = pushable->stackLimit;
	
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
		auto bboxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, false);
	
		if (item.Status != ITEM_INVISIBLE && pushable.hasFloorCeiling && bboxHeight.has_value())
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
		auto bboxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, true);

		if (item.Status != ITEM_INVISIBLE && pushable.hasFloorCeiling && bboxHeight.has_value())
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
}
