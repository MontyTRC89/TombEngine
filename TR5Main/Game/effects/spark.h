#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include "Specific/phd_global.h"

namespace TEN
{
	namespace Effects
	{
		namespace Spark
		{
			struct SparkParticle
			{
				Vector3 pos;
				Vector3 velocity;
				Vector4 sourceColor;
				Vector4 destinationColor;
				Vector4 color;
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
			void TriggerFlareSparkParticles(Vector3Int* pos, Vector3Int* velocity, CVECTOR* color, int room);
			void TriggerRicochetSpark(GameVector* pos, float angle, int num);
		}
	}
}
