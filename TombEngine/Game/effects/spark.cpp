#pragma once
#include "framework.h"
#include "Game/effects/spark.h"

#include <array>
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Math/Math.h"
#include "Math/Random.h"

using namespace DirectX::SimpleMath;
using namespace TEN::Math::Random;

namespace TEN::Effects::Spark
{
	std::array<SparkParticle, 128> SparkParticles;
	void UpdateSparkParticles()
	{
		for (int i = 0; i < SparkParticles.size(); i++)
		{
			auto& s = SparkParticles[i];

			if (!s.active)
				continue;

			s.age += 1;
			if (s.age > s.life)
			{
				s.active = false;
				continue;
			}

			s.velocity.y += s.gravity;
			s.velocity *= s.friction;
			s.pos += s.velocity;
		}
	}

	SparkParticle& GetFreeSparkParticle()
	{
		for (int i = 0; i < SparkParticles.size(); i++)
		{
			if (!SparkParticles[i].active)
				return SparkParticles[i];
		}

		return SparkParticles[0];
	}

	void TriggerFlareSparkParticles(Vector3i* pos, Vector3i* vel, ColorData* color, int room)
	{
		auto& s = GetFreeSparkParticle();
		s = {};
		s.age = 0;
		s.life = GenerateFloat(10, 20);
		s.friction = 0.98f;
		s.gravity = 1.2f;
		s.width = 8.0f;
		s.height = 48.0f;
		s.room = room;
		s.pos = pos->ToVector3();
		auto v = vel->ToVector3();
		v += Vector3(GenerateFloat(-64, 64), GenerateFloat(-64, 64), GenerateFloat(-64, 64));
		v.Normalize(v);
		s.velocity = v *GenerateFloat(17,24);
		s.sourceColor = Vector4(1, 1, 1, 1);
		s.destinationColor = Vector4(color->r / 255.0f, color->g / 255.0f, color->b / 255.0f, 1.0f);
		s.active = true;
	}

	void TriggerRicochetSpark(GameVector* pos, short angle, int num)
	{
		for (int i = 0; i < num; i++) 
		{
			auto& s = GetFreeSparkParticle();
			s = {};
			s.age = 0;
			s.life = GenerateFloat(10, 20);
			s.friction = 0.98f;
			s.gravity = 1.2f;
			s.width = 8.0f;
			s.height = 64.0f;
			s.room = pos->RoomNumber;
			s.pos = Vector3(pos->x, pos->y, pos->z);
			float ang = TO_RAD(angle);
			Vector3 v = Vector3(sin(ang + GenerateFloat(-PI / 2, PI / 2)), GenerateFloat(-1, 1), cos(ang + GenerateFloat(-PI / 2, PI / 2)));
			v += Vector3(GenerateFloat(-64, 64), GenerateFloat(-64, 64), GenerateFloat(-64, 64));
			v.Normalize(v);
			s.velocity = v * GenerateFloat(17, 24);
			s.sourceColor = Vector4(1, 0.8f, 0.2f, 1) * 3;
			s.destinationColor = Vector4(0, 0, 0, 0);
			s.active = true;
		}
	}

	void TriggerFrictionSpark(GameVector* pos, EulerAngles angle, float length, int num)
	{
		for (int i = 0; i < num; i++) 
		{
			auto& s = GetFreeSparkParticle();
			s = {};
			s.age = 0;
			s.life = GenerateFloat(8, 15);
			s.friction = 0.1f;
			s.gravity = 0.0f;
			s.height = length;
			s.width = GenerateFloat(16.0f, 32.0f);
			s.room = pos->RoomNumber;
			s.pos = Vector3(pos->x + GenerateFloat(-16, 16), pos->y + GenerateFloat(-16, 16), pos->z + GenerateFloat(-16, 16));
			float ang = TO_RAD(angle.y);
			float vAng = -TO_RAD(angle.x);
			Vector3 v = Vector3(sin(ang), vAng + GenerateFloat(-PI / 16, PI / 16), cos(ang));
			v.Normalize(v);
			s.velocity = v * GenerateFloat(32, 64);
			s.sourceColor = Vector4(1, 0.7f, 0.4f, 1) * 3;
			s.destinationColor = Vector4(0.4f, 0.1f, 0, 0.5f);
			s.active = true;
		}
	}

	void TriggerElectricSpark(GameVector* pos, EulerAngles angle, int num)
	{
		for (int i = 0; i < num; i++)
		{
			auto& s = GetFreeSparkParticle();
			s = {};
			s.age = 0;
			s.life = GenerateFloat(8, 15);
			s.friction = 1.0f;
			s.gravity = 2.0f;
			s.height = GenerateFloat(64.0f, 256.0f);
			s.width = GenerateFloat(8.0f, 16.0f);
			s.room = pos->RoomNumber;
			s.pos = Vector3(pos->x + GenerateFloat(-16, 16), pos->y + GenerateFloat(-16, 16), pos->z + GenerateFloat(-16, 16));
			float ang = TO_RAD(angle.y);
			float vAng = -TO_RAD(angle.x);
			Vector3 v = Vector3(sin(ang), vAng + GenerateFloat(-PI / 16, PI / 16), cos(ang));
			v.Normalize(v);
			s.velocity = v * GenerateFloat(8, 32);
			s.sourceColor = Vector4(0.4f, 0.6f, 1.0f, 1);
			s.destinationColor = Vector4(0.6f, 0.6f, 0.8f, 0.8f);
			s.active = true;
		}
	}

	void TriggerAttackSpark(SparkOffsets* pose, const Vector3& color)
	{
		auto& spark = *GetFreeParticle();

		spark.on = 1;
		spark.sR = 0;
		spark.sG = 0;
		spark.sB = 0;
		spark.dR = color.x;
		spark.dG = color.y;
		spark.dB = color.z;
		spark.life = 16;
		spark.sLife = 16;
		spark.colFadeSpeed = 4;
		spark.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		spark.fadeToBlack = 4;
		spark.x = pose->positionA.x;
		spark.y = pose->positionA.y;
		spark.z = pose->positionA.z;
		spark.xVel = pose->positionB.x;
		spark.yVel = pose->positionB.y;
		spark.zVel = pose->positionB.z;
		spark.friction = 34;
		spark.maxYvel = 0;
		spark.gravity = 0;
		spark.flags = SP_NONE;
	}
}
