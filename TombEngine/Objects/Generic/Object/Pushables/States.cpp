#include "framework.h"
#include "Objects/Generic/Object/Pushables/States.h"

#include "Game/animation.h"
#include "Game/control/flipeffect.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Object/Pushables/PushableBridge.h"
#include "Objects/Generic/Object/Pushables/PushableObject.h"
#include "Objects/Generic/Object/Pushables/PushableEffects.h"
#include "Objects/Generic/Object/Pushables/Context.h"
#include "Objects/Generic/Object/Pushables/Stack.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Input;

namespace TEN::Entities::Generic
{
	constexpr auto PUSHABLE_FALL_VELOCITY_MAX  = BLOCK(1 / 8.0f);
	constexpr auto PUSHABLE_WATER_VELOCITY_MAX = BLOCK(1 / 16.0f);

	constexpr auto PUSHABLE_RUMBLE_FALL_VELOCITY	 = 96.0f;
	constexpr auto PUSHABLE_EDGE_SLIP_FALL_VELOCITY	 = 0.8f;

	constexpr auto PUSHABLE_GRAVITY_AIRBORNE = 8.0f;
	constexpr auto PUSHABLE_GRAVITY_WATER	 = 4.0f;
	constexpr auto PUSHABLE_GRAVITY_ACCEL	 = 0.5f;

	constexpr auto PUSHABLE_WATER_SURFACE_DISTANCE = CLICK(0.5f);

