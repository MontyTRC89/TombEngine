#pragma once
#include "framework.h"
#include "Game/effects/spark.h"

#include <array>
#include "Specific/trmath.h"
#include "Specific/prng.h"

using namespace DirectX::SimpleMath;
using namespace TEN::Math::Random;

namespace TEN
{
	namespace Effects
	{
		namespace Spark
		{
			std::array<SparkParticle, 64> SparkParticles;
			void UpdateSparkParticles()
			{
				for (int i = 0; i < SparkParticles.size(); i++)
				{
					auto& spark = SparkParticles[i];

					if (!spark.active)
						continue;

					spark.age += 1;
					if (spark.age > spark.life)
					{
						spark.active = false;
						continue;
					}

					spark.velocity.y += spark.gravity;
					spark.velocity *= spark.friction;
					spark.pos += spark.velocity;

					float normalizedLife = spark.age / spark.life;
					spark.height = lerp(spark.width / 0.15625, 0, normalizedLife);
					spark.color = DirectX::SimpleMath::Vector4::Lerp(spark.sourceColor, spark.destinationColor, normalizedLife);
				}
			}

			SparkParticle& getFreeSparkParticle()
			{
				for (int i = 0; i < SparkParticles.size(); i++)
				{
					if (!SparkParticles[i].active)
						return SparkParticles[i];
				}

				return SparkParticles[0];
			}

			void TriggerFlareSparkParticles(Vector3Int* pos, Vector3Int* velocity, CVECTOR* color, int room)
			{
				auto& spark = getFreeSparkParticle();

				spark = {};
				spark.age = 0;
				spark.life = GenerateFloat(10, 20);
				spark.friction = 0.98f;
				spark.gravity = 1.2f;
				spark.width = 8;
				spark.room = room;
				spark.pos = Vector3(pos->x, pos->y, pos->z);

				Vector3 vector = Vector3(velocity->x, velocity->y, velocity->z);
				vector += Vector3(GenerateFloat(-64, 64), GenerateFloat(-64, 64), GenerateFloat(-64, 64));
				vector.Normalize(vector);

				spark.velocity = vector *GenerateFloat(17,24);
				spark.sourceColor = Vector4(1, 1, 1, 1);
				spark.destinationColor = Vector4(color->r / 255.0f , color->g / 255.0f, color->b / 255.0f, 1);
				spark.active = true;
			}

			void TriggerRicochetSpark(GameVector* pos, float angle, int num)
			{
				for (int i = 0; i < num; i++)
				{
					auto& spark = getFreeSparkParticle();

					spark = {};
					spark.age = 0;
					spark.life = GenerateFloat(10, 20);
					spark.friction = 0.98f;
					spark.gravity = 1.2f;
					spark.width = 8;
					spark.room = pos->roomNumber;
					spark.pos = Vector3(pos->x, pos->y, pos->z);

					Vector3 vector = Vector3(sin(angle + GenerateFloat(-M_PI / 2, M_PI / 2)), GenerateFloat(-1.0f, 1.0f), cos(angle + GenerateFloat(-M_PI / 2, M_PI / 2)));
					vector += Vector3(GenerateFloat(-64.0f, 64.0f), GenerateFloat(-64.0f, 64.0f), GenerateFloat(-64.0f, 64.0f));
					vector.Normalize(vector);

					spark.velocity = vector * GenerateFloat(17.0f, 24.0f);
					spark.sourceColor = Vector4(1, 0.8, 0.2f, 1) * 3;
					spark.destinationColor = Vector4(0, 0, 0, 0);
					spark.active = true;
				}
			}
		}
	}
}
