#pragma once
#include "Game/items.h"

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto FISH_COUNT_MAX = 240;

	struct FishData
	{
		bool on;
		Pose Pose;
		ItemInfo* target;
		ItemInfo* leader;
		short RoomNumber;
		short Velocity;
		short YTarget;
		short XTarget;
		short ZTarget;
		BYTE counter;

		Matrix Transform;
	};

	extern FishData FishSwarm[FISH_COUNT_MAX];
	extern int NextFish;

	void InitializeFishSwarm(short itemNumber);
	void FishSwarmControl(short itemNumber);

	short GetFreeFish();
	void ClearFishSwarm();
	void UpdateFishSwarm();
	void SpawnFishSwarm(ItemInfo* item);
	Vector3 GetRandomFishTarget(ItemInfo* item);
}
