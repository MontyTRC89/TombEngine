#pragma once
#include <array>
#include <d3d11.h>
#include <SimpleMath.h>

namespace TEN::Effects::Explosion
{
	struct ExplosionParticle
	{
		Vector3 pos;
		Vector3 vel;
		Vector4 tint;
		float size;
		float rotation;
		float angularVel;
		float age;
		float life;
		int room;
		int sprite;
		bool active;
	};
	extern std::array<ExplosionParticle, 64> explosionParticles;

	void TriggerExplosion(Vector3 const& pos, float size, bool triggerSparks, bool triggerSmoke, bool triggerShockwave, int room);
	void UpdateExplosionParticles();
	ExplosionParticle& getFreeExplosionParticle();
	void SpawnExplosionParticle(Vector3 const& pos);
}
