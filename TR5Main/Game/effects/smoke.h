#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include <array>
struct ITEM_INFO;
namespace TEN{
	namespace Effects {
		namespace Smoke {
			struct SmokeParticle {
				DirectX::SimpleMath::Vector4 sourceColor;
				DirectX::SimpleMath::Vector4 destinationColor;
				DirectX::SimpleMath::Vector4 color;
				DirectX::SimpleMath::Vector3 position;
				DirectX::SimpleMath::Vector3 velocity;
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

			extern std::array<SmokeParticle, 128> SmokeParticles;

			void UpdateSmokeParticles();
			void TriggerFlareSmoke(const DirectX::SimpleMath::Vector3& pos, DirectX::SimpleMath::Vector3& direction, int age, int room);
			void TriggerGunSmokeParticles(int x, int y, int z, int xv, int yv, int zv, byte initial, int weaponType, byte count);
			void TriggerQuadExhaustSmoke(int x, int y, int z, short angle, int speed, int moving);
			void TriggerRocketSmoke(int x, int y, int z, int bodyPart);
		}
	}

}
