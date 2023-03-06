#include "framework.h"
#include "Objects/TR5/Object/tr5_pushableblock.h"

//#include "Game/animation.h"
//#include "Game/items.h"
#include "Game/collision/collide_item.h"
//#include "Game/collision/collide_room.h"
//#include "Game/collision/floordata.h"
//#include "Game/Lara/lara.h"
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
	constexpr auto PUSHABLE_WATER_VELOCITY_MAX = BLOCK(1 / 16.0f);
	constexpr auto PUSHABLE_FALL_RUMBLE_VELOCITY = 96.0f;
	constexpr auto PUSHABLE_HEIGHT_TOLERANCE = 32;

	constexpr float GRAVITY_AIR = 8.0f;
	constexpr float GRAVITY_CHANGE_SPEED = 0.5f;
	constexpr float WATER_SURFACE_DISTANCE = CLICK(0.5f);

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
	
	std::unordered_map <FLOOR_MATERIAL, PushablesSounds> PushablesSoundsMap;
	std::vector<PushableAnimationInfo> PushableAnimationVector;

	PushableInfo& GetPushableInfo(const ItemInfo& item)
	{
		return (PushableInfo&)item.Data;
	}

	// Main functions

	void InitialisePushableBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		item.Data = PushableInfo();
		auto& pushableInfo = GetPushableInfo(item);

		pushableInfo.StartPos = item.Pose.Position;
		pushableInfo.StartPos.RoomNumber = item.RoomNumber;

		if (item.ObjectNumber >= ID_PUSHABLE_OBJECT_CLIMBABLE1 && item.ObjectNumber <= ID_PUSHABLE_OBJECT_CLIMBABLE10)
		{
			pushableInfo.HasFloorColission = true;
			TEN::Floordata::AddBridge(itemNumber);
		}
		else
		{
			pushableInfo.HasFloorColission = false;			
		}

		pushableInfo.Height = GetPushableHeight(item);

		
		// Read OCB flags.
		int ocb = item.TriggerFlags;

		pushableInfo.CanFall = (ocb & 0x01) != 0; // Check if bit 0 is set	(+1)
		pushableInfo.DoAlignCenter = (ocb & 0x02) != 0; // Check if bit 1 is set	(+2)
		pushableInfo.Buoyancy = (ocb & 0x04) != 0; // Check if bit 2 is set	(+4)
		pushableInfo.AnimationSystemIndex = ((ocb & 0x08) != 0) ? 1 : 0; // Check if bit 3 is set	(+8)
		
		SetStopperFlag(pushableInfo.StartPos, true);
	}

	void PushableBlockControl(short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		if (pushableItem.Status != ITEM_ACTIVE)
			return;

		//Check and do gravity routine if it must.
		if (PushableBlockManageGravity(itemNumber))
			return;

		Lara.InteractedItem = itemNumber;

		int pullAnim = PushableAnimationVector[pushableInfo.AnimationSystemIndex].PullAnimIndex;
		int pushAnim = PushableAnimationVector[pushableInfo.AnimationSystemIndex].PushAnimIndex;

		if (LaraItem->Animation.AnimNumber == pullAnim || LaraItem->Animation.AnimNumber == pushAnim) 
		{
			pushableInfo.GravityState = PushableGravityState::None;
			PushableBlockManageMoving(itemNumber);
		}
		else if (LaraItem->Animation.ActiveState == LS_IDLE)
		{
			//Do last actions and deactivate. (It's reactivated in collision function).
			PushableBlockManageIdle(itemNumber);
		}

		// Do sound effects.
		PushablesManageSounds(itemNumber);
	}

	void PushableBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);
		auto& laraInfo = *GetLaraInfo(laraItem);

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
					PushableBlockPos.x = pushableInfo.DoAlignCenter ? 0 : LaraItem->Pose.Position.x - pushableItem.Pose.Position.x;
					break;

				case SOUTH:
					PushableBlockPos.z = bounds.Z1 - CLICK(0.4f);
					PushableBlockPos.x = pushableInfo.DoAlignCenter ? 0 : pushableItem.Pose.Position.x - LaraItem->Pose.Position.x;
					break;

				case EAST:
					PushableBlockPos.z = bounds.X1 - CLICK(0.4f);
					PushableBlockPos.x = pushableInfo.DoAlignCenter ? 0 : pushableItem.Pose.Position.z - LaraItem->Pose.Position.z;
					break;

				case WEST:
					PushableBlockPos.z = bounds.X1 - CLICK(0.4f);
					PushableBlockPos.x = pushableInfo.DoAlignCenter ? 0 : LaraItem->Pose.Position.z - pushableItem.Pose.Position.z;
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
			//If player is not grabbing the pushable, then just do collision rutine if needed.
			if (laraItem->Animation.ActiveState != LS_PUSHABLE_GRAB ||
				!TestLastFrame(laraItem, LA_PUSHABLE_GRAB) ||
				laraInfo.NextCornerPos.Position.x != itemNumber)
			{
				if (!pushableInfo.HasFloorColission)
					ObjectCollision(itemNumber, laraItem, coll);

				return;
			}

			//Otherwise, player can input push/pull actions
			bool isPushAction = IsHeld(In::Forward);
			bool isPullAction = IsHeld(In::Back);

			if (!isPushAction && !isPullAction)
			{
				return;
			}

			int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);

			bool isQuadrantAvailable = false;
			GameVector DetectionPoint = pushableItem.Pose.Position;
			DetectionPoint.RoomNumber = pushableItem.RoomNumber;
		
			switch (quadrant)
			{
			case NORTH:
				if (isPushAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[NORTH].Pushable;
					DetectionPoint.z = DetectionPoint.z + BLOCK(1);
				}
				else if (isPullAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[NORTH].Pullable;
					DetectionPoint.z = DetectionPoint.z - BLOCK(1);
				}
				break;
			case EAST:
				if (isPushAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[EAST].Pushable;
					DetectionPoint.x = DetectionPoint.x + BLOCK(1);
				}
				else if (isPullAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[EAST].Pullable;
					DetectionPoint.x = DetectionPoint.x - BLOCK(1);
				}
				break;
			case SOUTH:
				if (isPushAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[SOUTH].Pushable;
					DetectionPoint.z = DetectionPoint.z - BLOCK(1);
				}
				else if (isPullAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[SOUTH].Pullable;
					DetectionPoint.z = DetectionPoint.z + BLOCK(1);
				}
				break;
			case WEST:
				if (isPushAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[WEST].Pushable;
					DetectionPoint.x = DetectionPoint.x - BLOCK(1);
				}
				else if (isPullAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[WEST].Pullable;
					DetectionPoint.x = DetectionPoint.x + BLOCK(1);
				}
				break;
			}

			if (!isQuadrantAvailable)
				return;
			
			if (!IsNextSectorValid(pushableItem, DetectionPoint, isPullAction))
				return;

			if (isPushAction)
			{
				int pushAnim = PushableAnimationVector[pushableInfo.AnimationSystemIndex].PushAnimIndex;
				SetAnimation(laraItem, pushAnim);
			}
			else if (isPullAction)
			{
				int pullAnim = PushableAnimationVector[pushableInfo.AnimationSystemIndex].PullAnimIndex;
				SetAnimation(laraItem, pullAnim);
			}

			RemovePushableFromStack(itemNumber);
			ManageStackBridges(itemNumber, false);

			SetStopperFlag(DetectionPoint, true);

			//If the object has started to move, we activate it to do its mechanics in the Control function 
			pushableItem.Status = ITEM_ACTIVE;
			AddActiveItem(itemNumber);
			ResetLaraFlex(laraItem);

			pushableInfo.StartPos = pushableItem.Pose.Position;
			pushableInfo.StartPos.RoomNumber = pushableItem.RoomNumber;
		}
	}

	// Behaviour functions
	bool PushableBlockManageGravity(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);
				
		auto col = GetCollision(&pushableItem);

		float currentY = pushableItem.Pose.Position.y;
		float velocityY = pushableItem.Animation.Velocity.y;

		int goalHeight = 0;

		int waterDepth = GetWaterSurface(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber);
		if (waterDepth != NO_HEIGHT)
		{
			goalHeight = waterDepth - WATER_SURFACE_DISTANCE + pushableInfo.Height;
		}
		else
		{
			goalHeight = col.Position.Ceiling + WATER_SURFACE_DISTANCE + pushableInfo.Height;
		}


		switch (pushableInfo.GravityState)
		{
		case PushableGravityState::None:
			return false;
			break;

		case PushableGravityState::Ground:

			if (TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
			{
				pushableInfo.GravityState = PushableGravityState::Sinking;
			}
			else
			{
				int heightDifference = abs(currentY - col.Position.Floor);
				if (heightDifference > 0)
				{
					pushableInfo.GravityState = PushableGravityState::Falling;
				}
				else
				{
					if ((col.FloorTilt.x != 0) || (col.FloorTilt.y != 0))
					{
						pushableInfo.GravityState = PushableGravityState::Sliding;
					}
					else
					{
						return false;
					}
				}
			}
			break;

		case PushableGravityState::Falling:
						
			if (currentY < col.Position.Floor - velocityY)
			{
				// Is on air.
				float newVelocityY = velocityY + pushableInfo.Gravity;
				pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_FALL_VELOCITY_MAX);

				// Update the pushable block's position and move the block's stack.
				pushableItem.Pose.Position.y = currentY + pushableItem.Animation.Velocity.y;
				MoveStackY(itemNumber, pushableItem.Animation.Velocity.y);
				UpdateRoomNumbers(itemNumber);

				if (TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
				{
					pushableInfo.GravityState = PushableGravityState::Sinking;
					
					//TODO [Effects Requirement] Add Water splash.
				}
			}
			else
			{
				// It has hit the ground.
				pushableInfo.GravityState = PushableGravityState::Ground;
				pushableItem.Pose.Position.y = col.Position.Floor;

				// Shake the floor if the pushable block fell at a high enough velocity.
				if (velocityY >= PUSHABLE_FALL_RUMBLE_VELOCITY)
				{
					FloorShake(&pushableItem);
				}
				
				pushableItem.Animation.Velocity.y = 0.0f;

				GameVector detectionPoint = pushableItem.Pose.Position;
				detectionPoint.RoomNumber = pushableItem.RoomNumber;

				SoundEffect(GetPushableSound(Fall, detectionPoint), &pushableItem.Pose, SoundEnvironment::Always);

				int differenceY = col.Position.Floor - currentY;
				MoveStackY(itemNumber, differenceY);

				DeactivationRoutine(itemNumber);
			}
			break;

		case PushableGravityState::Sinking:

			if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
			{
				pushableInfo.GravityState = PushableGravityState::Falling;
				pushableInfo.Gravity = GRAVITY_AIR;
				return true;
			}

			//TODO [Effects Requirement] Add bubbles during this phase

			if (pushableInfo.Buoyancy)
			{
				//It slowly reverses the gravity direction. If gravity is 0, then it pass to floating.
				pushableInfo.Gravity = pushableInfo.Gravity - GRAVITY_CHANGE_SPEED;
				if (pushableInfo.Gravity <= 0)
				{
					pushableInfo.GravityState = PushableGravityState::Floating;
					return true;
				}
			}
			else
			{
				//It decreases its gravity but keep falling till the ground.
				pushableInfo.Gravity = std::max(pushableInfo.Gravity - GRAVITY_CHANGE_SPEED, 4.0f);
			}

			if (currentY < col.Position.Floor - velocityY)
			{
				// Is on sunking down.
				float newVelocityY = velocityY + pushableInfo.Gravity;
				pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_WATER_VELOCITY_MAX);

				// Update the pushable block's position and move the block's stack.
				pushableItem.Pose.Position.y = currentY + pushableItem.Animation.Velocity.y;
				MoveStackY(itemNumber, pushableItem.Animation.Velocity.y);
				UpdateRoomNumbers(itemNumber);
			}
			else
			{
				// It has hit the water ground.
				pushableInfo.GravityState = PushableGravityState::Ground;
				pushableItem.Pose.Position.y = col.Position.Floor;
								
				pushableItem.Animation.Velocity.y = 0.0f;

				int differenceY = col.Position.Floor - currentY;
				MoveStackY(itemNumber, differenceY);

				DeactivationRoutine(itemNumber);
			}

			break;

		case PushableGravityState::Floating:

			if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
			{
				pushableInfo.GravityState = PushableGravityState::Falling;
				pushableInfo.Gravity = GRAVITY_AIR;
				return true;
			}

			pushableInfo.Gravity = std::max(pushableInfo.Gravity - GRAVITY_CHANGE_SPEED, -4.0f);

			if (currentY > goalHeight)
			{
				// Is floating up.
				float newVelocityY = velocityY + pushableInfo.Gravity;
				pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_WATER_VELOCITY_MAX);

				// Update the pushable block's position and move the block's stack.
				pushableItem.Pose.Position.y = currentY + pushableItem.Animation.Velocity.y;
				MoveStackY(itemNumber, pushableItem.Animation.Velocity.y);
				UpdateRoomNumbers(itemNumber);
			}
			else
			{
				// It has reached the water surface.
				pushableInfo.GravityState = PushableGravityState::OnWater;
				pushableItem.Pose.Position.y = goalHeight;

				pushableItem.Animation.Velocity.y = 0.0f;

				int differenceY = goalHeight - currentY;
				MoveStackY(itemNumber, differenceY);

				DeactivationRoutine(itemNumber);
			}
			break;

		case PushableGravityState::OnWater:

			if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
			{
				pushableInfo.GravityState = PushableGravityState::Falling;
				pushableInfo.Gravity = GRAVITY_AIR;
				pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);
				return true;
			}

			FloatingSolidItem(pushableItem, pushableInfo.FloatingForce);

			//TODO [Effects Requirement] Spawn circular water waves

			break;

		case PushableGravityState::Sliding:
			break;

		default:
			return false;
			break;
		}

		return true;

	}
	
	void PushableBlockManageIdle(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		//If it's not moving, it places at center, do some last checks and then it deactivate itself.
		pushableItem.Pose.Position = GetNearestSectorCenter(pushableItem.Pose.Position);

		MoveStackXZ(itemNumber);
		
		pushableInfo.GravityState = PushableGravityState::Ground;

		DeactivationRoutine(itemNumber);
	}
	
	void PushableBlockManageMoving(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		// Moves pushable based on player bounds.Z2.

		bool isLaraPulling = LaraItem->Animation.AnimNumber == LA_PUSHABLE_PULL || LaraItem->Animation.AnimNumber == LA_PUSHABLE_BLOCK_PULL; //else, she is pushing.

		int quadrantDir = GetQuadrant(LaraItem->Pose.Orientation.y);
		int newPosX = pushableInfo.StartPos.x;
		int newPosZ = pushableInfo.StartPos.z;
		int displaceDepth = 0;
		int displaceBox = GameBoundingBox(LaraItem).Z2;

		if (pushableInfo.CurrentSoundState == PushableSoundState::Moving)
			pushableInfo.CurrentSoundState = PushableSoundState::Stopping;

		displaceDepth = GetLastFrame(GAME_OBJECT_ID::ID_LARA, LaraItem->Animation.AnimNumber)->boundingBox.Z2;

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
						pushableInfo.CurrentSoundState = PushableSoundState::Moving;
					}
				}
				else
				{
					if ((quadrantDir == NORTH && pushableItem.Pose.Position.z < newPosZ) ||
						(quadrantDir == SOUTH && pushableItem.Pose.Position.z > newPosZ))
					{
						pushableItem.Pose.Position.z = newPosZ;
						pushableInfo.CurrentSoundState = PushableSoundState::Moving;
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
						pushableInfo.CurrentSoundState = PushableSoundState::Moving;
					}
				}
				else
				{
					if ((quadrantDir == EAST && pushableItem.Pose.Position.x < newPosX) ||
						(quadrantDir == WEST && pushableItem.Pose.Position.x > newPosX))
					{
						pushableItem.Pose.Position.x = newPosX;
						pushableInfo.CurrentSoundState = PushableSoundState::Moving;
					}
				}
			}

			MoveStackXZ(itemNumber);

		}
		else
		{
			//Manage the end of the animation

			pushableItem.Pose.Position = GetNearestSectorCenter(pushableItem.Pose.Position);

			MoveStackXZ(itemNumber);
			UpdateRoomNumbers(itemNumber);

			TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

			//TODO: [Bug] If the pushable is pasing over another pushable, this is removing the stopper flag of the old one.
			//If should check if there remains another pushable in the place to decide if quit the StopperFlag or not.
			SetStopperFlag(pushableInfo.StartPos, false);

			// Check if pushing pushable through an edge into falling.Then she can't keep pushing/pulling
			if (pushableInfo.CanFall && !isLaraPulling)
			{
				int floorHeight = GetCollision(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y + 10, pushableItem.Pose.Position.z, pushableItem.RoomNumber).Position.Floor;// repeated?
				if (floorHeight > pushableItem.Pose.Position.y)
				{
					LaraItem->Animation.TargetState = LS_IDLE;
					pushableInfo.GravityState = PushableGravityState::Falling;
					pushableInfo.CurrentSoundState = PushableSoundState::None;

					return;
				}
			}

			// Check if is using block animation system as it can't go on looping (affects the stopper flag).
			if (!PushableAnimationVector[pushableInfo.AnimationSystemIndex].AllowLoop)
				return;

			//Otherwise, just check if action key is still pressed.
			GameVector NextPos = pushableItem.Pose.Position;
			NextPos.RoomNumber = pushableItem.RoomNumber;

			if (isLaraPulling)
				quadrantDir = (quadrantDir + 2) % 4; //Rotates the orientation 180º.

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

			if (IsHeld(In::Action) &&
				IsNextSectorValid(pushableItem, NextPos, isLaraPulling))
			{
				pushableInfo.StartPos = pushableItem.Pose.Position;
				pushableInfo.StartPos.RoomNumber = pushableItem.RoomNumber;
				SetStopperFlag(NextPos, true);
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
			{FLOOR_MATERIAL::Mud,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Snow,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Sand,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Gravel,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Ice,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Water,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Stone,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Wood,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Metal,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Marble,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Grass,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Concrete,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::OldWood,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::OldMetal,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom1,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom2,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom3,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom4,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom5,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom6,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom7,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom8,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)}
		};
	}

	int GetPushableSound(const PushableSoundsType& type, const GameVector& detectionPoint)
	{
		auto col = GetCollision(detectionPoint);
		auto materialID = col.BottomBlock->Material;

		int resultSound = 0;
		switch (type)
		{ 
		case (Loop):
			
			resultSound = PushablesSoundsMap[materialID].LoopSound;
			break;

		case (Stop):
			resultSound = PushablesSoundsMap[materialID].StopSound;
			break;

		case (Fall):
			resultSound = PushablesSoundsMap[materialID].FallSound;
			break;

		default:
			TENLog("Error, requesting an inexistent pushable sound type", LogLevel::Error, LogConfig::All, true);
			break;
		}

		return resultSound;
	}

	void PushablesManageSounds(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		GameVector detectionPoint = pushableItem.Pose.Position;
		detectionPoint.RoomNumber = pushableItem.RoomNumber;

		if (pushableInfo.CurrentSoundState == PushableSoundState::Moving)
		{
			SoundEffect(GetPushableSound(Loop, detectionPoint), &pushableItem.Pose, SoundEnvironment::Always);
		}
		else if (pushableInfo.CurrentSoundState == PushableSoundState::Stopping)
		{
			pushableInfo.CurrentSoundState = PushableSoundState::None;
			SoundEffect(GetPushableSound(Stop, detectionPoint), &pushableItem.Pose, SoundEnvironment::Always);
		}
	}

	//General functions

	void InitialisePushablesGeneral()
	{
		//To execute on level start and on level loading.

		PushableAnimationVector =
		{
			PushableAnimationInfo(LA_PUSHABLE_PULL, LA_PUSHABLE_PUSH, true),				//TR4-TR5 animations
			PushableAnimationInfo(LA_PUSHABLE_BLOCK_PULL, LA_PUSHABLE_BLOCK_PUSH, false)	//TR1-TR3 animations
		};

		InitializePushablesSoundsMap();
		UpdateAllPushablesStackLinks();
	}

	void DeactivationRoutine(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		//Re-apply the bridges colliders
		ManageStackBridges(itemNumber, true);

		//Check if it has fall over another pushable.
		UpdateAllPushablesStackLinks();

		// If it has fallen on top of existing pushable
		// Or it's floating in water, don't test triggers.
		if ((pushableInfo.StackLowerItem == NO_ITEM) || (pushableInfo.GravityState == PushableGravityState::Floating))
		{
			TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);
		}

		if (pushableItem.Status == ITEM_ACTIVE && (pushableInfo.GravityState <= PushableGravityState::Falling))
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
				pushables.push_back(i);
		}

		return pushables;
	}

	void UpdateRoomNumbers(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		//Check and update the room number of the pushables and others linked in the stack.
		auto col = GetCollision(&pushableItem);
		if (col.RoomNumber != pushableItem.RoomNumber)
		{
			ItemNewRoom(itemNumber, col.RoomNumber);
			pushableInfo.StartPos.RoomNumber = col.RoomNumber;
		}

		if (pushableInfo.StackUpperItem == NO_ITEM)
			return;

		auto pushableLinkedItem = g_Level.Items[itemNumber];
		auto& pushableLinkedInfo = GetPushableInfo(pushableLinkedItem);

		while (pushableLinkedInfo.StackUpperItem != NO_ITEM)
		{
			auto col = GetCollision(&pushableLinkedItem);
			if (col.RoomNumber != pushableLinkedItem.RoomNumber)
			{
				ItemNewRoom(itemNumber, col.RoomNumber);
				pushableLinkedInfo.StartPos.RoomNumber = col.RoomNumber;
			}

			pushableLinkedItem = g_Level.Items[pushableLinkedInfo.StackUpperItem];
			pushableLinkedInfo = GetPushableInfo(pushableLinkedItem);
		}
	}

	int GetPushableHeight(ItemInfo& item)
	{
		int heightBoundingBox = -GameBoundingBox(&item).Y1;
		int heightWorldAligned = (heightBoundingBox / CLICK(0.5)) * CLICK(0.5);
		return heightWorldAligned;
	}
	
	// Test functions

	bool IsPushableOnValidSurface(ItemInfo& pushableItem)
	{
		auto pushableInfo = GetPushableInfo(pushableItem);

		CollisionResult col;

		if (pushableInfo.HasFloorColission)
		{
			RemoveBridge(pushableItem.Index);
			col = GetCollision(&pushableItem);
			AddBridge(pushableItem.Index);
		}
		else
		{
			col = GetCollision(&pushableItem);
		}

		//It's in a wall
		if (col.Block->IsWall(pushableItem.Pose.Position.x, pushableItem.Pose.Position.z))
			return false;

		//Or it isn't on the floor
		int floorDiference = abs(col.Position.Floor - pushableItem.Pose.Position.y);
		if ((floorDiference >= PUSHABLE_HEIGHT_TOLERANCE))
			return false;

		return true;
	}

	bool IsNextSectorValid(ItemInfo& pushableItem, const GameVector& targetPoint, const bool checkIfLaraFits)
	{
		if (!IsPushableOnValidSurface(pushableItem))
			return false;

		if (!CheckStackLimit(pushableItem))
			return false;

		auto& pushableInfo = GetPushableInfo(pushableItem);
		auto col = GetCollision(targetPoint);

		//It's in a wall
		if (col.Block->IsWall(targetPoint.x, targetPoint.z))
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
		{
			if (col.Position.Floor <= pushableItem.Pose.Position.y)
			{
				return false;
			}
			//else
			//{
				// Maybe the stopper is because there is another pushable down there.
				// TODO: Need better search tools

			//}
		}
			

		//Is a gap or a step?, (Can it fall down?) (Only available for pushing).
		int floorDiference = abs(col.Position.Floor - pushableItem.Pose.Position.y);
		if (pushableInfo.CanFall)
		{
			if ((col.Position.Floor < pushableItem.Pose.Position.y) && (floorDiference >= PUSHABLE_HEIGHT_TOLERANCE))
				return false;
		}
		else
		{
			if (floorDiference >= PUSHABLE_HEIGHT_TOLERANCE)
				return false;
		}

		//Is ceiling (square or diagonal) high enough?
		int distanceToCeiling = abs(col.Position.Ceiling - col.Position.Floor);
		int blockHeight = GetStackHeight(pushableItem);
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

			auto& object = Objects[CollidedItems[i]->ObjectNumber];
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
		auto dirVector = targetPoint.ToVector3i() - pushableItem.Pose.Position;
		
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
		if (pushableInfo.HasFloorColission)
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
	void MoveStack(const short itemNumber, const Vector3i& GoalPos)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		if (pushableInfo.StackUpperItem == NO_ITEM)
			return;

		auto& pushableLinkedItem = g_Level.Items[pushableInfo.StackUpperItem];
		auto& pushableLinkedInfo = GetPushableInfo(pushableLinkedItem);

		while (true)
		{
			pushableLinkedItem.Pose.Position.x = GoalPos.x;
			pushableLinkedItem.Pose.Position.z = GoalPos.z;
			pushableLinkedItem.Pose.Position.y += GoalPos.y; //The vertical movement receives a velocity, not a fixed value.

			pushableLinkedInfo.StartPos = pushableLinkedItem.Pose.Position;
			pushableLinkedInfo.StartPos.RoomNumber = pushableLinkedItem.RoomNumber;

			if (pushableLinkedInfo.StackUpperItem == NO_ITEM)
				break;

			pushableLinkedItem = g_Level.Items[pushableLinkedInfo.StackUpperItem];
			pushableLinkedInfo = GetPushableInfo(pushableLinkedItem);

		}
	}

	void MoveStackXZ (const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		MoveStack(itemNumber, Vector3i (pushableItem.Pose.Position.x, 0, pushableItem.Pose.Position.z));
	}
	void MoveStackY (const short itemNumber, const int y)
	{
		MoveStack(itemNumber, Vector3i (0, y, 0));
	}

	void ManageStackBridges(short itemNumber, bool addBridge)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		if (pushableInfo.HasFloorColission)
		{
			if (addBridge)
				TEN::Floordata::AddBridge(itemNumber);
			else
				TEN::Floordata::RemoveBridge(itemNumber);
		}

		if (pushableInfo.StackUpperItem == NO_ITEM)
		{
			return;
		}
		
		auto pushableLinkedItem = g_Level.Items[pushableInfo.StackUpperItem];
		auto pushableLinkedInfo = GetPushableInfo(pushableLinkedItem);

		while (pushableLinkedInfo.StackUpperItem != NO_ITEM)
		{
			if (pushableLinkedInfo.HasFloorColission)
			{
				if (addBridge)
					TEN::Floordata::AddBridge(pushableLinkedItem.Index);
				else
					TEN::Floordata::RemoveBridge(pushableLinkedItem.Index);
			}

			pushableLinkedItem = g_Level.Items[pushableLinkedInfo.StackUpperItem];
			pushableLinkedInfo = GetPushableInfo(pushableLinkedItem);
		}
	}

	void RemovePushableFromStack(short itemNumber) 
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		if (pushableInfo.StackLowerItem != NO_ITEM)
		{
			auto& lowerPushable = g_Level.Items[pushableInfo.StackLowerItem];
			auto& lowerPushableInfo = GetPushableInfo(lowerPushable);
			
			lowerPushableInfo.StackUpperItem = NO_ITEM;
			pushableInfo.StackLowerItem = NO_ITEM;
		}
	}	

	void UpdateAllPushablesStackLinks()
	{
		auto& pushablesNumbersList = FindAllPushables(g_Level.Items);
	
		if (pushablesNumbersList.empty())
			return;

		std::sort(pushablesNumbersList.begin(), pushablesNumbersList.end(), CompareItemByXZ);

		for (std::size_t i = 0; i < pushablesNumbersList.size() - 1; ++i) 
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
				auto& upperPushableInfo = GetPushableInfo(upperPushableItem);
				auto& lowerPushableInfo = GetPushableInfo(lowerPushableItem);
				upperPushableInfo.StackLowerItem = lowerPushableItem.Index;
				lowerPushableInfo.StackUpperItem = upperPushableItem.Index;
			}
		}
	}

	int GetStackHeight(ItemInfo& item)
	{
		auto pushableItem = item;
		auto pushableInfo = GetPushableInfo(pushableItem);
		
		int height = pushableInfo.Height;
		
		while (pushableInfo.StackUpperItem != NO_ITEM)
		{
			pushableItem = g_Level.Items[pushableInfo.StackUpperItem];
			pushableInfo = GetPushableInfo(pushableItem);

			height += pushableInfo.Height;
		}

		return height;
	}
	bool CheckStackLimit(ItemInfo& item)
	{
		auto pushableItem = item;
		auto pushableInfo = GetPushableInfo(pushableItem);

		int limit = pushableInfo.StackLimit;
		int count = 1;
		
		while (pushableInfo.StackUpperItem != NO_ITEM)
		{
			pushableItem = g_Level.Items[pushableInfo.StackUpperItem];
			pushableInfo = GetPushableInfo(pushableItem);

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
		auto& pushableInfo = GetPushableInfo(pushableItem);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, false);
	
		if (pushableItem.Status != ITEM_INVISIBLE && pushableInfo.HasFloorColission && boxHeight.has_value())
		{
			const auto height = pushableItem.Pose.Position.y - GetPushableHeight(pushableItem);

			return std::optional{height};
		}

		return std::nullopt;
	}

	std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, true);

		if (pushableItem.Status != ITEM_INVISIBLE && pushableInfo.HasFloorColission && boxHeight.has_value())
			return std::optional{ pushableItem.Pose.Position.y};

		return std::nullopt;
	}

	int PushableBlockFloorBorder(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		auto height = item.Pose.Position.y - GetPushableHeight(item);

		return height;
	}

	int PushableBlockCeilingBorder(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		return item->Pose.Position.y;
	}
}
