#include "framework.h"
#include "Game/effects/Ripple.h"

#include "Game/effects/effects.h"
#include "Math/Math.h"
#include "Objects/objectslist.h"
#include "Specific/clock.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Effects::Ripple
{
	constexpr auto RIPPLE_COUNT_MAX	  = 1024;
	constexpr auto RIPPLE_OPACITY_MAX = 0.5f;

	std::deque<Ripple> Ripples = {};

	void SpawnRipple(const Vector3& pos, int roomNumber, float scale, int flags, const Vector3& normal)
	{
		constexpr auto LIFE_WATER_SURFACE_MAX = 1.0f;
		constexpr auto LIFE_WATER_SURFACE_MIN = LIFE_WATER_SURFACE_MAX / 2;
		constexpr auto LIFE_GROUND_MAX		  = 0.4f;
		constexpr auto LIFE_GROUND_MIN		  = LIFE_GROUND_MAX / 2;
		constexpr auto COLOR_WHITE			  = Vector4(1.0f, 1.0f, 1.0f, RIPPLE_OPACITY_MAX);

		auto& ripple = GetNewEffect(Ripples, RIPPLE_COUNT_MAX);

		float lifeInSec = (flags & (int)RippleFlags::OnGround) ?
			Random::GenerateFloat(LIFE_GROUND_MIN, LIFE_GROUND_MAX) :
			Random::GenerateFloat(LIFE_WATER_SURFACE_MIN, LIFE_WATER_SURFACE_MAX);
		float fadeTimeInSec = lifeInSec / ((ripple.Flags & ((int)RippleFlags::SlowFade)) ? 3 : 2);

		ripple.SpriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES;
		ripple.Position = pos;
		ripple.RoomNumber = roomNumber;
		ripple.Normal = normal;
		ripple.Color = COLOR_WHITE;
		ripple.Life =
		ripple.LifeMax = round(lifeInSec * FPS);
		ripple.LifeFullOpacity = round((lifeInSec - fadeTimeInSec) * FPS);
		ripple.LifeStartFading = round(fadeTimeInSec * FPS);
		ripple.Scale = scale;
		ripple.Flags = flags;
	}

	void UpdateRipples()
	{
		constexpr auto SCALE_MAX	   = BLOCK(0.5f);
		constexpr auto SCALE_RATE_FAST = 4.0f;
		constexpr auto SCALE_RATE_SLOW = 2.0f;

		if (Ripples.empty())
			return;

		for (auto& ripple : Ripples)
		{
			if (ripple.Life <= 0.0f)
				continue;

			// Update scale.
			if (ripple.Scale < SCALE_MAX)
				ripple.Scale += (ripple.Flags & ((int)RippleFlags::SlowFade | (int)RippleFlags::OnGround)) ? SCALE_RATE_SLOW : SCALE_RATE_FAST;

			// Update opacity.
			if (ripple.Life >= ripple.LifeFullOpacity)
			{
				float alpha = 1.0f - (ripple.Life - ripple.LifeFullOpacity) / (ripple.LifeMax - ripple.LifeFullOpacity);
				ripple.Color.w = Lerp(0.0f, RIPPLE_OPACITY_MAX, alpha);
			}
			else if (ripple.Life <= ripple.LifeStartFading)
			{
				float alpha = 1.0f - (ripple.Life / ripple.LifeStartFading);
				ripple.Color.w = Lerp(RIPPLE_OPACITY_MAX, 0.0f, alpha);
			}

			// Update life.
			ripple.Life -= 1.0f;
		}

		ClearInactiveEffects(Ripples);
	}

	void ClearRipples()
	{
		Ripples.clear();
	}
}
