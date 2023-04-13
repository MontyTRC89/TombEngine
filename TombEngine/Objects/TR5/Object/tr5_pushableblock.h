#pragma once
#include "Game/collision/floordata.h"
#include "Game/Lara/lara.h"
#include "Sound/sound_effects.h"

class GameVector;
class Vector3i;
struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	struct PushableInfo;

	enum PushableSoundType
	{
		Loop,
		Stop,
		Fall
	};

	struct PushableSoundData
	{
		int LoopSfx = 0; // Looped sound during moving.
		int StopSfx = 0; // Ending sound after movement.
		int LandSfx = 0; // Landing sound following drop.
	};

	struct PushableAnimationInfo
	{
		int	 PullAnimNumber = 0;
		int	 PushAnimNumber = 0;
		bool EnableAnimLoop = 0;

		PushableAnimationInfo()
		{
			PullAnimNumber = LA_PUSHABLE_PULL;
			PushAnimNumber = LA_PUSHABLE_PUSH;
			EnableAnimLoop = true;
		}

		PushableAnimationInfo(int pullAnimNumber, int pushAnimNumber, bool enableAnimLoop)
		{
			PullAnimNumber = pullAnimNumber;
			PushAnimNumber = pushAnimNumber;
			EnableAnimLoop = enableAnimLoop;
		}
	};

	// Main functions
	void InitialisePushableBlock(int itemNumber);
	void PushableBlockControl(int itemNumber);
	void PushableBlockCollision(int itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	// Behaviour functions
	bool PushableBlockManageGravity(int itemNumber);
	void PushableBlockManageIdle(int itemNumber);
	void PushableBlockManageMoving(int itemNumber);

	// Sound functions
	int	 GetPushableSfx(PushableSoundType soundType, const GameVector& pos);
	void PushablesManageSounds(int itemNumber);
	
	// General functions
	void InitialisePushablesGeneral();
	void DeactivationPushablesRoutine(int itemNumber);
	std::vector<int> FindAllPushables(const std::vector<ItemInfo>& objectsList);
	int GetPushableHeight(ItemInfo& item);
	void UpdatePushablesRoomNumbers(int itemNumber);
	void ForcePushableActivation(int itemNumber);

	// Collision test functions
	bool IsNextSectorValid(ItemInfo& item, const GameVector& target, bool checkIfPlayerFits);
	bool IsValidForLara(const ItemInfo& pushableItem, const PushableInfo& pushable, const GameVector& target);
	bool IsPushableOnValidSurface(ItemInfo& item);

	// Stack utility functions
	void UpdateAllPushablesStackLinks();
	
	void MoveStack(int itemNumber, const Vector3i& target);
	void MoveStackXZ(int itemNumber);
	void MoveStackY(int itemNumber, int y);

	void ManageStackBridges(int itemNumber, bool addBridge);
	void RemovePushableFromStack(int itemNumber);
	
	int GetStackHeight(ItemInfo& item);
	bool CheckStackLimit(ItemInfo& item);

	// Floordata collision functions
	std::optional<int> PushableBlockFloor(int itemNumber, int x, int y, int z);
	std::optional<int> PushableBlockCeiling(int itemNumber, int x, int y, int z);
	int PushableBlockFloorBorder(int itemNumber);
	int PushableBlockCeilingBorder(int itemNumber);
}
