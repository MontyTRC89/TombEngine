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
	using namespace DirectX::SimpleMath;

	std::array<DripParticle, NUM_DRIPS> dripParticles;
	constexpr Vector4 DRIP_COLOR = Vector4(1, 1, 1, 1);

	void DisableDripParticles()
	{
		for (int i = 0; i < dripParticles.size(); i++)
			dripParticles[i].active = false;
	}

	void UpdateDripParticles()
	{
		for (int i = 0; i < dripParticles.size(); i++) 
		{
			auto& drip = dripParticles[i];

			if (!drip.active)
				continue;

			drip.age++;
			if (drip.age > drip.life)
				drip.active = false;

			drip.velocity.y += drip.gravity;

			if (TestEnvironment(ENV_FLAG_WIND, drip.room))
			{
				drip.velocity.x = Weather.Wind().x;
				drip.velocity.z = Weather.Wind().z;
			}

			drip.pos += drip.velocity;
			float normalizedAge = drip.age / drip.life;
			drip.color = Vector4::Lerp(DRIP_COLOR, Vector4::Zero, normalizedAge);
			drip.height = Lerp(DRIP_WIDTH / 0.15625f, 0, normalizedAge);
			short room = drip.room;
			FloorInfo* floor = GetFloor(drip.pos.x, drip.pos.y, drip.pos.z, &room);
			int floorheight = floor->FloorHeight(drip.pos.x, drip.pos.z);
			int waterHeight = GetWaterHeight(drip.pos.x, drip.pos.y, drip.pos.z, drip.room);

			if (drip.pos.y > floorheight) 
				drip.active = false;

			if (drip.pos.y > waterHeight) 
			{
				drip.active = false;
				SetupRipple(drip.pos.x, waterHeight, drip.pos.z, Random::GenerateInt(16, 24), RIPPLE_FLAG_SHORT_INIT | RIPPLE_FLAG_LOW_OPACITY);
			}
		}
	}

	DripParticle& getFreeDrip()
	{
		for (int i = 0; i < dripParticles.size(); i++)
		{
			if (!dripParticles[i].active)
				return dripParticles[i];
		}

		return dripParticles[0];
	}

	void SpawnWetnessDrip(const Vector3& pos, int room)
	{
		auto& drip = getFreeDrip();
		drip = {};
		drip.active = true;
		drip.pos = pos;
		drip.room = room;
		drip.life = DRIP_LIFE;
		drip.gravity = Random::GenerateFloat(3, 6);
	}

	void SpawnSplashDrips(const Vector3& pos, int number, int room)
	{
		for (int i = 0; i < number; i++) 
		{
			auto dripPos = pos + Vector3(Random::GenerateFloat(-128, 128), Random::GenerateFloat(-128, 128), Random::GenerateFloat(-128, 128));
			auto direction = (dripPos - pos);
			direction.Normalize();

			auto& drip = getFreeDrip();
			drip = {};
			drip.pos = dripPos;
			drip.velocity = direction*16;
			drip.velocity -= Vector3(0, Random::GenerateFloat(32, 64), 0);
			drip.gravity = Random::GenerateFloat(3, 6);
			drip.room = room;
			drip.life = DRIP_LIFE_LONG;
			drip.active = true;
		}
	}

	void SpawnGunshellDrips(const Vector3& pos, int room)
	{
		for (int i = 0; i < 4; i++) 
		{
			auto dripPos = pos + Vector3(Random::GenerateFloat(-16, 16), Random::GenerateFloat(-16, 16), Random::GenerateFloat(-16, 16));
			auto direction = dripPos - pos;
			direction.Normalize();

			auto& drip = getFreeDrip();
			drip = {};
			drip.pos = dripPos;
			drip.velocity = direction * 16;
			drip.velocity -= Vector3(0, Random::GenerateFloat(16, 24), 0);
			drip.gravity = Random::GenerateFloat(2, 3);
			drip.room = room;
			drip.life = DRIP_LIFE_LONG;
			drip.active = true;
		}
	}
}
