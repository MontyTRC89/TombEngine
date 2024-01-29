#include "framework.h"
#include "Objects/TR2/Entity/Bartoli.h"

#include "Game/camera.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Objects/game_object_ids.h"

// NOTES:
// item.ItemFlags[0]: Effect time counter in frame time.
// item.ItemFlags[1]: Timer for explosion 1 in frame time.
// item.ItemFlags[2]: Timer for explosion 2 in frame time.
// item.ItemFlags[3]: Timer for explosion 3 in frame time.

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto TRANSFORM_EFFECT_1_TIME = 100;
	constexpr auto TRANSFORM_EFFECT_2_TIME = 115;
	constexpr auto TRANSFORM_EFFECT_3_TIME = 130;

	constexpr auto EXPLOSION_LIVE_TIME = 100;

	void InitializeBartoli(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		InitializeCreature(itemNumber);

		if (item.ItemFlags[1] == 0)	item.ItemFlags[1] = TRANSFORM_EFFECT_1_TIME;
		if (item.ItemFlags[2] == 0)	item.ItemFlags[2] = TRANSFORM_EFFECT_2_TIME;
		if (item.ItemFlags[3] == 0)	item.ItemFlags[3] = TRANSFORM_EFFECT_3_TIME;

	}

	static void SpawnBartoliTransformEffect(const ItemInfo& item, GAME_OBJECT_ID objectID)
	{
		int explosionItemNumber = SpawnItem(item, objectID);

		//Activates the new item
		AddActiveItem(explosionItemNumber);
		auto& explosionItem = g_Level.Items[explosionItemNumber];
		explosionItem.Status = ITEM_ACTIVE;

		// Time before fading away.
		explosionItem.Timer = EXPLOSION_LIVE_TIME;

	}

	void ControlBartoli(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		short& effectTimer = item.ItemFlags[0];
		short& timeExplosion1 = item.ItemFlags[1];
		short& timeExplosion2 = item.ItemFlags[2];
		short& timeExplosion3 = item.ItemFlags[3];

		if (item.Animation.FrameNumber == 0)
		{
			if (!TriggerActive(&item))
				return;
		}

		AnimateItem(&item);

		effectTimer++;
		if ((effectTimer & 7) == 0)
			Camera.bounce = item.Timer;

		// Spawn light.
		auto lightPos = item.Pose.Position.ToVector3() + Vector3(0.0f, -CLICK(1), 0.0f);
		auto lightColor = Color(
			Random::GenerateFloat(0.8f, 0.9f),
			Random::GenerateFloat(0.4f, 0.5f),
			Random::GenerateFloat(0.2f, 0.3f));
		float lightFalloff = Random::GenerateFloat(0.1f, 0.4f);
		TriggerDynamicLight(lightPos, lightColor, lightFalloff);

		// Handle transformation.
		if (effectTimer == timeExplosion1)
		{
			SpawnBartoliTransformEffect(item, ID_SPHERE_OF_DOOM);
			SoundEffect(SFX_TR2_MARCO_BARTOLLI_TRANSFORM, &item.Pose);
		}
		if (effectTimer == timeExplosion2)
		{
			SpawnBartoliTransformEffect(item, ID_SPHERE_OF_DOOM2);
			SoundEffect(SFX_TR2_MARCO_BARTOLLI_TRANSFORM, &item.Pose);
		}
		if (effectTimer == timeExplosion3)
		{
			SpawnBartoliTransformEffect(item, ID_SPHERE_OF_DOOM3);
			SoundEffect(SFX_TR2_MARCO_BARTOLLI_TRANSFORM, &item.Pose);
			KillItem(itemNumber);

			TestTriggers(&item, true);

		}
	}

	void ControlBartoliTransformEffect(int itemNumber)
	{
		constexpr auto SCALE_RATE = Vector3(0.4f);
		constexpr auto OPACITY_CHANGE_RATE = 0.05f;

		auto& item = g_Level.Items[itemNumber];

		// Expand over time.
		if (item.Timer > 0)
		{
			item.Timer--;

			if (!item.Model.Mutators.empty())
				item.Model.Mutators[0].Scale += SCALE_RATE;
		}
		else
		{
			item.Model.Color.w -= OPACITY_CHANGE_RATE;
			if (item.Model.Color.w <= 0.0f)
				KillItem(itemNumber);
		}
	}
}
