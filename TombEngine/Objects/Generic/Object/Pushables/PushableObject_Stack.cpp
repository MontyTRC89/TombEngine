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
	void InitializePushablesStacks()
	{
		static bool StacksInitialized = false;

		if (StacksInitialized)
			return;

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

		StacksInitialized = true;
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
						return currentItemNumber;
					}
					else
					{
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
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int count = 1;

		while (pushable.StackUpperItem != 0)
		{
			if (pushable.StackUpperItem == itemNumber) //It shouldn't happens, but just in case.
				break;

			count++;

			pushableItem = g_Level.Items[pushable.StackUpperItem];
			pushable = GetPushableInfo(pushableItem);
		}

		return count;
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