	// TODO: Do it this way.
	std::unordered_map<PushableState, std::function<void(int)>> PUSHABLE_STATE_MAP;/* =
	{
		{ PushableState::Idle, HandleIdleState },
		{ PushableState::EdgeSlip, HandleEdgeSlipState },
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
		if (PUSHABLE_STATE_MAP.empty())
		{
			PUSHABLE_STATE_MAP.clear();

			PUSHABLE_STATE_MAP.emplace(PushableState::Idle, &HandleIdleState);
			PUSHABLE_STATE_MAP.emplace(PushableState::Move, &HandleMoveState);
			PUSHABLE_STATE_MAP.emplace(PushableState::EdgeSlip, &HandleEdgeSlipState);
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
		auto& playerItem = *LaraItem;
		auto& player = GetLaraInfo(playerItem);

		// 1) Check if player is interacting.
		if (Lara.Context.InteractedItem == itemNumber)
		{
			if (PushableIdleConditions(itemNumber))
			{
				// Pushing.
				if (IsHeld(In::Forward))
				{
					int pushAnimNumber = (pushable.IsOnEdge) ? 
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

				// Unstack lower pushables.
				UnstackPushable(itemNumber);

				// Prepare upper pushables in stack for movement.
				StartMovePushableStack(itemNumber);

				ResetPlayerFlex(LaraItem);
				RemovePushableBridge(itemNumber);
			}
			else if (playerItem.Animation.ActiveState != LS_PUSHABLE_GRAB &&
				playerItem.Animation.ActiveState != LS_PUSHABLE_PULL &&
				playerItem.Animation.ActiveState != LS_PUSHABLE_PUSH &&
				playerItem.Animation.ActiveState != LS_PUSHABLE_EDGE_SLIP)
			{
				player.Context.InteractedItem = NO_ITEM;
			}
		}

		// 2) Check environment.
		int floorHeight = 0;
		auto envState = GetPushableEnvironmentType(itemNumber, floorHeight);

		switch (envState)
		{	
		case PushableEnvironmentType::FlatFloor:
			if (floorHeight != pushableItem.Pose.Position.y)
			{
				pushableItem.Pose.Position.y = floorHeight;
				int heightdifference = floorHeight - pushableItem.Pose.Position.y;
				VerticalPosAddition(itemNumber, heightdifference);
			}

			break;

		case PushableEnvironmentType::ShallowWater:
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

		case PushableEnvironmentType::Air:

			// Only pass to falling if distance to bigger than 1 click. If is small, just stuck it to the ground.
			if (abs(floorHeight - pushableItem.Pose.Position.y) > CLICK(0.75f))
			{
				pushable.BehaviourState = PushableState::Fall;
				SetPushableStopperFlag(false, pushableItem.Pose.Position, pushableItem.RoomNumber);
				RemovePushableBridge(itemNumber);
			}
			else
			{
				pushableItem.Pose.Position.y = floorHeight;
				int heightdifference = floorHeight - pushableItem.Pose.Position.y;
				VerticalPosAddition(itemNumber, heightdifference);
			}

			break;

		case PushableEnvironmentType::DeepWater:
			RemovePushableBridge(itemNumber);
			SetPushableStopperFlag(false, pushableItem.Pose.Position, pushableItem.RoomNumber);

			if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
			{
				pushable.BehaviourState = PushableState::Float;
				pushable.Gravity = 0.0f;
			}
			else
			{
				pushable.BehaviourState = PushableState::Sink;
				pushable.Gravity = PUSHABLE_GRAVITY_WATER;
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

		int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);
		int newPosX = pushable.StartPos.x;
		int newPosZ = pushable.StartPos.z;

		int displaceDepth = 0;
		int displaceBox = GameBoundingBox(LaraItem).Z2;

		if (pushable.SoundState == PushableSoundState::Move)
			pushable.SoundState = PushableSoundState::Stop;

		displaceDepth = GetLastFrame(GAME_OBJECT_ID::ID_LARA, LaraItem->Animation.AnimNumber)->BoundingBox.Z2;
		
		if (isPlayerPulling)
		{
			displaceBox -= (displaceDepth + BLOCK(1));
		}
		else
		{
			if (pushable.IsOnEdge)
			{
				displaceBox -= (displaceDepth - BLOCK(0.5f));
			}
			else
			{
				displaceBox -= (displaceDepth - BLOCK(1));
			}
		}

		// Player is pushing or pulling.
		if (LaraItem->Animation.FrameNumber != g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd - 1)
		{
			// 1) Determine displacement.
			switch (quadrant)
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

			// 2) Move pushable
			
			// Don't pushable if distance is too far (may happen because animation bound change).
			if (abs(pushableItem.Pose.Position.z - newPosZ) > BLOCK(0.75f))
				return;

			if (abs(pushableItem.Pose.Position.x - newPosX) > BLOCK(0.75f))
				return;

			int travelledDist = Vector3i::Distance(pushableItem.Pose.Position, pushable.StartPos.ToVector3i());
			if (pushable.IsOnEdge && travelledDist >= BLOCK(0.5f))
			{
				pushable.BehaviourState = PushableState::EdgeSlip;
				return;
			}
			
			// move only if the move direction is oriented to the action
			// So pushing only moves pushable forward, and pulling only moves backwards

			//Z axis
			if (isPlayerPulling)
			{
				if ((quadrant == NORTH && pushableItem.Pose.Position.z > newPosZ) ||
					(quadrant == SOUTH && pushableItem.Pose.Position.z < newPosZ))
				{
					pushableItem.Pose.Position.z = newPosZ;
					pushable.SoundState = PushableSoundState::Move;
				}
			}
			else
			{
				if ((quadrant == NORTH && pushableItem.Pose.Position.z < newPosZ) ||
					(quadrant == SOUTH && pushableItem.Pose.Position.z > newPosZ))
				{
					pushableItem.Pose.Position.z = newPosZ;
					pushable.SoundState = PushableSoundState::Move;
				}
			}

			//X axis
			if (isPlayerPulling)
			{
				if ((quadrant == EAST && pushableItem.Pose.Position.x > newPosX) ||
					(quadrant == WEST && pushableItem.Pose.Position.x < newPosX))
				{
					pushableItem.Pose.Position.x = newPosX;
					pushable.SoundState = PushableSoundState::Move;
				}
			}
			else
			{
				if ((quadrant == EAST && pushableItem.Pose.Position.x < newPosX) ||
					(quadrant == WEST && pushableItem.Pose.Position.x > newPosX))
				{
					pushableItem.Pose.Position.x = newPosX;
					pushable.SoundState = PushableSoundState::Move;
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
			PushableEnvironmentType envState = GetPushableEnvironmentType(itemNumber, floorHeight);

			switch (envState)
			{
			case PushableEnvironmentType::FlatFloor:
			case PushableEnvironmentType::ShallowWater:
				//Activate trigger
				TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

				// Check if it has to stop the pushing/pulling movement.
				if (!PushableAnimInfos[pushable.AnimationSystemIndex].EnableAnimLoop ||
					!IsHeld(In::Action) ||
					!PushableMovementConditions(itemNumber, !isPlayerPulling, isPlayerPulling) ||
					!IsPushableValid(itemNumber))
				{
					LaraItem->Animation.TargetState = LS_IDLE;
					pushable.BehaviourState = PushableState::Idle;
					StopMovePushableStack(itemNumber); //Set the upper pushables back to normal.
					AddPushableBridge(itemNumber);

					//The pushable is going to stop here, do the checks to conect it with another Stack.
					int FoundStack = SearchNearPushablesStack(itemNumber);
					StackPushable(itemNumber, FoundStack);

					pushable.SoundState = PushableSoundState::Stop;

				}
				else if (LaraItem->Animation.ActiveState == LS_PUSHABLE_PUSH && pushable.IsOnEdge)
				{
					LaraItem->Animation.TargetState = LS_PUSHABLE_EDGE_SLIP;

					Vector3 movementDirection = pushableItem.Pose.Position.ToVector3() - LaraItem->Pose.Position.ToVector3();
					movementDirection.Normalize();
					LaraItem->Pose.Position = LaraItem->Pose.Position + movementDirection * BLOCK(1);
				}
				//Otherwise, player continues the pushing/pull animation movement.
				break;

			case PushableEnvironmentType::Air:
				// It's now in the air, player can't keep pushing nor pulling. And pushable starts to fall.
				LaraItem->Animation.TargetState = LS_IDLE;
				Lara.Context.InteractedItem = NO_ITEM;
				pushable.BehaviourState = PushableState::Fall;
				pushable.SoundState = PushableSoundState::None;

				return;
			break;

			case PushableEnvironmentType::SlopedFloor:
				//TODO: if it's a slope ground?...
				//Then proceed to the sliding state.
				break;

			case PushableEnvironmentType::DeepWater:
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

	void HandleEdgeSlipState(int itemNumber)
	{
		constexpr auto LEAN_ANGLE_MAX = ANGLE(40.0f);

		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int floorHeight;
		auto envState = GetPushableEnvironmentType(itemNumber, floorHeight);
		
		// Calculate movement direction
		auto moveDir = (pushableItem.Pose.Position - pushable.StartPos.ToVector3i()).ToVector3();
		moveDir.y = 0.0f;
		moveDir.Normalize();

		// Define origin and target.
		auto origin = pushable.StartPos.ToVector3() + (moveDir * BLOCK(0.5f));
		auto target = pushable.StartPos.ToVector3() + (moveDir * BLOCK(1));
		target.y = pushable.StartPos.y + BLOCK(1);

		// Calculate current position based on interpolation
		auto currentPos = pushableItem.Pose.Position.ToVector3();

		float& elapsedTime = pushableItem.Animation.Velocity.y;
		float alpha = std::clamp(elapsedTime / PUSHABLE_EDGE_SLIP_FALL_VELOCITY, 0.0f, 1.0f);

		currentPos = Vector3(
			InterpolateCubic(origin.x, origin.x, target.x, target.x, alpha),
			InterpolateCubic(origin.y, origin.y, target.y - 700, target.y, alpha),
			InterpolateCubic(origin.z, origin.z, target.z, target.z, alpha));

		// Calculate lean angle based on movement direction.
		float leanAngle = LEAN_ANGLE_MAX * alpha;

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

		// Handle sounds.
		if (alpha <= 0.5f)
		{
			pushable.SoundState = PushableSoundState::Move;
		}
		else
		{
			if (pushable.SoundState == PushableSoundState::Move)
				pushable.SoundState = PushableSoundState::Stop;
		}

		// Check if movement is completed.
		if (alpha >= 1.0f)
		{
			currentPos = GetNearestSectorCenter(pushableItem.Pose.Position).ToVector3();

			switch (envState)
			{
			case PushableEnvironmentType::Air:
				pushable.BehaviourState = PushableState::Fall;
				pushableItem.Animation.Velocity.y = PUSHABLE_FALL_VELOCITY_MAX / 2;
				break;

			case PushableEnvironmentType::FlatFloor:
			case PushableEnvironmentType::ShallowWater:
				pushable.BehaviourState = PushableState::Idle;
				pushableItem.Pose.Position.y = floorHeight;
				pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);
				pushableItem.Animation.Velocity.y = 0.0f;
				break;

			case PushableEnvironmentType::DeepWater:
				pushable.BehaviourState = PushableState::Sink;
				pushableItem.Animation.Velocity.y = PUSHABLE_WATER_VELOCITY_MAX / 2;
					
				DoPushableSplash(itemNumber);
				break;

			case PushableEnvironmentType::SlopedFloor:
				pushable.BehaviourState = PushableState::Idle;
				pushableItem.Animation.Velocity.y = 0.0f;
				break;

			default:
				TENLog("Error handling pushable environment type during edge slip state for pushable item number " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All);
				break;
			}			
		}
	}

	void HandleFallState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int floorHeight;
		auto envState = GetPushableEnvironmentType(itemNumber, floorHeight);

		int FoundStack = NO_ITEM;

		switch (envState)
		{
			case PushableEnvironmentType::Air:
				//Is still falling.
				pushableItem.Pose.Position.y += pushableItem.Animation.Velocity.y;
				pushableItem.Animation.Velocity.y = std::min(pushableItem.Animation.Velocity.y + pushable.Gravity, PUSHABLE_FALL_VELOCITY_MAX);

				//Fixing orientation slowly:
				PushableFallingOrientation(pushableItem);
				break;

			case PushableEnvironmentType::FlatFloor:
			case PushableEnvironmentType::ShallowWater:
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
				if (pushableItem.Animation.Velocity.y >= PUSHABLE_RUMBLE_FALL_VELOCITY)
				{
					FloorShake(&pushableItem);
					pushable.SoundState = PushableSoundState::Fall;
				}

				//place on ground
				pushable.BehaviourState = PushableState::Idle;
				pushableItem.Pose.Position.y = floorHeight;
				pushableItem.Animation.Velocity.y = 0.0f;

				AddPushableBridge(itemNumber);
				break;

			case PushableEnvironmentType::SlopedFloor:
				pushable.BehaviourState = PushableState::Slide;
				pushableItem.Pose.Position.y = floorHeight;
				pushableItem.Animation.Velocity.y = 0.0f;
				break;

			case PushableEnvironmentType::DeepWater:

				pushable.BehaviourState = PushableState::Sink;

				// Effect: Water splash.
				DoPushableSplash(itemNumber);
				break;

			default:
				TENLog("Error detecting pushable environment state during FALLING for pushable with item number  " + std::to_string(itemNumber), LogLevel::Warning, LogConfig::All, false);
				break;
			}
	}

	void HandleSinkState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int floorHeight = 0;
		auto envState = GetPushableEnvironmentType(itemNumber, floorHeight);

		int FoundStack = NO_ITEM;

		switch (envState)
		{
			case PushableEnvironmentType::Air:
			case PushableEnvironmentType::FlatFloor:
			case PushableEnvironmentType::SlopedFloor:
				//Set Stopper Flag
				if (pushable.StackLowerItem == NO_ITEM)
					SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

				pushable.BehaviourState = PushableState::Fall;
				pushable.Gravity = PUSHABLE_GRAVITY_AIRBORNE;

				break;

			case PushableEnvironmentType::DeepWater:
			{
				//Manage gravity force
				if (pushable.IsBuoyant && pushable.StackUpperItem == NO_ITEM)
				{
					pushable.Gravity = pushable.Gravity - PUSHABLE_GRAVITY_ACCEL;
					if (pushable.Gravity <= 0.0f)
					{
						pushable.BehaviourState = PushableState::Float;
						return;
					}
				}
				else
				{
					pushable.Gravity = std::max(pushable.Gravity - PUSHABLE_GRAVITY_ACCEL, PUSHABLE_GRAVITY_WATER);
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

			case PushableEnvironmentType::ShallowWater:
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
					pushable.Gravity = PUSHABLE_GRAVITY_WATER;
					pushableItem.Animation.Velocity.y = 0.0f;
					AddPushableBridge(itemNumber);

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
		PushableEnvironmentType envState = GetPushableEnvironmentType(itemNumber, floorHeight, &ceilingHeight);

		int goalHeight = 0;

		switch (envState)
		{
			case PushableEnvironmentType::Air:
			case PushableEnvironmentType::FlatFloor:
			case PushableEnvironmentType::SlopedFloor:	

				//Set Stopper Flag
				if (pushable.StackLowerItem == NO_ITEM)
					SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

				pushable.BehaviourState = PushableState::Fall;
				pushable.Gravity = PUSHABLE_GRAVITY_AIRBORNE;

			break;

			case PushableEnvironmentType::DeepWater:
			case PushableEnvironmentType::ShallowWater:

				//Calculate goal height
				if (pushable.WaterSurfaceHeight == NO_HEIGHT)
				{
					//Check if there are space for the floating effect:
					if (abs(ceilingHeight - floorHeight) >= GetPushableHeight(pushableItem) + PUSHABLE_WATER_SURFACE_DISTANCE)
					{
						//If so, put pushable to float under the ceiling.
						goalHeight = ceilingHeight + PUSHABLE_WATER_SURFACE_DISTANCE + pushable.Height;
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
					goalHeight = pushable.WaterSurfaceHeight - PUSHABLE_WATER_SURFACE_DISTANCE + pushable.Height;
				}

				//Manage gravity force
				pushable.Gravity = std::max(pushable.Gravity - PUSHABLE_GRAVITY_ACCEL, -PUSHABLE_GRAVITY_WATER);

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
					pushable.Gravity = PUSHABLE_GRAVITY_WATER;
					pushableItem.Animation.Velocity.y = 0.0f;
					pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);
					AddPushableBridge(itemNumber);
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
		PushableEnvironmentType envState = GetPushableEnvironmentType(itemNumber, floorHeight);

		switch (envState)
		{
			case PushableEnvironmentType::FlatFloor:
			case PushableEnvironmentType::SlopedFloor:
			case PushableEnvironmentType::Air:
				//Set Stopper Flag
				if (pushable.StackLowerItem == NO_ITEM)
					SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

				pushableItem.Pose.Position.y = floorHeight;
				pushable.BehaviourState = PushableState::Idle;
				pushableItem.Animation.Velocity.y = 0.0f;
				pushable.Gravity = PUSHABLE_GRAVITY_AIRBORNE;	
				break;

			case PushableEnvironmentType::ShallowWater:
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
						pushable.Gravity = PUSHABLE_GRAVITY_AIRBORNE;
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

			case PushableEnvironmentType::DeepWater:
				
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
		PushableEnvironmentType envState = GetPushableEnvironmentType(itemNumber, floorHeight, &ceilingHeight);

		switch (envState)
		{
			case PushableEnvironmentType::FlatFloor:
			case PushableEnvironmentType::SlopedFloor:
			case PushableEnvironmentType::Air:
				pushable.BehaviourState = PushableState::Fall;
				pushableItem.Animation.Velocity.y = 0.0f;
				pushable.Gravity = PUSHABLE_GRAVITY_AIRBORNE;
				pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);
				RemovePushableBridge(itemNumber);
				break;

			case PushableEnvironmentType::DeepWater:

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

			case PushableEnvironmentType::ShallowWater:
			{
				//If shallow water, change to idle
				int waterheight = abs(floorHeight - pushable.WaterSurfaceHeight);
				if (waterheight < GetPushableHeight(pushableItem))
				{
					pushable.BehaviourState = PushableState::Idle;
					pushable.Gravity = PUSHABLE_GRAVITY_AIRBORNE;
				}
				else
				{
					pushable.BehaviourState = PushableState::UnderwaterIdle;
					pushable.Gravity = PUSHABLE_GRAVITY_WATER;
				}
				pushableItem.Pose.Position.y = floorHeight;
				pushableItem.Animation.Velocity.y = 0.0f;
				pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);
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
