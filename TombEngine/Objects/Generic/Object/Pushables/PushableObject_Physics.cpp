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
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::OnWater, &HandleOnWaterState);
			PUSHABLES_STATES_MAP.emplace(PushablePhysicState::Sliding, &HandleSlidingState);
		}
	}

	void HandleIdleState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		TENLog("STATE: IDLE for " + std::to_string(static_cast<int>(itemNumber)), LogLevel::Error, LogConfig::All, true);

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

				pushable.BehaviourState = PushablePhysicState::Moving;
			}
			else if (	LaraItem->Animation.ActiveState != LS_PUSHABLE_GRAB &&
						LaraItem->Animation.ActiveState != LS_PUSHABLE_PULL &&
						LaraItem->Animation.ActiveState != LS_PUSHABLE_PUSH)
			{
				Lara.Context.InteractedItem = NO_ITEM;
			}
		}
	}

	void HandleMovingState(int itemNumber)
	{
		TENLog("STATE: MOVING for " + std::to_string(static_cast<int>(itemNumber)), LogLevel::Error, LogConfig::All, true);
	}

	void HandleFallingState(int itemNumber)
	{
		// Your code for handling the Falling state goes here
	}

	void HandleSinkingState(int itemNumber)
	{
		// Your code for handling the Sinking state goes here
	}

	void HandleFloatingState(int itemNumber)
	{
		// Your code for handling the Floating state goes here
	}

	void HandleOnWaterState(int itemNumber)
	{
		// Your code for handling the OnWater state goes here
	}

	void HandleSlidingState(int itemNumber)
	{
		// Your code for handling the Sliding state goes here
	}

}
