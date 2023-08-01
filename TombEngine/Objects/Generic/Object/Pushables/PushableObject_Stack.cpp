#include "framework.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Stack.h"

#include "Game/collision/floordata.h"
#include "Game/Setup.h"
#include "Objects/Generic/Object/Pushables/PushableObject.h"
#include "Objects/Generic/Object/Pushables/PushableObject_BridgeCol.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;

namespace TEN::Entities::Generic
{
	//Required to can use our Vector3i in the std::unordered_map
	struct Vector3iHasher 
	{
		std::size_t operator()(const Vector3i& v) const 
		{
			std::size_t h1 = std::hash<int>()(v.x);
			std::size_t h2 = std::hash<int>()(v.y);
			std::size_t h3 = std::hash<int>()(v.z);
			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};

	void InitializePushablesStacks()
	{
		//1. Make the function had an unique call
		//Not a singleton, but it follows the idea...

		static bool StacksInitialized = false;

		if (StacksInitialized)
			return;
		else
			StacksInitialized = true;

		//2. Collect all the pushables placed in the level
		auto& pushablesNumbersList = FindAllPushables(g_Level.Items);

		if (pushablesNumbersList.empty())
			return;

		//3. Prepare data to create several lists per each stack group. (One for each position XZ, and the ground floor height).
		//PROBLEM: Missing hash function in Vector3i, creating custom version Vector3iHasher at the top of this source code.
		std::unordered_map < Vector3i, std::vector<int>, Vector3iHasher> stackGroups; // stack Position - pushable itemNumber  

		//4. Iterate through the pushables list, to put them in their different stack groups. According to their XZ and ground floor height).
		//Extra, I moved also the .Data initialization here, to can store data in all the pushables objects (even the ones not initialized yet).
		for (int itemNumber : pushablesNumbersList)
		{
			auto& pushableItem = g_Level.Items[itemNumber];
			pushableItem.Data = PushableInfo();
			
			int x = pushableItem.Pose.Position.x;
			int z = pushableItem.Pose.Position.z;
			
			auto collisionResult = GetCollision(&pushableItem);
			int y = collisionResult.Position.Floor;
			
			stackGroups.emplace(Vector3i(x, y, z), std::vector<int>()).first->second.push_back(itemNumber);
		}

		//5. Iterate through the stack groups lists, on each one, it has to sort them by height, and iterate through it to make the stack links.
		for (auto& group : stackGroups)
		//for (auto it = stackGroups.begin(); it != stackGroups.end(); ++it)
		{
			//const auto& group = *it;
			auto& pushablesInGroup = group.second;

			//If there is only 1 pushable in that position, no stack check is required.
			if (pushablesInGroup.size() <= 1)
				continue;

			//If there are 2 or more pushables in the group, sort them from bottom to top. (Highest Y to Lowest Y).
			std::sort(pushablesInGroup.begin(), pushablesInGroup.end(), [](int a, int b) 
				{
					auto& pushableA = g_Level.Items[a];
					auto& pushableB = g_Level.Items[b];
					return pushableA.Pose.Position.y > pushableB.Pose.Position.y; //a is over B
				});

			//Iterate through each group to set the stack links).
			for (size_t i = 0; i < pushablesInGroup.size() - 1; ++i)
			{
				// Set the stackUpperItem and stackLowerItem variables accordingly
				int lowerItemNumber = pushablesInGroup[i];
				auto& lowerPushableItem = g_Level.Items[lowerItemNumber];
				auto& lowerPushable = GetPushableInfo(lowerPushableItem);
				
				int upperItemNumber = pushablesInGroup[i + 1];
				auto& upperPushableItem = g_Level.Items[upperItemNumber];
				auto& upperPushable = GetPushableInfo(upperPushableItem);
				
				lowerPushable.StackUpperItem = upperItemNumber;
				upperPushable.StackLowerItem = lowerItemNumber;
			}
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

	void StackPushable(int itemNumber, int itemNumber_target)
	{
		if (itemNumber_target == NO_ITEM)
			return;

		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		pushable.StackLowerItem = itemNumber_target;

		auto& lowerPushableItem = g_Level.Items[itemNumber_target];
		auto& lowerPushable = GetPushableInfo(lowerPushableItem);

		lowerPushable.StackUpperItem = itemNumber;
	}

	void UnpilePushable(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.StackLowerItem == NO_ITEM)
			return;

		auto& lowerPushableItem = g_Level.Items[pushable.StackLowerItem];
		auto& lowerPushable = GetPushableInfo(lowerPushableItem);

		pushable.StackLowerItem = NO_ITEM;
		lowerPushable.StackUpperItem = NO_ITEM;
	}

	int SearchNearPushablesStack(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];

		int pushabelStackFound = FindPushableStackInRoom(itemNumber, pushableItem.RoomNumber);

		if (pushabelStackFound != NO_ITEM)
			return pushabelStackFound;

		//Otherwise, check the room below.
		//auto collisionResult = GetCollision(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber);		
		//auto roomNumberBelow = collisionResult.Block->GetRoomNumberBelow(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z).value();
		//pushabelStackFound = FindPushableStackInRoom(itemNumber, roomNumberBelow);

		return NO_ITEM;
	}

