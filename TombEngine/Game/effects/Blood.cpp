#include "framework.h"
#include "Game/effects/Blood.h"

#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Math/Math.h"
#include "Specific/clock.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Effects::Blood
{
	constexpr auto UW_BLOOD_COUNT_MAX = 512;
	constexpr auto UW_BLOOD_SCALE_MAX = BLOCK(0.25f);

	std::deque<UnderwaterBlood> UnderwaterBloodParticles = {};

	void SpawnUnderwaterBlood(const Vector3& pos, int roomNumber, float scale)
	{
		constexpr auto LIFE_MAX		= 8.5f;
		constexpr auto LIFE_MIN		= 8.0f;
		constexpr auto SPAWN_RADIUS = BLOCK(0.25f);

		auto& uwBlood = GetNewEffect(UnderwaterBloodParticles, UW_BLOOD_COUNT_MAX);

		auto sphere = BoundingSphere(pos, SPAWN_RADIUS);

		uwBlood.SpriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
		uwBlood.Position = Random::GeneratePointInSphere(sphere);
		uwBlood.Life = std::round(Random::GenerateFloat(LIFE_MIN, LIFE_MAX) * FPS);
		uwBlood.Init = 1.0f;
		uwBlood.Scale = scale;
	}

	void SpawnUnderwaterBloodCloud(const Vector3& pos, int roomNumber, float scaleMax, unsigned int count)
	{
		if (!TestEnvironment(ENV_FLAG_WATER, roomNumber))
			return;

		for (int i = 0; i < count; i++)
			SpawnUnderwaterBlood(pos, roomNumber, scaleMax);
	}

	void UpdateUnderwaterBloodParticles()
	{
		if (UnderwaterBloodParticles.empty())
			return;

		for (auto& uwBlood : UnderwaterBloodParticles)
		{
			if (uwBlood.Life <= 0.0f)
				continue;

			// Update scale.
			if (uwBlood.Scale < UW_BLOOD_SCALE_MAX)
				uwBlood.Scale += 4.0f;

			// Update life.
			if (uwBlood.Init == 0.0f)
			{
				uwBlood.Life -= 3.0f;
			}
			else if (uwBlood.Init < uwBlood.Life)
			{
				uwBlood.Init += 4.0f;

				if (uwBlood.Init >= uwBlood.Life)
					uwBlood.Init = 0.0f;
			}
		}

		ClearInactiveEffects(UnderwaterBloodParticles);
	}

	void ClearUnderwaterBloodParticles()
	{
		UnderwaterBloodParticles.clear();
	}
}
