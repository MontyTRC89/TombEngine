#pragma once
#include <array>
#include <unordered_map>
#include <d3d11.h>
#include <SimpleMath.h>
#include "Objects\objectslist.h"

struct ItemInfo;

namespace TEN::Effects
{
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
	};
	extern std::array<SimpleParticle, 15> simpleParticles;

	SimpleParticle& getFreeSimpleParticle();
	void TriggerSnowmobileSnow(ItemInfo* snowMobile);
	void TriggerSpeedboatFoam(ItemInfo* boat, Vector3 offset);
	void updateSimpleParticles();
}
