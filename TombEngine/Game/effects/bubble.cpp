#include "framework.h"
#include "Game/effects/Bubble.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Objects/objectslist.h"
#include "Math/Math.h"
#include "Specific/clock.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Effects::Bubble
{
	constexpr auto BUBBLE_LIFE_MAX = 60.0f * FPS;

	// TODO: Use deque instead? Depends how the compiler handles it. -- Sezz 2023.01.27
	std::vector<Bubble> Bubbles = {};

	void SpawnBubble(const Vector3& pos, int roomNumber, const Vector3& inertia, int flags)
	{
		constexpr auto COLOR_END		   = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
		constexpr auto OPACTY_MAX		   = 0.8f;
		constexpr auto OPACTY_MIN		   = 0.3f;
		constexpr auto AMPLITUDE_HIGH	   = BLOCK(0.25f);
		constexpr auto AMPLITUDE_LOW	   = BLOCK(1 / 32.0f);
		constexpr auto SCALE_LARGE_MAX	   = BLOCK(0.5f);
		constexpr auto SCALE_LARGE_MIN	   = BLOCK(0.25f);
		constexpr auto SCALE_SMALL_MAX	   = BLOCK(1 / 8.0f);
		constexpr auto SCALE_SMALL_MIN	   = BLOCK(1 / 32.0f);
		constexpr auto VELOCITY_MIN		   = 8.0f;
		constexpr auto VELOCITY_SINGLE_MAX = 12.0f;
		constexpr auto VELOCITY_CLUMP_MAX  = 16.0f;
		constexpr auto WAVE_VELOCITY_MAX   = 1 / 8.0f;
		constexpr auto WAVE_VELOCITY_MIN   = 1 / 16.0f;
		constexpr auto OSC_VELOCITY_MAX	   = 0.5f;
		constexpr auto OSC_VELOCITY_MIN	   = 0.1f;

		if (!TestEnvironment(ENV_FLAG_WATER, roomNumber))
			return;

		auto& bubble = GetFreeEffect(Bubbles, BUBBLE_NUM_MAX);

		float amplitudeMax = (flags & BubbleFlags::HighAmplitude) ? AMPLITUDE_HIGH : AMPLITUDE_LOW;
		auto sphere = BoundingSphere(Vector3::Zero, amplitudeMax);

		bubble.SpriteIndex = (flags & BubbleFlags::Clump) ? SPR_UNKNOWN1 : SPR_BUBBLES;
		bubble.Position =
		bubble.PositionBase = pos;
		bubble.RoomNumber = roomNumber;
		bubble.Inertia = inertia;

		bubble.Color =
		bubble.ColorStart = Vector4(1.0f, 1.0f, 1.0f, Random::GenerateFloat(OPACTY_MIN, OPACTY_MAX));
		bubble.ColorEnd = COLOR_END;

		bubble.Amplitude = Random::GeneratePointInSphere(sphere);
		bubble.WavePeriod = Vector3::Zero;
		bubble.WaveVelocity = Vector3(
			Random::GenerateFloat(WAVE_VELOCITY_MIN, WAVE_VELOCITY_MAX),
			Random::GenerateFloat(WAVE_VELOCITY_MIN, WAVE_VELOCITY_MAX),
			Random::GenerateFloat(WAVE_VELOCITY_MIN, WAVE_VELOCITY_MAX));
		
		bubble.Scale =
		bubble.ScaleMax = (flags & BubbleFlags::Large) ?
			Vector2(Random::GenerateFloat(SCALE_LARGE_MIN, SCALE_LARGE_MAX)) :
			Vector2(Random::GenerateFloat(SCALE_SMALL_MIN, SCALE_SMALL_MAX));
		bubble.ScaleMin = bubble.Scale * 0.7f;

		bubble.Life = BUBBLE_LIFE_MAX;
		bubble.Velocity = Random::GenerateFloat(VELOCITY_MIN, (flags & BubbleFlags::Clump) ? VELOCITY_CLUMP_MAX : VELOCITY_SINGLE_MAX);
		bubble.OscillationPeriod = Random::GenerateFloat(0.0f, (bubble.ScaleMax.x + bubble.ScaleMax.y / 2));
		bubble.OscillationVelocity = (flags & BubbleFlags::Clump) ?
			0.0f : Lerp(OSC_VELOCITY_MAX, OSC_VELOCITY_MIN, ((bubble.ScaleMax.x + bubble.ScaleMax.y / 2)) / SCALE_LARGE_MAX);
	}

	void SpawnBubble(const Vector3& pos, int roomNumber, int flags)
	{
		SpawnBubble(pos, roomNumber, Vector3::Zero, flags);
	}

	void UpdateBubbles()
	{
		constexpr auto LIFE_START_FADING = std::min(1.0f * FPS, BUBBLE_LIFE_MAX);

		if (Bubbles.empty())
			return;

		for (auto& bubble : Bubbles)
		{
			if (bubble.Life <= 0.0f)
				continue;

			auto pointColl = GetCollision(bubble.Position.x, bubble.Position.y, bubble.Position.z, bubble.RoomNumber);

			// Hit floor or ceiling; set to despawn.
			if (bubble.Position.y >= pointColl.Position.Floor ||
				bubble.Position.y <= pointColl.Position.Ceiling)
			{
				bubble.Life = 0.0f;
				continue;
			}

			// Reached water surface; spawn ripple.
			if (!TestEnvironment(ENV_FLAG_WATER, pointColl.RoomNumber))
			{
				bubble.Life = 0.0f;

				SetupRipple(
					bubble.Position.x, g_Level.Rooms[bubble.RoomNumber].maxceiling, bubble.Position.z,
					Random::GenerateFloat(48.0f, 64.0f),
					RIPPLE_FLAG_SHORT_INIT);

				continue;
			}

			// Update color.
			float alpha = 1.0f - (bubble.Life / LIFE_START_FADING);
			bubble.Color = Vector4::Lerp(bubble.ColorStart, bubble.ColorEnd, alpha);

			// Oscillate scale according to period.
			bubble.OscillationPeriod += bubble.OscillationVelocity;
			bubble.Scale = Vector2(
				(bubble.ScaleMin.x / 2) + ((bubble.ScaleMax.x - bubble.ScaleMin.x) * (0.5f + (0.5f * sin(bubble.OscillationPeriod)))),
				(bubble.ScaleMin.y / 2) + ((bubble.ScaleMax.y - bubble.ScaleMin.y) * (0.5f + (0.5f * cos(bubble.OscillationPeriod + 1.0f)))));

			// Update position.
			bubble.WavePeriod += bubble.WaveVelocity;
			bubble.PositionBase.y -= bubble.Velocity;
			bubble.PositionBase += bubble.Inertia;
			bubble.Position = bubble.PositionBase + (bubble.Amplitude * Vector3(sin(bubble.WavePeriod.x), sin(bubble.WavePeriod.y), sin(bubble.WavePeriod.z)));

			// Update intertia.
			bubble.Inertia *= 0.75f;

			// Update life.
			bubble.Life -= 1.0f;
		}

		// Clear inactive effects.
		Bubbles.erase(
			std::remove_if(
				Bubbles.begin(), Bubbles.end(),
				[](const Bubble& bubble) { return bubble.Life <= 0.0f; }), Bubbles.end());
	}

	void ClearBubbles()
	{
		Bubbles.clear();
	}
}
