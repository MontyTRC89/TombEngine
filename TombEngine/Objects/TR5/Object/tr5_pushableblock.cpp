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
#include "Sound/sound_effects.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Floordata;
using namespace TEN::Input;

namespace TEN::Entities::Generic
{
	constexpr auto PUSHABLE_FALL_VELOCITY_MAX	 = BLOCK(1 / 8.0f);
	constexpr auto PUSHABLE_WATER_VELOCITY_MAX	 = BLOCK(1 / 16.0f);
	constexpr auto PUSHABLE_FALL_RUMBLE_VELOCITY = 96.0f;
	constexpr auto PUSHABLE_HEIGHT_TOLERANCE	 = 32;

	constexpr auto GRAVITY_AIR			  = 8.0f;
	constexpr auto GRAVITY_CHANGE_SPEED	  = 0.5f;
	constexpr auto WATER_SURFACE_DISTANCE = CLICK(0.5f);

	static auto PushableBlockPos = Vector3i::Zero;
	ObjectCollisionBounds PushableBlockBounds = 
	{
		GameBoundingBox(
			0, 0,
			-CLICK(0.25f), 0,
			0, 0),
		std::pair(
			EulerAngles(ANGLE(-10.0f), -LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), LARA_GRAB_THRESHOLD, ANGLE(10.0f)))};
	
	std::unordered_map <MaterialType, PushablesSounds> PushablesSoundsMap		 = {};
	std::vector<PushableAnimationInfo>					 PushableAnimationVector = {};

	static PushableInfo& GetPushableInfo(const ItemInfo& item)
	{
		return (PushableInfo&)item.Data;
	}

	// Main functions

	void InitialisePushableBlock(int itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		item.Data = PushableInfo();
		auto& pushable = GetPushableInfo(item);

		pushable.StartPos = item.Pose.Position;
		pushable.StartPos.RoomNumber = item.RoomNumber;

		if (item.ObjectNumber >= ID_PUSHABLE_OBJECT_CLIMBABLE1 && item.ObjectNumber <= ID_PUSHABLE_OBJECT_CLIMBABLE10)
		{
			pushable.HasFloorColission = true;
			TEN::Floordata::AddBridge(itemNumber);
		}
		else
		{
			pushable.HasFloorColission = false;			
		}

		pushable.Height = GetPushableHeight(item);

		
		// Read OCB flags.
		int ocb = item.TriggerFlags;

		pushable.CanFall = (ocb & 0x01) != 0;						 // Check if bit 0 is set.
		pushable.DoAlignCenter = (ocb & 0x02) != 0;					 // Check if bit 1 is set.
		pushable.Buoyancy = (ocb & 0x04) != 0;						 // Check if bit 2 is set.
		pushable.AnimationSystemIndex = ((ocb & 0x08) != 0) ? 1 : 0; // Check if bit 3 is set.
		
		SetStopperFlag(pushable.StartPos, true);
	}

	void PushableBlockControl(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushableItem.Status != ITEM_ACTIVE)
			return;

		// Check and do gravity routine if necessary.
		if (PushableBlockManageGravity(itemNumber))
			return;

		Lara.InteractedItem = itemNumber;

		int pullAnim = PushableAnimationVector[pushable.AnimationSystemIndex].PullAnimIndex;
		int pushAnim = PushableAnimationVector[pushable.AnimationSystemIndex].PushAnimIndex;

		if (LaraItem->Animation.AnimNumber == pullAnim || LaraItem->Animation.AnimNumber == pushAnim) 
		{
			pushable.GravityState = PushableGravityState::None;
			PushableBlockManageMoving(itemNumber);
		}
		else if (LaraItem->Animation.ActiveState == LS_IDLE)
		{
			// Do last actions and deactivate (reactivated in collision function).
			PushableBlockManageIdle(itemNumber);
		}

		// Do sound effects.
		PushablesManageSounds(itemNumber);
	}

	void PushableBlockCollision(int itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);
		auto& player = *GetLaraInfo(laraItem);

		if ((IsHeld(In::Action) &&
			 !IsHeld(In::Forward) &&
			 laraItem->Animation.ActiveState == LS_IDLE &&
			 laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
			 !laraItem->Animation.IsAirborne &&
			 player.Control.HandStatus == HandStatus::Free &&
			 pushableItem.Status != ITEM_INVISIBLE &&
			 pushableItem.TriggerFlags >= 0) ||
			 (player.Control.IsMoving && player.InteractedItem == itemNumber))
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
					PushableBlockPos.x = pushable.DoAlignCenter ? 0 : LaraItem->Pose.Position.x - pushableItem.Pose.Position.x;
					break;

				case SOUTH:
					PushableBlockPos.z = bounds.Z1 - CLICK(0.4f);
					PushableBlockPos.x = pushable.DoAlignCenter ? 0 : pushableItem.Pose.Position.x - LaraItem->Pose.Position.x;
					break;

				case EAST:
					PushableBlockPos.z = bounds.X1 - CLICK(0.4f);
					PushableBlockPos.x = pushable.DoAlignCenter ? 0 : pushableItem.Pose.Position.z - LaraItem->Pose.Position.z;
					break;

				case WEST:
					PushableBlockPos.z = bounds.X1 - CLICK(0.4f);
					PushableBlockPos.x = pushable.DoAlignCenter ? 0 : LaraItem->Pose.Position.z - pushableItem.Pose.Position.z;
					break;

				default:
					break;
				}

				if (MoveLaraPosition(PushableBlockPos, &pushableItem, laraItem))
				{
					SetAnimation(laraItem, LA_PUSHABLE_GRAB);
					laraItem->Pose.Orientation = pushableItem.Pose.Orientation;
					player.Control.IsMoving = false;
					player.Control.HandStatus = HandStatus::Busy;
					player.NextCornerPos.Position.x = itemNumber;
				}
				else
				{
					player.InteractedItem = itemNumber;
				}
			}
			else
			{
				if (player.Control.IsMoving && player.InteractedItem == itemNumber)
				{
					player.Control.IsMoving = false;
					player.Control.HandStatus = HandStatus::Free;
				}
			}

			pushableItem.Pose.Orientation.y = yOrient;
		}
		else
		{
			// If player is not grabbing pushable, simply do collision routine if needed.
			if (laraItem->Animation.ActiveState != LS_PUSHABLE_GRAB ||
				!TestLastFrame(laraItem, LA_PUSHABLE_GRAB) ||
				player.NextCornerPos.Position.x != itemNumber)
			{
				if (!pushable.HasFloorColission)
					ObjectCollision(itemNumber, laraItem, coll);

				return;
			}

			// Otherwise, player can push/pull.
			bool hasPushAction = IsHeld(In::Forward);
			bool hasPullAction = IsHeld(In::Back);

			if (!hasPushAction && !hasPullAction)
				return;

			int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);

			bool isQuadrantAvailable = false;
			auto pos = GameVector(pushableItem.Pose.Position, pushableItem.RoomNumber);
		
			switch (quadrant)
			{
			case NORTH:
				if (hasPushAction)
				{
					isQuadrantAvailable = pushable.SidesMap[NORTH].Pushable;
					pos.z = pos.z + BLOCK(1);
				}
				else if (hasPullAction)
				{
					isQuadrantAvailable = pushable.SidesMap[NORTH].Pullable;
					pos.z = pos.z - BLOCK(1);
				}

				break;

			case EAST:
				if (hasPushAction)
				{
					isQuadrantAvailable = pushable.SidesMap[EAST].Pushable;
					pos.x = pos.x + BLOCK(1);
				}
				else if (hasPullAction)
				{
					isQuadrantAvailable = pushable.SidesMap[EAST].Pullable;
					pos.x = pos.x - BLOCK(1);
				}

				break;

			case SOUTH:
				if (hasPushAction)
				{
					isQuadrantAvailable = pushable.SidesMap[SOUTH].Pushable;
					pos.z = pos.z - BLOCK(1);
				}
				else if (hasPullAction)
				{
					isQuadrantAvailable = pushable.SidesMap[SOUTH].Pullable;
					pos.z = pos.z + BLOCK(1);
				}

				break;

			case WEST:
				if (hasPushAction)
				{
					isQuadrantAvailable = pushable.SidesMap[WEST].Pushable;
					pos.x = pos.x - BLOCK(1);
				}
				else if (hasPullAction)
				{
					isQuadrantAvailable = pushable.SidesMap[WEST].Pullable;
					pos.x = pos.x + BLOCK(1);
				}

				break;
			}

			if (!isQuadrantAvailable)
				return;
			
			if (!IsNextSectorValid(pushableItem, pos, hasPullAction))
				return;

			if (hasPushAction)
			{
				int pushAnim = PushableAnimationVector[pushable.AnimationSystemIndex].PushAnimIndex;
				SetAnimation(laraItem, pushAnim);
			}
			else if (hasPullAction)
			{
				int pullAnim = PushableAnimationVector[pushable.AnimationSystemIndex].PullAnimIndex;
				SetAnimation(laraItem, pullAnim);
			}

			RemovePushableFromStack(itemNumber);
			ManageStackBridges(itemNumber, false);

			SetStopperFlag(pos, true);

			// If object has started to move, activate it to do its mechanics in control function.
			pushableItem.Status = ITEM_ACTIVE;
			AddActiveItem(itemNumber);
			ResetLaraFlex(laraItem);

			pushable.StartPos = pushableItem.Pose.Position;
			pushable.StartPos.RoomNumber = pushableItem.RoomNumber;
		}
	}

	// Behaviour functions.
	bool PushableBlockManageGravity(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);
		
		auto pointColl = GetCollision(&pushableItem);

		float currentY = pushableItem.Pose.Position.y;
		float velocityY = pushableItem.Animation.Velocity.y;

		int goalHeight = 0;

		int waterDepth = GetWaterSurface(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber);
		if (waterDepth != NO_HEIGHT)
			goalHeight = waterDepth - WATER_SURFACE_DISTANCE + pushable.Height;
		else
			goalHeight = pointColl.Position.Ceiling + WATER_SURFACE_DISTANCE + pushable.Height;

		switch (pushable.GravityState)
		{
		case PushableGravityState::None:
			return false;

		case PushableGravityState::Grounded:

			if (TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
			{
				pushable.GravityState = PushableGravityState::Sinking;
			}
			else
			{
				int heightDifference = abs(currentY - pointColl.Position.Floor);
				if (heightDifference > 0)
				{
					pushable.GravityState = PushableGravityState::Falling;
				}
				else
				{
					if (pointColl.FloorTilt.x != 0 || pointColl.FloorTilt.y != 0)
						pushable.GravityState = PushableGravityState::Sliding;
					else
						return false;
				}
			}

			break;

		case PushableGravityState::Falling:
						
			if (currentY < (pointColl.Position.Floor - velocityY))
			{
				// Is in air.
				float newVelocityY = velocityY + pushable.Gravity;
				pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_FALL_VELOCITY_MAX);

				// Update pushable's position and move its stack.
				pushableItem.Pose.Position.y = currentY + pushableItem.Animation.Velocity.y;
				MoveStackY(itemNumber, pushableItem.Animation.Velocity.y);
				UpdatePushablesRoomNumbers(itemNumber);

				if (TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
				{
					pushable.GravityState = PushableGravityState::Sinking;
					
					// TODO: [Effects Requirement] Add Water splash.
				}
			}
			else
			{
				// Hit ground.
				pushable.GravityState = PushableGravityState::Grounded;
				pushableItem.Pose.Position.y = pointColl.Position.Floor;

				// Shake floor if pushable landed at high enough velocity.
				if (velocityY >= PUSHABLE_FALL_RUMBLE_VELOCITY)
					FloorShake(&pushableItem);
				
				pushableItem.Animation.Velocity.y = 0.0f;

				auto pos = GameVector(pushableItem.Pose.Position, pushableItem.RoomNumber);

				SoundEffect(GetPushableSound(Fall, pos), &pushableItem.Pose, SoundEnvironment::Always);

				int differenceY = pointColl.Position.Floor - currentY;
				MoveStackY(itemNumber, differenceY);

				DeactivationPushablesRoutine(itemNumber);
			}

			break;

		case PushableGravityState::Sinking:

			if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
			{
				pushable.GravityState = PushableGravityState::Falling;
				pushable.Gravity = GRAVITY_AIR;
				return true;
			}

			// TODO: [Effects Requirement] Add bubbles during this phase.

			if (pushable.Buoyancy)
			{
				// Slowly reverses gravity direction. If gravity is 0, then it floats.
				pushable.Gravity = pushable.Gravity - GRAVITY_CHANGE_SPEED;
				if (pushable.Gravity <= 0.0f)
				{
					pushable.GravityState = PushableGravityState::Floating;
					return true;
				}
			}
			else
			{
				// Decreases gravity, continues to fall until hits ground.
				pushable.Gravity = std::max(pushable.Gravity - GRAVITY_CHANGE_SPEED, 4.0f);
			}

			if (currentY < pointColl.Position.Floor - velocityY)
			{
				// Sinking down.
				float newVelocityY = velocityY + pushable.Gravity;
				pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_WATER_VELOCITY_MAX);

				// Update pushable's position and move its stack.
				pushableItem.Pose.Position.y = currentY + pushableItem.Animation.Velocity.y;
				MoveStackY(itemNumber, pushableItem.Animation.Velocity.y);
				UpdatePushablesRoomNumbers(itemNumber);
			}
			else
			{
				// Hit water bed.
				if (pushable.Buoyancy)
				{
					pushable.Gravity = 0.0f;
					pushable.GravityState = PushableGravityState::Floating;
				}
				else
				{
					pushable.GravityState = PushableGravityState::Grounded;
					pushableItem.Pose.Position.y = pointColl.Position.Floor;
				}
								
				pushableItem.Animation.Velocity.y = 0.0f;

				int differenceY = pointColl.Position.Floor - currentY;
				MoveStackY(itemNumber, differenceY);
			}

			break;

		case PushableGravityState::Floating:

			if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
			{
				pushable.GravityState = PushableGravityState::Falling;
				pushable.Gravity = GRAVITY_AIR;
				return true;
			}

			pushable.Gravity = std::max(pushable.Gravity - GRAVITY_CHANGE_SPEED, -4.0f);

			if (currentY > goalHeight)
			{
				// Floating up.
				float newVelocityY = velocityY + pushable.Gravity;
				pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_WATER_VELOCITY_MAX);

				// Update pushable's position and move its stack.
				pushableItem.Pose.Position.y = currentY + pushableItem.Animation.Velocity.y;
				MoveStackY(itemNumber, pushableItem.Animation.Velocity.y);
				UpdatePushablesRoomNumbers(itemNumber);
			}
			else
			{
				// Reached water surface.
				pushable.GravityState = PushableGravityState::OnWater;
				pushableItem.Pose.Position.y = goalHeight;

				pushableItem.Animation.Velocity.y = 0.0f;

				int differenceY = goalHeight - currentY;
				MoveStackY(itemNumber, differenceY);
				UpdatePushablesRoomNumbers(itemNumber);

				DeactivationPushablesRoutine(itemNumber);
			}

			break;

		case PushableGravityState::OnWater:

			if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
			{
				pushable.GravityState = PushableGravityState::Falling;
				pushable.Gravity = GRAVITY_AIR;
				pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);
				return true;
			}

			FloatItem(pushableItem, pushable.FloatingForce);

			// TODO: [Effects Requirement] Spawn ripples.

			break;

		case PushableGravityState::Sliding:
			break;

		default:
			return false;
		}

		return true;

	}
	
	void PushableBlockManageIdle(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		// If not moving, places at center, does some last checks and then deactivates itself.
		pushableItem.Pose.Position = GetNearestSectorCenter(pushableItem.Pose.Position);

		MoveStackXZ(itemNumber);
		
		pushable.GravityState = PushableGravityState::Grounded;

		DeactivationPushablesRoutine(itemNumber);
	}
	
	void PushableBlockManageMoving(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		// Moves pushable based on player bounds.Z2.

		bool isLaraPulling = LaraItem->Animation.AnimNumber == LA_PUSHABLE_PULL || LaraItem->Animation.AnimNumber == LA_PUSHABLE_BLOCK_PULL; //else, she is pushing.

		int quadrantDir = GetQuadrant(LaraItem->Pose.Orientation.y);
		int newPosX = pushable.StartPos.x;
		int newPosZ = pushable.StartPos.z;
		int displaceDepth = 0;
		int displaceBox = GameBoundingBox(LaraItem).Z2;

		if (pushable.CurrentSoundState == PushableSoundState::Moving)
			pushable.CurrentSoundState = PushableSoundState::Stopping;

		displaceDepth = GetLastFrame(GAME_OBJECT_ID::ID_LARA, LaraItem->Animation.AnimNumber)->BoundingBox.Z2;

		displaceBox -= isLaraPulling ? BLOCK(1) + displaceDepth : displaceDepth - BLOCK(1);
		
		if (LaraItem->Animation.FrameNumber != g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd - 1)
		{
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
						pushable.CurrentSoundState = PushableSoundState::Moving;
					}
				}
				else
				{
					if ((quadrantDir == NORTH && pushableItem.Pose.Position.z < newPosZ) ||
						(quadrantDir == SOUTH && pushableItem.Pose.Position.z > newPosZ))
					{
						pushableItem.Pose.Position.z = newPosZ;
						pushable.CurrentSoundState = PushableSoundState::Moving;
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
						pushable.CurrentSoundState = PushableSoundState::Moving;
					}
				}
				else
				{
					if ((quadrantDir == EAST && pushableItem.Pose.Position.x < newPosX) ||
						(quadrantDir == WEST && pushableItem.Pose.Position.x > newPosX))
					{
						pushableItem.Pose.Position.x = newPosX;
						pushable.CurrentSoundState = PushableSoundState::Moving;
					}
				}
			}

			MoveStackXZ(itemNumber);

		}
		else
		{
			// Manage animation end.

			pushableItem.Pose.Position = GetNearestSectorCenter(pushableItem.Pose.Position);

			MoveStackXZ(itemNumber);
			UpdatePushablesRoomNumbers(itemNumber);

			TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

			// TODO: [Bug] If pushable is passing over another pushable, it removes the stopper flag of the old one.
			// If should check if there remains another pushable in the place to decide if quit the StopperFlag or not.
			SetStopperFlag(pushable.StartPos, false);

			// Check if pushing pushable over edge. Then can't keep pushing/pulling.
			if (pushable.CanFall && !isLaraPulling)
			{
				int floorHeight = GetCollision(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y + 10, pushableItem.Pose.Position.z, pushableItem.RoomNumber).Position.Floor;// repeated?
				if (floorHeight > pushableItem.Pose.Position.y)
				{
					LaraItem->Animation.TargetState = LS_IDLE;
					pushable.GravityState = PushableGravityState::Falling;
					pushable.CurrentSoundState = PushableSoundState::None;

					return;
				}
			}

			// Check if is using block animation which can't loop (affects stopper flag).
			if (!PushableAnimationVector[pushable.AnimationSystemIndex].AllowLoop)
				return;

			// Otherwise, just check if action key is still pressed.
			auto nextPos = GameVector(pushableItem.Pose.Position, pushableItem.RoomNumber);

			// Rotates 180 degrees.
			if (isLaraPulling)
				quadrantDir = (quadrantDir + 2) % 4;

			switch (quadrantDir)
			{
			case NORTH:
				nextPos.z = nextPos.z + BLOCK(1);
				break;

			case EAST:
				nextPos.x = nextPos.x + BLOCK(1);
				break;

			case SOUTH:
				nextPos.z = nextPos.z - BLOCK(1);
				break;

			case WEST:
				nextPos.x = nextPos.x - BLOCK(1);
				break;
			}

			if (IsHeld(In::Action) &&
				IsNextSectorValid(pushableItem, nextPos, isLaraPulling))
			{
				pushable.StartPos = pushableItem.Pose.Position;
				pushable.StartPos.RoomNumber = pushableItem.RoomNumber;
				SetStopperFlag(nextPos, true);
			}
			else 
			{
				LaraItem->Animation.TargetState = LS_IDLE;
			}
		}

		return;
	}

	// Sound functions

	void InitializePushablesSoundsMap()
	{
		PushablesSoundsMap =
		{
			{ MaterialType::Mud,		PushablesSounds(SFX_TEN_PUSHABLES_MOVE_MUD, SFX_TEN_PUSHABLES_STOP_MUD, SFX_TEN_PUSHABLES_COLLIDE_MUD) },
			{ MaterialType::Snow,		PushablesSounds(SFX_TEN_PUSHABLES_MOVE_SNOW, SFX_TEN_PUSHABLES_STOP_SNOW, SFX_TEN_PUSHABLES_COLLIDE_SNOW) },
			{ MaterialType::Sand,		PushablesSounds(SFX_TEN_PUSHABLES_MOVE_SAND, SFX_TEN_PUSHABLES_STOP_SAND, SFX_TEN_PUSHABLES_COLLIDE_SAND) },
			{ MaterialType::Gravel,		PushablesSounds(SFX_TEN_PUSHABLES_MOVE_GRAVEL, SFX_TEN_PUSHABLES_STOP_GRAVEL, SFX_TEN_PUSHABLES_COLLIDE_GRAVEL) },
			{ MaterialType::Ice,		PushablesSounds(SFX_TEN_PUSHABLES_MOVE_ICE, SFX_TEN_PUSHABLES_STOP_ICE, SFX_TEN_PUSHABLES_COLLIDE_ICE) },
			{ MaterialType::Water,		PushablesSounds(SFX_TEN_PUSHABLES_MOVE_WATER, SFX_TEN_PUSHABLES_STOP_WATER, SFX_TEN_PUSHABLES_COLLIDE_WATER) },
			{ MaterialType::Stone,		PushablesSounds(SFX_TEN_PUSHABLES_STOP_MOVE_STONE, SFX_TEN_PUSHABLES_STOP_STONE, SFX_TEN_PUSHABLES_COLLIDE_STONE) },
			{ MaterialType::Wood,		PushablesSounds(SFX_TEN_PUSHABLES_MOVE_WOOD, SFX_TEN_PUSHABLES_STOP_WOOD, SFX_TEN_PUSHABLES_COLLIDE_WOOD) },
			{ MaterialType::Metal,		PushablesSounds(SFX_TEN_PUSHABLES_MOVE_METAL, SFX_TEN_PUSHABLES_STOP_METAL, SFX_TEN_PUSHABLES_COLLIDE_METAL) },
			{ MaterialType::Marble,		PushablesSounds(SFX_TEN_PUSHABLES_MOVE_MARBLE, SFX_TEN_PUSHABLES_STOP_MARBLE, SFX_TEN_PUSHABLES_COLLIDE_MARBLE) },
			{ MaterialType::Grass,		PushablesSounds(SFX_TEN_PUSHABLES_MOVE_GRASS, SFX_TEN_PUSHABLES_STOP_GRASS, SFX_TEN_PUSHABLES_COLLIDE_GRASS) },
			{ MaterialType::Concrete,	PushablesSounds(SFX_TEN_PUSHABLES_MOVE_CONCRETE, SFX_TEN_PUSHABLES_STOP_CONCRETE, SFX_TEN_PUSHABLES_COLLIDE_CONCRETE) },
			{ MaterialType::OldWood,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL) },
			{ MaterialType::OldMetal,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL) },
			{ MaterialType::Custom1,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL) },
			{ MaterialType::Custom2,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL) },
			{ MaterialType::Custom3,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL) },
			{ MaterialType::Custom4,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL) },
			{ MaterialType::Custom5,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL) },
			{ MaterialType::Custom6,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL) },
			{ MaterialType::Custom7,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL) },
			{ MaterialType::Custom8,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL) }
		};
	}

	int GetPushableSound(PushableSoundType soundType, const GameVector& pos)
	{
		auto pointColl = GetCollision(pos);
		auto material = pointColl.BottomBlock->Material;

		int resultSound = 0;
		switch (soundType)
		{ 
		case PushableSoundType::Loop:
			
			resultSound = PushablesSoundsMap[material].LoopSound;
			break;

		case PushableSoundType::Stop:
			resultSound = PushablesSoundsMap[material].StopSound;
			break;

		case PushableSoundType::Fall:
			resultSound = PushablesSoundsMap[material].FallSound;
			break;

		default:
			TENLog("Error, requesting an inexistent pushable sound type", LogLevel::Error, LogConfig::All, true);
			break;
		}

		return resultSound;
	}

	void PushablesManageSounds(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		auto pos = GameVector(pushableItem.Pose.Position, pushableItem.RoomNumber);

		if (pushable.CurrentSoundState == PushableSoundState::Moving)
		{
			SoundEffect(GetPushableSound(Loop, pos), &pushableItem.Pose, SoundEnvironment::Always);
		}
		else if (pushable.CurrentSoundState == PushableSoundState::Stopping)
		{
			pushable.CurrentSoundState = PushableSoundState::None;
			SoundEffect(GetPushableSound(Stop, pos), &pushableItem.Pose, SoundEnvironment::Always);
		}
	}

	// General functions

	void InitialisePushablesGeneral()
	{
		// To execute on level start and on level loading.

		PushableAnimationVector =
		{
			PushableAnimationInfo(LA_PUSHABLE_PULL, LA_PUSHABLE_PUSH, true),			 //TR4-TR5 animations
			PushableAnimationInfo(LA_PUSHABLE_BLOCK_PULL, LA_PUSHABLE_BLOCK_PUSH, false) //TR1-TR3 animations
		};

		InitializePushablesSoundsMap();
		UpdateAllPushablesStackLinks();
	}

	void DeactivationPushablesRoutine(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		// Re-apply the bridges colliders
		ManageStackBridges(itemNumber, true);

		// Check if it has fall over another pushable.
		UpdateAllPushablesStackLinks();

		// If it has fallen on top of existing pushable
		// Or it's floating in water, don't test triggers.
		if (pushable.StackLowerItem == NO_ITEM || pushable.GravityState == PushableGravityState::Floating)
			TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

		if (pushableItem.Status == ITEM_ACTIVE && pushable.GravityState <= PushableGravityState::Falling)
		{
			RemoveActiveItem(itemNumber);
			pushableItem.Status = ITEM_NOT_ACTIVE;
		}
	}

	std::vector<int> FindAllPushables(const std::vector<ItemInfo>& objectsList)
	{
		std::vector<int> pushables;

		for (int i = 0; i < objectsList.size(); i++)
		{
			auto& item = objectsList[i];

			if ((item.ObjectNumber >= ID_PUSHABLE_OBJECT1 && item.ObjectNumber <= ID_PUSHABLE_OBJECT10) ||
				(item.ObjectNumber >= ID_PUSHABLE_OBJECT_CLIMBABLE1 && item.ObjectNumber <= ID_PUSHABLE_OBJECT_CLIMBABLE10))
			{
				pushables.push_back(i);
			}
		}

		return pushables;
	}

	void UpdatePushablesRoomNumbers(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		// Check and update the room number of the pushables and others linked in the stack.
		auto pointColl = GetCollision(&pushableItem);
		if (pointColl.RoomNumber != pushableItem.RoomNumber)
		{
			ItemNewRoom(itemNumber, pointColl.RoomNumber);
			pushable.StartPos.RoomNumber = pointColl.RoomNumber;
		}

		if (pushable.StackUpperItem == NO_ITEM)
			return;

		auto pushableLinkedItem = g_Level.Items[itemNumber];
		auto& pushableLinked = GetPushableInfo(pushableLinkedItem);

		while (pushableLinked.StackUpperItem != NO_ITEM)
		{
			auto col = GetCollision(&pushableLinkedItem);
			if (col.RoomNumber != pushableLinkedItem.RoomNumber)
			{
				ItemNewRoom(itemNumber, col.RoomNumber);
				pushableLinked.StartPos.RoomNumber = col.RoomNumber;
			}

			pushableLinkedItem = g_Level.Items[pushableLinked.StackUpperItem];
			pushableLinked = GetPushableInfo(pushableLinkedItem);
		}
	}

	int GetPushableHeight(ItemInfo& item)
	{
		int heightBoundingBox = -GameBoundingBox(&item).Y1;
		int heightWorldAligned = (heightBoundingBox / CLICK(0.5)) * CLICK(0.5);
		return heightWorldAligned;
	}

	void ForcePushableActivation(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.HasFloorColission)
		{
			RemovePushableFromStack(itemNumber);
			ManageStackBridges(itemNumber, false);
		}

		pushableItem.Status = ITEM_ACTIVE;
		AddActiveItem(itemNumber);
	}
	
	// Test functions

	bool IsPushableOnValidSurface(ItemInfo& pushableItem)
	{
		auto& pushable = GetPushableInfo(pushableItem);

		auto pointColl = CollisionResult{};
		if (pushable.HasFloorColission)
		{
			RemoveBridge(pushableItem.Index);
			pointColl = GetCollision(&pushableItem);
			AddBridge(pushableItem.Index);
		}
		else
		{
			pointColl = GetCollision(&pushableItem);
		}

		// Check for wall.
		if (pointColl.Block->IsWall(pushableItem.Pose.Position.x, pushableItem.Pose.Position.z))
			return false;

		// Check if pushable is isn't on floor.
		int floorDifference = abs(pointColl.Position.Floor - pushableItem.Pose.Position.y);
		if ((floorDifference >= PUSHABLE_HEIGHT_TOLERANCE))
			return false;

		return true;
	}

	bool IsNextSectorValid(ItemInfo& pushableItem, const GameVector& target, bool checkIfLaraFits)
	{
		if (!IsPushableOnValidSurface(pushableItem))
			return false;

		if (!CheckStackLimit(pushableItem))
			return false;

		auto& pushable = GetPushableInfo(pushableItem);
		auto pointColl = GetCollision(target);

		// Check for wall.
		if (pointColl.Block->IsWall(target.x, target.z))
			return false;

		// Check for floor slope.
		if (pointColl.Position.FloorSlope)
			return false;

		// Check for diagonal floor.
		if (pointColl.Position.DiagonalStep)
			return false;

		if ((pointColl.Block->GetSurfaceSlope(0, true) != Vector2::Zero) || (pointColl.Block->GetSurfaceSlope(1, true) != Vector2::Zero))
			return false;

		// Check for stopper flag.
		if (pointColl.Block->Stopper)
		{
			if (pointColl.Position.Floor <= pushableItem.Pose.Position.y)
			{
				return false;
			}
			//else
			//{
				// Maybe the stopper is because there is another pushable down there.
				// TODO: Need better search tools

			//}
		}
			

		// Check for gap or a step. (Can it fall down?) (Only available for pushing).
		int floorDifference = abs(pointColl.Position.Floor - pushableItem.Pose.Position.y);
		if (pushable.CanFall)
		{
			if ((pointColl.Position.Floor < pushableItem.Pose.Position.y) && (floorDifference >= PUSHABLE_HEIGHT_TOLERANCE))
				return false;
		}
		else
		{
			if (floorDifference >= PUSHABLE_HEIGHT_TOLERANCE)
				return false;
		}

		// Is ceiling (square or diagonal) high enough?
		int distanceToCeiling = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);
		int blockHeight = GetStackHeight(pushableItem);
		if (distanceToCeiling < blockHeight)
			return false;

		// Is there any enemy or object?
		auto prevPos = pushableItem.Pose.Position;
		pushableItem.Pose.Position = target.ToVector3i();
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

			auto& object = Objects[CollidedItems[i]->ObjectNumber];
			int collidedIndex = CollidedItems[i] - g_Level.Items.data(); // Index of CollidedItems[i].

			auto colPos = CollidedItems[i]->Pose.Position;

			// Check if floor function returns nullopt.
			if (object.floor(collidedIndex, colPos.x, colPos.y, colPos.z) == std::nullopt)
				return false;
		}

		if (checkIfLaraFits)
		{
			if (!IsValidForLara(pushableItem, pushable, target))
				return false;
		}

		return true;
	}

	bool IsValidForLara(const ItemInfo& pushableItem, const PushableInfo& pushable, const GameVector& target)
	{
		auto playerOffset = Vector3i::Zero;
		auto dirVector = target.ToVector3i() - pushableItem.Pose.Position;
		
		if (dirVector.z > 0)
		{
			playerOffset.z = GetBestFrame(*LaraItem).Offset.z + BLOCK(1);
		}
		else if (dirVector.x > 0)
		{
			playerOffset.x = GetBestFrame(*LaraItem).Offset.z + BLOCK(1);
		}
		else if (dirVector.z < 0)
		{
			playerOffset.z = -GetBestFrame(*LaraItem).Offset.z - BLOCK(1);
		}
		else
		{
			playerOffset.x = -GetBestFrame(*LaraItem).Offset.z - BLOCK(1);
		}
		
		GameVector laraDetectionPoint = LaraItem->Pose.Position + playerOffset;
		laraDetectionPoint.RoomNumber = LaraItem->RoomNumber;

		CollisionResult pointColl; 
		if (pushable.HasFloorColission)
		{
			RemoveBridge(pushableItem.Index);
			pointColl = GetCollision(laraDetectionPoint);
			AddBridge(pushableItem.Index);
		}
		else
		{
			pointColl = GetCollision(laraDetectionPoint);
		}

		//Is a stopper flag tile? (Lara may not need this, otherwise, it's needed to remove the stopper flag in the pushable to check this condition).
		//if (col.Block->Stopper)
			//return false;

		// If floor is not flat
		if (pointColl.Position.Floor != LaraItem->Pose.Position.y)
			return false;

		// Is ceiling (square or diagonal) high enough?
		int distanceToCeiling = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);
		if (distanceToCeiling < LARA_HEIGHT)
			return false;

		// Is there any enemy or object?
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
				auto& object = Objects[CollidedItems[i]->ObjectNumber];
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
	void MoveStack(const int itemNumber, const Vector3i& target)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.StackUpperItem == NO_ITEM)
			return;

		auto& pushableLinkedItem = g_Level.Items[pushable.StackUpperItem];
		auto& pushableLinked = GetPushableInfo(pushableLinkedItem);

		while (true)
		{
			pushableLinkedItem.Pose.Position.x = target.x;
			pushableLinkedItem.Pose.Position.z = target.z;
			pushableLinkedItem.Pose.Position.y += target.y; //The vertical movement receives a velocity, not a fixed value.

			pushableLinked.StartPos = pushableLinkedItem.Pose.Position;
			pushableLinked.StartPos.RoomNumber = pushableLinkedItem.RoomNumber;

			if (pushableLinked.StackUpperItem == NO_ITEM)
				break;

			pushableLinkedItem = g_Level.Items[pushableLinked.StackUpperItem];
			pushableLinked = GetPushableInfo(pushableLinkedItem);

		}
	}

	void MoveStackXZ (int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		MoveStack(itemNumber, Vector3i(pushableItem.Pose.Position.x, 0, pushableItem.Pose.Position.z));
	}

	void MoveStackY (int itemNumber, int y)
	{
		MoveStack(itemNumber, Vector3i(0, y, 0));
	}

	void ManageStackBridges(int itemNumber, bool addBridge)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.HasFloorColission)
		{
			if (addBridge)
				TEN::Floordata::AddBridge(itemNumber);
			else
				TEN::Floordata::RemoveBridge(itemNumber);
		}

		if (pushable.StackUpperItem == NO_ITEM)
			return;
		
		auto pushableLinkedItem = g_Level.Items[pushable.StackUpperItem];
		auto pushableLinked = GetPushableInfo(pushableLinkedItem);

		while (pushableLinked.StackUpperItem != NO_ITEM)
		{
			if (pushableLinked.HasFloorColission)
			{
				if (addBridge)
					TEN::Floordata::AddBridge(pushableLinkedItem.Index);
				else
					TEN::Floordata::RemoveBridge(pushableLinkedItem.Index);
			}

			pushableLinkedItem = g_Level.Items[pushableLinked.StackUpperItem];
			pushableLinked = GetPushableInfo(pushableLinkedItem);
		}
	}

	void RemovePushableFromStack(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.StackLowerItem != NO_ITEM)
		{
			auto& lowerPushableItem = g_Level.Items[pushable.StackLowerItem];
			auto& lowerPushable = GetPushableInfo(lowerPushableItem);
			
			lowerPushable.StackUpperItem = NO_ITEM;
			pushable.StackLowerItem = NO_ITEM;
		}
	}	

	void UpdateAllPushablesStackLinks()
	{
		auto& pushablesNumbersList = FindAllPushables(g_Level.Items);
	
		if (pushablesNumbersList.empty())
			return;

		std::sort(pushablesNumbersList.begin(), pushablesNumbersList.end(), CompareItem2DPositions);

		for (int i = 0; i < pushablesNumbersList.size() - 1; ++i) 
		{
			auto& objectA = g_Level.Items[pushablesNumbersList[i]];
			auto& objectB = g_Level.Items[pushablesNumbersList[i + 1]];

			// Are they in the same sector?
			if ((objectA.Pose.Position.x == objectB.Pose.Position.x) && (objectA.Pose.Position.z == objectB.Pose.Position.z))
			{
				// Determine which object is up and which is down
				auto& upperPushableItem = (objectA.Pose.Position.y < objectB.Pose.Position.y) ? objectA : objectB;
				auto& lowerPushableItem = (objectA.Pose.Position.y < objectB.Pose.Position.y) ? objectB : objectA;
				
				// Set the stackUpperItem and stackLowerItem variables accordingly
				auto& upperPushable = GetPushableInfo(upperPushableItem);
				auto& lowerPushable = GetPushableInfo(lowerPushableItem);
				upperPushable.StackLowerItem = lowerPushableItem.Index;
				lowerPushable.StackUpperItem = upperPushableItem.Index;
			}
		}
	}

	int GetStackHeight(ItemInfo& item)
	{
		auto pushableItem = item;
		auto pushable = GetPushableInfo(pushableItem);
		
		int height = pushable.Height;
		
		while (pushable.StackUpperItem != NO_ITEM)
		{
			pushableItem = g_Level.Items[pushable.StackUpperItem];
			pushable = GetPushableInfo(pushableItem);

			height += pushable.Height;
		}

		return height;
	}

	bool CheckStackLimit(ItemInfo& item)
	{
		auto pushableItem = item;
		auto pushable = GetPushableInfo(pushableItem);

		int limit = pushable.StackLimit;
		int count = 1;
		
		while (pushable.StackUpperItem != NO_ITEM)
		{
			pushableItem = g_Level.Items[pushable.StackUpperItem];
			pushable = GetPushableInfo(pushableItem);

			count++;

			if (count > limit)
				return false;
		}

		return true;
	}

	// Floor data collision functions

	std::optional<int> PushableBlockFloor(int itemNumber, int x, int y, int z)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, false);
	
		if (pushableItem.Status != ITEM_INVISIBLE && pushable.HasFloorColission && boxHeight.has_value())
		{
			int height = pushableItem.Pose.Position.y - GetPushableHeight(pushableItem);
			return std::optional{ height };
		}

		return std::nullopt;
	}

	std::optional<int> PushableBlockCeiling(int itemNumber, int x, int y, int z)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, true);

		if (pushableItem.Status != ITEM_INVISIBLE && pushable.HasFloorColission && boxHeight.has_value())
			return std::optional{ pushableItem.Pose.Position.y};

		return std::nullopt;
	}

	int PushableBlockFloorBorder(int itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		auto height = item.Pose.Position.y - GetPushableHeight(item);
		return height;
	}

	int PushableBlockCeilingBorder(int itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		return item->Pose.Position.y;
	}
}
