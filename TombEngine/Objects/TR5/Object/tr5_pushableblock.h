#pragma once
#include "Game/collision/floordata.h"
#include "Sound/sound_effects.h"

class Vector3i;
class GameVector;
struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	struct PushableInfo;
	PushableInfo* GetPushableInfo(ItemInfo* item);

	enum PushableSoundsType
	{
		LOOP = 0,
		STOP = 1,
		FALL = 2
	};

	struct PushablesSounds
	{
		int loopSound;			// looped sound index for movement
		int stopSound;			// ending sound index
		int fallSound;			// sound on hitting floor (if dropped)

		PushablesSounds()
		{
			loopSound = SFX_TR4_PUSHABLE_SOUND;
			stopSound = SFX_TR4_PUSH_BLOCK_END;
			fallSound = SFX_TR4_BOULDER_FALL;
		}

		PushablesSounds(int loopValue, int stopValue, int fallValue)
		{
			loopSound = loopValue;
			stopSound = stopValue;
			fallSound = fallValue;
		}
	};

	// Main functions
	void InitialisePushableBlock(short itemNumber);
	void PushableBlockControl(short itemNumber);
	void PushableBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	// Behaviour functions
	bool PushableBlockManageGravity(ItemInfo& pushableItem, PushableInfo& pushableInfo, const short itemNumber); //Maybe itemNumber could be cut out.
	void PushableBlockManageIdle(ItemInfo& pushableItem, PushableInfo& pushableInfo, const short itemNumber);
	bool PushableBlockManageMoving(ItemInfo& pushableItem, PushableInfo& pushableInfo, const short itemNumber);

	// Sound functions
	void InitializePushablesSoundsMap();
	int PushableGetSound(const PushableSoundsType& type, const GameVector& detectionPoint);
	void PushableBlockManageSounds(const ItemInfo& pushableItem, PushableInfo& pushableInfo);

	//Floor Data update functions
	void ClearMovableBlockSplitters(const Vector3i& pos, short roomNumber);

	// Test functions
	bool IsNextSectorValid(ItemInfo& item, const int blockHeight, const GameVector& quadrant, const bool isPulling = true);
	bool IsValidForLara(const ItemInfo& pushableItem, const PushableInfo& pushableInfo, const GameVector& quadrant);
	bool IsPushableOnValidSurface(ItemInfo& item);

	// Stack utilities functions
	void MoveStack(short itemNumber, const Vector3i& GoalPos);
	void MoveStackXZ(short itemNumber);
	void MoveStackY(short itemNumber, int y);
	void UpdateBridgeStack(short itemNumber, bool addBridge);
	void RemoveFromStack(short itemNumber);
	std::vector<int> FindAllPushables();
	void UpdateAllPushablesStackLinks();
	int GetStackHeight(ItemInfo& item);
	bool CheckStackLimit(ItemInfo& item);

	// Floor data collision functions
	std::optional<int> PushableBlockFloor(short itemNumber, int x, int y, int z);
	std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z);
	int PushableBlockFloorBorder(short itemNumber);
	int PushableBlockCeilingBorder(short itemNumber);
}
