#pragma once

#include "Objects\game_object_ids.h"

enum class BlendMode;
struct ItemInfo;

namespace TEN::Effects
{
	struct SimpleParticle
	{
		GAME_OBJECT_ID SpriteSeqAssetID = GAME_OBJECT_ID::ID_DEFAULT_SPRITES;
		int			   SpriteAssetID	= 0;

		Vector3 worldPosition;
		float size;
		float age;
		float ageRate;
		float life;
		int room;
		bool active;
		BlendMode blendMode;
	};
	extern std::array<SimpleParticle, 15> simpleParticles;

	SimpleParticle& GetFreeSimpleParticle();
	void TriggerSnowmobileSnow(ItemInfo* snowMobile);
	void TriggerSpeedboatFoam(ItemInfo* boat, Vector3 offset);
	void UpdateSimpleParticles();
}
