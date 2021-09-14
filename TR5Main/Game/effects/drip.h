#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include <array>
namespace TEN {
	namespace Effects {
		namespace Drip {
			constexpr float DRIP_LIFE = 25.0f;
			constexpr float DRIP_LIFE_LONG = 120.0f;
			constexpr float DRIP_WIDTH = 4;
			constexpr int NUM_DRIPS = 512;
			struct DripParticle {
				DirectX::SimpleMath::Vector3 pos;
				DirectX::SimpleMath::Vector3 velocity;
				DirectX::SimpleMath::Vector4 color;
				int room;
				float gravity;
				float life;
				float age;
				float height;
				bool active;
			};
			extern std::array<DripParticle, NUM_DRIPS> dripParticles;
			void UpdateDrips();
			DripParticle& getFreeDrip();
			void SpawnWetnessDrip(DirectX::SimpleMath::Vector3 pos, int room);
			void SpawnSplashDrips(Vector3 const & pos, int num, int room);
			void SpawnGunshellDrips(Vector3 const & pos, int room);
		}
	}
}