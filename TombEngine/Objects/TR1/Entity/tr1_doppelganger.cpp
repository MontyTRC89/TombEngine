#include "framework.h"
#include "Objects/TR1/Entity/tr1_doppelganger.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/misc.h"
#include "Specific/level.h"

namespace TEN::Entities::Creatures::TR1
{
	ItemInfo* FindReference(ItemInfo* item, short objectNumber)
	{
		for (int i = 0; i < g_Level.NumItems; i++)
		{
			auto* currentItem = &g_Level.Items[i];
			if (currentItem->ObjectNumber == objectNumber && item->TriggerFlags == currentItem->TriggerFlags)
			{
				return currentItem;
			}
		}
		return nullptr;
	}

	short GetWeaponDamage(LaraWeaponType weaponType)
	{
		return short(Weapons[(int)weaponType].Damage) * 10;
	}

	void DoppelgangerControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		if (item->HitPoints < LARA_HEALTH_MAX)
		{
			item->HitPoints = LARA_HEALTH_MAX;
			DoDamage(LaraItem, GetWeaponDamage(Lara.Control.Weapon.GunType));
		}

		auto* reference = FindReference(item, ID_BACON_REFERENCE);
		if (reference == nullptr)
		{
			TENLog("Doppelganger require ID_BACON_REFERENCE to be placed on the center (on floor) of the room to be used !", LogLevel::Warning);
			return;
		}

		switch (item->ItemFlags[7])
		{
		case 0:
		{
			int laraFloorHeight = GetCollision(LaraItem).Position.Floor;

			// Get floor heights for comparison.
			Vector3i pos(2 * reference->Pose.Position.x - LaraItem->Pose.Position.x, LaraItem->Pose.Position.y, 2 * reference->Pose.Position.z - LaraItem->Pose.Position.z);
			item->Floor = GetCollision(pos.x, pos.y, pos.z, item->RoomNumber).Position.Floor;

			// Animate bacon Lara, mirroring Lara's position.
			item->Animation.AnimNumber = LaraItem->Animation.AnimNumber;
			item->Animation.FrameNumber = LaraItem->Animation.FrameNumber;
			item->Pose.Position = pos;
			item->Pose.Orientation.x = LaraItem->Pose.Orientation.x;
			item->Pose.Orientation.y = LaraItem->Pose.Orientation.y - ANGLE(180.0f);
			item->Pose.Orientation.z = LaraItem->Pose.Orientation.z;
			item->Animation.IsAirborne = LaraItem->Animation.IsAirborne;

			// Compare floor heights.
			if (item->Floor >= (laraFloorHeight + SECTOR(1) + 1) && !LaraItem->Animation.IsAirborne)
			{
				SetAnimation(item, LA_FREEFALL);
				item->Animation.IsAirborne = true;
				item->Pose.Position.y += 64;
				item->ItemFlags[7] = 1;
			}
			break;
		}
		case 1:
			if (item->Animation.Velocity.x > 0.0f)
				item->Animation.Velocity.x -= 2;
			else if (item->Animation.Velocity.x < 0.0f)
				item->Animation.Velocity.x += 2;
			else
				item->Animation.Velocity.x = 0.0f;

			if (item->Animation.Velocity.z > 0.0f)
				item->Animation.Velocity.z -= 2;
			else if (item->Animation.Velocity.z < 0.0f)
				item->Animation.Velocity.z += 2;
			else
				item->Animation.Velocity.z = 0.0f;

			TestTriggers(item, true);
			item->Floor = GetCollision(item).Position.Floor;
			if (item->Pose.Position.y >= item->Floor)
			{
				item->Pose.Position.y = item->Floor;

				TestTriggers(item, true);
				SetAnimation(item, LA_FREEFALL_DEATH);

				item->Animation.IsAirborne = false;
				item->Animation.Velocity.y = 0.0f;

				const auto& anim = GetAnimData(*item);
				if (item->Animation.AnimNumber == LA_FREEFALL_DEATH &&
					item->Animation.FrameNumber >= anim.EndFrameNumber) // TODO: Check.
					item->ItemFlags[7] = 2;
			}

			break;

		case 2:
			DisableEntityAI(itemNumber);
			RemoveActiveItem(itemNumber);
			item->Collidable = FALSE;
			break;
		}

		ItemNewRoom(itemNumber, GetCollision(item).RoomNumber);
		AnimateItem(item);
	}
}