	int FindPushableStackInRoom(int itemNumber, int roomNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (roomNumber != NO_ROOM)
		{
			short currentItemNumber = g_Level.Rooms[roomNumber].itemNumber;
			while (currentItemNumber != NO_ITEM)
			{
				auto& currentItem = g_Level.Items[currentItemNumber];

				//If is a climbable pushable, is in the same XZ position and is at a lower height.
				if ((currentItem.ObjectNumber >= ID_PUSHABLE_OBJECT_CLIMBABLE1 && currentItem.ObjectNumber <= ID_PUSHABLE_OBJECT_CLIMBABLE10) &&
					(currentItem.Pose.Position.x == pushableItem.Pose.Position.x) && (currentItem.Pose.Position.z == pushableItem.Pose.Position.z) &&
					(currentItem.Pose.Position.y > pushableItem.Pose.Position.y))
				{
					if (pushable.StackUpperItem == NO_ITEM)
					{
						//If the found item is at the top of a stack, return it.
						return currentItemNumber;
					}
					else
					{
						//Otherwise, iterate through its stack till find the topest one, and return that one.
						int topItemNumber = pushable.StackUpperItem;
						while (topItemNumber != NO_ITEM)
						{
							auto& topItem = g_Level.Items[topItemNumber];
							auto& topPushable = GetPushableInfo(topItem);

							if (topPushable.StackUpperItem == NO_ITEM)
							{
								return topItemNumber;
							}
							else
							{
								topItemNumber = topPushable.StackUpperItem;
							}
						}
					}
				}
				else
				{
					//Else, check the next item.
					currentItemNumber = currentItem.NextItem;
				}
			}
		}
		return NO_ITEM;
	}

	int CountPushablesInStack(int itemNumber)
	{
		auto pushableItemCopy = g_Level.Items[itemNumber];
		auto& pushableCopy = GetPushableInfo(pushableItemCopy);

		int count = 1;

		while (pushableCopy.StackUpperItem != NO_ITEM)
		{
			if (pushableCopy.StackUpperItem == itemNumber) //It shouldn't happens, but just in case.
				break;

			count++;

			pushableItemCopy = g_Level.Items[pushableCopy.StackUpperItem];
			pushableCopy = GetPushableInfo(pushableItemCopy);
		}

		return count;
	}

	bool IsUnderStackLimit(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		auto count = CountPushablesInStack(itemNumber);

		return count <= pushable.StackLimit;
	}

	void StartMovePushableStack(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int currentItemNumber = pushable.StackUpperItem;

		while (currentItemNumber != NO_ITEM)
		{
			auto& currentPushableItem = g_Level.Items[currentItemNumber];
			auto& currentPushable = GetPushableInfo(currentPushableItem);

			if (currentPushable.UsesRoomCollision)
				DeactivateClimbablePushableCollider(currentItemNumber); // Deactivate collision

			currentPushable.BehaviourState = PushablePhysicState::StackHorizontalMove;

			currentItemNumber = currentPushable.StackUpperItem;
		}
	}

	void StopMovePushableStack(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int currentItemNumber = pushable.StackUpperItem;

		while (currentItemNumber != NO_ITEM)
		{
			auto& currentPushableItem = g_Level.Items[currentItemNumber];
			auto& currentPushable = GetPushableInfo(currentPushableItem);

			currentPushableItem.Pose.Position = GetNearestSectorCenter(currentPushableItem.Pose.Position);

			if (currentPushable.UsesRoomCollision)
				ActivateClimbablePushableCollider(currentItemNumber); // Activate collision

			currentPushable.BehaviourState = PushablePhysicState::Idle;

			currentItemNumber = currentPushable.StackUpperItem;
		}
	}
	
	void StartFallPushableStack(int itemNumber)
	{

	}

	void StopFallPushableStack(int itemNumber)
	{

	}
}
