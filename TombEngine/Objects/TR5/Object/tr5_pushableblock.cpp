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
	constexpr auto PUSHABLE_FALL_VELOCITY_MAX	 = BLOCK(1 / 8.0f);
	constexpr auto PUSHABLE_FALL_RUMBLE_VELOCITY = 96.0f;

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

	PushableInfo* GetPushableInfo(ItemInfo* item)
	{
		return (PushableInfo*)item->Data;
	}

	void InitialisePushableBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		item.Data = PushableInfo();
		auto& pushableInfo = *GetPushableInfo(&item);

		item.ItemFlags[1] = NO_ITEM; // NOTE: ItemFlags[1] stores linked index.

		pushableInfo.moveX = item.Pose.Position.x;
		pushableInfo.moveZ = item.Pose.Position.z;

		// TODO: Attributes.
		pushableInfo.stackLimit = 3;
		pushableInfo.gravity = 8;
		pushableInfo.weight = 100;
		pushableInfo.loopSound = SFX_TR4_PUSHABLE_SOUND;
		pushableInfo.stopSound = SFX_TR4_PUSH_BLOCK_END;
		pushableInfo.fallSound = SFX_TR4_BOULDER_FALL;

		// Read OCB flags.
		int ocb = item.TriggerFlags;

		pushableInfo.canFall = ocb & 0x20;
		pushableInfo.disablePull = ocb & 0x80;
		pushableInfo.disablePush = ocb & 0x100;
		pushableInfo.disableW = pushableInfo.disableE = ocb & 0x200;
		pushableInfo.disableN = pushableInfo.disableS = ocb & 0x400;

		// TODO: Must be a better way.  //Struct with booleans N,S,E,W,Diagonal
		pushableInfo.climb = 0;
		/*
		pushable->climb |= (OCB & 0x800) ? CLIMB_WEST : 0;
		pushable->climb |= (OCB & 0x1000) ? CLIMB_NORTH : 0;
		pushable->climb |= (OCB & 0x2000) ? CLIMB_EAST : 0;
		pushable->climb |= (OCB & 0x4000) ? CLIMB_SOUTH : 0;
		*/
		pushableInfo.hasFloorCeiling = false;

		int height = 0;
		if (ocb & 0x40 && (ocb & 0x1F) >= 2)
		{
			pushableInfo.hasFloorCeiling = true;
			TEN::Floordata::AddBridge(itemNumber);
			height = (ocb & 0x1F) * CLICK(1);
		}
		else
		{
			height = -GameBoundingBox(&item).Y1;
		}

		pushableInfo.height = height;

		// Check for stack formation.
		FindStack(itemNumber);
	}

	void ClearMovableBlockSplitters(const Vector3i& pos, short roomNumber) // TODO: Update with the new collision functions
	{
		FloorInfo* floor = GetFloor(pos.x, pos.y, pos.z, &roomNumber);
		if (floor->Box == NO_BOX) //Not walkable (blue floor flag)
			return;

		g_Level.Boxes[floor->Box].flags &= ~BLOCKED;  //Unblock the isolated box (grey floor flag)

		int height = g_Level.Boxes[floor->Box].height;
		int baseRoomNumber = roomNumber;
	
		floor = GetFloor(pos.x + BLOCK(1), pos.y, pos.z, &roomNumber);
		if (floor->Box != NO_BOX)
		{
			if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
				ClearMovableBlockSplitters(Vector3i(pos.x + BLOCK(1), pos.y, pos.z), roomNumber);
		}

		roomNumber = baseRoomNumber;
		floor = GetFloor(pos.x - BLOCK(1), pos.y, pos.z, &roomNumber);
		if (floor->Box != NO_BOX)
		{
			if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
				ClearMovableBlockSplitters(Vector3i(pos.x - BLOCK(1), pos.y, pos.z), roomNumber);
		}

		roomNumber = baseRoomNumber;
		floor = GetFloor(pos.x, pos.y, pos.z + BLOCK(1), &roomNumber);
		if (floor->Box != NO_BOX)
		{
			if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
				ClearMovableBlockSplitters(Vector3i(pos.x, pos.y, pos.z + BLOCK(1)), roomNumber);
		}

		roomNumber = baseRoomNumber;
		floor = GetFloor(pos.x, pos.y, pos.z - BLOCK(1), &roomNumber);
		if (floor->Box != NO_BOX)
		{
			if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
				ClearMovableBlockSplitters(Vector3i(pos.x, pos.y, pos.z - BLOCK(1)), roomNumber);
		}
	}

	void PushableBlockControl(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = *GetPushableInfo(&pushableItem);

		Lara.InteractedItem = itemNumber;

		const int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);

		// Check if the pushable block is falling.
		if (pushableItem.Animation.IsAirborne)
		{ 
			const int floorHeight = GetCollision(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y + pushableInfo.gravity, pushableItem.Pose.Position.z, pushableItem.RoomNumber).Position.Floor;

			if (pushableItem.Pose.Position.y < (floorHeight - pushableItem.Animation.Velocity.y))
			{
				// Apply gravity to the pushable block.
				if ((pushableItem.Animation.Velocity.y + pushableInfo.gravity) < PUSHABLE_FALL_VELOCITY_MAX)
				{
					pushableItem.Animation.Velocity.y += pushableInfo.gravity;
				}
				else
				{
					pushableItem.Animation.Velocity.y = PUSHABLE_FALL_VELOCITY_MAX; //Originally item.Animation.Velocity.y++;
				}

				// Update the pushable block's position and move the block's stack.
				pushableItem.Pose.Position.y += pushableItem.Animation.Velocity.y;
				MoveStackY(itemNumber, pushableItem.Animation.Velocity.y);
			}
			else
			{
				// The pushable block has hit the ground.
				pushableItem.Animation.IsAirborne = false;
				const int relY = floorHeight - pushableItem.Pose.Position.y;
				pushableItem.Pose.Position.y = floorHeight;

				// Shake the floor if the pushable block fell at a high enough velocity.
				if (pushableItem.Animation.Velocity.y >= PUSHABLE_FALL_RUMBLE_VELOCITY)
				{
					FloorShake(&pushableItem);
				}

				pushableItem.Animation.Velocity.y = 0.0f;
				SoundEffect(pushableInfo.fallSound, &pushableItem.Pose, SoundEnvironment::Always);

				MoveStackY(itemNumber, relY);
				AddBridgeStack(itemNumber);

				// If fallen on top of existing pushable, don't test triggers.
				if (FindStack(itemNumber) == NO_ITEM)
					TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);
			
				RemoveActiveItem(itemNumber);
				pushableItem.Status = ITEM_NOT_ACTIVE;

				if (pushableInfo.hasFloorCeiling)
				{
					//AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
					AdjustStopperFlag(&pushableItem, pushableItem.ItemFlags[0] + ANGLE(180));
				}
			}

			return;
		}

		// Move pushable based on player bounds.Z2.
		int blockHeight = GetStackHeight(&pushableItem);
		int x, z;
		int displaceDepth = 0;
		int displaceBox = GameBoundingBox(LaraItem).Z2;
		auto prevPos = pushableItem.Pose.Position;

		switch (LaraItem->Animation.AnimNumber)
		{
		case LA_PUSHABLE_PUSH:
			displaceDepth = GetLastFrame(GAME_OBJECT_ID::ID_LARA, LaraItem->Animation.AnimNumber)->boundingBox.Z2;
			displaceBox -= displaceDepth - BLOCK(1);

			if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase)
			{
				RemoveFromStack(itemNumber);
				RemoveBridgeStack(itemNumber);
			}

			switch (quadrant) 
			{
			case NORTH:
				z = pushableInfo.moveZ + displaceBox;

				if (abs(pushableItem.Pose.Position.z - z) < BLOCK(0.5f) && pushableItem.Pose.Position.z < z)
					pushableItem.Pose.Position.z = z;
				
				break;

			case EAST:
				x = pushableInfo.moveX + displaceBox;

				if (abs(pushableItem.Pose.Position.x - x) < BLOCK(0.5f) && pushableItem.Pose.Position.x < x)
					pushableItem.Pose.Position.x = x;

				break;

			case SOUTH:
				z = pushableInfo.moveZ - displaceBox;

				if (abs(pushableItem.Pose.Position.z - z) < BLOCK(0.5f) && pushableItem.Pose.Position.z > z)
					pushableItem.Pose.Position.z = z;
				
				break;

			case WEST:
				x = pushableInfo.moveX - displaceBox;

				if (abs(pushableItem.Pose.Position.x - x) < BLOCK(0.5f) && pushableItem.Pose.Position.x > x)
					pushableItem.Pose.Position.x = x;
				
				break;

			default:
				break;
			}

			MoveStackXZ(itemNumber);

			if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd - 1)
			{
				// Check if pushable is about to fall.
				if (pushableInfo.canFall)
				{
					int floorHeight = GetCollision(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y + 10, pushableItem.Pose.Position.z, pushableItem.RoomNumber).Position.Floor;// repeated?
					if (floorHeight > pushableItem.Pose.Position.y)
					{
						pushableItem.Pose.Position.x = pushableItem.Pose.Position.x & 0xFFFFFE00 | 0x200;
						pushableItem.Pose.Position.z = pushableItem.Pose.Position.z & 0xFFFFFE00 | 0x200;
						MoveStackXZ(itemNumber);
						LaraItem->Animation.TargetState = LS_IDLE;

						pushableInfo.MovementState = PushableMovementState::None;

						pushableItem.Animation.IsAirborne = true;
						return;
					}
				}

				if (IsHeld(In::Action))
				{
					if (!TestBlockPush(&pushableItem, blockHeight, quadrant))
					{
						LaraItem->Animation.TargetState = LS_IDLE;
					}
					else
					{
						pushableItem.Pose.Position.x = pushableInfo.moveX = pushableItem.Pose.Position.x & 0xFFFFFE00 | 0x200;
						pushableItem.Pose.Position.z = pushableInfo.moveZ = pushableItem.Pose.Position.z & 0xFFFFFE00 | 0x200;
						TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);
					}
				}
				else
				{
					LaraItem->Animation.TargetState = LS_IDLE;
				}
			}

			break;

		case LA_PUSHABLE_PULL:
			displaceDepth = GetLastFrame(GAME_OBJECT_ID::ID_LARA, LaraItem->Animation.AnimNumber)->boundingBox.Z2;
			displaceBox -= BLOCK(1) + displaceDepth;

			if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase)
			{
				RemoveFromStack(itemNumber);
				RemoveBridgeStack(itemNumber);
			}

			switch (quadrant)
			{
			case NORTH:
				z = pushableInfo.moveZ + displaceBox;
				if (abs(pushableItem.Pose.Position.z - z) < BLOCK(0.5f) && pushableItem.Pose.Position.z > z)
					pushableItem.Pose.Position.z = z;

				break;

			case EAST:
				x = pushableInfo.moveX + displaceBox;
				if (abs(pushableItem.Pose.Position.x - x) < BLOCK(0.5f) && pushableItem.Pose.Position.x > x)
					pushableItem.Pose.Position.x = x;

				break;

			case SOUTH:
				z = pushableInfo.moveZ - displaceBox;
				if (abs(pushableItem.Pose.Position.z - z) < BLOCK(0.5f) && pushableItem.Pose.Position.z < z)
					pushableItem.Pose.Position.z = z;

				break;

			case WEST:
				x = pushableInfo.moveX - displaceBox;
				if (abs(pushableItem.Pose.Position.x - x) < BLOCK(0.5f) && pushableItem.Pose.Position.x < x)
					pushableItem.Pose.Position.x = x;

				break;

			default:
				break;
			}

			MoveStackXZ(itemNumber);

			if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd - 1)
			{
				if (IsHeld(In::Action))
				{
					if (!TestBlockPull(&pushableItem, blockHeight, quadrant))
					{
						LaraItem->Animation.TargetState = LS_IDLE;
					}
					else
					{
						pushableItem.Pose.Position.x = pushableInfo.moveX = pushableItem.Pose.Position.x & 0xFFFFFE00 | 0x200;
						pushableItem.Pose.Position.z = pushableInfo.moveZ = pushableItem.Pose.Position.z & 0xFFFFFE00 | 0x200;
						TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);
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
			if (pushableItem.Status == ITEM_ACTIVE)
			{
				pushableItem.Pose.Position.x = pushableItem.Pose.Position.x & 0xFFFFFE00 | 0x200;
				pushableItem.Pose.Position.z = pushableItem.Pose.Position.z & 0xFFFFFE00 | 0x200;

				MoveStackXZ(itemNumber);
				FindStack(itemNumber);
				AddBridgeStack(itemNumber);

				TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

				RemoveActiveItem(itemNumber);
				pushableItem.Status = ITEM_NOT_ACTIVE;

				if (pushableInfo.hasFloorCeiling)
				{
					//AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
					AdjustStopperFlag(&pushableItem, pushableItem.ItemFlags[0] + ANGLE(180));
				}
			}

			break;
		}

		// Set pushable movement state.
		float distance = Vector3i::Distance(pushableItem.Pose.Position, prevPos);
		if (distance > 0.0f)
		{
			pushableInfo.MovementState = PushableMovementState::Moving;
		}
		else
		{
			if (pushableInfo.MovementState == PushableMovementState::Moving)
				pushableInfo.MovementState = PushableMovementState::Stopping;
		}

		// Do sound effects.
		if (pushableInfo.MovementState == PushableMovementState::Moving)
		{
			SoundEffect(pushableInfo.loopSound, &pushableItem.Pose, SoundEnvironment::Always);
		}
		else if (pushableInfo.MovementState == PushableMovementState::Stopping)
		{
			pushableInfo.MovementState = PushableMovementState::None;
			SoundEffect(pushableInfo.stopSound, &pushableItem.Pose, SoundEnvironment::Always);
		}
	}

	void PushableBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];
		auto& pushableInfo = *GetPushableInfo(&item);
		auto& laraInfo = *GetLaraInfo(laraItem);

		int blockHeight = GetStackHeight(&item);
	
		if ((!IsHeld(In::Action) ||
			laraItem->Animation.ActiveState != LS_IDLE ||
			laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
			laraItem->Animation.IsAirborne ||
			laraInfo.Control.HandStatus != HandStatus::Free ||
			item.Status == ITEM_INVISIBLE ||
			item.TriggerFlags < 0) &&
			(!laraInfo.Control.IsMoving || laraInfo.InteractedItem != itemNumber))
		{
			if (laraItem->Animation.ActiveState != LS_PUSHABLE_GRAB ||
				!TestLastFrame(laraItem, LA_PUSHABLE_GRAB) ||
				laraInfo.NextCornerPos.Position.x != itemNumber)
			{
				if (!pushableInfo.hasFloorCeiling)
					ObjectCollision(itemNumber, laraItem, coll);

				return;
			}

			int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);
			bool isQuadrantDisabled = false;
			switch (quadrant)
			{
			case NORTH:
				isQuadrantDisabled = pushableInfo.disableN;
				break;

			case EAST:
				isQuadrantDisabled = pushableInfo.disableE;
				break;

			case SOUTH:
				isQuadrantDisabled = pushableInfo.disableS;
				break;

			case WEST:
				isQuadrantDisabled = pushableInfo.disableW;
				break;
			}

			if (isQuadrantDisabled)
				return;

			if (!CheckStackLimit(&item))
				return;

			if (IsHeld(In::Forward))
			{
				if (!TestBlockPush(&item, blockHeight, quadrant) || pushableInfo.disablePush)
					return;

				laraItem->Animation.TargetState = LS_PUSHABLE_PUSH;
			}
			else if (IsHeld(In::Back))
			{
				if (!TestBlockPull(&item, blockHeight, quadrant) || pushableInfo.disablePull)
					return;

				laraItem->Animation.TargetState = LS_PUSHABLE_PULL;
			}
			else
			{
				return;
			}

			item.Status = ITEM_ACTIVE;
			AddActiveItem(itemNumber);
			ResetLaraFlex(laraItem);
		
			pushableInfo.moveX = item.Pose.Position.x; //psitionX instead of moveX?
			pushableInfo.moveZ = item.Pose.Position.z;

			if (pushableInfo.hasFloorCeiling)
			{
				AdjustStopperFlag(&item, item.ItemFlags[0]);
			}
		}
		else
		{
			auto bounds = GameBoundingBox(&item);
			PushableBlockBounds.BoundingBox.X1 = (bounds.X1 / 2) - 100;
			PushableBlockBounds.BoundingBox.X2 = (bounds.X2 / 2) + 100;
			PushableBlockBounds.BoundingBox.Z1 = bounds.Z1 - 200;
			PushableBlockBounds.BoundingBox.Z2 = 0;

			short yOrient = item.Pose.Orientation.y;
			item.Pose.Orientation.y = GetQuadrant(laraItem->Pose.Orientation.y) * ANGLE(90.0f);

			if (TestLaraPosition(PushableBlockBounds, &item, laraItem))
			{
				int quadrant = GetQuadrant(item.Pose.Orientation.y);
				switch (quadrant)
				{
				case NORTH:
				case SOUTH:
					PushableBlockPos.z = bounds.Z1 - CLICK(0.4f);
					break;

				case EAST:
				case WEST:
					PushableBlockPos.z = bounds.X1 - CLICK(0.4f);
					break;

				default:
					break;
				}

				if (pushableInfo.hasFloorCeiling)
				{					
					// NOTE: Not using auto align function because it interferes with vaulting.

					SetAnimation(laraItem, LA_PUSHABLE_GRAB);
					laraItem->Pose.Orientation = item.Pose.Orientation;
					laraInfo.Control.IsMoving = false;
					laraInfo.Control.HandStatus = HandStatus::Busy;
					laraInfo.NextCornerPos.Position.x = itemNumber;
					item.Pose.Orientation.y = yOrient;
				}
				else
				{
					if (MoveLaraPosition(PushableBlockPos, &item, laraItem))
					{
						SetAnimation(laraItem, LA_PUSHABLE_GRAB);
						laraInfo.Control.IsMoving = false;
						laraInfo.Control.HandStatus = HandStatus::Busy;
						laraInfo.NextCornerPos.Position.x = itemNumber;
						item.Pose.Orientation.y = yOrient;
					}
					else
					{
						laraInfo.InteractedItem = itemNumber;
						item.Pose.Orientation.y = yOrient;
					}
				}
			}
			else
			{
				if (laraInfo.Control.IsMoving && laraInfo.InteractedItem == itemNumber)
				{
					laraInfo.Control.IsMoving = false;
					laraInfo.Control.HandStatus = HandStatus::Free;
				}

				item.Pose.Orientation.y = yOrient;
			}
		}
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

	bool TestBlockPush(ItemInfo* item, int blockHeight, int quadrant)
	{
		if (!TestBlockMovable(item, blockHeight))
			return false;

		const auto& pushable = GetPushableInfo(item);

		auto pos = item->Pose.Position;
		switch (quadrant)
		{
		case NORTH:
			pos.z += BLOCK(1);
			break;

		case EAST:
			pos.x += BLOCK(1);
			break;

		case SOUTH:
			pos.z -= BLOCK(1);
			break;

		case WEST:
			pos.x -= BLOCK(1);
			break;
		}

		auto pointColl = GetCollision(pos.x, pos.y - blockHeight, pos.z, item->RoomNumber);

		auto* room = &g_Level.Rooms[pointColl.RoomNumber];
		if (GetSector(room, pos.x - room->x, pos.z - room->z)->Stopper)
			return false;

		if (pointColl.Position.FloorSlope || pointColl.Position.DiagonalStep ||
			pointColl.Block->FloorSlope(0) != Vector2::Zero || pointColl.Block->FloorSlope(1) != Vector2::Zero)
			return false;

		if (pushable->canFall)
		{
			if (pointColl.Position.Floor < pos.y)
				return false;
		}
		else
		{
			if (pointColl.Position.Floor != pos.y)
				return false;
		}

		int ceiling = pos.y - blockHeight + 100;

		if (GetCollision(pos.x, ceiling, pos.z, item->RoomNumber).Position.Ceiling > ceiling)
			return false;

		auto prevPos = item->Pose.Position;
		item->Pose.Position = pos;
		GetCollidedObjects(item, BLOCK(0.25f), true, &CollidedItems[0], &CollidedMeshes[0], true);
		item->Pose.Position = prevPos;

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
			int collidedIndex = CollidedItems[i] - g_Level.Items.data(); // Index of CollidedItems[i].

			auto colPos = CollidedItems[i]->Pose.Position;

			// Check if floor function returns nullopt.
			if (object.floor(collidedIndex, colPos.x, colPos.y, colPos.z) == std::nullopt)
				return false;
		}

		return true;
	}

	bool TestBlockPull(ItemInfo* item, int blockHeight, int quadrant)
	{
		if (!TestBlockMovable(item, blockHeight))
			return false;

		auto offset = Vector3i::Zero;
		switch (quadrant)
		{
		case NORTH:
			offset = Vector3i(0, 0, -BLOCK(1));
			break;

		case EAST:
			offset = Vector3i(-BLOCK(1), 0, 0);
			break;

		case SOUTH:
			offset = Vector3i(0, 0, BLOCK(1));
			break;

		case WEST:
			offset = Vector3i(BLOCK(1), 0, 0);
			break;
		}

		auto pos = item->Pose.Position + offset;

		short roomNumber = item->RoomNumber;
		auto* room = &g_Level.Rooms[roomNumber];
		if (GetSector(room, pos.x - room->x, pos.z - room->z)->Stopper)
			return false;

		auto probe = GetCollision(pos.x, pos.y - blockHeight, pos.z, item->RoomNumber);

		if (probe.Position.Floor != pos.y)
			return false;

		if (probe.Position.FloorSlope || probe.Position.DiagonalStep ||
			probe.Block->FloorSlope(0) != Vector2::Zero || probe.Block->FloorSlope(1) != Vector2::Zero)
			return false;

		int ceiling = pos.y - blockHeight + 100;

		if (GetCollision(pos.x, ceiling, pos.z, item->RoomNumber).Position.Ceiling > ceiling)
			return false;

		int oldX = item->Pose.Position.x;
		int oldZ = item->Pose.Position.z;
		item->Pose.Position.x = pos.x;
		item->Pose.Position.z = pos.z;
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

		auto playerOffset = Vector3i::Zero;
		switch (quadrant)
		{
		case NORTH:
			playerOffset.z = GetBestFrame(LaraItem)->offsetZ;
			break;

		case EAST:
			playerOffset.x = GetBestFrame(LaraItem)->offsetZ;
			break;

		case SOUTH:
			playerOffset.z = -GetBestFrame(LaraItem)->offsetZ;
			break;

		case WEST:
			playerOffset.x = -GetBestFrame(LaraItem)->offsetZ;
			break;
		}

		pos = LaraItem->Pose.Position + offset + playerOffset;
		roomNumber = LaraItem->RoomNumber;

		probe = GetCollision(pos.x, pos.y - LARA_HEIGHT, pos.z, LaraItem->RoomNumber);

		room = &g_Level.Rooms[roomNumber];
		if (GetSector(room, pos.x - room->x, pos.z - room->z)->Stopper)
			return false;

		if (probe.Position.Floor != pos.y)
			return false;

		if (probe.Block->CeilingHeight(pos.x, pos.z) > pos.y - LARA_HEIGHT)
			return false;

		oldX = LaraItem->Pose.Position.x;
		oldZ = LaraItem->Pose.Position.z;
		LaraItem->Pose.Position.x = pos.x;
		LaraItem->Pose.Position.z = pos.z;
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
		while (stackItem->ItemFlags[1] != NO_ITEM) //If it has created a data structure for pushables, couldn't this be put there?
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
		auto* itemPtr = &g_Level.Items[itemNumber];

		short probedRoomNumber = GetCollision(itemPtr).RoomNumber;
		if (probedRoomNumber != itemPtr->RoomNumber)
			ItemNewRoom(itemNumber, probedRoomNumber);

		// Move stack together with bottom pushable->
		while (itemPtr->ItemFlags[1] != NO_ITEM)
		{
			int stackIndex = itemPtr->ItemFlags[1];
			itemPtr = &g_Level.Items[stackIndex];

			itemPtr->Pose.Position.y += y;

			probedRoomNumber = GetCollision(itemPtr).RoomNumber;
			if (probedRoomNumber != itemPtr->RoomNumber)
				ItemNewRoom(stackIndex, probedRoomNumber);
		}
	}

	void AddBridgeStack(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		const auto* pushable = GetPushableInfo(item);

		if (pushable->hasFloorCeiling)
			TEN::Floordata::AddBridge(itemNumber);

		int stackIndex = g_Level.Items[itemNumber].ItemFlags[1];
		while (stackIndex != NO_ITEM)
		{
			if (pushable->hasFloorCeiling)
				TEN::Floordata::AddBridge(stackIndex);

			stackIndex = g_Level.Items[stackIndex].ItemFlags[1];
		}
	}

	void RemoveBridgeStack(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		const auto* pushable = GetPushableInfo(item);

		if (pushable->hasFloorCeiling)
			TEN::Floordata::RemoveBridge(itemNumber);

		int stackIndex = g_Level.Items[itemNumber].ItemFlags[1];
		while (stackIndex != NO_ITEM)
		{
			if (pushable->hasFloorCeiling)
				TEN::Floordata::RemoveBridge(stackIndex);

			stackIndex = g_Level.Items[stackIndex].ItemFlags[1];
		}
	}

	void RemoveFromStack(short itemNumber) 
	{
		// Unlink pushable from stack.
		for (int i = 0; i < g_Level.NumItems; i++)
		{
			if (i == itemNumber)
				continue;

			auto& itemBelow = g_Level.Items[i];

			int objectNumber = itemBelow.ObjectNumber;
			if (objectNumber >= ID_PUSHABLE_OBJECT1 && objectNumber <= ID_PUSHABLE_OBJECT10)
			{
				if (itemBelow.ItemFlags[1] == itemNumber)
					itemBelow.ItemFlags[1] = NO_ITEM;
			}
		}
	}

	int FindStack(short itemNumber)
	{
		int stackTop = NO_ITEM;		// Index of heighest pushable in stack.
		int stackYmin = CLICK(256); // Set starting height.

		// Check for pushable directly below current one.
		for (int i = 0; i < g_Level.NumItems; i++)
		{
			if (i == itemNumber)
				continue;

			auto* itemBelow = &g_Level.Items[i];

			int objectNumber = itemBelow->ObjectNumber;
			if (objectNumber >= ID_PUSHABLE_OBJECT1 && objectNumber <= ID_PUSHABLE_OBJECT10)
			{
				const auto* item = &g_Level.Items[itemNumber];

				auto pos = item->Pose.Position;

				if (itemBelow->Pose.Position.x == pos.x &&
					itemBelow->Pose.Position.z == pos.z)
				{
					// Set heighest pushable so far as top of stack.
					int belowY = itemBelow->Pose.Position.y;
					if (belowY > pos.y && belowY < stackYmin)
					{
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
		auto* pushable = GetPushableInfo(item);

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
		auto* pushable = GetPushableInfo(item);

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
		auto* item = &g_Level.Items[itemNumber];
		const auto* pushable = GetPushableInfo(item);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, false);
	
		if (item->Status != ITEM_INVISIBLE && pushable->hasFloorCeiling && boxHeight.has_value())
		{
			const auto height = item->Pose.Position.y - (item->TriggerFlags & 0x1F) * CLICK(1); //Here goes the bounding box height.
			return std::optional{height};
		}

		return std::nullopt;
	}

	std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z)
	{
		auto* item = &g_Level.Items[itemNumber];
		const auto* pushable = GetPushableInfo(item);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, true);

		if (item->Status != ITEM_INVISIBLE && pushable->hasFloorCeiling && boxHeight.has_value())
			return std::optional{item->Pose.Position.y};

		return std::nullopt;
	}

	int PushableBlockFloorBorder(short itemNumber) //Again?
	{
		const auto* item = &g_Level.Items[itemNumber];

		const auto height = item->Pose.Position.y - (item->TriggerFlags & 0x1F) * CLICK(1);
		return height;
	}

	int PushableBlockCeilingBorder(short itemNumber) //Redundant isn't it?
	{
		const auto* item = &g_Level.Items[itemNumber];

		return item->Pose.Position.y;
	}
}
