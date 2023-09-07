#pragma once
#include "Game/Lara/lara_struct.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	struct PushableInfo;

	struct PushableAnimationInfo
	{
		int	 PullAnimNumber = 0;
		int	 PushAnimNumber = 0;
		int	 EdgeAnimNumber = 0;
		bool EnableAnimLoop = 0;

		// TODO: Constants.
		PushableAnimationInfo()
		{
			PullAnimNumber = LA_PUSHABLE_PULL;
			PushAnimNumber = LA_PUSHABLE_PUSH;
			EdgeAnimNumber = LA_PUSHABLE_EDGE;
			EnableAnimLoop = true;
		}

		PushableAnimationInfo(int pullAnimNumber, int pushAnimNumber, int edgeAnimNumber, bool enableAnimLoop)
		{
			PullAnimNumber = pullAnimNumber;
			PushAnimNumber = pushAnimNumber;
			EdgeAnimNumber = edgeAnimNumber;
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
