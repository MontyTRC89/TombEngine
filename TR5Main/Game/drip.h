#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include <array>
namespace T5M {
	namespace Effects {
		namespace Drip {
			constexpr float DRIP_LIFE = 25.0f;
			constexpr float DRIP_LIFE_LONG = 60.0f;
			constexpr float DRIP_WIDTH = 4;
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
			extern std::array<DripParticle, 256> dripParticles;
			void UpdateDrips();
			DripParticle& getFreeDrip();
			void SpawnWetnessDrip(DirectX::SimpleMath::Vector3 pos, int room);
			void SpawnSplashDrips(Vector3& pos, int num, int room);
		}
	}
}