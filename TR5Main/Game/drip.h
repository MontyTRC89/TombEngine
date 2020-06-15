#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include <array>
namespace T5M {
	namespace Effects {
		namespace Drip {
			constexpr float DRIP_LIFE = 25.0f;
			constexpr float DRIP_WIDTH = 4;
			struct DripParticle {
				DirectX::SimpleMath::Vector3 pos;
				DirectX::SimpleMath::Vector3 velocity;
				DirectX::SimpleMath::Vector4 color;
				int room;
				float gravity;
				float age;
				float height;
				bool active;
			};
			extern std::array<DripParticle, 128> dripParticles;
			void UpdateDrips();
			DripParticle& getFreeDrip();
			void SpawnDrip(DirectX::SimpleMath::Vector3 pos, int room);
		}
	}
}