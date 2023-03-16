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
		Loop = 0,
		Stop = 1,
		Fall = 2
	};

	struct PushablesSounds
	{
		int LoopSound;			// looped sound index for movement
		int StopSound;			// ending sound index
		int FallSound;			// sound on hitting floor (if dropped)

		PushablesSounds()
		{
			LoopSound = SFX_TR4_PUSHABLE_SOUND;
			StopSound = SFX_TR4_PUSH_BLOCK_END;
			FallSound = SFX_TR4_BOULDER_FALL;
		}

		PushablesSounds(int loopValue, int stopValue, int fallValue)
		{
			LoopSound = loopValue;
			StopSound = stopValue;
			FallSound = fallValue;
		}
	};

	struct PushableAnimationInfo
	{
		int PullAnimIndex;
		int PushAnimIndex;
		bool AllowLoop;

		PushableAnimationInfo()
		{
			PullAnimIndex = LA_PUSHABLE_PULL;
			PushAnimIndex = LA_PUSHABLE_PUSH;
			AllowLoop = true;
		}

		PushableAnimationInfo(int pullAnimNumber, int pushAnimNumber, bool isLoop)
		{
			PullAnimIndex = pullAnimNumber;
			PushAnimIndex = pushAnimNumber;
			AllowLoop = isLoop;
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

	// Sounds functions
	void InitializePushablesSoundsMap();
	int GetPushableSound(PushableSoundType soundType, const GameVector& pos);
	void PushablesManageSounds(int itemNumber);
	
	// General functions
	void InitialisePushablesGeneral();
	void DeactivationPushablesRoutine(int itemNumber);
	std::vector<int> FindAllPushables(const std::vector<ItemInfo>& objectsList);
	int GetPushableHeight(ItemInfo& item);
	void UpdatePushablesRoomNumbers(int itemNumber);
	void ForcePushableActivation(int itemNumber);

	// Collision test functions
	bool IsNextSectorValid(ItemInfo& item, const GameVector& target, bool checkIfLaraFits);
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
