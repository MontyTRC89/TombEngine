#include "framework.h"
#include "Objects/TR1/Entity/tr1_doppelganger.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/misc.h"
#include "Specific/level.h"

// TODO:
// - Bacon Lara cannot be targeted.
// - Bacon Lara cannot move like Lara.

namespace TEN::Entities::TR1
{
	// Original:
	void InitialiseDoppelganger(short itemNumber)
	{
		ClearItem(itemNumber);
	}

	ItemInfo* FindReference(ItemInfo* item, short objectNumber)
	{
		bool found = false;
		int itemNumber;
		for (int i = 0; i < g_Level.NumItems; i++)
		{
			auto* currentItem = &g_Level.Items[i];
			if (currentItem->ObjectNumber == objectNumber && currentItem->RoomNumber == item->RoomNumber)
			{
				itemNumber = i;
				found = true;
			}
		}

		if (!found)
			itemNumber = NO_ITEM;

		return (itemNumber == NO_ITEM ? NULL : &g_Level.Items[itemNumber]);
	}

	static short GetWeaponDamage(LaraWeaponType weaponType)
	{
		return short(Weapons[(int)weaponType].Damage) * 25;
	}

	void DoppelgangerControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->HitPoints < LARA_HEALTH_MAX)
		{
			item->HitPoints = LARA_HEALTH_MAX;
			DoDamage(LaraItem, GetWeaponDamage(Lara.Control.Weapon.GunType));
		}

		auto* reference = FindReference(item, ID_BACON_REFERENCE);

		if (item->Data == NULL)
		{
			Vector3Int pos;
			if (reference == nullptr)
			{
				pos.x = item->Pose.Position.x;
				pos.y = LaraItem->Pose.Position.y;
				pos.z = item->Pose.Position.z;
			}
			else
			{
				pos.x = 2 * reference->Pose.Position.x - LaraItem->Pose.Position.x;
				pos.y = LaraItem->Pose.Position.y;
				pos.z = 2 * reference->Pose.Position.z - LaraItem->Pose.Position.z;
			}

			// Get floor heights for comparison.
			item->Floor = GetCollision(item).Position.Floor;
			int laraFloorHeight = GetCollision(LaraItem).Position.Floor;

			// Animate bacon Lara, mirroring Lara's position.
			item->Animation.FrameNumber = LaraItem->Animation.FrameNumber;
			item->Animation.AnimNumber = LaraItem->Animation.AnimNumber;
			item->Pose.Position.x = pos.x;
			item->Pose.Position.y = pos.y;
			item->Pose.Position.z = pos.z;
			item->Pose.Orientation.x = LaraItem->Pose.Orientation.x;
			item->Pose.Orientation.y = LaraItem->Pose.Orientation.y - ANGLE(180.0f);
			item->Pose.Orientation.z = LaraItem->Pose.Orientation.z;
			ItemNewRoom(itemNumber, LaraItem->RoomNumber);

			// Compare floor heights.
			if (item->Floor >= laraFloorHeight + SECTOR(1) + 1 &&	// Add 1 to avoid bacon Lara dying when exiting water.
				!LaraItem->Animation.IsAirborne)
			{
				SetAnimation(item, LA_JUMP_WALL_SMASH_START);
				item->Animation.Velocity = 0;
				item->Animation.VerticalVelocity = 0;
				item->Animation.IsAirborne = true;
				item->Data = -1;
				item->Pose.Position.y += 50;
			}
		}

		if (item->Data)
		{
			AnimateItem(item);
			TestTriggers(item, true);

			item->Floor = GetCollision(item).Position.Floor;
			if (item->Pose.Position.y >= item->Floor)
			{
				item->Pose.Position.y = item->Floor;
				TestTriggers(item, true);

				item->Animation.VerticalVelocity = 0;
				item->Animation.IsAirborne = false;
				item->Animation.TargetState = LS_DEATH;
				item->Animation.RequiredState = LS_DEATH;
			}
		}
	}

	// TODO: DrawLara not exist ! use Renderer11.cpp DrawLara instead or create DrawLara() function with old behaviour.
	void DrawEvilLara(ItemInfo* item)
	{
		/*
		short* meshstore[15];
		short** meshpp;
		int i;

		meshpp = &Meshes[Objects[item->objectNumber].meshIndex];           	// Save Laras Mesh Pointers
		for (i = 0; i < 15; i++)
		{
			meshstore[i] = Lara.meshPtrs[i];
			Lara.meshPtrs[i] = *(meshpp++);
		}

		DrawLara(item);

		for (i = 0; i < 15; i++)
			Lara.meshPtrs[i] = meshstore[i];*/
	}
}
