#pragma once
#include "framework.h"
#include "Game/effects/spark.h"

#include <array>
#include "Specific/trmath.h"
#include "Specific/prng.h"

using namespace DirectX::SimpleMath;
using namespace TEN::Math::Random;

namespace TEN {
	namespace Effects {
		namespace Spark {
			std::array<SparkParticle, 64> SparkParticles;
			void UpdateSparkParticles()
			{
				for (int i = 0; i < SparkParticles.size(); i++) {
					SparkParticle& s = SparkParticles[i];
					if (!s.active)continue;
					s.age += 1;
					if (s.age > s.life) {
						s.active = false;
						continue;
					}
					s.velocity.y += s.gravity;
					s.velocity *= s.friction;
					s.pos += s.velocity;
					float normalizedLife = s.age / s.life;
					s.height = lerp(s.width / 0.15625, 0, normalizedLife);
					s.color = DirectX::SimpleMath::Vector4::Lerp(s.sourceColor, s.destinationColor, normalizedLife);
				}
			}

			SparkParticle& getFreeSparkParticle()
			{
				for (int i = 0; i < SparkParticles.size(); i++)
				{
					if (!SparkParticles[i].active) {
						return SparkParticles[i];
					}
				}
				return SparkParticles[0];
			}

			void TriggerFlareSparkParticles(PHD_VECTOR* pos, PHD_VECTOR* vel, CVECTOR* color,int room)
			{
				SparkParticle& s = getFreeSparkParticle();
				s = {};
				s.age = 0;
				s.life = GenerateFloat(10, 20);
				s.friction = 0.98f;
				s.gravity = 1.2f;
				s.width = 8;
				s.room = room;
				s.pos = Vector3(pos->x, pos->y, pos->z);
				Vector3 v = Vector3(vel->x, vel->y, vel->z);
				v += Vector3(GenerateFloat(-64, 64), GenerateFloat(-64, 64), GenerateFloat(-64, 64));
				v.Normalize(v);
				s.velocity = v *GenerateFloat(17,24);
				s.sourceColor = Vector4(1, 1, 1, 1);
				s.destinationColor = Vector4(color->r/255.0f,color->g/255.0f,color->b/255.0f,1);
				s.active = true;
			}

			void TriggerRicochetSpark(GAME_VECTOR* pos, short angle, int num)
			{
				for (int i = 0; i < num; i++) {
					SparkParticle& s = getFreeSparkParticle();
					s = {};
					s.age = 0;
					s.life = GenerateFloat(10, 20);
					s.friction = 0.98f;
					s.gravity = 1.2f;
					s.width = 8;
					s.room = pos->roomNumber;
					s.pos = Vector3(pos->x, pos->y, pos->z);
					float ang = TO_RAD(angle);
					Vector3 v = Vector3(sin(ang + GenerateFloat(-PI / 2, PI / 2)), GenerateFloat(-1, 1), cos(ang + GenerateFloat(-PI / 2, PI / 2)));
					v += Vector3(GenerateFloat(-64, 64), GenerateFloat(-64, 64), GenerateFloat(-64, 64));
					v.Normalize(v);
					s.velocity = v * GenerateFloat(17, 24);
					s.sourceColor = Vector4(1, 0.8, 0.2f, 1) * 3;
					s.destinationColor = Vector4(0, 0, 0, 0);
					s.active = true;
				}
			}
		}
	}
}

