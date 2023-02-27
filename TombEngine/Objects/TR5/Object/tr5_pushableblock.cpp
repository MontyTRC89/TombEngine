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

	// Main functions

	void InitialisePushableBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		item.Data = PushableInfo();
		auto& pushableInfo = *GetPushableInfo(&item);

		item.ItemFlags[1] = NO_ITEM; // NOTE: ItemFlags[1] stores linked index.

		pushableInfo.StartPos = item.Pose.Position;
		pushableInfo.StartPos.RoomNumber = item.RoomNumber;

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
		pushableInfo.doAlignCenter = false;
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
		//pushableInfo.hasFloorCeiling = false;


		pushableInfo.hasFloorColission = true;
		TEN::Floordata::AddBridge(itemNumber);
		int height = -GameBoundingBox(&item).Y1;

		pushableInfo.height = height;

		SetStopperFlag(pushableInfo.StartPos, true);

		// Check for stack formation.
		FindStack(itemNumber);
	}

	void PushableBlockControl(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = *GetPushableInfo(&pushableItem);

		if (pushableItem.Status != ITEM_ACTIVE)
			return;

		Lara.InteractedItem = itemNumber;
		bool isPushableMoving = false;
			
		if (PushableBlockManageGravity(pushableItem, pushableInfo, itemNumber))
			return;

		switch (LaraItem->Animation.AnimNumber)
		{
		case LA_PUSHABLE_PULL:
		case LA_PUSHABLE_PUSH:
			isPushableMoving = PushableBlockManageMoving(pushableItem, pushableInfo, itemNumber);
			break;

		case LA_PUSHABLE_GRAB:
		case LA_PUSHABLE_RELEASE:
		case LA_PUSHABLE_PUSH_TO_STAND:
		case LA_PUSHABLE_PULL_TO_STAND:
			break;

		default:
			PushableBlockManageIdle(pushableItem, pushableInfo, itemNumber);
			break;
		}

		// Set pushable movement state.
		if (isPushableMoving)
		{
			pushableInfo.MovementState = PushableMovementState::Moving;
		}
		else
		{
			if (pushableInfo.MovementState == PushableMovementState::Moving)
				pushableInfo.MovementState = PushableMovementState::Stopping;
		}

		// Do sound effects.
		PushableBlockManageSounds(pushableItem,pushableInfo);
	}

	void PushableBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = *GetPushableInfo(&pushableItem);
		auto& laraInfo = *GetLaraInfo(laraItem);

		const int blockHeight = GetStackHeight(&pushableItem);

		if ((IsHeld(In::Action) &&
			 !IsHeld(In::Forward) &&
			 laraItem->Animation.ActiveState == LS_IDLE &&
			 laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
			 !laraItem->Animation.IsAirborne &&
			 laraInfo.Control.HandStatus == HandStatus::Free &&
			 pushableItem.Status != ITEM_INVISIBLE &&
			 pushableItem.TriggerFlags >= 0) ||
			 (laraInfo.Control.IsMoving && laraInfo.InteractedItem == itemNumber))
		{

			auto bounds = GameBoundingBox(&pushableItem);
			PushableBlockBounds.BoundingBox.X1 = (bounds.X1 / 2) - 100;
			PushableBlockBounds.BoundingBox.X2 = (bounds.X2 / 2) + 100;
			PushableBlockBounds.BoundingBox.Z1 = bounds.Z1 - 200;
			PushableBlockBounds.BoundingBox.Z2 = 0;

			short yOrient = pushableItem.Pose.Orientation.y;
			pushableItem.Pose.Orientation.y = GetQuadrant(laraItem->Pose.Orientation.y) * ANGLE(90.0f);

			if (TestLaraPosition(PushableBlockBounds, &pushableItem, laraItem))
			{
				int quadrant = GetQuadrant(pushableItem.Pose.Orientation.y);
				switch (quadrant)
				{
				case NORTH:
					PushableBlockPos.z = bounds.Z1 - CLICK(0.4f);
					PushableBlockPos.x = pushableInfo.doAlignCenter ? 0 : LaraItem->Pose.Position.x - pushableItem.Pose.Position.x;
					break;

				case SOUTH:
					PushableBlockPos.z = bounds.Z1 - CLICK(0.4f);
					PushableBlockPos.x = pushableInfo.doAlignCenter ? 0 : pushableItem.Pose.Position.x - LaraItem->Pose.Position.x;
					break;

				case EAST:
					PushableBlockPos.z = bounds.X1 - CLICK(0.4f);
					PushableBlockPos.x = pushableInfo.doAlignCenter ? 0 : pushableItem.Pose.Position.z - LaraItem->Pose.Position.z;
					break;

				case WEST:
					PushableBlockPos.z = bounds.X1 - CLICK(0.4f);
					PushableBlockPos.x = pushableInfo.doAlignCenter ? 0 : LaraItem->Pose.Position.z - pushableItem.Pose.Position.z;
					break;

				default:
					break;
				}

				if (MoveLaraPosition(PushableBlockPos, &pushableItem, laraItem))
				{
					SetAnimation(laraItem, LA_PUSHABLE_GRAB);
					laraItem->Pose.Orientation = pushableItem.Pose.Orientation;
					laraInfo.Control.IsMoving = false;
					laraInfo.Control.HandStatus = HandStatus::Busy;
					laraInfo.NextCornerPos.Position.x = itemNumber;
				}
				else
				{
					laraInfo.InteractedItem = itemNumber;
				}
			}
			else
			{
				if (laraInfo.Control.IsMoving && laraInfo.InteractedItem == itemNumber)
				{
					laraInfo.Control.IsMoving = false;
					laraInfo.Control.HandStatus = HandStatus::Free;
				}
			}

			pushableItem.Pose.Orientation.y = yOrient;
		}
		else
		{
			//If player is not grabbing it, then just do object collision rutine.
			if (laraItem->Animation.ActiveState != LS_PUSHABLE_GRAB ||
				!TestLastFrame(laraItem, LA_PUSHABLE_GRAB) ||
				laraInfo.NextCornerPos.Position.x != itemNumber)
			{
				if (!pushableInfo.hasFloorColission)
					ObjectCollision(itemNumber, laraItem, coll);

				return;
			}

			//Otherwise, player can input push/pull actions, but first we check the requirements.
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

			if (!CheckStackLimit(&pushableItem))
				return;

			//All good, we manage now the input and actions.
			if (IsHeld(In::Forward))
			{
				GameVector NextPos = pushableItem.Pose.Position;
				NextPos.RoomNumber = pushableItem.RoomNumber;
				
				switch (quadrant)
				{
				case NORTH:
					NextPos.z = NextPos.z + BLOCK(1);
					break;

				case EAST:
					NextPos.x = NextPos.x + BLOCK(1);
					break;

				case SOUTH:
					NextPos.z = NextPos.z - BLOCK(1);
					break;

				case WEST:
					NextPos.x = NextPos.x - BLOCK(1);
					break;
				}

				if (!IsNextSectorValid(pushableItem, blockHeight, NextPos, false) || pushableInfo.disablePush)
					return;

				laraItem->Animation.TargetState = LS_PUSHABLE_PUSH;
				SetStopperFlag(NextPos, true);
			}
			else if (IsHeld(In::Back))
			{
				GameVector NextPos = pushableItem.Pose.Position;
				NextPos.RoomNumber = pushableItem.RoomNumber;

				switch (quadrant)
				{
				case NORTH:
					NextPos.z = NextPos.z - BLOCK(1);
					break;

				case EAST:
					NextPos.x = NextPos.x - BLOCK(1);
					break;

				case SOUTH:
					NextPos.z = NextPos.z + BLOCK(1);
					break;

				case WEST:
					NextPos.x = NextPos.x + BLOCK(1);
					break;
				}

				if (!IsNextSectorValid(pushableItem, blockHeight, NextPos, true) || pushableInfo.disablePull)
					return;

				laraItem->Animation.TargetState = LS_PUSHABLE_PULL;
				SetStopperFlag(NextPos, true);
			}
			else
			{
				return;
			}

			//If the object has started to move, we activate it to do its mechanics in the Control function.
			pushableItem.Status = ITEM_ACTIVE;
			AddActiveItem(itemNumber);
			ResetLaraFlex(laraItem);

			pushableInfo.StartPos = pushableItem.Pose.Position;
			pushableInfo.StartPos.RoomNumber = pushableItem.RoomNumber;
		}
	}

	// Behaviour functions

	bool PushableBlockManageGravity(ItemInfo& pushableItem, PushableInfo& pushableInfo, const short itemNumber)
	{
		// Check if the pushable block is falling.
		if (!pushableItem.Animation.IsAirborne)
		{
			return false;
		}

		const int floorHeight = GetCollision(
												pushableItem.Pose.Position.x, 
												pushableItem.Pose.Position.y + pushableInfo.gravity, 
												pushableItem.Pose.Position.z, 
												pushableItem.RoomNumber
											).Position.Floor;

		const float currentY = pushableItem.Pose.Position.y;
		const float velocityY = pushableItem.Animation.Velocity.y;

		if (currentY < (floorHeight - velocityY))
		{
			// Apply gravity to the pushable block.
			const float newVelocityY = velocityY + pushableInfo.gravity;
			pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_FALL_VELOCITY_MAX);
			
			// Update the pushable block's position and move the block's stack.
			const float deltaY = pushableItem.Animation.Velocity.y;
			pushableItem.Pose.Position.y = currentY + deltaY;
			MoveStackY(itemNumber, deltaY);
		}
		else
		{
			// The pushable block has hit the ground.
			pushableItem.Animation.IsAirborne = false;
			const int relY = floorHeight - currentY;
			pushableItem.Pose.Position.y = floorHeight;

			// Shake the floor if the pushable block fell at a high enough velocity.
			if (velocityY >= PUSHABLE_FALL_RUMBLE_VELOCITY)
			{
				FloorShake(&pushableItem);
			}

			pushableItem.Animation.Velocity.y = 0.0f;
			SoundEffect(pushableInfo.fallSound, &pushableItem.Pose, SoundEnvironment::Always);

			MoveStackY(itemNumber, relY);
			AddBridgeStack(itemNumber);

			// If fallen on top of existing pushable, don't test triggers.
			if (FindStack(itemNumber) == NO_ITEM)
			{
				const bool pushableActivationFlag = pushableItem.Flags & IFLAG_ACTIVATION_MASK;
				TestTriggers(&pushableItem, true, pushableActivationFlag);
			}

			RemoveActiveItem(itemNumber);
			pushableItem.Status = ITEM_NOT_ACTIVE;

			SetStopperFlag(pushableInfo.StartPos, false);
			pushableInfo.StartPos = pushableItem.Pose.Position;
			pushableInfo.StartPos.RoomNumber = pushableItem.RoomNumber;
			SetStopperFlag(pushableInfo.StartPos, true);
		}

		return true;
	}

	void PushableBlockManageIdle(ItemInfo& pushableItem, PushableInfo& pushableInfo, const short itemNumber)
	{
		if (pushableItem.Status != ITEM_ACTIVE)
			return;


		//If it's not moving, it places at center, do some last checks and then it deactivate itself.
		pushableItem.Pose.Position = PlaceInSectorCenter(pushableItem);

		MoveStackXZ(itemNumber);
		FindStack(itemNumber);
		AddBridgeStack(itemNumber);

		TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

		RemoveActiveItem(itemNumber);
		pushableItem.Status = ITEM_NOT_ACTIVE;
	}
	
	bool PushableBlockManageMoving(ItemInfo& pushableItem, PushableInfo& pushableInfo, const short itemNumber)
	{
		// Moves pushable based on player bounds.Z2.

		const int blockHeight = GetStackHeight(&pushableItem);
		const bool isLaraPulling = LaraItem->Animation.AnimNumber == LA_PUSHABLE_PULL; //else, she is pushing.

		int quadrantDir = GetQuadrant(LaraItem->Pose.Orientation.y);
		int newPosX = pushableInfo.StartPos.x;
		int newPosZ = pushableInfo.StartPos.z;
		int displaceDepth = 0;
		int displaceBox = GameBoundingBox(LaraItem).Z2;

		bool isMovingResult = false;

		displaceDepth = GetLastFrame(GAME_OBJECT_ID::ID_LARA, LaraItem->Animation.AnimNumber)->boundingBox.Z2;

		displaceBox -= isLaraPulling ? BLOCK(1) + displaceDepth : displaceDepth - BLOCK(1);

		if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase)
		{
			RemoveFromStack(itemNumber);
			RemoveBridgeStack(itemNumber);
		}

		// Update the position
		switch (quadrantDir)
		{
		case NORTH:
			newPosZ += displaceBox;
			break;

		case EAST:
			newPosX += displaceBox;
			break;

		case SOUTH:
			newPosZ -= displaceBox;
			break;

		case WEST:
			newPosX -= displaceBox;
			break;

		default:
			break;
		}

		if (abs(pushableItem.Pose.Position.z - newPosZ) < BLOCK(0.5f))
		{
			if (isLaraPulling)
			{
				if ((quadrantDir == NORTH && pushableItem.Pose.Position.z > newPosZ) ||
					(quadrantDir == SOUTH && pushableItem.Pose.Position.z < newPosZ))
				{
					pushableItem.Pose.Position.z = newPosZ;
					isMovingResult = true;
				}
			}
			else
			{
				if ((quadrantDir == NORTH && pushableItem.Pose.Position.z < newPosZ) ||
					(quadrantDir == SOUTH && pushableItem.Pose.Position.z > newPosZ))
				{
					pushableItem.Pose.Position.z = newPosZ;
					isMovingResult = true;
				}
			}
		}

		if (abs(pushableItem.Pose.Position.x - newPosX) < BLOCK(0.5f))
		{
			if (isLaraPulling)
			{
				if ((quadrantDir == EAST && pushableItem.Pose.Position.x > newPosX) ||
					(quadrantDir == WEST && pushableItem.Pose.Position.x < newPosX))
				{
					pushableItem.Pose.Position.x = newPosX;
					isMovingResult = true;
				}
			}
			else
			{
				if ((quadrantDir == EAST && pushableItem.Pose.Position.x < newPosX) ||
					(quadrantDir == WEST && pushableItem.Pose.Position.x > newPosX))
				{
					pushableItem.Pose.Position.x = newPosX;
					isMovingResult = true;
				}
			}
		}

		MoveStackXZ(itemNumber);
		
		//Manage the end of the animation
		if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd - 1)
		{
			SetStopperFlag(pushableInfo.StartPos, false);

			// Check if pushing through an edge into falling.
			if (pushableInfo.canFall && !isLaraPulling)
			{
				int floorHeight = GetCollision(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y + 10, pushableItem.Pose.Position.z, pushableItem.RoomNumber).Position.Floor;// repeated?
				if (floorHeight > pushableItem.Pose.Position.y)
				{
					pushableItem.Pose.Position = PlaceInSectorCenter(pushableItem);
					MoveStackXZ(itemNumber);
					LaraItem->Animation.TargetState = LS_IDLE;

					pushableInfo.MovementState = PushableMovementState::None;

					pushableItem.Animation.IsAirborne = true;
					return true;
				}
			}

			if (IsHeld(In::Action))
			{
				GameVector NextPos = pushableItem.Pose.Position;
				NextPos.RoomNumber = pushableItem.RoomNumber;

				if (isLaraPulling)
					quadrantDir = (quadrantDir + 2) % 4;

				switch (quadrantDir)
				{
				case NORTH:
					NextPos.z = NextPos.z + BLOCK(1);
					break;

				case EAST:
					NextPos.x = NextPos.x + BLOCK(1);
					break;

				case SOUTH:
					NextPos.z = NextPos.z - BLOCK(1);
					break;

				case WEST:
					NextPos.x = NextPos.x - BLOCK(1);
					break;
				}

				if (IsNextSectorValid(pushableItem, blockHeight, NextPos, isLaraPulling))
				{
					pushableItem.Pose.Position = PlaceInSectorCenter(pushableItem);
					TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

					pushableInfo.StartPos = pushableItem.Pose.Position;
					pushableInfo.StartPos.RoomNumber = pushableItem.RoomNumber;
					SetStopperFlag(NextPos, true);
				}
				else
				{
					LaraItem->Animation.TargetState = LS_IDLE;
				}
			}
			else
			{
				LaraItem->Animation.TargetState = LS_IDLE;
			}
		}

		return isMovingResult;
	}

	void PushableBlockManageSounds(const ItemInfo& pushableItem, PushableInfo& pushableInfo)
	{
		auto SoundSourcePose = pushableItem.Pose;
		if (pushableInfo.MovementState == PushableMovementState::Moving)
		{
			SoundEffect(pushableInfo.loopSound, &SoundSourcePose, SoundEnvironment::Always);
		}
		else if (pushableInfo.MovementState == PushableMovementState::Stopping)
		{
			pushableInfo.MovementState = PushableMovementState::None;
			SoundEffect(pushableInfo.stopSound, &SoundSourcePose, SoundEnvironment::Always);
		}
	}

	//Floor Data update functions
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

	// Test functions

	bool TestBlockMovable(ItemInfo& item, int blockHeight)
	{
		RemoveBridge(item.Index);
		auto pointColl = GetCollision(&item);
		AddBridge(item.Index);

		//It's in a wall
		if (pointColl.Block->IsWall(pointColl.Block->SectorPlane(item.Pose.Position.x, item.Pose.Position.z)))
			return false;

		//Or it isn't on the floor
		if (pointColl.Position.Floor != item.Pose.Position.y)
			return false;

		return true;
	}

	bool IsNextSectorValid(ItemInfo& pushableItem, const int blockHeight, const GameVector& targetPoint, const bool checkIfLaraFits)
	{
		if (!TestBlockMovable(pushableItem, blockHeight))
			return false;

		const auto& pushableInfo = *GetPushableInfo(&pushableItem);
		
		auto col = GetCollision(targetPoint);

		//It's in a wall
		if (col.Block->IsWall(targetPoint.x, targetPoint.z))
			return false;

		//Is a floor higher step
		if (col.Position.Floor < pushableItem.Pose.Position.y)
			return false;

		//Is it a sliding slope?
		if (col.Position.FloorSlope)
			return false;

		//Is diagonal floor?
		if (col.Position.DiagonalStep)
			return false;

		if ((col.Block->FloorSlope(0) != Vector2::Zero) || (col.Block->FloorSlope(1) != Vector2::Zero))
			return false;

		//Is a stopper flag tile?
		if (col.Block->Stopper)
			return false;

		//Is a gap, (Can it fall down?) (Only available for pushing).
		if (pushableInfo.canFall)
		{
			if (col.Position.Floor < pushableItem.Pose.Position.y)
				return false;
		}
		else
		{
			if (col.Position.Floor != pushableItem.Pose.Position.y)
				return false;
		}

		//Is ceiling (square or diagonal) high enough?
		int distanceToCeiling = abs(col.Position.Ceiling - col.Position.Floor);
		if (distanceToCeiling < blockHeight)
			return false;

		//Is there any enemy or object?
		auto prevPos = pushableItem.Pose.Position;
		pushableItem.Pose.Position = targetPoint.ToVector3i();
		GetCollidedObjects(&pushableItem, BLOCK(0.25f), true, &CollidedItems[0], &CollidedMeshes[0], true);
		pushableItem.Pose.Position = prevPos;

		if (CollidedMeshes[0])
			return false;

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			if (!CollidedItems[i])
				break;

			if (Objects[CollidedItems[i]->ObjectNumber].isPickup)
				continue;

			if (Objects[CollidedItems[i]->ObjectNumber].floor == nullptr) //¿¿??
				return false;

			const auto& object = Objects[CollidedItems[i]->ObjectNumber];
			int collidedIndex = CollidedItems[i] - g_Level.Items.data(); // Index of CollidedItems[i].

			auto colPos = CollidedItems[i]->Pose.Position;

			// Check if floor function returns nullopt.
			if (object.floor(collidedIndex, colPos.x, colPos.y, colPos.z) == std::nullopt)
				return false;
		}

		if (checkIfLaraFits)
		{
			if (!IsValidForLara(pushableItem, pushableInfo, targetPoint))
				return false;
		}

		return true;
	}

	bool IsValidForLara(const ItemInfo& pushableItem, const PushableInfo& pushableInfo, const GameVector& targetPoint)
	{
		auto playerOffset = Vector3i::Zero;
		auto dirVector = pushableItem.Pose.Position - targetPoint.ToVector3i();
		
		if (dirVector.z > 0)
		{
			playerOffset.z = GetBestFrame(LaraItem)->offsetZ + BLOCK(1);
		}
		else if (dirVector.x > 0)
		{
			playerOffset.x = GetBestFrame(LaraItem)->offsetZ + BLOCK(1);
		}
		else if (dirVector.z < 0)
		{
			playerOffset.z = -GetBestFrame(LaraItem)->offsetZ - BLOCK(1);
		}
		else
		{
			playerOffset.x = -GetBestFrame(LaraItem)->offsetZ - BLOCK(1);
		}
		
		GameVector laraDetectionPoint = LaraItem->Pose.Position + playerOffset;
		laraDetectionPoint.RoomNumber = LaraItem->RoomNumber;

		CollisionResult col; 
		if (pushableInfo.hasFloorColission)
		{
			RemoveBridge(pushableItem.Index);
			col = GetCollision(laraDetectionPoint);
			AddBridge(pushableItem.Index);
		}
		else
		{
			col = GetCollision(laraDetectionPoint);
		}

		//Is a stopper flag tile? (Lara may not need this, otherwise, it's needed to remove the stopper flag in the pushable to check this condition).
		//if (col.Block->Stopper)
			//return false;

		//If floor is not flat
		if (col.Position.Floor != LaraItem->Pose.Position.y)
			return false;

		//Is ceiling (square or diagonal) high enough?
		int distanceToCeiling = abs(col.Position.Ceiling - col.Position.Floor);
		if (distanceToCeiling < LARA_HEIGHT)
			return false;

		//Is there any enemy or object?
		auto prevPos = LaraItem->Pose.Position;
		LaraItem->Pose.Position = laraDetectionPoint.ToVector3i();
		GetCollidedObjects(LaraItem, LARA_RADIUS, true, &CollidedItems[0], &CollidedMeshes[0], true);
		LaraItem->Pose.Position = prevPos;

		if (CollidedMeshes[0])
			return false;

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			if (!CollidedItems[i])
				break;

			if (CollidedItems[i] == &pushableItem) // If collided item is not pushblock in which lara embedded
				continue;

			if (Objects[CollidedItems[i]->ObjectNumber].isPickup) // If it isn't a picukp
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

	// Stack utilities functions

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

		if (pushable->hasFloorColission)
			TEN::Floordata::AddBridge(itemNumber);

		int stackIndex = g_Level.Items[itemNumber].ItemFlags[1];
		while (stackIndex != NO_ITEM)
		{
			if (pushable->hasFloorColission)
				TEN::Floordata::AddBridge(stackIndex);

			stackIndex = g_Level.Items[stackIndex].ItemFlags[1];
		}
	}

	void RemoveBridgeStack(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		const auto* pushable = GetPushableInfo(item);

		if (pushable->hasFloorColission)
			TEN::Floordata::RemoveBridge(itemNumber);

		int stackIndex = g_Level.Items[itemNumber].ItemFlags[1];
		while (stackIndex != NO_ITEM)
		{
			if (pushable->hasFloorColission)
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

	// Floor data collision functions

	std::optional<int> PushableBlockFloor(short itemNumber, int x, int y, int z)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		const auto& pushableInfo = *GetPushableInfo(&pushableItem);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, false);
	
		if (pushableItem.Status != ITEM_INVISIBLE && pushableInfo.hasFloorColission && boxHeight.has_value())
		{
			const auto height = pushableItem.Pose.Position.y + GameBoundingBox(&pushableItem).Y1;
			//const auto height = item->Pose.Position.y - (item->TriggerFlags & 0x1F) * CLICK(1);

			return std::optional{height};
		}

		return std::nullopt;
	}

	std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z)
	{
		auto* item = &g_Level.Items[itemNumber];
		const auto* pushable = GetPushableInfo(item);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, true);

		if (item->Status != ITEM_INVISIBLE && pushable->hasFloorColission && boxHeight.has_value())
			return std::optional{item->Pose.Position.y};

		return std::nullopt;
	}

	int PushableBlockFloorBorder(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		const auto height = item.Pose.Position.y + GameBoundingBox(&item).Y1;
		//const auto height = item->Pose.Position.y - (item->TriggerFlags & 0x1F) * CLICK(1);

		return height;
	}

	int PushableBlockCeilingBorder(short itemNumber)
	{
		const auto* item = &g_Level.Items[itemNumber];

		return item->Pose.Position.y;
	}
}
