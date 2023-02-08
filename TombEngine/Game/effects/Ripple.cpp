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
	constexpr auto RIPPLE_COUNT_MAX = 1024;

	extern std::deque<Ripple> Ripples = {};

	void SpawnRipple(const Vector3& pos, int roomNumber, float scale, int flags, const Vector3& normal)
	{
		constexpr auto LIFE_WATER_SURFACE_MAX = 0.75f;
		constexpr auto LIFE_WATER_SURFACE_MIN = 0.5f;
		constexpr auto LIFE_GROUND_MAX		  = 0.4f;
		constexpr auto LIFE_GROUND_MIN		  = 0.25f;
		constexpr auto FADE_TIME_LONG		  = 0.2f;
		constexpr auto FADE_TIME_SHORT		  = 0.1f;
		constexpr auto COLOR_WHITE			  = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		constexpr auto SPAWN_RADIUS_2D		  = BLOCK(1 / 16.0f);

		auto& ripple = GetNewEffect(Ripples, RIPPLE_COUNT_MAX);

		float lifeMax = (flags & RippleFlags::OnGround) ? LIFE_GROUND_MAX : LIFE_WATER_SURFACE_MAX;
		float lifeMin = (flags & RippleFlags::OnGround) ? LIFE_GROUND_MIN : LIFE_WATER_SURFACE_MIN;
		float life = Random::GenerateFloat(lifeMin, lifeMax);
		float fadeTime = (flags & (RippleFlags::ShortInit | RippleFlags::OnGround)) ? FADE_TIME_SHORT : FADE_TIME_LONG;

		ripple.SpriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES;
		ripple.Position = pos;
		ripple.RoomNumber = roomNumber;
		ripple.Normal = normal;
		ripple.Color = COLOR_WHITE;
		ripple.Life =
		ripple.LifeMax = round(life * FPS);
		ripple.LifeFullOpacity = round((life - fadeTime) * FPS);
		ripple.LifeStartFading = round(fadeTime * FPS);
		ripple.Scale = scale;
		ripple.Flags = flags;

		// Slightly randomize 2D ripple position.
		if (ripple.Flags & RippleFlags::RandomizePosition)
		{
			auto direction2D = Random::GenerateDirection2D();
			auto offset = direction2D * Random::GenerateFloat(0.0f, SPAWN_RADIUS_2D);
			ripple.Position += Vector3(offset.x, 0.0f, offset.y);
		}
	}

	void UpdateRipples()
	{
		constexpr auto SCALE_MAX		= BLOCK(0.5f);
		constexpr auto SCALE_RATE_LARGE = 4.0f;
		constexpr auto SCALE_RATE_SMALL = 2.0f;

		if (Ripples.empty())
			return;

		for (auto& ripple : Ripples)
		{
			if (ripple.Life <= 0.0f)
				continue;

			// Update scale.
			if (ripple.Scale < SCALE_MAX)
				ripple.Scale += (ripple.Flags & (RippleFlags::ShortInit | RippleFlags::OnGround)) ? SCALE_RATE_SMALL : SCALE_RATE_LARGE;

			// Update opacity.
			if (ripple.Life >= ripple.LifeFullOpacity)
			{
				float alpha = 1.0f - (ripple.Life - ripple.LifeFullOpacity) / (ripple.LifeMax - ripple.LifeFullOpacity);
				ripple.Color.w = Lerp(0.0f, 1.0f, alpha);
			}
			else if (ripple.Life <= ripple.LifeStartFading)
			{
				float alpha = 1.0f - (ripple.Life / ripple.LifeStartFading);
				ripple.Color.w = Lerp(1.0f, 0.0f, alpha);
			}

			//opacity *= (ripple.Flags & RippleFlags::LowOpacity) ? 0.5f : 1.0f;
			//ripple.Color.w = opacity;

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
