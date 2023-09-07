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
	constexpr auto PUSHABLE_FALL_VELOCITY_MAX  = BLOCK(1 / 8.0f);
	constexpr auto PUSHABLE_WATER_VELOCITY_MAX = BLOCK(1 / 16.0f);

	constexpr auto PUSHABLE_EDGE_FALL_VELOCITY = 0.8f;
	constexpr auto PUSHABLE_EDGE_ANGLE_MAX = 60.0f;

	constexpr auto GRAVITY_AIR	 = 8.0f;
	constexpr auto GRAVITY_WATER = 4.0f;
	constexpr auto GRAVITY_ACCEL = 0.5f;

	constexpr auto PUSHABLE_FALL_RUMBLE_VELOCITY = 96.0f;

	constexpr auto WATER_SURFACE_DISTANCE = CLICK(0.5f);

	// TODO: Do it this way.
	std::unordered_map<PushableState, std::function<void(int)>> PUSHABLE_STATE_MAP;/* =
	{
		{ PushableState::Idle, HandleIdleState },
		{ PushableState::EdgeFall, HandleEdgeFallState },
		{ PushableState::Fall, HandleFallState },
		{ PushableState::Sink, HandleSinkState },
		{ PushableState::Float, HandleFloatState },
		{ PushableState::UnderwaterIdle, HandleUnderwaterState },
		{ PushableState::WaterSurfaceIdle, HandleWaterSurfaceState },
		{ PushableState::Slide, HandleSlideState },
		{ PushableState::MoveStackHorizontal, HandleMoveStackHorizontalState }
	};*/

	// TODO: Remove.
	void InitializePushableStateMap()
	{
		if (PUSHABLE_STATE_MAP.empty() )
		{
			PUSHABLE_STATE_MAP.clear();

			PUSHABLE_STATE_MAP.emplace(PushableState::Idle, &HandleIdleState);
			PUSHABLE_STATE_MAP.emplace(PushableState::Move, &HandleMoveState);
			PUSHABLE_STATE_MAP.emplace(PushableState::EdgeFall, &HandleEdgeFallState);
			PUSHABLE_STATE_MAP.emplace(PushableState::Fall, &HandleFallState);
			PUSHABLE_STATE_MAP.emplace(PushableState::Sink, &HandleSinkState);
			PUSHABLE_STATE_MAP.emplace(PushableState::Float, &HandleFloatState);
			PUSHABLE_STATE_MAP.emplace(PushableState::UnderwaterIdle, &HandleUnderwaterState);
			PUSHABLE_STATE_MAP.emplace(PushableState::WaterSurfaceIdle, &HandleWaterSurfaceState);
			PUSHABLE_STATE_MAP.emplace(PushableState::Slide, &HandleSlideState);
			PUSHABLE_STATE_MAP.emplace(PushableState::MoveStackHorizontal, &HandleMoveStackHorizontalState);
		}
	}

	void HandleIdleState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		// 1) Check if player is interacting.
		if (Lara.Context.InteractedItem == itemNumber)
		{
			if (PushableIdleConditions(itemNumber))
			{
				// Pushing.
				if (IsHeld(In::Forward))
				{
					int pushAnimNumber = (pushable.isOnEdge) ? 
						PushableAnimInfos[pushable.AnimationSystemIndex].EdgeAnimNumber :
						PushableAnimInfos[pushable.AnimationSystemIndex].PushAnimNumber;
					SetAnimation(LaraItem, pushAnimNumber);
				}
				// Pulling.
				else if (IsHeld(In::Back))
				{
					int pullAnimNumber = PushableAnimInfos[pushable.AnimationSystemIndex].PullAnimNumber;
					SetAnimation(LaraItem, pullAnimNumber);
				}

				pushable.StartPos = pushableItem.Pose.Position;
				pushable.StartPos.RoomNumber = pushableItem.RoomNumber;
				pushable.BehaviourState = PushableState::Move;

				// Cut links with  lower pushables in stack.
				UnpilePushable(itemNumber);

				// Prepare upper pushables in stack for movement.
				StartMovePushableStack(itemNumber);

				ResetPlayerFlex(LaraItem);
				DeactivateClimbablePushableCollider(itemNumber);
			}
			else if (LaraItem->Animation.ActiveState != LS_PUSHABLE_GRAB &&
				LaraItem->Animation.ActiveState != LS_PUSHABLE_PULL &&
				LaraItem->Animation.ActiveState != LS_PUSHABLE_PUSH &&
				LaraItem->Animation.ActiveState != LS_PUSHABLE_EDGE)
			{
				Lara.Context.InteractedItem = NO_ITEM;
			}
		}

		// 2) Check environment.
		int floorHeight;
		auto envState = CheckPushableEnvironment(itemNumber, floorHeight);

		switch (envState)
		{	
		case PushableEnvironmentState::Ground:
			if (floorHeight != pushableItem.Pose.Position.y)
			{
				pushableItem.Pose.Position.y = floorHeight;
				int heightdifference = floorHeight - pushableItem.Pose.Position.y;
				VerticalPosAddition(itemNumber, heightdifference);
			}

			break;

		case PushableEnvironmentState::GroundWater:
		{
			if (floorHeight != pushableItem.Pose.Position.y)
			{
				pushableItem.Pose.Position.y = floorHeight;
				int heightdifference = floorHeight - pushableItem.Pose.Position.y;
				VerticalPosAddition(itemNumber, heightdifference);
			}

			int waterheight = abs(floorHeight - pushable.WaterSurfaceHeight);
			if (waterheight > GetPushableHeight(pushableItem))
			{
				if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
				{
					pushable.BehaviourState = PushableState::Float;
					pushable.Gravity = 0.0f;
				}
			}

			DoPushableRipples(itemNumber);
		}
			break;

		case PushableEnvironmentState::Air:

			// Only pass to falling if distance to bigger than 1 click. If is small, just stuck it to the ground.
			if (abs(floorHeight - pushableItem.Pose.Position.y) > CLICK(0.75f))
			{
				pushable.BehaviourState = PushableState::Fall;
				SetPushableStopperFlag(false, pushableItem.Pose.Position, pushableItem.RoomNumber);
				DeactivateClimbablePushableCollider(itemNumber);
			}
			else
			{
				pushableItem.Pose.Position.y = floorHeight;
				int heightdifference = floorHeight - pushableItem.Pose.Position.y;
				VerticalPosAddition(itemNumber, heightdifference);
			}

			break;

		case PushableEnvironmentState::Water:
			DeactivateClimbablePushableCollider(itemNumber);
			SetPushableStopperFlag(false, pushableItem.Pose.Position, pushableItem.RoomNumber);

			if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
			{
				pushable.BehaviourState = PushableState::Float;
				pushable.Gravity = 0.0f;
			}
			else
			{
				pushable.BehaviourState = PushableState::Sink;
				pushable.Gravity = GRAVITY_WATER;
			}

			break;

		default:
			TENLog("Error detecting pushable environment state during IDLE for pushable with item number " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All);
			break;
		}
	}

	void HandleMoveState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		bool isPlayerPulling = LaraItem->Animation.ActiveState == LS_PUSHABLE_PULL;

		int quadrantDir = GetQuadrant(LaraItem->Pose.Orientation.y);
		int newPosX = pushable.StartPos.x;
		int newPosZ = pushable.StartPos.z;

		int displaceDepth = 0;
		int displaceBox = GameBoundingBox(LaraItem).Z2;


		if (pushable.SoundState == PushableSoundState::Moving)
			pushable.SoundState = PushableSoundState::Stopping;

		displaceDepth = GetLastFrame(GAME_OBJECT_ID::ID_LARA, LaraItem->Animation.AnimNumber)->BoundingBox.Z2;
		
		if (isPlayerPulling)
		{
			displaceBox -= (displaceDepth + BLOCK(1));
		}
		else
		{
			if (pushable.isOnEdge)
			{
				displaceBox -= (displaceDepth - BLOCK(0.5f));
			}
			else
			{
				displaceBox -= (displaceDepth - BLOCK(1));
			}
		}

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


			int travelledDistance = Vector3i::Distance(pushableItem.Pose.Position, pushable.StartPos.ToVector3i());
			if (pushable.isOnEdge && travelledDistance >= BLOCK(0.5f))
			{
				pushable.BehaviourState = PushableState::EdgeFall;
				return;
			}
			
			// move only if the move direction is oriented to the action
			// So pushing only moves pushable forward, and pulling only moves backwards

			//Z axis
			if (isPlayerPulling)
			{
				if ((quadrantDir == NORTH && pushableItem.Pose.Position.z > newPosZ) ||
					(quadrantDir == SOUTH && pushableItem.Pose.Position.z < newPosZ))
				{
					pushableItem.Pose.Position.z = newPosZ;
					pushable.SoundState = PushableSoundState::Moving;
				}
			}
			else
			{
				if ((quadrantDir == NORTH && pushableItem.Pose.Position.z < newPosZ) ||
					(quadrantDir == SOUTH && pushableItem.Pose.Position.z > newPosZ))
				{
					pushableItem.Pose.Position.z = newPosZ;
					pushable.SoundState = PushableSoundState::Moving;
				}
			}

			//X axis
			if (isPlayerPulling)
			{
				if ((quadrantDir == EAST && pushableItem.Pose.Position.x > newPosX) ||
					(quadrantDir == WEST && pushableItem.Pose.Position.x < newPosX))
				{
					pushableItem.Pose.Position.x = newPosX;
					pushable.SoundState = PushableSoundState::Moving;
				}
			}
			else
			{
				if ((quadrantDir == EAST && pushableItem.Pose.Position.x < newPosX) ||
					(quadrantDir == WEST && pushableItem.Pose.Position.x > newPosX))
				{
					pushableItem.Pose.Position.x = newPosX;
					pushable.SoundState = PushableSoundState::Moving;
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
			
			//5. Check environment
			int floorHeight;
			PushableEnvironmentState envState = CheckPushableEnvironment(itemNumber, floorHeight);

			switch (envState)
			{
			case PushableEnvironmentState::Ground:
			case PushableEnvironmentState::GroundWater:
				//Activate trigger
				TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

				// Check if it has to stop the pushing/pulling movement.
				if (!PushableAnimInfos[pushable.AnimationSystemIndex].EnableAnimLoop ||
					!IsHeld(In::Action) ||
					!PushableMovementConditions(itemNumber, !isPlayerPulling, isPlayerPulling))
				{
					LaraItem->Animation.TargetState = LS_IDLE;
					pushable.BehaviourState = PushableState::Idle;
					StopMovePushableStack(itemNumber); //Set the upper pushables back to normal.
					ActivateClimbablePushableCollider(itemNumber);

					//The pushable is going to stop here, do the checks to conect it with another Stack.
					int FoundStack = SearchNearPushablesStack(itemNumber);
					StackPushable(itemNumber, FoundStack);

					pushable.SoundState = PushableSoundState::Stopping;

				}
				else if (LaraItem->Animation.ActiveState == LS_PUSHABLE_PUSH && pushable.isOnEdge)
				{
					LaraItem->Animation.TargetState = LS_PUSHABLE_EDGE;

					Vector3 movementDirection = pushableItem.Pose.Position.ToVector3() - LaraItem->Pose.Position.ToVector3();
					movementDirection.Normalize();
					LaraItem->Pose.Position = LaraItem->Pose.Position + movementDirection * BLOCK(1);
				}
				//Otherwise, Lara continues the pushing/pull animation movement.
				break;

			case PushableEnvironmentState::Air:
				// It's now in the air, Lara can't keep pushing nor pulling. And pushable starts to fall.
				LaraItem->Animation.TargetState = LS_IDLE;
				Lara.Context.InteractedItem = NO_ITEM;
				pushable.BehaviourState = PushableState::Fall;
				pushable.SoundState = PushableSoundState::None;

				return;
			break;

			case PushableEnvironmentState::Slope:
				//TODO: if it's a slope ground?...
				//Then proceed to the sliding state.
				break;

			case PushableEnvironmentState::Water:
				// It's still in water, but there is not ground, Lara can't keep pushing nor pulling. And pushable starts to sink.
				LaraItem->Animation.TargetState = LS_IDLE;
				Lara.Context.InteractedItem = NO_ITEM;
				pushable.BehaviourState = PushableState::Sink;
				pushable.SoundState = PushableSoundState::None;
				break;

			default:
				TENLog("Error detecting the pushable environment state during MOVING with pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
				break;
			}
		}
	}

	void HandleEdgeFallState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int floorHeight;
		auto envState = CheckPushableEnvironment(itemNumber, floorHeight);
		
		// Calculate movement direction
		auto moveDir = (pushableItem.Pose.Position - pushable.StartPos.ToVector3i()).ToVector3();
		moveDir.y = 0.0f;
		moveDir.Normalize();

		// Define origin and target.
		auto origin = pushable.StartPos.ToVector3() + (moveDir * 512);
		auto target = pushable.StartPos.ToVector3() + (moveDir * 1024);
		target.y = pushable.StartPos.y + 1024;

		// Calculate current position based on interpolation
		auto currentPos = pushableItem.Pose.Position.ToVector3();

		float& elapsedTime = pushableItem.Animation.Velocity.y;
		float alpha = std::clamp(elapsedTime / PUSHABLE_EDGE_FALL_VELOCITY, 0.0f, 1.0f);

		currentPos = Vector3(
			InterpolateCubic(origin.x, origin.x, target.x, target.x, alpha),
			InterpolateCubic(origin.y, origin.y, target.y - 700, target.y, alpha),
			InterpolateCubic(origin.z, origin.z, target.z, target.z, alpha));

		// Calculate the orientation angle based on the movement direction
		float leanAngleMax = 40.0f; // Maximum lean angle (60 degrees)
		float leanAngle = ANGLE (leanAngleMax * alpha); // Gradually increase the lean angle

		if (currentPos.y > floorHeight)
		{
			currentPos.y = floorHeight;
			pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);
		}
		else
		{
			// Manage angular leaning
			int pushableQuadrant = GetQuadrant(pushableItem.Pose.Orientation.y);
			int movementQuadrant = GetQuadrant(FROM_RAD(atan2(moveDir.z, moveDir.x)));
			
			movementQuadrant = (movementQuadrant + pushableQuadrant) % 4;

			switch (movementQuadrant)
			{
				// TODO: Use CardinalDirection enum.
				case 0: //EAST
					pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, leanAngle);
					break;
				case 1: //NORTH
					pushableItem.Pose.Orientation = EulerAngles(-leanAngle, pushableItem.Pose.Orientation.y, 0);
					break;
				case 2: //WEST
					pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, -leanAngle);
					break;
				case 3: //SOUTH
					pushableItem.Pose.Orientation = EulerAngles(leanAngle, pushableItem.Pose.Orientation.y, 0);
					break;
			}
		}

		pushableItem.Pose.Position = currentPos;
		elapsedTime += DELTA_TIME;

		// Manage Sound
		if (alpha <= 0.5f)
		{
			pushable.SoundState = PushableSoundState::Moving;
		}
		else
		{
			if (pushable.SoundState == PushableSoundState::Moving)
				pushable.SoundState = PushableSoundState::Stopping;
		}

		//Check if the movement is completed.
		if (alpha >= 1.0f)
		{
			currentPos = GetNearestSectorCenter(pushableItem.Pose.Position).ToVector3();

			switch (envState)
			{
			case PushableEnvironmentState::Air:
				pushable.BehaviourState = PushableState::Fall;
				pushableItem.Animation.Velocity.y = PUSHABLE_FALL_VELOCITY_MAX / 2;
			break;

			case PushableEnvironmentState::Ground:
			case PushableEnvironmentState::GroundWater:
				pushable.BehaviourState = PushableState::Idle;
				pushableItem.Pose.Position.y = floorHeight;
				pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);
				pushableItem.Animation.Velocity.y = 0.0f;
				break;

			case PushableEnvironmentState::Water:
				pushable.BehaviourState = PushableState::Sink;
				pushableItem.Animation.Velocity.y = PUSHABLE_WATER_VELOCITY_MAX / 2;
					
				// Effect: Water splash.
				DoPushableSplash(itemNumber);
				break;

			case PushableEnvironmentState::Slope:
				pushable.BehaviourState = PushableState::Idle;
				pushableItem.Animation.Velocity.y = 0.0f;
				break;

			default:
				TENLog("Error detecting the pushable environment state during the exit of MOVING EDGE state, with pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
				break;
			}			
		}
	}

	void HandleFallState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int floorHeight;
		auto envState = CheckPushableEnvironment(itemNumber, floorHeight);

		int FoundStack = NO_ITEM;

		switch (envState)
		{
			case PushableEnvironmentState::Air:
				//Is still falling...
				pushableItem.Pose.Position.y += pushableItem.Animation.Velocity.y;
				pushableItem.Animation.Velocity.y = std::min(	pushableItem.Animation.Velocity.y + pushable.Gravity,
																PUSHABLE_FALL_VELOCITY_MAX);

				//Fixing orientation slowly:
				PushableFallingOrientation(pushableItem);
			break;

			case PushableEnvironmentState::Ground:
			case PushableEnvironmentState::GroundWater:
				//The pushable is going to stop here, do the checks to conect it with another Stack.
				FoundStack = SearchNearPushablesStack(itemNumber);
				StackPushable(itemNumber, FoundStack);

				pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);

				//Set Stopper Flag
				if (pushable.StackLowerItem == NO_ITEM)
					SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

				//Activate trigger
				TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

				// Shake floor if pushable landed at high enough velocity.
				if (pushableItem.Animation.Velocity.y >= PUSHABLE_FALL_RUMBLE_VELOCITY)
				{
					FloorShake(&pushableItem);
					pushable.SoundState = PushableSoundState::Falling;
				}

				//place on ground
				pushable.BehaviourState = PushableState::Idle;
				pushableItem.Pose.Position.y = floorHeight;
				pushableItem.Animation.Velocity.y = 0;

				ActivateClimbablePushableCollider(itemNumber);
				break;

			case PushableEnvironmentState::Slope:
				pushable.BehaviourState = PushableState::Slide;
				pushableItem.Pose.Position.y = floorHeight;
				pushableItem.Animation.Velocity.y = 0;
				break;

			case PushableEnvironmentState::Water:

				pushable.BehaviourState = PushableState::Sink;

				// Effect: Water splash.
				DoPushableSplash(itemNumber);
				break;

			default:
				TENLog("Error detecting the pushable environment state during FALLING with pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
				break;
		}
	}

	void HandleSinkState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int floorHeight;
		PushableEnvironmentState envState = CheckPushableEnvironment(itemNumber, floorHeight);

		int FoundStack = NO_ITEM;

		switch (envState)
		{
			case PushableEnvironmentState::Air:
			case PushableEnvironmentState::Ground:
			case PushableEnvironmentState::Slope:
				//Set Stopper Flag
				if (pushable.StackLowerItem == NO_ITEM)
					SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

				pushable.BehaviourState = PushableState::Fall;
				pushable.Gravity = GRAVITY_AIR;

				break;

			case PushableEnvironmentState::Water:
			{
				//Manage gravity force
				if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
				{
					pushable.Gravity = pushable.Gravity - GRAVITY_ACCEL;
					if (pushable.Gravity <= 0.0f)
					{
						pushable.BehaviourState = PushableState::Float;
						return;
					}
				}
				else
				{
					pushable.Gravity = std::max(pushable.Gravity - GRAVITY_ACCEL, GRAVITY_WATER);
				}

				//Move Object
				pushableItem.Pose.Position.y += pushableItem.Animation.Velocity.y;
				pushableItem.Animation.Velocity.y = std::min(pushableItem.Animation.Velocity.y + pushable.Gravity,
					PUSHABLE_WATER_VELOCITY_MAX);
				//Fixing orientation slowly:
				PushableFallingOrientation(pushableItem);

				int waterheight = abs(floorHeight - pushable.WaterSurfaceHeight);
				if (waterheight > GetPushableHeight(pushableItem))
				{
					//Shallow Water
					// Effects: Spawn ripples.
					DoPushableRipples(itemNumber);
				}
				else
				{
					//Deep Water
					// Effects: Spawn bubbles.
					DoPushableBubbles(itemNumber);
				}
			}
			break;

			case PushableEnvironmentState::GroundWater:
				if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
				{
					pushableItem.Pose.Position.y = floorHeight;
					pushable.BehaviourState = PushableState::Float;
					pushable.Gravity = 0.0f;
				}
				else
				{
					pushableItem.Pose.Position.y = floorHeight;
					pushable.BehaviourState = PushableState::UnderwaterIdle;
					pushable.Gravity = GRAVITY_WATER;
					pushableItem.Animation.Velocity.y = 0.0f;
					ActivateClimbablePushableCollider(itemNumber);

					pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);

					//Activate trigger
					TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);
				}
			break;

			default:
				TENLog("Error detecting the pushable environment state during SINKING with pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
				break;

		}
	}

	void HandleFloatState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int floorHeight;
		int ceilingHeight;
		PushableEnvironmentState envState = CheckPushableEnvironment(itemNumber, floorHeight, &ceilingHeight);

		int goalHeight = 0;

		switch (envState)
		{
			case PushableEnvironmentState::Air:
			case PushableEnvironmentState::Ground:
			case PushableEnvironmentState::Slope:	

				//Set Stopper Flag
				if (pushable.StackLowerItem == NO_ITEM)
					SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

				pushable.BehaviourState = PushableState::Fall;
				pushable.Gravity = GRAVITY_AIR;

			break;

			case PushableEnvironmentState::Water:
			case PushableEnvironmentState::GroundWater:

				//Calculate goal height
				if (pushable.WaterSurfaceHeight == NO_HEIGHT)
				{
					//Check if there are space for the floating effect:
					if (abs(ceilingHeight - floorHeight) >= GetPushableHeight(pushableItem) + WATER_SURFACE_DISTANCE)
					{
						//If so, put pushable to float under the ceiling.
						goalHeight = ceilingHeight + WATER_SURFACE_DISTANCE + pushable.Height;
					}
					else
					{
						//Otherwise, the pushable is "blocking all the gap", so we won't move it.
						goalHeight = pushableItem.Pose.Position.y;
					}
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
					//Fixing orientation slowly:
					PushableFallingOrientation(pushableItem);
				}
				else
				{
					//Reached goal height
					pushableItem.Pose.Position.y = goalHeight;
					pushable.BehaviourState = PushableState::WaterSurfaceIdle;
					pushable.Gravity = GRAVITY_WATER;
					pushableItem.Animation.Velocity.y = 0.0f;
					pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);
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
		PushableEnvironmentState envState = CheckPushableEnvironment(itemNumber, floorHeight);

		switch (envState)
		{
			case PushableEnvironmentState::Ground:
			case PushableEnvironmentState::Slope:
			case PushableEnvironmentState::Air:
				//Set Stopper Flag
				if (pushable.StackLowerItem == NO_ITEM)
					SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

				pushableItem.Pose.Position.y = floorHeight;
				pushable.BehaviourState = PushableState::Idle;
				pushableItem.Animation.Velocity.y = 0.0f;
				pushable.Gravity = GRAVITY_AIR;	
				break;

			case PushableEnvironmentState::GroundWater:
			{
				//If shallow water, change to idle
				if (pushable.WaterSurfaceHeight != NO_HEIGHT)
				{
					int waterheight = abs(floorHeight - pushable.WaterSurfaceHeight);
					if (waterheight < GetPushableHeight(pushableItem))
					{
						pushableItem.Pose.Position.y = floorHeight;
						pushable.BehaviourState = PushableState::Idle;
						pushableItem.Animation.Velocity.y = 0.0f;
						pushable.Gravity = GRAVITY_AIR;
					}
				}
				else if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
				{
					pushable.BehaviourState = PushableState::Float;
					pushableItem.Animation.Velocity.y = 0.0f;
					pushable.Gravity = 0.0f;
				}

				//Otherwise, remain stuck to the floor.
				pushableItem.Pose.Position.y = floorHeight;
				int heightdifference = floorHeight - pushableItem.Pose.Position.y;
				VerticalPosAddition(itemNumber, heightdifference);
			}
			break;

			case PushableEnvironmentState::Water:
				
				if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
				{
					pushable.BehaviourState = PushableState::Float;
					pushableItem.Animation.Velocity.y = 0.0f;
					pushable.Gravity = 0.0f;
					return;
				}

				//Only pass to sinking if distance to noticeable. If is small, just stuck it to the ground.
				if (abs(floorHeight - pushableItem.Pose.Position.y) > CLICK(0.75f))
				{
					//Reset Stopper Flag
					if (pushable.StackLowerItem == NO_ITEM)
						SetPushableStopperFlag(false, pushableItem.Pose.Position, pushableItem.RoomNumber);
					
					pushable.BehaviourState = PushableState::Sink;
					pushableItem.Animation.Velocity.y = 0.0f;
					pushable.Gravity = 0.0f;
				}
				else
				{
					pushableItem.Pose.Position.y = floorHeight;
				}

			break;
		
			default:
				TENLog("Error detecting the pushable environment state during UNDERWATER IDLE with pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
				break;
		}
	}

	void HandleWaterSurfaceState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int floorHeight;
		int ceilingHeight;
		PushableEnvironmentState envState = CheckPushableEnvironment(itemNumber, floorHeight, &ceilingHeight);

		switch (envState)
		{
			case PushableEnvironmentState::Ground:
			case PushableEnvironmentState::Slope:
			case PushableEnvironmentState::Air:
				pushable.BehaviourState = PushableState::Fall;
				pushableItem.Animation.Velocity.y = 0.0f;
				pushable.Gravity = GRAVITY_AIR;
				pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);
				DeactivateClimbablePushableCollider(itemNumber);
				break;

			case PushableEnvironmentState::Water:

				// Effects: Do water ondulation effect.
				if (!pushable.UsesRoomCollision)
				{
					FloatingItem(pushableItem, pushable.FloatingForce);
				}
				else
				{
					FloatingBridge(pushableItem, pushable.FloatingForce);
				}

				// Effects: Spawn ripples.
				DoPushableRipples(itemNumber);
				break;

			case PushableEnvironmentState::GroundWater:
			{
				//If shallow water, change to idle
				int waterheight = abs(floorHeight - pushable.WaterSurfaceHeight);
				if (waterheight < GetPushableHeight(pushableItem))
				{
					pushable.BehaviourState = PushableState::Idle;
					pushable.Gravity = GRAVITY_AIR;
				}
				else
				{
					pushable.BehaviourState = PushableState::UnderwaterIdle;
					pushable.Gravity = GRAVITY_WATER;
				}
				pushableItem.Pose.Position.y = floorHeight;
				pushableItem.Animation.Velocity.y = 0.0f;
			}
			break;

			default:
				TENLog("Error detecting the pushable environment state during WATER SURFACE IDLE with pushable ID: " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
				break;
		}
	}

	void HandleSlideState(int itemNumber)
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

	void HandleMoveStackHorizontalState(int itemNumber)
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
