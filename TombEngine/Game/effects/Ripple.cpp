#include "framework.h"
#include "Game/effects/Ripple.h"

#include "Math/Math.h"
#include "Specific/BitField.h"
#include "Objects/objectslist.h"

using namespace TEN::Math;

namespace TEN::Effects::Ripple
{
	constexpr auto RIPPLE_SCALE_MAX = BLOCK(1 / 4.0f);

	constexpr auto RIPPLE_COLOR_WHITE = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	constexpr auto RIPPLE_COLOR_RED	  = Vector4(0.8f, 0.0f, 0.0f, 1.0f);

	extern std::array<Ripple, RIPPLE_NUM_MAX> Ripples = {};

	Ripple& GetFreeRipple()
	{
		float shortestLife = INFINITY;
		auto* oldestRipplePtr = &Ripples[0];

		for (auto& ripple : Ripples)
		{
			if (!ripple.IsActive)
				return ripple;

			if (ripple.Life < shortestLife)
			{
				shortestLife = ripple.Life;
				oldestRipplePtr = &ripple;
			}
		}

		return *oldestRipplePtr;
	}

	void SpawnRipple(const Vector3& pos, float scale, const std::vector<unsigned int>& flags, const Vector3& normal)
	{
		auto& ripple = GetFreeRipple();

		ripple = {};
		ripple.IsActive = true;
		ripple.SpriteIndex = SPR_RIPPLES;
		ripple.Position = pos;
		ripple.Normal = normal;
		ripple.Life = Random::GenerateFloat(16.0f, 64.0f);
		ripple.Init = 1.0f;
		ripple.Scale = scale;
		ripple.Flags.Set(flags);

		if (ripple.Flags.Test(RippleFlags::NoRandom))
			ripple.Position += Vector3(Random::GenerateFloat(-64.0f, 64.0f), 0.0f, Random::GenerateFloat(-64.0f, 64.0f));

		if (ripple.Flags.Test(RippleFlags::Ground))
			ripple.Life = Random::GenerateFloat(16.0f, 24.0f);
	}

	void SpawnUnderwaterBlood(const Vector3& pos, float scale)
	{
		static const auto flags = std::vector<unsigned int>{ RippleFlags::LowOpacity, RippleFlags::Blood };

		auto& ripple = GetFreeRipple();

		ripple = {};
		ripple.IsActive = true;
		ripple.SpriteIndex = 0;
		ripple.Position = pos + Vector3(Random::GenerateFloat(-32.0f, 32.0f), 0.0f, Random::GenerateFloat(-32.0f, 32.0f));
		ripple.Normal = Vector3::Zero; // Unused by underwater blood rendering.
		ripple.Life = Random::GenerateFloat(240.0f, 248.0f);
		ripple.Init = 1.0f;
		ripple.Scale = scale;
		ripple.Flags.Set(flags);
	}

	void UpdateRipples()
	{
		for (auto& ripple : Ripples)
		{
			if (!ripple.IsActive)
				continue;

			// Update scale.
			if (ripple.Scale < RIPPLE_SCALE_MAX)
			{
				if (ripple.Flags.Test(RippleFlags::ShortInit))
					ripple.Scale += 2.0f;
				else
					ripple.Scale += 4.0f;
			}

			if (!ripple.Init)
			{
				ripple.Life -= 3.0f;
				if (ripple.Life > 250.0f)
					ripple.Flags.ClearAll();
			}
			else if (ripple.Init < ripple.Life)
			{
				if (ripple.Flags.Test(RippleFlags::ShortInit))
					ripple.Init += 8.0f;
				else
					ripple.Init += 4.0f;

				if (ripple.Init >= ripple.Life)
					ripple.Init = 0.0f;
			}
		}
	}

	void ClearRipples()
	{
		for (auto& ripple : Ripples)
			ripple = {};
	}
}
