#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include "Math/Math.h"

namespace TEN::Effects::Spark
{
	constexpr auto SPARK_NUM_MAX = 128;

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

	extern std::array<SparkParticle, SPARK_NUM_MAX> SparkParticles;
			
	void UpdateSparkParticles();
	SparkParticle& GetFreeSparkParticle();
	void TriggerFlareSparkParticles(Vector3i* pos, Vector3i* vel, ColorData* color, int room);
	void TriggerRicochetSpark(GameVector* pos, short angle, int num);
	void TriggerFrictionSpark(GameVector* pos, EulerAngles angle, float length, int num);
}
