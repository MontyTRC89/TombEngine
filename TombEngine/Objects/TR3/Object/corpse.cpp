#include "framework.h"
#include "Objects/TR3/Object/corpse.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/floordata.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/effects/Ripple.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"


#define TRAIN_VEL	260
#define LARA_TRAIN_DEATH_ANIM 3;

using namespace TEN::Effects::Ripple;

namespace TEN::Entities
{

	enum CadaverState
	{
		CadaverLying = 0,
		CadaverHanging = 1,
		CadaverFalling = 2,

	};

	void InitialiseCorpse(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.Effect.Type = EffectType::Cadaver;

		if (item.TriggerFlags == 1)
			item.ItemFlags[1] = CadaverHanging;
		else
			item.ItemFlags[1] = CadaverLying;

		AddActiveItem(itemNumber);
		item.Status = ITEM_ACTIVE;
	}


	bool TestCorpseNewRoom(ItemInfo& item, const CollisionResult& coll)
	{
		// Has corpse changed room?
		if (item.RoomNumber == coll.RoomNumber)
			return false;

		// If currently in water and previously on land, add a ripple.
		if (TestEnvironment(ENV_FLAG_WATER, item.RoomNumber) != TestEnvironment(ENV_FLAG_WATER, coll.RoomNumber))
		{
			int floorDiff = abs(coll.Position.Floor - item.Pose.Position.y);
			int ceilingDiff = abs(coll.Position.Ceiling - item.Pose.Position.y);
			int yPoint = (floorDiff > ceilingDiff) ? coll.Position.Ceiling : coll.Position.Floor;

			SpawnRipple(Vector3(item.Pose.Position.x, yPoint, item.Pose.Position.z), item.RoomNumber, Random::GenerateInt(8, 16));
		}

		ItemNewRoom(item.Index, coll.RoomNumber);
		return true;
	}

	void CorpseControl(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
			return;

		if (item.ItemFlags[1] == CadaverFalling)
		{
			int oldRoom = item.RoomNumber;

			//item->Pose.Position.y += item->ItemFlags[1];



			short roomNumber = item.RoomNumber;
			auto floor = GetFloor(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, &roomNumber);
			//h = GetHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);

			item.Pose.Position.y = item.Floor;


			auto pointColl = GetCollision(&item);
			TestCorpseNewRoom(item, pointColl);


			//if (item.RoomNumber != roomNumber)
				//ItemNewRoom(itemNumber, roomNumber);

			item.Animation.IsAirborne = true;

			if (item.Pose.Position.y >= item.Floor || (TestEnvironment(ENV_FLAG_WATER, item.RoomNumber)))
			{
				item.Pose.Position.y = item.Floor;
				item.Animation.IsAirborne = false;
				item.Animation.Velocity.y = 0.0f;


				//item->Pose.Position.y = h;
				//item->ItemFlags[1] = 0;
				item.Pose.Orientation.z = 0x4000;
				item.ItemFlags[1] = CadaverLying;
				return;
			}

			item.Pose.Orientation.z = item.Floor; // += item->ItemFlags[1];




		}

	}

	void CorpseHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex)
	{
		const auto& player = *GetLaraInfo(&source);
		const auto& object = Objects[target.ObjectNumber];


		if (pos.has_value() && (player.Control.Weapon.GunType == LaraWeaponType::Pistol ||
			player.Control.Weapon.GunType == LaraWeaponType::Shotgun ||
			player.Control.Weapon.GunType == LaraWeaponType::Uzi ||
			player.Control.Weapon.GunType == LaraWeaponType::HK ||
			player.Control.Weapon.GunType == LaraWeaponType::Revolver))
		{
			DoBloodSplat(pos->x, pos->y, pos->z, Random::GenerateInt(4, 8), source.Pose.Orientation.y, pos->RoomNumber);

			if (target.ItemFlags[1] = CadaverHanging)
				target.ItemFlags[1] = CadaverFalling;

		}
		else if (player.Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo == WeaponAmmoType::Ammo2)
		{
			DoItemHit(&target,0, isExplosive, false);
		}
		else
		{
			DoItemHit(&target, 0, isExplosive, false);
		}
	}

}
