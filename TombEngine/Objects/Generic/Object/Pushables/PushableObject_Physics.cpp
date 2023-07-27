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
	std::unordered_map < pushableObjects_Physics::PushablePhysicState, std::function <void(int)>> pushableObjects_Physics::PUSHABLES_STATES_MAP;
	
	// Call this function to initialize the state functions handlers
	void pushableObjects_Physics::InitializeStateHandlers()
	{
		PUSHABLES_STATES_MAP[PushablePhysicState::Idle] = std::bind(&pushableObjects_Physics::HandleIdleState, this, std::placeholders::_1);
		PUSHABLES_STATES_MAP[PushablePhysicState::Moving] = std::bind(&pushableObjects_Physics::HandleMovingState, this, std::placeholders::_1);
		PUSHABLES_STATES_MAP[PushablePhysicState::Falling] = std::bind(&pushableObjects_Physics::HandleFallingState, this, std::placeholders::_1);
		PUSHABLES_STATES_MAP[PushablePhysicState::Sinking] = std::bind(&pushableObjects_Physics::HandleSinkingState, this, std::placeholders::_1);
		PUSHABLES_STATES_MAP[PushablePhysicState::Floating] = std::bind(&pushableObjects_Physics::HandleFloatingState, this, std::placeholders::_1);
		PUSHABLES_STATES_MAP[PushablePhysicState::OnWater] = std::bind(&pushableObjects_Physics::HandleOnWaterState, this, std::placeholders::_1);
		PUSHABLES_STATES_MAP[PushablePhysicState::Sliding] = std::bind(&pushableObjects_Physics::HandleSlidingState, this, std::placeholders::_1);
	}

	void pushableObjects_Physics::HandleIdleState(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

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
					int pushAnim = PushableAnimInfos[pushable.AnimationSystemIndex].PushAnimNumber;
					SetAnimation(LaraItem, pushAnim);
				}
				else if (hasPullAction)
				{
					int pullAnim = PushableAnimInfos[pushable.AnimationSystemIndex].PullAnimNumber;
					SetAnimation(LaraItem, pullAnim);
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

		/*if (Lara.Context.InteractedItem == 10)
		{
			//Happy
		}
		
		int pullAnimNumber = PushableAnimInfos[pushable.AnimationSystemIndex].PullAnimNumber;
		int pushAnimNumber = PushableAnimInfos[pushable.AnimationSystemIndex].PushAnimNumber;

		if (LaraItem->Animation.AnimNumber == pullAnimNumber || LaraItem->Animation.AnimNumber == pushAnimNumber)
		{
			pushable.BehaviourState = PushablePhysicState::None;
			//PushableBlockManageMoving(itemNumber);
		}
		else if (LaraItem->Animation.ActiveState == LS_IDLE)
		{
			// Do last actions and deactivate (reactivated in collision function).
			//PushableBlockManageIdle(itemNumber);
		}
		*/
	}

	void pushableObjects_Physics::HandleMovingState(int itemNumber)
	{

	}

	void pushableObjects_Physics::HandleFallingState(int itemNumber)
	{
		// Your code for handling the Falling state goes here
	}

	void pushableObjects_Physics::HandleSinkingState(int itemNumber)
	{
		// Your code for handling the Sinking state goes here
	}

	void pushableObjects_Physics::HandleFloatingState(int itemNumber)
	{
		// Your code for handling the Floating state goes here
	}

	void pushableObjects_Physics::HandleOnWaterState(int itemNumber)
	{
		// Your code for handling the OnWater state goes here
	}

	void pushableObjects_Physics::HandleSlidingState(int itemNumber)
	{
		// Your code for handling the Sliding state goes here
	}

}
