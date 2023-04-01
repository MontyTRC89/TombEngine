#pragma once
#include "Objects\objectslist.h"

enum BLEND_MODES;
struct ItemInfo;

namespace TEN::Effects
{
	constexpr auto SIMPLE_PARTICLE_NUM_MAX = 15;

	struct SimpleParticle
	{
		DirectX::SimpleMath::Vector3 worldPosition;
		float size;
		float age;
		float ageRate;
		float life;
		int room;
		unsigned int sprite;
		GAME_OBJECT_ID sequence;
		bool active;
		BLEND_MODES blendMode;
	};

	extern std::array<SimpleParticle, SIMPLE_PARTICLE_NUM_MAX> simpleParticles;

	SimpleParticle& GetFreeSimpleParticle();
	void TriggerSnowmobileSnow(ItemInfo* snowMobile);
	void TriggerSpeedboatFoam(ItemInfo* boat, Vector3 offset);
	void UpdateSimpleParticles();
}
