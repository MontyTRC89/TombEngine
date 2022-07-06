#pragma once
#include <array>
#include <d3d11.h>
#include <SimpleMath.h>

namespace TEN::Effects::Drip
{
	constexpr float DRIP_LIFE = 25.0f;
	constexpr float DRIP_LIFE_LONG = 120.0f;
	constexpr float DRIP_WIDTH = 4.0f;
	constexpr int NUM_DRIPS = 512;

	struct DripParticle
	{
		Vector3 pos;
		Vector3 velocity;
		Vector4 color;
		int room;
		float gravity;
		float life;
		float age;
		float height;
		bool active;
	};
	extern std::array<DripParticle, NUM_DRIPS> dripParticles;

	void UpdateDripParticles();
	void DisableDripParticles();
	DripParticle& getFreeDrip();
	void SpawnWetnessDrip(Vector3 pos, int room);
	void SpawnSplashDrips(Vector3 const& pos, int num, int room);
	void SpawnGunshellDrips(Vector3 const& pos, int room);
}
