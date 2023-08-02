#pragma once
#include "Game/Lara/lara.h"

//class GameVector;
//class Vector3i;
struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	struct PushableInfo;

	struct PushableAnimationInfo
	{
		int	 PullAnimNumber;
		int	 PushAnimNumber;
		bool EnableAnimLoop;

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

	extern std::vector<PushableAnimationInfo> PushableAnimInfos;

	PushableInfo& GetPushableInfo(const ItemInfo& item);
	
	void InitializePushableBlock(int itemNumber);
	void PushableBlockControl(int itemNumber);
	void PushableBlockCollision(int itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	int GetPushableHeight(ItemInfo& item);

	void SetPushableStopperFlag(bool value, Vector3i& pos, int roomNumber);
}
