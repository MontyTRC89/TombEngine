#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include "Specific/phd_global.h"

namespace TEN {
	namespace Effects {
		namespace Spark {
			struct SparkParticle {
				DirectX::SimpleMath::Vector3 pos;
				DirectX::SimpleMath::Vector3 velocity;
				DirectX::SimpleMath::Vector4 sourceColor;
				DirectX::SimpleMath::Vector4 destinationColor;
				DirectX::SimpleMath::Vector4 color;
				int room;
				float gravity;
				float friction;
				float age;
				float life;
				float width;
				float height;
				bool active;
			};

			extern std::array<SparkParticle,64> SparkParticles;
			void UpdateSparkParticles();
			SparkParticle& getFreeSparkParticle();
			void TriggerFlareSparkParticles(PHD_VECTOR* pos, PHD_VECTOR* vel, CVECTOR* color,int room);
			void TriggerRicochetSpark(GAME_VECTOR* pos, short angle, int num);
		}
	}
}