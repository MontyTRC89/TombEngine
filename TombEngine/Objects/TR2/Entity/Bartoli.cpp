#include "framework.h"
#include "Objects/TR2/Entity/Bartoli.h"

#include "Game/camera.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Objects/game_object_ids.h"

// NOTES:
// item.ItemFlags[0]: Effect counter in frame time.

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto DRAGON_SPAWN_RANGE = BLOCK(9);

	constexpr auto DRAGON_EXPLOSION_1_TIME = 100;
	constexpr auto DRAGON_EXPLOSION_2_TIME = 115;
	constexpr auto DRAGON_EXPLOSION_3_TIME = 130;

	void InitializeBartoli(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		InitializeCreature(itemNumber);

		if (item.TriggerFlags == 0)
			item.Status = ITEM_INVISIBLE;
	}

	void ControlBartoli(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		
		if (item.Animation.FrameNumber == 0)
		{
			// Activate when within player's range.
			if (item.TriggerFlags == 0)
			{
				float distFromPlayer = Vector3i::Distance(LaraItem->Pose.Position, item.Pose.Position);
				if (distFromPlayer > DRAGON_SPAWN_RANGE)
					return;
			}
			// Activate by trigger.
			else
			{
				if (!TriggerActive(&item))
					return;
			}
		}
		
		AnimateItem(&item);

		short& effectCounter = item.ItemFlags[0];
		effectCounter++;
			
		if (!(effectCounter & 7))
			Camera.bounce = item.Timer;

		TriggerDynamicLight(
			item.Pose.Position.x, item.Pose.Position.y - CLICK(1), item.Pose.Position.z,
			(GetRandomControl() & 75) + 25,
			(GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 100, (GetRandomControl() & 20) + 50);
		
		if (effectCounter == DRAGON_EXPLOSION_1_TIME)
		{
			SpawnDragonExplosion(item, ID_SPHERE_OF_DOOM);
		}
		if (effectCounter == DRAGON_EXPLOSION_2_TIME)
		{
			SpawnDragonExplosion(item, ID_SPHERE_OF_DOOM2);
		}
		if (effectCounter == DRAGON_EXPLOSION_3_TIME)
		{
			SpawnDragonExplosion(item, ID_SPHERE_OF_DOOM3);

			// TODO: Spawn dragon.

			KillItem(itemNumber);
		}
	}

	void SpawnDragonExplosion(const ItemInfo& originItem, GAME_OBJECT_ID objectID)
	{
		int expItemNumber = CreateItem();
		auto& expItem = g_Level.Items[expItemNumber];
		
		expItem.ObjectNumber = objectID;
		expItem.Pose.Position = originItem.Pose.Position + Vector3i(0, CLICK(1), 0);
		expItem.RoomNumber = originItem.RoomNumber;
		expItem.Model.Color = originItem.Model.Color;

		InitializeItem(expItemNumber);
		AddActiveItem(expItemNumber);

		// Time before fading away.
		expItem.Timer = 100;
		expItem.Status = ITEM_ACTIVE;
	}

	void ControlDragonExplosion(int itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		// Expand over time.
		if (item.Timer > 0)
		{
			item.Timer--;
			if (!item.Model.Mutators.empty())
				item.Model.Mutators[0].Scale += Vector3(0.4f);
		}
		else
		{
			item.Model.Color.w -= 0.05f;
			if (item.Model.Color.w <= 0.0f)
				KillItem(itemNumber);
		}
	}
}
