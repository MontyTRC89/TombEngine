#include "framework.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Physics.h"

#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Object/Pushables/PushableObject.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"


using namespace TEN::Input;

namespace TEN::Entities::Generic
{
	std::unordered_map<PushablePhysicState, std::function<void(int)>> PUSHABLES_STATES_MAP;

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
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Underwater, &HandleUnderwaterState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Watersurface, &HandleWatersurfaceState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Sliding, &HandleSlidingState);
		}
	}

	void HandleIdleState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//TENLog("STATE: IDLE for " + std::to_string(static_cast<int>(itemNumber)), LogLevel::Error, LogConfig::All, true);

		//1. CHECK IF LARA IS INTERACTING WITH IT.
		if (Lara.Context.InteractedItem == itemNumber)
		{
			//If Lara is grabbing, check the push pull actions.
			if (LaraItem->Animation.ActiveState == LS_PUSHABLE_GRAB || TestLastFrame(LaraItem, LA_PUSHABLE_GRAB))
			{
				bool hasPushAction = IsHeld(In::Forward);
				bool hasPullAction = IsHeld(In::Back);

				if (!hasPushAction && !hasPullAction)
					return;

				/* This segment should go to the scan module.
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
					*/

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
		bool isUnderwater = false; // IsUnderwaterRoom(pushableItem.RoomNumber); // Implement this function in scan to check if the room is underwater.
		if (isUnderwater)
		{
			if (pushable.IsBuoyant)
			{
				pushable.BehaviourState = PushablePhysicState::Sinking; 
			}
			else
			{
				pushable.BehaviourState = PushablePhysicState::Floating;
			}
			return;
		}

		//3. CHECK IF FLOOR HAS CHANGED.
		/*int currentFloorHeight = 0; // GetFloorHeight(pushableItem);	// Implement this function to get the floor height at the current position of the pushable item.
		if (currentFloorHeight > pushableItem.Pose.Position.y)			//The floor has decresed. (Flip map, trapdoor, etc...)
		{
			//Maybe add an extra, if the diffence is not very big, just teleport it.
			pushable.BehaviourState = PushablePhysicState::Falling; 
			return;
		}
		else if (currentFloorHeight < pushableItem.Pose.Position.y)		//The floor has risen. (Elevator, raising block, etc...)
		{
			pushableItem.Pose.Position.y = currentFloorHeight;
			return;
		}*/

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

			//PENDING, GET NEW TARGET POSITION TO SCAN IF IT'S ALLOWED.
			/*// Otherwise, just check if action key is still pressed.
			auto nextPos = GameVector(pushableItem.Pose.Position, pushableItem.RoomNumber);

			// Rotate orientation 180 degrees.
			if (isPlayerPulling)
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
			}*/

			if (!IsHeld(In::Action)) //&& IsNextSectorValid(pushableItem, nextPos, isPlayerPulling))
			{
				LaraItem->Animation.TargetState = LS_IDLE;
				pushable.BehaviourState = PushablePhysicState::Idle;
			}
		}

		return;
	}

	void HandleFallingState(int itemNumber)
	{

	}

	void HandleSinkingState(int itemNumber)
	{

	}

	void HandleFloatingState(int itemNumber)
	{

	}

	void HandleUnderwaterState(int itemNumber)
	{

	}

	void HandleWatersurfaceState(int itemNumber)
	{

	}

	void HandleSlidingState(int itemNumber)
	{

	}

}
