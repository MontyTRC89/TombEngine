#include "framework.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Physics.h"

#include "Game/animation.h"
#include "Game/control/flipeffect.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Object/Pushables/PushableObject.h"
#include "Objects/Generic/Object/Pushables/PushableObject_BridgeCol.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Effects.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Scans.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Stack.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Input;

namespace TEN::Entities::Generic
{
	std::unordered_map<PushablePhysicState, std::function<void(int)>> PUSHABLES_STATES_MAP;

	constexpr auto PUSHABLE_FALL_VELOCITY_MAX = BLOCK(1 / 8.0f);
	constexpr auto PUSHABLE_WATER_VELOCITY_MAX = BLOCK(1 / 16.0f);
	constexpr auto GRAVITY_AIR = 8.0f;
	constexpr auto GRAVITY_WATER = 4.0f;
	constexpr auto GRAVITY_ACCEL = 0.5f;
	constexpr auto PUSHABLE_FALL_RUMBLE_VELOCITY = 96.0f;
	constexpr auto WATER_SURFACE_DISTANCE = CLICK(0.5f);

	void InitializePushablesStatesMap()
	{
		if (PUSHABLES_STATES_MAP.empty() )
		{
			PUSHABLES_STATES_MAP.clear();

			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Idle, &HandleIdleState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Moving, &HandleMovingState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Falling, &HandleFallingState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Sinking, &HandleSinkingState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Floating, &HandleFloatingState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::UnderwaterIdle, &HandleUnderwaterState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::WatersurfaceIdle, &HandleWatersurfaceState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Sliding, &HandleSlidingState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::StackHorizontalMove, &HandleStackHorizontalMoveState);
		}
	}

	void HandleIdleState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//1. Check if Lara is interacting with it
		if (Lara.Context.InteractedItem == itemNumber)
		{
			//If Lara is grabbing, check the input push pull conditions
			if (PushableIdleConditions(itemNumber))
			{
				//If all good, then do the interaction.
				if (IsHeld(In::Forward)) //Pushing
				{
					int pushAnimNumber = PushableAnimInfos[pushable.AnimationSystemIndex].PushAnimNumber;
					SetAnimation(LaraItem, pushAnimNumber);
				}
				else if (IsHeld(In::Back)) //Pulling
				{
					int pullAnimNumber = PushableAnimInfos[pushable.AnimationSystemIndex].PullAnimNumber;
					SetAnimation(LaraItem, pullAnimNumber);
				}

				pushable.StartPos = pushableItem.Pose.Position;
				pushable.StartPos.RoomNumber = pushableItem.RoomNumber;
				pushable.BehaviourState = PushablePhysicState::Moving;

				UnpilePushable(itemNumber);				//Cut the link with the lower pushables in the stack.
				StartMovePushableStack(itemNumber);		//Prepare the upper pushables in the stack for the move.

				ResetPlayerFlex(LaraItem);

				DeactivateClimbablePushableCollider(itemNumber);
			}
			else if (	LaraItem->Animation.ActiveState != LS_PUSHABLE_GRAB &&
						LaraItem->Animation.ActiveState != LS_PUSHABLE_PULL &&
						LaraItem->Animation.ActiveState != LS_PUSHABLE_PUSH)
			{
				Lara.Context.InteractedItem = NO_ITEM;
			}
		}

		//2. Check environment
		int floorHeight;
		PushableEnvironemntState envState = CheckPushableEnvironment(itemNumber, floorHeight);

		switch (envState)
		{	
			case PushableEnvironemntState::Ground:
				if (floorHeight != pushableItem.Pose.Position.y)
				{
					//As the diffence is not very big, just teleport it.
					int heightdifference = floorHeight - pushableItem.Pose.Position.y;
					pushableItem.Pose.Position.y += heightdifference;
					VerticalPosAddition(itemNumber, heightdifference); //WIP: function to elevate the stack.
				}
			break;

			case PushableEnvironemntState::Air:
				pushable.BehaviourState = PushablePhysicState::Falling;
				SetPushableStopperFlag(false, pushableItem.Pose.Position, pushableItem.RoomNumber);
				DeactivateClimbablePushableCollider(itemNumber);
			break;

			case PushableEnvironemntState::ShallowWater:

				// Effects: Spawn ripples.
				DoPushableRipples(itemNumber);
				break;

			case PushableEnvironemntState::DeepWater:
				DeactivateClimbablePushableCollider(itemNumber);
				SetPushableStopperFlag(false, pushableItem.Pose.Position, pushableItem.RoomNumber);

				if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
				{
					pushable.BehaviourState = PushablePhysicState::Floating;
					pushable.Gravity = 0.0f;
				}
				else
				{
					pushable.BehaviourState = PushablePhysicState::Sinking;
					pushable.Gravity = 0.0f;
				}
			break;

			default:
				TENLog("Error detecting the pushable environment state during IDLE with pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
				break;
		}
	}

	void HandleMovingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		bool isPlayerPulling = LaraItem->Animation.ActiveState == LS_PUSHABLE_PULL;

		int quadrantDir = GetQuadrant(LaraItem->Pose.Orientation.y);
		int newPosX = pushable.StartPos.x;
		int newPosZ = pushable.StartPos.z;

		int displaceDepth = 0;
		int displaceBox = GameBoundingBox(LaraItem).Z2;

		pushable.CurrentSoundState = PushableSoundState::None;

		displaceDepth = GetLastFrame(GAME_OBJECT_ID::ID_LARA, LaraItem->Animation.AnimNumber)->BoundingBox.Z2;
		displaceBox -= isPlayerPulling ? (displaceDepth + BLOCK(1)) : (displaceDepth - BLOCK(1));

		//Lara is doing the pushing / pulling animation
		if (LaraItem->Animation.FrameNumber != g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd - 1)
		{
			//1. Decide the goal position
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

			//2. Move pushable
			
			//Don't move the pushable if the distance is too big.
			//(it may happens because the animation bounds changes in the animation loop of push pull process).
			if (abs(pushableItem.Pose.Position.z - newPosZ) > BLOCK(0.75f))
				return;
			if (abs(pushableItem.Pose.Position.x - newPosX) > BLOCK(0.75f))
				return;

			// move only if the move direction is oriented to the action
			// So pushing only moves pushable forward, and pulling only moves backwards

			//Z axis
			if (isPlayerPulling)
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

			//X axis
			if (isPlayerPulling)
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

			if (pushable.WaterSurfaceHeight != NO_HEIGHT)
			{
				// Effects: Spawn ripples.
				DoPushableRipples(itemNumber);
			}
		}
		else
		{
			//Pushing Pulling animation ended
			
			//1. Realign with sector center
			pushableItem.Pose.Position = GetNearestSectorCenter(pushableItem.Pose.Position);

			//2. The pushable is going to stop here, do the checks to conect it with another Stack.
			int FoundStack = SearchNearPushablesStack(itemNumber);
			StackPushable(itemNumber, FoundStack);

			//3.: It only should do it if there is not any other pushable remaining there
			SetPushableStopperFlag(false, pushable.StartPos.ToVector3i(), pushable.StartPos.RoomNumber);

			pushable.StartPos = pushableItem.Pose.Position;
			pushable.StartPos.RoomNumber = pushableItem.RoomNumber;

			//4. Activate trigger
			TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

			//5. Check environment
			int floorHeight;
			PushableEnvironemntState envState = CheckPushableEnvironment(itemNumber, floorHeight);

			switch (envState)
			{
				case PushableEnvironemntState::Ground:
				case PushableEnvironemntState::ShallowWater:
					// Check if it has to stop the pushing/pulling movement.
					if (!PushableAnimInfos[pushable.AnimationSystemIndex].EnableAnimLoop ||
						!IsHeld(In::Action) ||
						!PushableMovementConditions(itemNumber, !isPlayerPulling, isPlayerPulling))
					{
						LaraItem->Animation.TargetState = LS_IDLE;
						pushable.BehaviourState = PushablePhysicState::Idle;
						StopMovePushableStack(itemNumber); //Set the upper pushables back to normal.
						ActivateClimbablePushableCollider(itemNumber);

						//The pushable is going to stop here, do the checks to conect it with another Stack.
						int FoundStack = SearchNearPushablesStack(itemNumber);
						StackPushable(itemNumber, FoundStack);

						pushable.CurrentSoundState = PushableSoundState::Stopping;

					}
					//Otherwise, continue the movement.
				break;

				case PushableEnvironemntState::Air:
					// It's now in the air, Lara can't keep pushing nor pulling. And pushable starts to fall.
					LaraItem->Animation.TargetState = LS_IDLE;
					Lara.Context.InteractedItem = NO_ITEM;
					pushable.BehaviourState = PushablePhysicState::Falling;
					pushable.CurrentSoundState = PushableSoundState::None;

					return;
				break;

				case PushableEnvironemntState::Slope:
					//TODO: if it's a slope ground?...
					//Then proceed to the sliding state.
					break;

				case PushableEnvironemntState::DeepWater:
					TENLog("Warning, detecting DeepWater environment in pushable move state. Pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
				break;

				default:
					TENLog("Error detecting the pushable environment state during MOVING with pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
				break;
			}
		}
	}

	void HandleFallingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int floorHeight;
		PushableEnvironemntState envState = CheckPushableEnvironment(itemNumber, floorHeight);

		int FoundStack = NO_ITEM;

		switch (envState)
		{
			case PushableEnvironemntState::Air:
				//Is still falling...
				pushableItem.Pose.Position.y += pushableItem.Animation.Velocity.y;
				pushableItem.Animation.Velocity.y = std::min(	pushableItem.Animation.Velocity.y + pushable.Gravity,
																PUSHABLE_FALL_VELOCITY_MAX);
			break;

			case PushableEnvironemntState::Slope: // TODO, the Slope will use Ground routine while it gets its own version created.
			case PushableEnvironemntState::Ground:
				//The pushable is going to stop here, do the checks to conect it with another Stack.
				FoundStack = SearchNearPushablesStack(itemNumber);
				StackPushable(itemNumber, FoundStack);

				//Set Stopper Flag
				if (pushable.StackLowerItem == NO_ITEM)
					SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

				//Activate trigger
				TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

				// Shake floor if pushable landed at high enough velocity.
				if (pushableItem.Animation.Velocity.y >= PUSHABLE_FALL_RUMBLE_VELOCITY)
					FloorShake(&pushableItem);

				//place on ground
				pushable.BehaviourState = PushablePhysicState::Idle;
				pushableItem.Pose.Position.y = floorHeight;
				pushableItem.Animation.Velocity.y = 0;

				ActivateClimbablePushableCollider(itemNumber);
				
			break;

			//case PushableEnvironemntState::Slope:
				//TODO: if it's a slope ground?...
				//Then proceed to the sliding state.
			//break;

			case PushableEnvironemntState::ShallowWater:
			case PushableEnvironemntState::DeepWater:
				pushable.BehaviourState = PushablePhysicState::Sinking;

				// Effect: Water splash.
				DoPushableSplash(itemNumber);
			break;

			default:
				TENLog("Error detecting the pushable environment state during FALLING with pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
			break;
		}
	}

	void HandleSinkingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int floorHeight;
		PushableEnvironemntState envState = CheckPushableEnvironment(itemNumber, floorHeight);

		int FoundStack = NO_ITEM;

		switch (envState)
		{
			case PushableEnvironemntState::Air:
			case PushableEnvironemntState::Ground:
			case PushableEnvironemntState::Slope:
				//Set Stopper Flag
				if (pushable.StackLowerItem == NO_ITEM)
					SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

				pushable.BehaviourState = PushablePhysicState::Falling;
				pushable.Gravity = GRAVITY_AIR;

				break;

			case PushableEnvironemntState::ShallowWater:
				//Manage gravity force
				// Decreases gravity, continues to fall until hits ground.
				pushable.Gravity = std::max(pushable.Gravity - GRAVITY_ACCEL, GRAVITY_WATER);

				//Move Object
				if (floorHeight > pushableItem.Pose.Position.y + pushableItem.Animation.Velocity.y)
				{
					// Sinking down.
					pushableItem.Pose.Position.y += pushableItem.Animation.Velocity.y;
					pushableItem.Animation.Velocity.y = std::min(pushableItem.Animation.Velocity.y + pushable.Gravity,
						PUSHABLE_WATER_VELOCITY_MAX);
				}
				else
				{
					//Landing
					pushableItem.Pose.Position.y = floorHeight;
					pushable.BehaviourState = PushablePhysicState::Idle;
					pushable.Gravity = GRAVITY_AIR;
					pushableItem.Animation.Velocity.y = 0.0f;
					ActivateClimbablePushableCollider(itemNumber);

					//Activate trigger
					TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);
				}

				// Effects: Spawn ripples.
				DoPushableRipples(itemNumber);

				break;

			case PushableEnvironemntState::DeepWater:
				//Manage gravity force
				if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
				{
					// Slowly reverses gravity direction. If gravity is < 0, then it floats up.
					pushable.Gravity = pushable.Gravity - GRAVITY_ACCEL;
					if (pushable.Gravity <= 0.0f)
					{
						pushable.BehaviourState = PushablePhysicState::Floating;
						return;
					}
				}
				else
				{
					// Decreases gravity, continues to fall until hits ground.
					pushable.Gravity = std::max(pushable.Gravity - GRAVITY_ACCEL, GRAVITY_WATER);
				}

				//Move Object
				if (floorHeight > pushableItem.Pose.Position.y + pushableItem.Animation.Velocity.y)
				{
					// Sinking down.
					pushableItem.Pose.Position.y += pushableItem.Animation.Velocity.y;
					pushableItem.Animation.Velocity.y = std::min(	pushableItem.Animation.Velocity.y + pushable.Gravity,
																	PUSHABLE_WATER_VELOCITY_MAX);
				}
				else
				{
					//Landing
					if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
					{
						pushableItem.Pose.Position.y = floorHeight;
						pushable.BehaviourState = PushablePhysicState::Floating;
						pushable.Gravity = 0.0f;
					}
					else
					{
						pushableItem.Pose.Position.y = floorHeight;
						pushable.BehaviourState = PushablePhysicState::UnderwaterIdle;
						pushable.Gravity = GRAVITY_WATER;
						pushableItem.Animation.Velocity.y = 0.0f;
						ActivateClimbablePushableCollider(itemNumber);

						//Activate trigger
						TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);
					}
				}

				// Effects: Spawn bubbles.
				DoPushableBubbles(itemNumber);

				break;

			default:
				TENLog("Error detecting the pushable environment state during SINKING with pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
				break;

		}
	}

	void HandleFloatingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int floorHeight;
		PushableEnvironemntState envState = CheckPushableEnvironment(itemNumber, floorHeight);

		int goalHeight = 0;
				
		switch (envState)
		{
			case PushableEnvironemntState::Air:
			case PushableEnvironemntState::Ground:
			case PushableEnvironemntState::Slope:				
				//Set Stopper Flag
				if (pushable.StackLowerItem == NO_ITEM)
					SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

				pushable.BehaviourState = PushablePhysicState::Falling;
				pushable.Gravity = GRAVITY_AIR;
			break;

			case PushableEnvironemntState::ShallowWater:
				TENLog("Warning, detected unexpected shallowWater routine during FLOATING state, passing to IDLE state, pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
				
				
				pushableItem.Pose.Position.y = floorHeight;
				pushable.BehaviourState = PushablePhysicState::Idle;
				pushable.Gravity = GRAVITY_AIR;
				pushableItem.Animation.Velocity.y = 0.0f;
				ActivateClimbablePushableCollider(itemNumber);

			break;

			case PushableEnvironemntState::DeepWater:

				//Calculate goal height
				if (pushable.WaterSurfaceHeight == NO_HEIGHT)
				{
					//There is ceiling, put pushable under it.
					goalHeight = GetCollision(&pushableItem).Position.Ceiling + WATER_SURFACE_DISTANCE + pushable.Height;
				}
				else
				{
					//No ceiling, so rise above water
					goalHeight = pushable.WaterSurfaceHeight - WATER_SURFACE_DISTANCE + pushable.Height;
				}

				//Manage gravity force
				pushable.Gravity = std::max(pushable.Gravity - GRAVITY_ACCEL, -GRAVITY_WATER);

				//Move pushable upwards
				if (goalHeight < pushableItem.Pose.Position.y)
				{
					// Floating up.
					pushableItem.Pose.Position.y += pushableItem.Animation.Velocity.y;
					pushableItem.Animation.Velocity.y = std::min(	pushableItem.Animation.Velocity.y + pushable.Gravity,
																	PUSHABLE_WATER_VELOCITY_MAX);
				}
				else
				{
					//Reached goal height
					pushableItem.Pose.Position.y = goalHeight;
					pushable.BehaviourState = PushablePhysicState::WatersurfaceIdle;
					pushable.Gravity = GRAVITY_WATER;
					pushableItem.Animation.Velocity.y = 0.0f;
					ActivateClimbablePushableCollider(itemNumber);
				}

			break;

			default:
				TENLog("Error detecting the pushable environment state during FLOATING with pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
			break;
		}
	}

	void HandleUnderwaterState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int floorHeight;
		PushableEnvironemntState envState = CheckPushableEnvironment(itemNumber, floorHeight);

		switch (envState)
		{
			case PushableEnvironemntState::Ground:
			case PushableEnvironemntState::Slope:
			case PushableEnvironemntState::Air:
			case PushableEnvironemntState::ShallowWater:
				//Set Stopper Flag
				if (pushable.StackLowerItem == NO_ITEM)
					SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

				pushableItem.Pose.Position.y = floorHeight;
				pushable.BehaviourState = PushablePhysicState::Idle;
				pushableItem.Animation.Velocity.y = 0.0f;
				pushable.Gravity = GRAVITY_AIR;	
			break;
			
			case PushableEnvironemntState::DeepWater:
				if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
				{
					//If it's buoyant, it needs to float. (case for sudden flipmaps).
					//Reset Stopper Flag
					if (pushable.StackLowerItem == NO_ITEM)
						SetPushableStopperFlag(false, pushableItem.Pose.Position, pushableItem.RoomNumber);

					pushable.BehaviourState = PushablePhysicState::Floating;
					pushableItem.Animation.Velocity.y = 0.0f;
					pushable.Gravity = 0.0f;
				}

			break;
		
			default:
				TENLog("Error detecting the pushable environment state during UNDERWATER IDLE with pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
			break;
		}
	}

	void HandleWatersurfaceState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int floorHeight;
		PushableEnvironemntState envState = CheckPushableEnvironment(itemNumber, floorHeight);

		switch (envState)
		{
			case PushableEnvironemntState::Ground:
			case PushableEnvironemntState::Slope:
			case PushableEnvironemntState::Air:
			case PushableEnvironemntState::ShallowWater:
				pushable.BehaviourState = PushablePhysicState::Falling;
				pushableItem.Animation.Velocity.y = 0.0f;
				pushable.Gravity = GRAVITY_AIR;
				DeactivateClimbablePushableCollider(itemNumber);

				if (!pushable.UsesRoomCollision)
					pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);
			break;
		
			case PushableEnvironemntState::DeepWater:
				// Effects: Do water ondulation effect.
					if (!pushable.UsesRoomCollision)
						FloatingItem(pushableItem, pushable.FloatingForce);
					else
						FloatingBridge(pushableItem, pushable.FloatingForce);

				// Effects: Spawn ripples.
				DoPushableRipples(itemNumber);
			break;

			default:
				TENLog("Error detecting the pushable environment state during WATER SURFACE IDLE with pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
			break;
		}
	}

	void HandleSlidingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//TODO:
		//1. DETECT FLOOR, (Slope orientation)
		//2. CALCULATE DIRECTION AND SPEED
		//3. MOVE OBJECT
		//4. DETECT END CONDITIONS OF NEXT SECTOR 
			//	Is a slope-> keep sliding
			//	Is a flat floor -> so ends slide
			//	Is a fall -> so passes to falling
			//	Is a forbiden Sector -> freezes the slide
		//5. Incorporate effects? Smoke or sparkles?

	}

	void HandleStackHorizontalMoveState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		auto& MovingPushableItem = g_Level.Items[Lara.Context.InteractedItem];
		pushableItem.Pose.Position.x = MovingPushableItem.Pose.Position.x;
		pushableItem.Pose.Position.z = MovingPushableItem.Pose.Position.z;
	}

	/*TODO: 
	void HandleStackFallingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		auto& MovingPushableItem = g_Level.Items[Lara.Context.InteractedItem];
		pushableItem.Pose.Position.y = MovingPushableItem.Pose.Position.y;
	}*/
}
