#include "framework.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Physics.h"

#include "Game/animation.h"
#include "Game/control/flipeffect.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Object/Pushables/PushableObject.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Scans.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"


using namespace TEN::Input;

namespace TEN::Entities::Generic
{
	std::unordered_map<PushablePhysicState, std::function<void(int)>> PUSHABLES_STATES_MAP;

	constexpr auto PUSHABLE_FALL_VELOCITY_MAX = BLOCK(1 / 8.0f);
	constexpr auto PUSHABLE_WATER_VELOCITY_MAX = BLOCK(1 / 16.0f);
	constexpr auto GRAVITY_AIR = 8.0f;
	constexpr auto GRAVITY_ACCEL = 0.5f;

	constexpr auto PUSHABLE_FALL_RUMBLE_VELOCITY = 96.0f;
	//constexpr auto PUSHABLE_HEIGHT_TOLERANCE = 32;

	constexpr auto WATER_SURFACE_DISTANCE = CLICK(0.5f);

	void InitializePushablesStatesMap()
	{
		static bool isInitialized = false;
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
		}
	}

	void HandleIdleState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//1. CHECK IF LARA IS INTERACTING WITH IT.
		if (Lara.Context.InteractedItem == itemNumber)
		{
			//If Lara is grabbing, check the push pull actions.
			if (LaraItem->Animation.ActiveState == LS_PUSHABLE_GRAB || TestLastFrame(LaraItem, LA_PUSHABLE_GRAB))
			{
				//First checks conditions.
				bool hasPushAction = IsHeld(In::Forward);
				bool hasPullAction = IsHeld(In::Back);

				//Cond 1: Is pressing Forward or Back?
				if (!hasPushAction && !hasPullAction)
					return;

				//Cond 2: Can do the interaction with that side of the pushable?
				int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);
				auto& pushableSidesAttributes = pushable.SidesMap[quadrant]; //0 North, 1 East, 2 South or 3 West.

				if ((hasPushAction && !pushableSidesAttributes.Pushable) ||
					(hasPullAction && !pushableSidesAttributes.Pullable))
					return;

				//Cond 3: Does it comply with the room collision conditions?.
				if (!PushableMovementConditions(itemNumber, hasPushAction, hasPullAction))
					return;

				//Then Do the interaction.
				if (hasPushAction)
				{
					int pushAnimNumber = PushableAnimInfos[pushable.AnimationSystemIndex].PushAnimNumber;
					SetAnimation(LaraItem, pushAnimNumber);
				}
				else if (hasPullAction)
				{
					int pullAnimNumber = PushableAnimInfos[pushable.AnimationSystemIndex].PullAnimNumber;
					SetAnimation(LaraItem, pullAnimNumber);
				}

				pushable.StartPos = pushableItem.Pose.Position;
				pushable.StartPos.RoomNumber = pushableItem.RoomNumber;
				pushable.BehaviourState = PushablePhysicState::Moving;
			}
			else if (	LaraItem->Animation.ActiveState != LS_PUSHABLE_GRAB &&
						LaraItem->Animation.ActiveState != LS_PUSHABLE_PULL &&
						LaraItem->Animation.ActiveState != LS_PUSHABLE_PUSH)
			{
				Lara.Context.InteractedItem = NO_ITEM;
			}

			return;
		}

		//2. CHECK IF IS IN WATER ROOM.
		if (TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
		{
			if (pushable.IsBuoyant)
			{
				pushable.BehaviourState = PushablePhysicState::Floating;
				pushable.Gravity = 0.0f;
			}
			else
			{
				pushable.BehaviourState = PushablePhysicState::Sinking;
				pushable.Gravity = 0.0f;
			}
			return;
		}

		//3. CHECK IF FLOOR HAS CHANGED.
		int floorHeight = GetCollision(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber).Position.Floor;
		if (floorHeight > pushableItem.Pose.Position.y)			//The floor has decresed. (Flip map, trapdoor, etc...)
		{
			//If the diffence is not very big, just teleport it.
			if (abs(pushableItem.Pose.Position.y - floorHeight) >= CLICK(1))
			{
				pushable.BehaviourState = PushablePhysicState::Falling;
			}
			else
			{
				pushableItem.Pose.Position.y = floorHeight;
			}
			
			return;
		}
		else if (floorHeight < pushableItem.Pose.Position.y)	//The floor has risen. (Elevator, raising block, etc...)
		{
			pushableItem.Pose.Position.y = floorHeight;
			return;
		}

		return;
	}

	void HandleMovingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		bool isPlayerPulling = (LaraItem->Animation.AnimNumber == LA_PUSHABLE_PULL);// || LaraItem->Animation.AnimNumber == LA_PUSHABLE_BLOCK_PULL);

		int quadrantDir = GetQuadrant(LaraItem->Pose.Orientation.y);
		int newPosX = pushable.StartPos.x;
		int newPosZ = pushable.StartPos.z;

		int displaceDepth = 0;
		int displaceBox = GameBoundingBox(LaraItem).Z2;

		if (pushable.CurrentSoundState == PushableSoundState::Moving)
			pushable.CurrentSoundState = PushableSoundState::Stopping;

		displaceDepth = GetLastFrame(GAME_OBJECT_ID::ID_LARA, LaraItem->Animation.AnimNumber)->BoundingBox.Z2;
		displaceBox -= isPlayerPulling ? (BLOCK(1) + displaceDepth) : (displaceDepth - BLOCK(1));

		//Lara is doing the pushing / pulling animation
		if (LaraItem->Animation.FrameNumber != g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd - 1)
		{
			//1. DECIDE GOAL POSITION.
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

			pushable.CurrentSoundState = PushableSoundState::Stopping;

			//2. MOVE PUSHABLES
			
			//Don't move the pushable if the distance is too big (it may happens because the animation bounding changes in the continue push pull process).
			if (abs(pushableItem.Pose.Position.z - newPosZ) > BLOCK(0.5f))
				return;
			if (abs(pushableItem.Pose.Position.x - newPosX) > BLOCK(0.5f))
				return;

			// move only if the move direction is oriented to the action
			// So pushing only moves pushable forward, and pulling only moves backwards

			//Z axis
			if (isPlayerPulling)
			{
				pushable.CurrentSoundState = PushableSoundState::Moving;
				if ((quadrantDir == NORTH && pushableItem.Pose.Position.z > newPosZ) ||
					(quadrantDir == SOUTH && pushableItem.Pose.Position.z < newPosZ))
				{
					pushableItem.Pose.Position.z = newPosZ;
				}
			}
			else
			{
				if ((quadrantDir == NORTH && pushableItem.Pose.Position.z < newPosZ) ||
					(quadrantDir == SOUTH && pushableItem.Pose.Position.z > newPosZ))
				{
					pushableItem.Pose.Position.z = newPosZ;
				}
			}

			//X axis
			pushable.CurrentSoundState = PushableSoundState::Moving;
			if (isPlayerPulling)
			{
				if ((quadrantDir == EAST && pushableItem.Pose.Position.x > newPosX) ||
					(quadrantDir == WEST && pushableItem.Pose.Position.x < newPosX))
				{
					pushableItem.Pose.Position.x = newPosX;
				}
			}
			else
			{
				if ((quadrantDir == EAST && pushableItem.Pose.Position.x < newPosX) ||
					(quadrantDir == WEST && pushableItem.Pose.Position.x > newPosX))
				{
					pushableItem.Pose.Position.x = newPosX;
				}
			}
		}
		else
		{
			//Pushing Pulling animation ended
			
			//1. REALIGN WITH SECTOR CENTER
			pushableItem.Pose.Position = GetNearestSectorCenter(pushableItem.Pose.Position);
			pushable.StartPos = pushableItem.Pose.Position;
			pushable.StartPos.RoomNumber = pushableItem.RoomNumber;

			//2. ACTIVATE TRIGGER
			TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);
			
			//3. CHECK FLOOR HEIGHT
			// Check if pushing pushable over edge. Then can't keep pushing/pulling and pushable start to fall.
			if (pushable.CanFall && !isPlayerPulling)
			{
				int floorHeight = GetCollision(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber).Position.Floor;
				if (floorHeight > pushableItem.Pose.Position.y)
				{
					LaraItem->Animation.TargetState = LS_IDLE;
					Lara.Context.InteractedItem = NO_ITEM;
					pushable.BehaviourState = PushablePhysicState::Falling;
					pushable.CurrentSoundState = PushableSoundState::None;

					return;
				}
			}

			//4. CHECK INPUT AND IF CAN KEEP THE MOVEMENT

			// Check the pushable animation system in use, if is using block animation which can't loop, go back to idle state.
			if (!PushableAnimInfos[pushable.AnimationSystemIndex].EnableAnimLoop)
			{
				pushable.BehaviourState = PushablePhysicState::Idle;
				return;
			}

			if (!IsHeld(In::Action) || !PushableMovementConditions(itemNumber, !isPlayerPulling, isPlayerPulling))
			{
				LaraItem->Animation.TargetState = LS_IDLE;
				pushable.BehaviourState = PushablePhysicState::Idle;
			}
		}

		return;
	}

	void HandleFallingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//1. PREPARE DATA, (floor height and velocities).
		auto pointColl = GetCollision(&pushableItem);

		float currentY = pushableItem.Pose.Position.y;
		float velocityY = pushableItem.Animation.Velocity.y;
		 
		//2. MOVE THE PUSHABLE DOWNWARDS
		// Move the pushable downwards if it hasn't reached the floor yet
		if (currentY < (pointColl.Position.Floor - velocityY))
		{
			float newVelocityY = velocityY + pushable.Gravity;
			pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_FALL_VELOCITY_MAX);

			pushableItem.Pose.Position.y = currentY + pushableItem.Animation.Velocity.y;
			
			//3. CHECK IF IS IN WATER ROOM
			if (TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
			{
				pushable.BehaviourState = PushablePhysicState::Sinking;

				// TODO: [Effects Requirement] Add Water splash.
			}
			return;
		}

		//Reached the ground
		//3. CHECK ENVIRONMENT
		
		//If it's a flat ground?
		
		//place on ground
		pushable.BehaviourState = PushablePhysicState::Idle;
		pushableItem.Pose.Position.y = pointColl.Position.Floor;
		pushableItem.Animation.Velocity.y = 0;

		// Shake floor if pushable landed at high enough velocity.
		if (velocityY >= PUSHABLE_FALL_RUMBLE_VELOCITY)
			FloorShake(&pushableItem);

		//TODO: [Effects Requirement] Is there low water? -> Spawn water splash

		//TODO: if it's a slope ground?...
		//Then proceed to the sliding state.
	}

	void HandleSinkingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//1. ENSURE IT'S IN WATER ROOM.
		if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
		{
			pushable.BehaviourState = PushablePhysicState::Falling;
			pushable.Gravity = GRAVITY_AIR;
			return;
		}

		//2. PREPARE DATA, (floor height and velocities).
		auto pointColl = GetCollision(&pushableItem);

		float currentY = pushableItem.Pose.Position.y;
		float velocityY = pushableItem.Animation.Velocity.y;

		// TODO: [Effects Requirement] Add bubbles during this phase.

		//3. MANAGE GRAVITY FORCE
		if (pushable.IsBuoyant)
		{
			// Slowly reverses gravity direction. If gravity is 0, then it floats.
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
			pushable.Gravity = std::max(pushable.Gravity - GRAVITY_ACCEL, 4.0f);
		}

		//4. MOVE OBJECT
		if (currentY < pointColl.Position.Floor - velocityY)
		{
			// Sinking down.
			float newVelocityY = velocityY + pushable.Gravity;
			pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_WATER_VELOCITY_MAX);

			pushableItem.Pose.Position.y = currentY + pushableItem.Animation.Velocity.y;

			return;
		}

		// 5. HIT GROUND
		if (pushable.IsBuoyant)
		{
			pushable.Gravity = 0.0f;
			pushable.BehaviourState = PushablePhysicState::Floating;
		}
		else
		{
			pushable.BehaviourState = PushablePhysicState::UnderwaterIdle;
			pushableItem.Pose.Position.y = pointColl.Position.Floor;
		}

		pushableItem.Animation.Velocity.y = 0.0f;

	}

	void HandleFloatingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//1. ENSURE IT'S IN WATER ROOM.
		if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
		{
			pushable.BehaviourState = PushablePhysicState::Falling;
			pushable.Gravity = GRAVITY_AIR;
			return;
		}

		//2. PREPARE DATA, (goal height and velocities).
		auto pointColl = GetCollision(&pushableItem);

		int goalHeight = 0;

		int waterDepth = GetWaterSurface(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber);
		if (waterDepth != NO_HEIGHT)
			goalHeight = waterDepth - WATER_SURFACE_DISTANCE + pushable.Height;
		else
			goalHeight = pointColl.Position.Ceiling + WATER_SURFACE_DISTANCE + pushable.Height;

		float currentY = pushableItem.Pose.Position.y;
		float velocityY = pushableItem.Animation.Velocity.y;

		pushable.Gravity = std::max(pushable.Gravity - GRAVITY_ACCEL, -4.0f);

		//3. MOVE PUSHABLE UPWARDS
		if (currentY > goalHeight)
		{
			// Floating up.
			float newVelocityY = velocityY + pushable.Gravity;
			pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_WATER_VELOCITY_MAX);

			// Update pushable's position and move its stack.
			pushableItem.Pose.Position.y = currentY + pushableItem.Animation.Velocity.y;
			return;
		}

		// Reached water surface.
		pushable.BehaviourState = PushablePhysicState::WatersurfaceIdle;
		pushableItem.Pose.Position.y = goalHeight;

		pushableItem.Animation.Velocity.y = 0.0f;
	}

	void HandleUnderwaterState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//1. ENSURE IT'S IN WATER ROOM.
		if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
		{
			pushable.BehaviourState = PushablePhysicState::Idle;
			pushable.Gravity = GRAVITY_AIR;
			return;
		}

		//2. IF IT'S BUOYANT, NEEDS TO FLOAT. (Maybe was on the floor and happened a flipmap that filled the room with water).
		if (pushable.IsBuoyant)
		{
			pushable.Gravity = 0.0f;
			pushable.BehaviourState = PushablePhysicState::Floating;
		}

	}

	void HandleWatersurfaceState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//1. ENSURE IT'S IN WATER ROOM.
		if (!TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
		{
			pushable.BehaviourState = PushablePhysicState::Falling;
			pushable.Gravity = GRAVITY_AIR;
			pushableItem.Pose.Orientation = EulerAngles(0, pushableItem.Pose.Orientation.y, 0);
			return;
		}

		//2. DO WATER ONDULATION EFFECT.
		FloatItem(pushableItem, pushable.FloatingForce);

		// TODO: [Effects Requirement] Spawn ripples.

	}

	void HandleSlidingState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//PENDING
		//1. DETECT FLOOR, (Slope orientation)
		//2. CALCULATE DIRECTION AND SPEED
		//3. MOVE OBJECT
		//4. DETECT END CONDITIONS OF NEXT SECTOR 
			//	Is a slope-> keep sliding
			//	Is a flat floor -> so end slide
			//	Is a fall -> so pass to falling
			//	Is a forbiden Sector -> freeze the slide
		//5. Incorporate effects? Smoke or sparkles?

	}

}
