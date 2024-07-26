#pragma once
#include "Objects\objectslist.h"

enum class BlendMode;
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
		unsigned int SpriteID;
		GAME_OBJECT_ID SpriteSeqAssetID;
		bool active;
		BlendMode blendMode;
	};
	extern std::array<SimpleParticle, 15> simpleParticles;

	SimpleParticle& GetFreeSimpleParticle();
	void TriggerSnowmobileSnow(ItemInfo* snowMobile);
	void TriggerSpeedboatFoam(ItemInfo* boat, Vector3 offset);
	void UpdateSimpleParticles();
}
