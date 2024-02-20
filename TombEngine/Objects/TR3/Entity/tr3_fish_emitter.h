#pragma once
#include "Game/items.h"

namespace TEN::Entities::Creatures::TR3
{
	struct FishData
	{
		bool on;
		Pose Pose;
		ItemInfo* target;
		short roomNumber;
		short randomRotation;
		short Velocity;
		short YTarget;
		short XTarget;
		short ZTarget;
		BYTE counter;

		Matrix Transform;
	};

	constexpr auto NUM_FISHES = 24;

	extern FishData FishSwarm[NUM_FISHES];
	extern int NextFish;

	void InitializeFishSwarm(short itemNumber);
	void FishSwarmControl(short itemNumber);
	short GetFreeFish();
	void ClearFishSwarm();
	void UpdateFishSwarm();
	void SpawnFishSwarm(ItemInfo* item);
}
