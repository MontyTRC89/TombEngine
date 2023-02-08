#include "framework.h"
#include "Game/effects/Bubble.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/effects/Ripple.h"
#include "Objects/objectslist.h"
#include "Math/Math.h"
#include "Specific/clock.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Ripple;
using namespace TEN::Math;

namespace TEN::Effects::Bubble
{
	constexpr auto BUBBLE_COUNT_MAX		   = 1024;
	constexpr auto BUBBLE_LIFE_MAX		   = 30.0f;
	constexpr auto BUBBLE_SCALE_MAX		   = BLOCK(0.5f);
	constexpr auto BUBBLE_OPACTY_MAX	   = 0.8f;
	constexpr auto BUBBLE_OPACTY_MIN	   = BUBBLE_OPACTY_MAX / 2;
	constexpr auto BUBBLE_OSC_VELOCITY_MAX = 0.4f;
	constexpr auto BUBBLE_OSC_VELOCITY_MIN = BUBBLE_OSC_VELOCITY_MAX / 4;

	std::deque<Bubble> Bubbles = {};

	void SpawnBubble(const Vector3& pos, int roomNumber, float scale, float amplitude, const Vector3& inertia)
	{
		constexpr auto GRAVITY_MAX		 = 12.0f;
		constexpr auto GRAVITY_MIN		 = GRAVITY_MAX * (2 / 3.0f);
		constexpr auto WAVE_VELOCITY_MAX = 1 / 8.0f;
		constexpr auto WAVE_VELOCITY_MIN = WAVE_VELOCITY_MAX / 2;

		if (!TestEnvironment(ENV_FLAG_WATER, roomNumber))
			return;

		auto& bubble = GetNewEffect(Bubbles, BUBBLE_COUNT_MAX);

		scale = std::min(scale, BUBBLE_SCALE_MAX);

		bubble.SpriteIndex = SPR_BUBBLES;
		bubble.Position =
		bubble.PositionBase = pos;
		bubble.RoomNumber = roomNumber;

		bubble.Color =
		bubble.ColorStart = Vector4(1.0f, 1.0f, 1.0f, Random::GenerateFloat(BUBBLE_OPACTY_MIN, BUBBLE_OPACTY_MAX));
		bubble.ColorEnd = Vector4(1.0f, 1.0f, 1.0f, 0.0f);

		bubble.Inertia = inertia;
		bubble.Amplitude = Random::GenerateDirection() * amplitude;
		bubble.WavePeriod = Vector3::Zero;
		bubble.WaveVelocity = Vector3(
			Random::GenerateFloat(WAVE_VELOCITY_MIN, WAVE_VELOCITY_MAX),
			Random::GenerateFloat(WAVE_VELOCITY_MIN, WAVE_VELOCITY_MAX),
			Random::GenerateFloat(WAVE_VELOCITY_MIN, WAVE_VELOCITY_MAX));
		
		bubble.Scale =
		bubble.ScaleMax = Vector2(scale);
		bubble.ScaleMin = bubble.Scale * 0.7f;

		bubble.Life = std::round(BUBBLE_LIFE_MAX * FPS);
		bubble.Gravity = Lerp(GRAVITY_MIN, GRAVITY_MAX, scale / BUBBLE_SCALE_MAX);
		bubble.OscillationPeriod = Random::GenerateFloat(0.0f, scale);
		bubble.OscillationVelocity = Lerp(BUBBLE_OSC_VELOCITY_MAX, BUBBLE_OSC_VELOCITY_MIN, scale / BUBBLE_SCALE_MAX);
	}

	void SpawnBubble(const Vector3& pos, int roomNumber, int flags, const Vector3& inertia)
	{
		constexpr auto SCALE_LARGE_MAX	  = BUBBLE_SCALE_MAX;
		constexpr auto SCALE_LARGE_MIN	  = SCALE_LARGE_MAX / 2;
		constexpr auto SCALE_SMALL_MAX	  = SCALE_LARGE_MIN / 2;
		constexpr auto SCALE_SMALL_MIN	  = SCALE_SMALL_MAX / 4;
		constexpr auto AMPLITUDE_MAX_HIGH = BLOCK(0.25f);
		constexpr auto AMPLITUDE_MAX_LOW  = AMPLITUDE_MAX_HIGH / 8;

		float scale = (flags & (int)BubbleFlags::Large) ?
			Random::GenerateFloat(SCALE_LARGE_MIN, SCALE_LARGE_MAX) :
			Random::GenerateFloat(SCALE_SMALL_MIN, SCALE_SMALL_MAX);
		float amplitude = (flags & (int)BubbleFlags::HighAmplitude) ? AMPLITUDE_MAX_HIGH : AMPLITUDE_MAX_LOW;

		SpawnBubble(pos, roomNumber, scale, amplitude, inertia);
	}

	void SpawnDiveBubbles(const Vector3& pos, int roomNumber, unsigned int count)
	{
		constexpr auto SPAWN_RADIUS = BLOCK(1 / 32.0f);

		auto sphere = BoundingSphere(pos, SPAWN_RADIUS);
		for (int i = 0; i < count; i++)
		{
			auto bubblePos = Random::GeneratePointInSphere(sphere);
			SpawnBubble(bubblePos, roomNumber, (int)BubbleFlags::HighAmplitude);
		}
	}
	
