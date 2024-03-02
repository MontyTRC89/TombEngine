#pragma once
#include "Game/items.h"
#include "Math/Math.h"

using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto FISH_COUNT_MAX = 512;

	struct FishData
	{
		float Life = 0.0f;

		Pose Pose;
		ItemInfo* target;
		ItemInfo* leader;
		short RoomNumber;
		short Velocity;
		Vector3i PositionTarget = Vector3::Zero;
		short Species;
		bool IsLethal;
		short YAngle = ANGLE(0.0f);
		float Undulation;

		Matrix Transform;
	};

	extern std::vector<FishData> FishSwarm;

	void InitializeFishSwarm(short itemNumber);
	void ControlFishSwarm(short itemNumber);

	void SpawnFishSwarm(ItemInfo* item);

	void UpdateFishSwarm();
	void ClearFishSwarm();
}
