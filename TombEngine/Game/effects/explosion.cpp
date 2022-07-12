#include "framework.h"
#include "Game/effects/explosion.h"

#include "Game/effects/effects.h"
#include "Game/effects/spark.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/prng.h"
#include "Specific/setup.h"
#include "Specific/trmath.h"

using namespace TEN::Math::Random;

namespace TEN::Effects::Explosion
{
	using namespace DirectX::SimpleMath;
	using namespace TEN::Effects::Spark;

	std::array<ExplosionParticle, 64> explosionParticles;

	constexpr float PARTICLE_DISTANCE = 512;

	void TriggerExplosion(Vector3 const& pos, float size, bool triggerSparks, bool triggerSmoke, bool triggerShockwave, int room)
	{
		SpawnExplosionParticle(pos);
		for (int i = 0; i < 3; i++)
		{
			Vector3 particlePos = pos + Vector3(GenerateFloat(-size / 2, size / 2), GenerateFloat(-size / 2, size / 2), GenerateFloat(-size / 2, size / 2));
			SpawnExplosionParticle(particlePos);
		}

		if (triggerSparks)
		{
			Vector3Int sparkPos;
			sparkPos.x = pos.x;
			sparkPos.y = pos.y;
			sparkPos.z = pos.z;
			//TriggerExplosionSparks(&sparkPos, room); @TODO
		}

		if (triggerShockwave)
		{
			PHD_3DPOS shockPos;
			shockPos.Position.x = pos.x;
			shockPos.Position.y = pos.y;
			shockPos.Position.z = pos.z;
			TriggerShockwave(&shockPos, 0, size, 64, 32, 32, 32, 30, rand() & 0xFFFF, 0);
		}
	}

	void UpdateExplosionParticles()
	{
		for (int i = 0; i < explosionParticles.size(); i++)
		{
			auto& e = explosionParticles[i];

			if (!e.active)
				continue;

			e.age++;
			if (e.age > e.life)
			{
				e.active = false;
				continue;
			};

			e.vel *= 0.98f;
			e.pos += e.vel;
			e.angularVel *= 0.98f;
			e.rotation += e.angularVel;
			int numSprites = -Objects[ID_EXPLOSION_SPRITES].nmeshes - 1;
			float normalizedAge = e.age / e.life;
			e.sprite = Lerp(0.0f, numSprites, normalizedAge);
			e.tint = Vector4::Lerp(Vector4(2, 2, 2, 1), Vector4(0, 0, 0, 0), normalizedAge);
		}
	}

	ExplosionParticle& getFreeExplosionParticle()
	{
		for (int i = 0; i < explosionParticles.size(); i++)
		{
			if (!explosionParticles[i].active)
				return explosionParticles[i];
		}

		return explosionParticles[0];
	}

	void SpawnExplosionParticle(Vector3 const & pos)
	{
		auto& e = getFreeExplosionParticle();
		e = {};
		e.pos = pos;
		const float maxVel = 10;
		e.vel = Vector3(GenerateFloat(-maxVel / 2, maxVel / 2), GenerateFloat(-maxVel / 2, maxVel / 2), GenerateFloat(-maxVel / 2, maxVel / 2));
		e.active = true;
		e.tint = Vector4(1, 1, 1, 1);
		e.life = GenerateFloat(60, 90);
		e.size = GenerateFloat(512, 768);
		e.angularVel = GenerateFloat(-RADIAN, RADIAN);
		e.rotation = GenerateFloat(-0.05f, 0.05f);
	}
}
