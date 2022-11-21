#include "framework.h"
#include "Game/effects/Bubble.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Objects/objectslist.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Effects::Bubble
{
	constexpr auto BUBBLE_COLOR_TRANSPARENT = Vector4(0, 0, 0, 1);

	extern std::array<Bubble, BUBBLE_NUM_MAX> Bubbles = {};

	Bubble& GetFreeBubble()
	{
		auto* oldestBubblePtr = &Bubbles[0];
		float oldestAge = 0.0f;

		for (auto& bubble : Bubbles)
		{
			if (!bubble.IsActive)
				return bubble;

			if (oldestAge < bubble.Life)
			{
				oldestAge = bubble.Life;
				oldestBubblePtr = &bubble;
			}
		}

		return *oldestBubblePtr;
	}

	void SpawnBubble(const Vector3& pos, int roomNumber, int unk1, int unk2, int flags, int xv, int yv, int zv)
	{
		if (!TestEnvironment(ENV_FLAG_WATER, roomNumber))
			return;

		float shade = Random::GenerateFloat(0.3f, 0.8f);

		auto& bubble = GetFreeBubble();

		bubble = {};
		bubble.IsActive = true;
		bubble.SpriteIndex = (flags & BubbleFlags::Clump) ? SPR_UNKNOWN1 : SPR_BUBBLES;

		bubble.Scale = 0.0f;
		bubble.Life = 0.0f;
		bubble.Velocity = (flags & BubbleFlags::Clump) ? Random::GenerateFloat(8.0f, 16.0f) : Random::GenerateFloat(8.0f, 12.0f);
		bubble.ColorStart = BUBBLE_COLOR_TRANSPARENT;


		bubble.ColorEnd = Vector4(shade, shade, shade, 1.0f);
		bubble.Color = bubble.ColorStart;
		bubble.ScaleMax = (flags & BubbleFlags::BigSize) ? Random::GenerateFloat(256.0f, 512.0f) : Random::GenerateFloat(32.0f, 128.0f);
		bubble.Rotation = 0.0f;
		bubble.Position = pos;

		float maxAmplitude = (flags & BubbleFlags::HighAmplitude) ? 256.0f : 32.0f;

		bubble.Amplitude = Vector3(Random::GenerateFloat(-maxAmplitude, maxAmplitude), Random::GenerateFloat(-maxAmplitude, maxAmplitude), Random::GenerateFloat(-maxAmplitude, maxAmplitude));
		bubble.PositionHome = bubble.Position;
		bubble.WavePeriod = Vector3::Zero;
		bubble.WaveVelocity = Vector3(1 / Random::GenerateFloat(8.0f, 16.0f), 1 / Random::GenerateFloat(8.0f, 16.0f), 1 / Random::GenerateFloat(8.0f, 16.0f));
		bubble.RoomNumber = roomNumber;
	}

	void UpdateBubbles()
	{
		for (auto& bubble : Bubbles)
		{
			if (!bubble.IsActive)
				continue;

			bubble.Life++;

			float alpha = std::min(bubble.Life / 15.0f, 1.0f);
			bubble.Scale = Lerp(0.0f, bubble.ScaleMax, alpha);
			bubble.Color = Vector4::Lerp(bubble.ColorStart, bubble.ColorEnd, alpha);

			int ceilingHeight = g_Level.Rooms[bubble.RoomNumber].maxceiling;
			short RoomNumber = bubble.RoomNumber;

			auto pointColl = GetCollision(bubble.Position.x, bubble.Position.y, bubble.Position.z, bubble.RoomNumber);

			if (bubble.Position.y > pointColl.Position.Floor ||
				!pointColl.BottomBlock)
			{
				bubble.IsActive = false;
				continue;
			}

			if (!TestEnvironment(ENV_FLAG_WATER, pointColl.RoomNumber))
			{
				bubble.IsActive = false;
				SetupRipple(bubble.Position.x, g_Level.Rooms[bubble.RoomNumber].maxceiling, bubble.Position.z, (GetRandomControl() & 0xF) + 48, RIPPLE_FLAG_SHORT_INIT);
				continue;
			}

			if (pointColl.Position.Ceiling == NO_HEIGHT ||
				bubble.Position.y <= pointColl.Position.Ceiling)
			{
				bubble.IsActive = false;
				continue;
			}

			// Update position.
			bubble.WavePeriod += bubble.WaveVelocity;
			bubble.PositionHome.y -= bubble.Velocity;
			bubble.Position = bubble.PositionHome + (bubble.Amplitude * Vector3(sin(bubble.WavePeriod.x), sin(bubble.WavePeriod.y), sin(bubble.WavePeriod.z)));
		}
	}

	void ClearBubbles()
	{
		for (auto& bubble : Bubbles)
			bubble = {};
	}
}
