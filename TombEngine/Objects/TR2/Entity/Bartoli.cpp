#include "framework.h"
#include "Objects/TR2/Entity/Bartoli.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"



namespace TEN::Entities::Creatures::TR2
{
	constexpr auto DRAGON_SPAWN_RANGE = BLOCK(9);
	constexpr auto EXPLOSION_TIME = 100;
	constexpr auto EXPLOSION_TIME_MIDDLE = 115;
	constexpr auto EXPLOSION_TIME_END = 130;


	void InitializeBartoli (short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.Pose.Position.x -= CLICK(2);
		item.Pose.Position.z -= CLICK(2);

		InitializeCreature(itemNumber);

		if (item.TriggerFlags == 0)
			item.Status = ITEM_INVISIBLE;
	}

	void ControlBartoli (short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		
		if (item.Animation.FrameNumber == 0)
		{
			if (item.TriggerFlags == 0)
			{
				//Trigger makes Bartoli appears. Distance activate it
				auto distance = Vector3i::Distance(LaraItem->Pose.Position, item.Pose.Position);
				if ( distance > DRAGON_SPAWN_RANGE)
					return;
			}
			else
			{
				//Level Loading makes Bartoli appears. Trigger activates it
				if (!TriggerActive(&item))
					return;
			}
		}
		
		AnimateItem(&item);

		//Effects
		auto& frameCounter = item.ItemFlags[0];
		frameCounter++;
			
		if (!(frameCounter & 7))
			Camera.bounce = item.Timer;

		TriggerDynamicLight(
			item.Pose.Position.x, item.Pose.Position.y - CLICK(1), item.Pose.Position.z,
			(GetRandomControl() & 75) + 25,
			(GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 100, (GetRandomControl() & 20) + 50);
		
		if (frameCounter == EXPLOSION_TIME)
		{
			SpawnDragonBlast(itemNumber, ID_SPHERE_OF_DOOM);
		}
		if (frameCounter == EXPLOSION_TIME_MIDDLE)
		{
			SpawnDragonBlast(itemNumber, ID_SPHERE_OF_DOOM2);
		}
		if (frameCounter == EXPLOSION_TIME_END)
		{
			SpawnDragonBlast(itemNumber, ID_SPHERE_OF_DOOM3);

			//Pending Spawn dragon

			KillItem(itemNumber); //Kill Bartoli object
		}
	}

	void SpawnDragonBlast(short sourceItemNumber, short ObjectToSpawn)
	{
		auto& sourceItem = g_Level.Items[sourceItemNumber];

		int SphereObjNumber;
		ItemInfo* SphereObjPtr = nullptr;
		

		SphereObjNumber = CreateItem();
		SphereObjPtr = &g_Level.Items[SphereObjNumber];
		
		SphereObjPtr->ObjectNumber = GAME_OBJECT_ID(ObjectToSpawn);
		SphereObjPtr->Pose.Position = sourceItem.Pose.Position + Vector3i(0, CLICK(1), 0);
		SphereObjPtr->RoomNumber = sourceItem.RoomNumber;
		SphereObjPtr->Model.Color = sourceItem.Model.Color;

		InitializeItem(SphereObjNumber);
		AddActiveItem(SphereObjNumber);

		// Time before fading away.
		SphereObjPtr->Timer = 100;
		SphereObjPtr->Status = ITEM_ACTIVE;
	}

	void ControlDragonBlast(short itemNumber)
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