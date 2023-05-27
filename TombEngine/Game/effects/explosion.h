#pragma once

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

	void TriggerExplosion(const Vector3& pos, float size, bool triggerSparks, bool triggerSmoke, bool triggerShockwave, int room);
	void UpdateExplosionParticles();
	ExplosionParticle& getFreeExplosionParticle();
	void SpawnExplosionParticle(const Vector3& pos);
}
