#include "framework.h"
#include "Game/effects/Drip.h"

#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/effects/Ripple.h"
#include "Game/effects/weather.h"
#include "Game/room.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Environment;
using namespace TEN::Effects::Ripple;
using namespace TEN::Math;

namespace TEN::Effects::Drip
{
	constexpr auto DRIP_LIFE_SHORT_MAX = 1.0f;
	constexpr auto DRIP_LIFE_LONG_MAX  = 4.0f;

	constexpr auto DRIP_HEIGHT_OFFSET = 4;

	constexpr auto DRIP_COLOR_WHITE = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	std::array<DripParticle, DRIP_NUM_MAX> DripParticles = {};

	DripParticle& GetFreeDrip()
	{
		float shortestLife = INFINITY;
		auto* oldestDripPtr = &DripParticles[0];

		for (auto& drip : DripParticles)
		{
			if (!drip.IsActive)
				return drip;

			if (drip.Life < shortestLife)
			{
				shortestLife = drip.Life;
				oldestDripPtr = &drip;
			}
		}

		return *oldestDripPtr;
	}

	void SpawnDripParticle(const Vector3& pos, int roomNumber, const Vector3& velocity, float lifeInSec, float gravity)
	{
		auto& drip = GetFreeDrip();

		drip = {};
		drip.IsActive = true;
		drip.Position = pos;
		drip.RoomNumber = roomNumber;
		drip.Velocity = velocity;
		drip.Life = std::round(lifeInSec * FPS);
		drip.LifeMax = drip.Life;
		drip.Gravity = gravity;
	}

	void SpawnWetnessDrip(const Vector3& pos, int roomNumber)
	{
		auto box = BoundingOrientedBox(pos, Vector3::One * BLOCK(1.0f / 32), Vector4::Zero);
		auto dripPos = Random::GenerateVector3InBox(box);

		SpawnDripParticle(dripPos, roomNumber, Vector3::Zero, DRIP_LIFE_SHORT_MAX, Random::GenerateFloat(3.0f, 6.0f));
	}

	void SpawnSplashDrips(const Vector3& pos, int roomNumber, unsigned int count)
	{
		for (int i = 0; i < count; i++)
		{
			auto box = BoundingOrientedBox(pos, Vector3::One * BLOCK(1.0f / 8), Vector4::Zero);
			auto dripPos = Random::GenerateVector3InBox(box);

			auto direction = dripPos - pos;
			direction.Normalize();

			auto velocity = (direction * 16) - Vector3(0.0f, Random::GenerateFloat(32.0f, 64.0f), 0.0f);

			SpawnDripParticle(dripPos, roomNumber, velocity, DRIP_LIFE_LONG_MAX, Random::GenerateFloat(3.0f, 6.0f));
		}
	}

	void SpawnGunshellSplashDrips(const Vector3& pos, int roomNumber, unsigned int count)
	{
		for (int i = 0; i < count; i++)
		{
			auto box = BoundingOrientedBox(pos, Vector3::One * BLOCK(1.0f / 64), Vector4::Zero);
			auto dripPos = Random::GenerateVector3InBox(box);

			auto direction = dripPos - pos;
			direction.Normalize();

			auto velocity = (direction * 16) - Vector3(0.0f, Random::GenerateFloat(16.0f, 24.0f), 0.0f);

			SpawnDripParticle(dripPos, roomNumber, velocity, DRIP_LIFE_LONG_MAX, Random::GenerateFloat(2.0f, 3.0f));
		}
	}

	void UpdateDripParticles()
	{
		for (auto& drip : DripParticles)
		{
			if (!drip.IsActive)
				continue;

			// Deactivate.
			drip.Life -= 1.0f; // NOTE: Life tracked in frame time.
			if (drip.Life <= 0.0f)
				drip.IsActive = false;

			// Update velocity.
			drip.Velocity.y += drip.Gravity;
			if (TestEnvironment(ENV_FLAG_WIND, drip.RoomNumber))
				drip.Velocity += Weather.Wind();

			int prevRoomNumber = drip.RoomNumber;
			auto pointColl = GetCollision(drip.Position.x, drip.Position.y, drip.Position.z, drip.RoomNumber);

			// Update position.
			drip.Position += drip.Velocity;
			drip.RoomNumber = pointColl.RoomNumber;

			float lifeAlpha = 1.0f - (drip.Life / drip.LifeMax);

			// Update appearance.
			drip.Color = Vector4::Lerp(DRIP_COLOR_WHITE, Vector4::Zero, lifeAlpha);
			drip.Height = Lerp(DRIP_WIDTH / (1 / 6.4f), 0, lifeAlpha);

			// Hit water.
			if (TestEnvironment(ENV_FLAG_WATER, drip.RoomNumber))
			{
				drip.IsActive = false;

				// Spawn ripple.
				if (!TestEnvironment(ENV_FLAG_WATER, prevRoomNumber))
				{
					int waterHeight = GetWaterHeight(drip.Position.x, drip.Position.y, drip.Position.z, drip.RoomNumber);
					SpawnRipple(
						Vector3(drip.Position.x, waterHeight - DRIP_HEIGHT_OFFSET, drip.Position.z),
						Random::GenerateFloat(16.0f, 24.0f),
						{ RippleFlags::ShortInit, RippleFlags::LowOpacity });
				}

			}
			// Hit floor; spawn ripple.
			else if (drip.Position.y >= pointColl.Position.Floor)
			{
				drip.IsActive = false;

				SpawnRipple(
					Vector3(drip.Position.x, pointColl.Position.Floor - DRIP_HEIGHT_OFFSET, drip.Position.z),
					Random::GenerateFloat(8.0f, 24.0f),
					{ RippleFlags::ShortInit, RippleFlags::Ground },
					Geometry::GetFloorNormal(pointColl.FloorTilt));
			}
			// Hit ceiling; deactivate.
			else if (drip.Position.y <= pointColl.Position.Ceiling)
			{
				drip.IsActive = false;
			}
		}
	}

	void ClearDripParticles()
	{
		for (auto& drip : DripParticles)
			drip = {};
	}
}