	void SpawnChaffBubble(const Vector3& pos, int roomNumber)
	{
		constexpr auto SCALE_MAX		 = BUBBLE_SCALE_MAX / 8;
		constexpr auto SCALE_MIN		 = SCALE_MAX / 2;
		constexpr auto GRAVITY_MAX		 = 16.0f;
		constexpr auto GRAVITY_MIN		 = GRAVITY_MAX / 4;
		constexpr auto AMPLITUDE_MAX	 = BLOCK(1 / 16.0f);
		constexpr auto WAVE_VELOCITY_MAX = 1 / 16.0f;
		constexpr auto WAVE_VELOCITY_MIN = WAVE_VELOCITY_MAX / 2;

		auto& bubble = GetNewEffect(Bubbles, BUBBLE_COUNT_MAX);

		float scale = Random::GenerateFloat(SCALE_MIN, SCALE_MAX);

		bubble.SpriteIndex = SPR_BUBBLES;
		bubble.Position =
		bubble.PositionBase = pos;
		bubble.RoomNumber = roomNumber;

		bubble.Color =
		bubble.ColorStart = Vector4(1.0f, 1.0f, 1.0f, Random::GenerateFloat(BUBBLE_OPACTY_MIN, BUBBLE_OPACTY_MAX));
		bubble.ColorEnd = Vector4(1.0f, 1.0f, 1.0f, 0.0f);

		bubble.Inertia = Vector3::Zero;
		bubble.Amplitude = Random::GenerateDirection() * AMPLITUDE_MAX;
		bubble.WavePeriod = Vector3(Random::GenerateFloat(-PI, PI), Random::GenerateFloat(-PI, PI), Random::GenerateFloat(-PI, PI));
		bubble.WaveVelocity = Vector3(
			Random::GenerateFloat(WAVE_VELOCITY_MIN, WAVE_VELOCITY_MAX),
			Random::GenerateFloat(WAVE_VELOCITY_MIN, WAVE_VELOCITY_MAX),
			Random::GenerateFloat(WAVE_VELOCITY_MIN, WAVE_VELOCITY_MAX));
		
		bubble.Scale =
		bubble.ScaleMax = Vector2(scale);
		bubble.ScaleMin = bubble.Scale * 0.7f;

		bubble.Life = std::round(BUBBLE_LIFE_MAX * FPS);
		bubble.Gravity = Lerp(GRAVITY_MIN, GRAVITY_MAX, scale / BUBBLE_SCALE_MAX);
		bubble.OscillationPeriod = Random::GenerateFloat(0.0f, scale);
		bubble.OscillationVelocity = Lerp(BUBBLE_OSC_VELOCITY_MAX, BUBBLE_OSC_VELOCITY_MIN, scale / SCALE_MAX);
	}

	void UpdateBubbles()
	{
		constexpr auto LIFE_FULL_SCALE	 = std::max(BUBBLE_LIFE_MAX - 0.25f, 0.0f);
		constexpr auto LIFE_START_FADING = std::min(1.0f, BUBBLE_LIFE_MAX);

		if (Bubbles.empty())
			return;

		for (auto& bubble : Bubbles)
		{
			if (bubble.Life <= 0.0f)
				continue;

			auto pointColl = GetCollision(bubble.Position.x, bubble.Position.y - bubble.Gravity, bubble.Position.z, bubble.RoomNumber);

			// Hit floor or ceiling; set to despawn.
			if ((bubble.Position.y - bubble.Gravity) >= pointColl.Position.Floor ||
				(bubble.Position.y - bubble.Gravity) <= pointColl.Position.Ceiling)
			{
				bubble.Life = 0.0f;
				continue;
			}

			// Hit water surface; spawn ripple.
			if (!TestEnvironment(ENV_FLAG_WATER, pointColl.RoomNumber))
			{
				SpawnRipple(
					Vector3(bubble.Position.x, g_Level.Rooms[bubble.RoomNumber].maxceiling, bubble.Position.z),
					pointColl.RoomNumber,
					((bubble.ScaleMax.x + bubble.ScaleMax.y) / 2) * 0.5f,
					RippleFlags::ShortInit);

				bubble.Life = 0.0f;
				continue;
			}

			// Update color.
			float alpha = 1.0f - (bubble.Life / std::round(LIFE_START_FADING * FPS));
			bubble.Color = Vector4::Lerp(bubble.ColorStart, bubble.ColorEnd, alpha);

			// Update position.
			bubble.WavePeriod += bubble.WaveVelocity;
			bubble.PositionBase += Vector3(0.0f, -bubble.Gravity, 0.0f) + bubble.Inertia;
			bubble.Position = bubble.PositionBase + (bubble.Amplitude * Vector3(sin(bubble.WavePeriod.x), sin(bubble.WavePeriod.y), sin(bubble.WavePeriod.z)));

			//  TODO: Let bubbles be affected by sinks.
			
			// Update intertia.
			bubble.Inertia *= 0.8f;

			// Oscillate scale according to period.
			bubble.OscillationPeriod += bubble.OscillationVelocity;
			bubble.Scale = Vector2(
				(bubble.ScaleMin.x / 2) + ((bubble.ScaleMax.x - bubble.ScaleMin.x) * (0.5f + (0.5f * sin(bubble.OscillationPeriod)))),
				(bubble.ScaleMin.y / 2) + ((bubble.ScaleMax.y - bubble.ScaleMin.y) * (0.5f + (0.5f * cos(bubble.OscillationPeriod + 1.0f)))));
			bubble.Scale *= Lerp(0.0f, 1.0f, bubble.Life / std::round(LIFE_FULL_SCALE * FPS));

			// Update life.
			bubble.Life -= 1.0f;
		}
		
		ClearInactiveEffects(Bubbles);
	}

	void ClearBubbles()
	{
		Bubbles.clear();
	}
}
