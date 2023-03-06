#pragma once
#include "Game/collision/floordata.h"
#include "Game/Lara/lara.h"
#include "Sound/sound_effects.h"

class Vector3i;
class GameVector;
struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	struct PushableInfo;
	PushableInfo& GetPushableInfo(const ItemInfo& item);

	enum PushableSoundsType
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

		PushableAnimationInfo(int pullAnim, int pushAnim, bool isLoop)
		{
			PullAnimIndex = pullAnim;
			PushAnimIndex = pushAnim;
			AllowLoop = isLoop;
		}
	};

	// Main functions
	void InitialisePushableBlock(const short itemNumber);
	void PushableBlockControl(short itemNumber);
	void PushableBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	// Behaviour functions
	bool PushableBlockManageGravity (const short itemNumber);
	void PushableBlockManageIdle	(const short itemNumber);
	void PushableBlockManageMoving	(const short itemNumber);

	// Sounds functions
	void InitializePushablesSoundsMap();
	int GetPushableSound(const PushableSoundsType& type, const GameVector& detectionPoint);
	void PushablesManageSounds(const short itemNumber);
	
	//General functions
	void InitialisePushablesGeneral();
	std::vector<int> FindAllPushables(const std::vector<ItemInfo>& objectsList);
	int GetPushableHeight(ItemInfo& item);
	void UpdateRoomNumbers(const short itemNumber);
	void DeactivationRoutine(const short itemNumber);
	
	// Collision test functions
	bool IsNextSectorValid(ItemInfo& item, const GameVector& targetPoint, const bool checkIfLaraFits);
	bool IsValidForLara(const ItemInfo& pushableItem, const PushableInfo& pushableInfo, const GameVector& targetPoint);
	bool IsPushableOnValidSurface(ItemInfo& item);

	// Stack utilities functions
	void UpdateAllPushablesStackLinks();
	
	void MoveStack(const short itemNumber, const Vector3i& GoalPos);
	void MoveStackXZ(const short itemNumber);
	void MoveStackY(const short itemNumber, const int y);

	void ManageStackBridges(short itemNumber, bool addBridge);
	void RemovePushableFromStack(short itemNumber);
	
	int GetStackHeight(ItemInfo& item);
	bool CheckStackLimit(ItemInfo& item);

	// Floor data collision functions
	std::optional<int> PushableBlockFloor(short itemNumber, int x, int y, int z);
	std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z);
	int PushableBlockFloorBorder(short itemNumber);
	int PushableBlockCeilingBorder(short itemNumber);

	void FloatingItem(ItemInfo& item);
}
