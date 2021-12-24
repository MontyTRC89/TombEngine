#pragma once
#include <array>
#include <d3d11.h>
#include <SimpleMath.h>

namespace TEN {
	namespace Effects {
		namespace Explosion {
			struct ExplosionParticle {
				DirectX::SimpleMath::Vector3 pos;
				DirectX::SimpleMath::Vector3 vel;
				DirectX::SimpleMath::Vector4 tint;
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
			void TriggerExplosion(DirectX::SimpleMath::Vector3 const & pos, float size, bool triggerSparks, bool triggerSmoke, bool triggerShockwave, int room);
			void UpdateExplosionParticles();
			ExplosionParticle& getFreeExplosionParticle();
			void SpawnExplosionParticle(DirectX::SimpleMath::Vector3 const & pos);
		}
	}
}