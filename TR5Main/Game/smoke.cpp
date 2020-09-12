#include "framework.h"
#include "trmath.h"
#include "smoke.h"
#include "room.h"
#include "control.h"
#include "level.h"
#include "setup.h"
#include "lara.h"
namespace T5M {
	namespace Effects {
		namespace Smoke {
			std::array<SmokeParticle, 128> SmokeParticles;
			SmokeParticle& getFreeSmokeParticle()
			{
				for(int i = 0; i < SmokeParticles.size(); i++){
					if(!SmokeParticles[i].active)
						return SmokeParticles[i];
				}
				return SmokeParticles[0];
			}
			void UpdateSmokeParticles()
			{
				for (int i = 0; i < SmokeParticles.size(); i++) {
					SmokeParticle& s = SmokeParticles[i];
					if (!s.active)continue;
					s.age += 1;
					if (s.age > s.life) {
						s.active = false;
						continue;
					}
					s.velocity.y += s.gravity;
					if (s.terminalVelocity != 0) {
						float velocityLength = s.velocity.Length();
						if (velocityLength > s.terminalVelocity) {
							s.velocity *= (s.terminalVelocity / velocityLength);
						}
					}
					s.position += s.velocity;
					if (s.affectedByWind) {
						if (g_Level.Rooms[s.room].flags & ENV_FLAG_WIND) {
							s.position.x += SmokeWindX / 2;
							s.position.z += SmokeWindZ / 2;
						}
					}
					float normalizedLife = s.age / s.life;
					s.size = lerp(s.sourceSize, s.destinationSize, normalizedLife);
					s.angularVelocity *= s.angularDrag;
					s.rotation += s.angularVelocity;
					s.color = DirectX::SimpleMath::Vector4::Lerp(s.sourceColor, s.destinationColor, normalizedLife);
				}
			}

			void TriggerFlareSmoke(const DirectX::SimpleMath::Vector3& pos, DirectX::SimpleMath::Vector3& direction, int age, int room) {
				using namespace DirectX::SimpleMath;
				SmokeParticle& const s = getFreeSmokeParticle();
				s = {};
				s.position = pos;
				s.age = 0;
				constexpr float d = 0.2f;
				Vector3 randomDir = Vector3(frandMinMax(-d, d), frandMinMax(-d, d), frandMinMax(-d, d));
				Vector3 dir;
				(direction + randomDir).Normalize(dir);
				s.velocity = dir * frandMinMax(7, 9);
				s.gravity = -0.2f;
				s.friction = frandMinMax(0.7f, 0.85f);
				s.sourceColor = Vector4(1, 131 / 255.0f, 100 / 255.0f, 1);
				s.destinationColor = Vector4(0, 0, 0, 0);
				s.life = frandMinMax(25, 35);
				s.angularVelocity = frandMinMax(-0.3f, 0.3f);
				s.angularDrag = 0.98f;
				s.sourceSize = age > 4 ? frandMinMax(16, 24) : frandMinMax(100, 128);
				s.destinationSize = age > 4 ? frandMinMax(160, 200) : frandMinMax(256, 300);
				s.affectedByWind = true;
				s.active = true;
				int numSprites = -Objects[ID_SMOKE_SPRITES].nmeshes;
				s.sprite = lerp(0, numSprites - 1, s.age / s.life);
				s.room = room;
			}

			void TriggerGunSmokeParticles(int x, int y, int z, short xv, short yv, short zv, byte initial, int weaponType, byte count)
			{
				SmokeParticle& s = getFreeSmokeParticle();
				s = {};
				s.active = true;
				s.position = Vector3(x, y, z);
				Vector3 dir = Vector3(xv, yv, zv);
				dir.Normalize();
				s.velocity = dir;
				s.gravity = -.1f;
				s.affectedByWind = g_Level.Rooms[LaraItem->roomNumber].flags & ENV_FLAG_WIND;
				s.sourceColor = Vector4(.4, .4, .4, 1);
				s.destinationColor = Vector4(0, 0, 0, 0);

				if (initial)
				{
					float size = (frand() * 25) + 48;
					s.sourceSize = size *2;
					s.destinationSize = size * 8;
					s.terminalVelocity = 0;
					s.friction = 0.88f;
					s.life = frand() * 30 + 60;
					s.velocity = getRandomVectorInCone(s.velocity, 10);
					s.velocity *= frand() * 14 + 16;
				}
				else
				{
					float size = (float)((GetRandomControl() & 0x0F) + 48); // -TriggerGunSmoke_SubFunction(weaponType);

					s.sourceSize = size / 2;
					s.destinationSize = size * 4;
					s.terminalVelocity = 0;
					s.friction = 0.97f;
					s.life = frand() * 20 + 42;
					s.velocity *= frand() * 24 + 16;
					
				}
				s.position = Vector3(x, y, z);
				s.position += Vector3(frandMinMax(-8, 8), frandMinMax(-8, 8), frandMinMax(-8, 8));
				s.angularVelocity = frandMinMax(-PI / 4, PI / 4);

				s.angularDrag = 0.95f;
				s.room = LaraItem->roomNumber;
				
			}

			void TriggerQuadExhaustSmoke(int x, int y, int z, short angle, int speed, int moving)
			{
				SmokeParticle& s = getFreeSmokeParticle();
				s = {};
				s.position = Vector3(x, y, z) + Vector3(frandMinMax(8, 16), frandMinMax(8, 16), frandMinMax(8, 16));
				float xVel = std::sin(TO_RAD(angle))*speed;
				float zVel = std::cos(TO_RAD(angle))*speed;
				s.velocity = Vector3(xVel, frandMinMax(-1, 4), zVel);
				s.sourceColor = Vector4(1, 1, 1, 1);
				s.destinationColor = Vector4(0, 0, 0, 0);
				s.sourceSize = frandMinMax(8,24);
				s.active = true;
				s.affectedByWind = true;
				s.friction = 0.999f;
				s.gravity = -0.1f;
				s.life = frandMinMax(16, 24);
				s.destinationSize = 128 + frand()*32;
				s.angularVelocity = frandMinMax(-1, 1);
				s.angularDrag = frandMinMax(0.97, 0.999);
			}

		}
	}
}
