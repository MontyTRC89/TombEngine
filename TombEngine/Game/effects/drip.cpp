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
	constexpr auto DRIP_COLOR = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

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

	void SpawnDripParticle(const Vector3& pos, int roomNumber, const Vector3& velocity, float life, float gravity)
	{
		auto& drip = GetFreeDrip();

		drip = {};
		drip.IsActive = true;
		drip.Position = pos;
		drip.RoomNumber = roomNumber;
		drip.Velocity = velocity;
		drip.Life = life;
		drip.Gravity = gravity;
	}

	void SpawnWetnessDrip(const Vector3& pos, int roomNumber)
	{
		SpawnDripParticle(pos, roomNumber, Vector3::Zero, DRIP_LIFE, Random::GenerateFloat(3.0f, 6.0f));
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

			SpawnDripParticle(dripPos, roomNumber, velocity, DRIP_LIFE_LONG, Random::GenerateFloat(3.0f, 6.0f));
		}
	}

	void SpawnGunshellDrips(const Vector3& pos, int roomNumber)
	{
		for (int i = 0; i < 4; i++)
		{
			auto box = BoundingOrientedBox(pos, Vector3(16.0f, 16.0f, 16.0f), Vector4::Zero);
			auto dripPos = Random::GenerateVector3InBox(box);

			auto direction = dripPos - pos;
			direction.Normalize();

			auto velocity = (direction * 16) - Vector3(0.0f, Random::GenerateFloat(16.0f, 24.0f), 0.0f);

			SpawnDripParticle(dripPos, roomNumber, velocity, DRIP_LIFE_LONG, Random::GenerateFloat(2.0f, 3.0f));
		}
	}

	void UpdateDripParticles()
	{
		for (auto& drip : DripParticles)
		{
			if (!drip.IsActive)
				continue;

			// Deactivate.
			drip.Age += 1.0f;
			if (drip.Age > drip.Life)
				drip.IsActive = false;

			drip.Velocity.y += drip.Gravity;

			// Update velocity according to wind.
			if (TestEnvironment(ENV_FLAG_WIND, drip.RoomNumber))
			{
				drip.Velocity.x = Weather.Wind().x;
				drip.Velocity.z = Weather.Wind().z;
			}

			drip.Position += drip.Velocity;
			float normalizedAge = drip.Age / drip.Life;
			drip.Color = Vector4::Lerp(DRIP_COLOR, Vector4::Zero, normalizedAge);
			drip.Height = Lerp(DRIP_WIDTH / 0.15625f, 0, normalizedAge);
			short RoomNumber = drip.RoomNumber;
			FloorInfo* floor = GetFloor(drip.Position.x, drip.Position.y, drip.Position.z, &RoomNumber);
			int floorHeight = floor->FloorHeight(drip.Position.x, drip.Position.z);
			int waterHeight = GetWaterHeight(drip.Position.x, drip.Position.y, drip.Position.z, drip.RoomNumber);

			// Land on floor.
			if (drip.Position.y > floorHeight)
				drip.IsActive = false;

			// Land in water.
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
