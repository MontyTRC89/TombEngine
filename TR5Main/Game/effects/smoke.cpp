#include <algorithm>
#include "framework.h"
#include "Specific\trmath.h"
#include "smoke.h"
#include "room.h"
#include "control/control.h"
#include "level.h"
#include "setup.h"
#include "lara.h"
#include "Specific\prng.h"
#include "effects\weather.h"
#include "items.h"

using namespace TEN::Effects::Environment;
using namespace TEN::Math::Random;

namespace TEN {
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
							s.position.x += Weather.Wind().x;
							s.position.z += Weather.Wind().z;
						}
					}
					float normalizedLife = std::clamp(s.age / s.life,0.0f,1.0f);
					s.size = lerp(s.sourceSize, s.destinationSize, normalizedLife);
					s.angularVelocity *= s.angularDrag;
					s.rotation += s.angularVelocity;
					s.color = DirectX::SimpleMath::Vector4::Lerp(s.sourceColor, s.destinationColor, normalizedLife);
					int numSprites = -Objects[ID_SMOKE_SPRITES].nmeshes;
					s.sprite = lerp(0, numSprites - 1, normalizedLife);
				}
			}

			void TriggerFlareSmoke(const DirectX::SimpleMath::Vector3& pos, DirectX::SimpleMath::Vector3& direction, int age, int room) {
				using namespace DirectX::SimpleMath;
				SmokeParticle & s = getFreeSmokeParticle();
				s = {};
				s.position = pos;
				s.age = 0;
				constexpr float d = 0.2f;
				Vector3 randomDir = Vector3(GenerateFloat(-d, d), GenerateFloat(-d, d), GenerateFloat(-d, d));
				Vector3 dir;
				(direction + randomDir).Normalize(dir);
				s.velocity = dir * GenerateFloat(7, 9);
				s.gravity = -0.2f;
				s.friction = GenerateFloat(0.7f, 0.85f);
				s.sourceColor = Vector4(1, 131 / 255.0f, 100 / 255.0f, 1);
				s.destinationColor = Vector4(1, 1, 1, 0);
				s.life = GenerateFloat(25, 35);
				s.angularVelocity = GenerateFloat(-0.3f, 0.3f);
				s.angularDrag = 0.97f;
				s.sourceSize = age > 4 ? GenerateFloat(16, 24) : GenerateFloat(100, 128);
				s.destinationSize = age > 4 ? GenerateFloat(160, 200) : GenerateFloat(256, 300);
				s.affectedByWind = true;
				s.active = true;
				s.room = room;
			}
			//TODO: add additional "Weapon Special" param or something. Currently initial == 2 means Rocket Launcher backwards smoke.
			//TODO: Refactor different weapon types out of it
			void TriggerGunSmokeParticles(int x, int y, int z, int xv, int yv, int zv, byte initial, int weaponType, byte count)
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
					
					if(weaponType == LARA_WEAPON_TYPE::WEAPON_ROCKET_LAUNCHER){
						float size = GenerateFloat(48, 80);
						s.sourceSize = size * 2;
						
						s.destinationSize = size * 8;
						s.sourceColor = {0.75,0.75,1,1};
						s.terminalVelocity = 0;
						s.friction = 0.82f;
						s.life = GenerateFloat(60, 90);
						if(initial == 1){
							float size = GenerateFloat(48, 80);
							s.sourceSize = size * 2;
							s.destinationSize = size * 16;
							s.velocity = getRandomVectorInCone(dir,25);
							s.velocity *= GenerateFloat(0, 32);
						} else{
							float size = GenerateFloat(48, 80);
							s.sourceSize = size;
							s.destinationSize = size * 8;
							s.velocity = getRandomVectorInCone(dir,3);
							s.velocity *= GenerateFloat(0, 16);
						}
						
					} else{
						float size = GenerateFloat(48, 73);
						s.sourceSize = size * 2;
						s.destinationSize = size * 8;
						s.terminalVelocity = 0;
						s.friction = 0.88f;
						s.life = GenerateFloat(60, 90);
						s.velocity = getRandomVectorInCone(dir, 10);
						s.velocity *= GenerateFloat(16, 30);
					}
				}
				else
				{
					float size = (float)((GetRandomControl() & 0x0F) + 48); // -TriggerGunSmoke_SubFunction(weaponType);
					if(weaponType == LARA_WEAPON_TYPE::WEAPON_ROCKET_LAUNCHER){
						s.sourceColor = {0.75,0.75,1,1};

					}
					s.sourceSize = size / 2;
					s.destinationSize = size * 4;
					s.terminalVelocity = 0;
					s.friction = 0.97f;
					s.life = GenerateFloat(42, 62);
					s.velocity *= GenerateFloat(16, 40);
					
				}
				s.position = Vector3(x, y, z);
				s.position += Vector3(GenerateFloat(-8, 8), GenerateFloat(-8, 8), GenerateFloat(-8, 8));
				s.angularVelocity = GenerateFloat(-PI / 4, PI / 4);

				s.angularDrag = 0.95f;
				s.room = LaraItem->roomNumber;
				
			}

			void TriggerQuadExhaustSmoke(int x, int y, int z, short angle, int speed, int moving)
			{
				SmokeParticle& s = getFreeSmokeParticle();
				s = {};
				s.position = Vector3(x, y, z) + Vector3(GenerateFloat(8, 16), GenerateFloat(8, 16), GenerateFloat(8, 16));
				float xVel = std::sin(TO_RAD(angle))*speed;
				float zVel = std::cos(TO_RAD(angle))*speed;
				s.velocity = Vector3(xVel, GenerateFloat(-1, 4), zVel);
				s.sourceColor = Vector4(1, 1, 1, 1);
				s.destinationColor = Vector4(0, 0, 0, 0);
				s.sourceSize = GenerateFloat(8,24);
				s.active = true;
				s.affectedByWind = true;
				s.friction = 0.999f;
				s.gravity = -0.1f;
				s.life = GenerateFloat(16, 24);
				s.destinationSize = GenerateFloat(128, 160);
				s.angularVelocity = GenerateFloat(-1, 1);
				s.angularDrag = GenerateFloat(0.97, 0.999);
			}

			void TriggerRocketSmoke(int x, int y, int z, int bodyPart)
			{
				SmokeParticle& s = getFreeSmokeParticle();
				s = {};
				s.position = Vector3(x, y, z) + Vector3(GenerateFloat(8, 16), GenerateFloat(8, 16), GenerateFloat(8, 16));
				s.sourceColor = Vector4(0.8, 0.8, 1, 1);
				s.destinationColor = Vector4(0, 0, 0, 0);
				s.sourceSize = GenerateFloat(32, 64);
				s.active = true;
				s.velocity = getRandomVector() * GenerateFloat(1, 3);
				s.affectedByWind = true;
				s.friction = 0.979f;
				s.gravity = -0.1f;
				s.life = GenerateFloat(80, 120);
				s.destinationSize = GenerateFloat(1024, 1152);
				s.angularVelocity = GenerateFloat(-0.6, 0.6);
				s.angularDrag = GenerateFloat(0.87, 0.99);
			}
		}
	}
}
