#pragma once
#include "Objects\objectslist.h"
#include <SimpleMath.h>

enum class BlendMode;
struct ItemInfo;

namespace TEN::Effects
{
	constexpr auto SIMPLE_PARTICLE_NUM_MAX = 15;

	struct SimpleParticle
	{
		Vector3 worldPosition;
		float size;
		float age;
		float ageRate;
		float life;
		int room;
		unsigned int sprite;
		GAME_OBJECT_ID sequence;
		bool active;
		BlendMode blendMode;

		Vector3 PrevWorldPosition = Vector3::Zero;
		float	PrevSize		  = 0.0f;

		void StoreInterpolationData()
		{
			PrevWorldPosition = worldPosition;
			PrevSize = size;
		}
	};

	extern std::array<SimpleParticle, SIMPLE_PARTICLE_NUM_MAX> simpleParticles;

	SimpleParticle& GetFreeSimpleParticle();
	void TriggerSnowmobileSnow(ItemInfo* snowMobile);
	void TriggerSpeedboatFoam(ItemInfo* boat, Vector3 offset);
	void UpdateSimpleParticles();
}
