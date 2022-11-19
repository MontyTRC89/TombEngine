#include "framework.h"
#include "Game/effects/drip.h"

#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/effects/weather.h"
#include "Game/room.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Environment;
using namespace TEN::Math;

namespace TEN::Effects::Drip
{
	constexpr auto DRIP_LIFE_SHORT_MAX = 25.0f;
	constexpr auto DRIP_LIFE_LONG_MAX  = 120.0f;

	constexpr auto DRIP_COLOR_WHITE = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	std::array<DripParticle, DRIPS_NUM_MAX> DripParticles = {};

	DripParticle& GetFreeDrip()
	{
		for (auto& drip : DripParticles)
		{
			if (!drip.IsActive)
				return drip;
		}

		return DripParticles[0];
	}

	void SpawnDripParticle(const Vector3& pos, int roomNumber, const Vector3& velocity, float maxLife, float gravity)
	{
		auto& drip = GetFreeDrip();

		drip = {};
		drip.IsActive = true;
		drip.Position = pos;
		drip.RoomNumber = roomNumber;
		drip.Velocity = velocity;
		drip.Life = maxLife;
		drip.LifeMax = drip.Life;
		drip.Gravity = gravity;
	}

	void SpawnWetnessDrip(const Vector3& pos, int roomNumber)
	{
		auto box = BoundingOrientedBox(pos, Vector3(32.0f, 32.0f, 32.0f), Vector4::Zero);
		auto dripPos = Random::GenerateVector3InBox(box);

		SpawnDripParticle(dripPos, roomNumber, Vector3::Zero, DRIP_LIFE_SHORT_MAX, Random::GenerateFloat(3.0f, 6.0f));
	}

	void SpawnSplashDrips(const Vector3& pos, int roomNumber, unsigned int count)
	{
		for (int i = 0; i < count; i++)
		{
			auto box = BoundingOrientedBox(pos, Vector3(128.0f, 128.0f, 128.0f), Vector4::Zero);
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
			auto box = BoundingOrientedBox(pos, Vector3(16.0f, 16.0f, 16.0f), Vector4::Zero);
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
			drip.Life -= 1.0f;
			if (drip.Life <= 0.0f)
				drip.IsActive = false;

			// Update velocity according to wind.
			if (TestEnvironment(ENV_FLAG_WIND, drip.RoomNumber))
			{
				drip.Velocity.x = Weather.Wind().x;
				drip.Velocity.z = Weather.Wind().z;
			}

			// Update position.
			drip.Velocity.y += drip.Gravity;
			drip.Position += drip.Velocity;

			float lifeAlpha = 1.0f - (drip.Life / drip.LifeMax);

			// Update appearance.
			drip.Color = Vector4::Lerp(DRIP_COLOR_WHITE, Vector4::Zero, lifeAlpha);
			drip.Height = Lerp(DRIP_WIDTH / 0.15625f, 0, lifeAlpha);

			int floorHeight = GetCollision(drip.Position.x, drip.Position.y, drip.Position.z, drip.RoomNumber).Position.Floor;
			int waterHeight = GetWaterHeight(drip.Position.x, drip.Position.y, drip.Position.z, drip.RoomNumber);

			// Drip hit floor; deactivate.
			if (drip.Position.y > floorHeight)
				drip.IsActive = false;

			// Land hit water; spawn ripple.
			if (drip.Position.y > waterHeight)
			{
				drip.IsActive = false;
				SetupRipple(drip.Position.x, waterHeight, drip.Position.z, Random::GenerateInt(16, 24), RIPPLE_FLAG_SHORT_INIT | RIPPLE_FLAG_LOW_OPACITY);
			}
		}
	}

	void DisableDripParticles()
	{
		for (auto& drip : DripParticles)
			drip.IsActive = false;
	}
}
