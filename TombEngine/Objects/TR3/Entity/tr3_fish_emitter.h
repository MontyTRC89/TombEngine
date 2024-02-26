#pragma once
#include "Game/items.h"
#include "Math/Math.h"

using namespace TEN::Math;

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
		Vector3i PositionTarget = Vector3::Zero;
		short Species;
		bool Lethal;
		short YAngle = ANGLE(0.0f);

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
