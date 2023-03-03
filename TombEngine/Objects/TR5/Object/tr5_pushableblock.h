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
	void InitialisePushableBlock(const short itemNumber);
	void PushableBlockControl(short itemNumber);
	void PushableBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	// Behaviour functions
	bool PushableBlockManageFalling	(const short itemNumber);
	void PushableBlockManageIdle	(const short itemNumber);
	void PushableBlockManageMoving	(const short itemNumber);

	// Sounds functions
	void InitializePushablesSoundsMap();
	int GetPushableSound(const PushableSoundsType& type, const GameVector& detectionPoint);
	void PushablesManageSounds(const short itemNumber);
	
	//General functions
	void InitialisePushablesGeneral();
	void DeactivationRoutine(const short itemNumber);
	std::vector<int> FindAllPushables(const std::vector<ItemInfo>& objectsList);
	bool IsClimbablePushable(const int ObjectNumber);
	bool IsObjectPushable(const int ObjectNumber);
	void UpdateRoomNumbers(const short itemNumber);
	void ClearMovableBlockSplitters(const Vector3i& pos, short roomNumber);
	
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
}
