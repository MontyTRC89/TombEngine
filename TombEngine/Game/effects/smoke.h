#pragma once

enum class LaraWeaponType;
struct ItemInfo;

namespace TEN::Effects::Smoke
{
	constexpr auto SMOKE_PARTICLE_NUM_MAX = 128;

	struct SmokeParticle
	{
		Vector4 sourceColor;
		Vector4 destinationColor;
		Vector4 color;
		Vector3 position;
		Vector3 velocity;
		int room;
		int sprite;
		float gravity;
		float friction;
		float sourceSize;
		float destinationSize;
		float size;
		float age;
		float life;
		float angularVelocity;
		float angularDrag;
		float rotation;
		float terminalVelocity;
		bool affectedByWind;
		bool active;
	};

	extern std::array<SmokeParticle, SMOKE_PARTICLE_NUM_MAX> SmokeParticles;

	void UpdateSmokeParticles();
	void DisableSmokeParticles();
	void TriggerFlareSmoke(const Vector3& pos, Vector3& direction, int life, int room);
	void TriggerGunSmokeParticles(int x, int y, int z, int xv, int yv, int zv, byte initial, LaraWeaponType weaponType, byte count);
	void TriggerQuadExhaustSmoke(int x, int y, int z, short angle, int velocity, int moving);
	void TriggerRocketSmoke(int x, int y, int z, int bodyPart);
	void TriggerBreathSmoke(long x, long y, long z, short angle);
}
