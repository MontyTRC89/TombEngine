#include "framework.h"
#include "Game/effects/Ripple.h"

#include "Game/effects/effects.h"
#include "Math/Math.h"
#include "Objects/objectslist.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Effects::Ripple
{
	constexpr auto RIPPLE_COUNT_MAX = 1024;

	extern std::deque<Ripple> Ripples = {};

	void SpawnRipple(const Vector3& pos, int roomNumber, float scale, int flags, const Vector3& normal)
	{
		constexpr auto LIFE_WATER_SURFACE_MAX = 64.0f;
		constexpr auto LIFE_GROUND_MAX		  = 24.0f;
		constexpr auto LIFE_MIN				  = 16.0f;
		constexpr auto COLOR_WHITE			  = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		constexpr auto SPAWN_RADIUS_2D		  = BLOCK(1 / 16.0f);

		auto& ripple = GetNewEffect(Ripples, RIPPLE_COUNT_MAX);

		ripple.SpriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES;
		ripple.Position = pos;
		ripple.RoomNumber = roomNumber;
		ripple.Normal = normal;
		ripple.Color = COLOR_WHITE;
		ripple.Life = Random::GenerateFloat(LIFE_MIN, (ripple.Flags & RippleFlags::OnGround) ? LIFE_GROUND_MAX : LIFE_GROUND_MAX);
		ripple.Init = 1.0f;
		ripple.Scale = scale;
		ripple.Flags = flags;

		// Slightly randomize 2D ripple position.
		if (ripple.Flags & RippleFlags::RandomizePosition)
		{
			auto direction2D = Random::GenerateDirection2D();
			auto offset = direction2D * Random::GenerateFloat(-SPAWN_RADIUS_2D, SPAWN_RADIUS_2D);
			ripple.Position += Vector3(offset.x, 0.0f, offset.y);
		}
	}

	void UpdateRipples()
	{
		constexpr auto SCALE_MAX = BLOCK(0.25f);

		if (Ripples.empty())
			return;

		for (auto& ripple : Ripples)
		{
			if (ripple.Life <= 0.0f)
				continue;

			// Update scale.
			if (ripple.Scale < SCALE_MAX)
			{
				if (ripple.Flags & (RippleFlags::ShortInit | RippleFlags::OnGround))
					ripple.Scale += 2.0f;
				else
					ripple.Scale += 4.0f;
			}

			// Update life and opacity.
			if (ripple.Init == 0.0f)
			{
				ripple.Life -= 3.0f;
			}
			else if (ripple.Init < ripple.Life)
			{
				if (ripple.Flags & (RippleFlags::ShortInit | RippleFlags::OnGround))
					ripple.Init += 8.0f;
				else
					ripple.Init += 4.0f;

				if (ripple.Init >= ripple.Life)
					ripple.Init = 0.0f;
			}
		}

		ClearInactiveEffects(Ripples);
	}

	void ClearRipples()
	{
		Ripples.clear();
	}
}
