#include "framework.h"
#include "Objects/TR2/Entity/Bartoli.h"

#include "Game/camera.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Objects/game_object_ids.h"

// NOTES:
// item.ItemFlags[0]: Effect timer in frame time.
// item.ItemFlags[1]: Object ID of item to transform into. Default: ID_DRAGON_FRONT.

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto TRANSFORM_SPAWN_RANGE = BLOCK(9);

	constexpr auto TRANSFORM_EFFECT_1_TIME = 100;
	constexpr auto TRANSFORM_EFFECT_2_TIME = 115;
	constexpr auto TRANSFORM_EFFECT_3_TIME = 130;

	void InitializeBartoli(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		InitializeCreature(itemNumber);

		if (item.TriggerFlags == 0)
			item.Status = ITEM_INVISIBLE;
	}

	static void SpawnBartoliTransformEffect(const ItemInfo& item, GAME_OBJECT_ID objectID)
	{
		int expItemNumber = CreateItem();
		auto& expItem = g_Level.Items[expItemNumber];

		expItem.ObjectNumber = objectID;
		expItem.Pose.Position = item.Pose.Position + Vector3i(0, CLICK(1), 0);
		expItem.RoomNumber = item.RoomNumber;
		expItem.Model.Color = item.Model.Color;

		InitializeItem(expItemNumber);
		AddActiveItem(expItemNumber);

		// Time before fading away.
		expItem.Timer = 100;
		expItem.Status = ITEM_ACTIVE;
	}

	void ControlBartoli(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		short& effectTimer = item.ItemFlags[0];
		short& transformObjectID = item.ItemFlags[1];
		
		if (item.Animation.FrameNumber == 0)
		{
			// Activate when within player's range.
			if (item.TriggerFlags == 0)
			{
				float distFromPlayer = Vector3i::Distance(LaraItem->Pose.Position, item.Pose.Position);
				if (distFromPlayer > TRANSFORM_SPAWN_RANGE)
					return;
			}
			// Activate via trigger.
			else
			{
				if (!TriggerActive(&item))
					return;
			}

			transformObjectID = ID_DRAGON_FRONT;
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
		SpawnDynamicLight(lightPos, lightColor, lightFalloff);
		
		// Handle transformation.
		if (effectTimer == TRANSFORM_EFFECT_1_TIME)
		{
			SpawnBartoliTransformEffect(item, ID_SPHERE_OF_DOOM);
		}
		if (effectTimer == TRANSFORM_EFFECT_2_TIME)
		{
			SpawnBartoliTransformEffect(item, ID_SPHERE_OF_DOOM2);
		}
		if (effectTimer == TRANSFORM_EFFECT_3_TIME)
		{
			SpawnBartoliTransformEffect(item, ID_SPHERE_OF_DOOM3);
			KillItem(itemNumber);

			int transformItemNumber = CreateItem();
			if (transformItemNumber == NO_ITEM)
				return;

			auto& transformItem = g_Level.Items[transformItemNumber];

			transformItem.ObjectNumber = (GAME_OBJECT_ID)transformObjectID;
			transformItem.Pose = item.Pose;
			transformItem.RoomNumber = item.RoomNumber;
			transformItem.Model.Color = Vector4::One;

			InitializeItem(transformItemNumber);
			AddActiveItem(transformItemNumber);
		}
	}

	void ControlBartoliTransformEffect(int itemNumber)
	{
		constexpr auto SCALE_RATE		   = Vector3(0.4f);
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
