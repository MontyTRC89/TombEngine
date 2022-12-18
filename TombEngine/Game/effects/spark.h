#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include "Math/Math.h"

namespace TEN::Effects::Spark
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
	extern std::array<SparkParticle, 128> SparkParticles;
			
	void UpdateSparkParticles();
	SparkParticle& GetFreeSparkParticle();
	void TriggerFlareSparkParticles(const Vector3i& pos, const Vector3i& vel, const ColorData& color, int roomNumber);
	void TriggerRicochetSpark(const GameVector& pos, short angle, int num);
	void TriggerFrictionSpark(const GameVector& pos, const EulerAngles& angle, float length, int count);
	void TriggerElectricSpark(const GameVector& pos, const EulerAngles& angle, int count);
	void TriggerAttackSpark(const Vector3& basePos, const Vector3& color);
	void SpawnCyborgSpark(const Vector3& pos);
}
