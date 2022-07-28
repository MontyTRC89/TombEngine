#include "framework.h"
#include "Game/effects/drip.h"

#include <d3d11.h>
#include <SimpleMath.h>
#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/effects/weather.h"
#include "Game/room.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"
#include "Specific/trmath.h"

using namespace TEN::Effects::Environment;
using namespace TEN::Math::Random;

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
			auto& d = dripParticles[i];

			if (!d.active)
				continue;

			d.age++;
			if (d.age > d.life)
				d.active = false;

			d.velocity.y += d.gravity;

			if (g_Level.Rooms[d.room].flags & ENV_FLAG_WIND) 
			{
				d.velocity.x = Weather.Wind().x;
				d.velocity.z = Weather.Wind().z;
			}

			d.pos += d.velocity;
			float normalizedAge = d.age / d.life;
			d.color = Vector4::Lerp(DRIP_COLOR, Vector4::Zero, normalizedAge);
			d.height = Lerp(DRIP_WIDTH / 0.15625f, 0, normalizedAge);
			short room = d.room;
			FloorInfo* floor = GetFloor(d.pos.x, d.pos.y, d.pos.z, &room);
			int floorheight = floor->FloorHeight(d.pos.x, d.pos.z);
			int wh = GetWaterHeight(d.pos.x, d.pos.y, d.pos.z, d.room);

			if (d.pos.y > floorheight) 
				d.active = false;

			if (d.pos.y > wh) 
			{
				d.active = false;
				SetupRipple(d.pos.x, wh, d.pos.z, GenerateInt(16, 24), RIPPLE_FLAG_SHORT_INIT | RIPPLE_FLAG_LOW_OPACITY);
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

	void SpawnWetnessDrip(Vector3 const & pos, int room)
	{
		auto& d = getFreeDrip();
		d = {};
		d.active = true;
		d.pos = pos;
		d.room = room;
		d.life = DRIP_LIFE;
		d.gravity = GenerateFloat(3, 6);
	}

	void SpawnSplashDrips(Vector3 const& pos, int num,int room)
	{
		for (int i = 0; i < num; i++) 
		{
			Vector3 dripPos = pos + Vector3(GenerateFloat(-128, 128), GenerateFloat(-128, 128), GenerateFloat(-128, 128));
			Vector3 dir = (dripPos - pos);
			dir.Normalize();
			DripParticle& drip = getFreeDrip();
			drip = {};
			drip.pos = dripPos;
			drip.velocity = dir*16;
			drip.velocity -= Vector3(0, GenerateFloat(32, 64), 0);
			drip.gravity = GenerateFloat(3, 6);
			drip.room = room;
			drip.life = DRIP_LIFE_LONG;
			drip.active = true;
		}
	}

	void SpawnGunshellDrips(Vector3 const & pos, int room)
	{
		for (int i = 0; i < 4; i++) 
		{
			Vector3 dripPos = pos + Vector3(GenerateFloat(-16, 16), GenerateFloat(-16, 16), GenerateFloat(-16, 16));
			Vector3 dir = (dripPos - pos);
			dir.Normalize();
			DripParticle& drip = getFreeDrip();
			drip = {};
			drip.pos = dripPos;
			drip.velocity = dir * 16;
			drip.velocity -= Vector3(0, GenerateFloat(16, 24), 0);
			drip.gravity = GenerateFloat(2, 3);
			drip.room = room;
			drip.life = DRIP_LIFE_LONG;
			drip.active = true;
		}
	}
}
