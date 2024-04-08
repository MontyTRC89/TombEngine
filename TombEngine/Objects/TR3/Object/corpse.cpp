#include "framework.h"
#include "Objects/TR3/Object/Corpse.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/effects/Ripple.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Effects::Ripple;
using namespace TEN::Math;

namespace TEN::Entities::TR3
{
	enum CorpseState
	{
		CORPSE_STATE_GROUNDED = 0,
		CORPSE_STATE_HANG = 1,
		CORPSE_STATE_FALL = 2,
		CORPSE_STATE_LAND = 3
	};

	enum CorpseAnim
	{
		CORPSE_ANIM_GROUNDED = 0,
		CORPSE_ANIM_HANG = 1,
		CORPSE_ANIM_FALL = 2,
		CORPSE_ANIM_LAND = 3
	};

	void InitializeCorpse(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		const auto& object = Objects[item.ObjectNumber];

		if (item.TriggerFlags == 1)
		{
			item.ItemFlags[1] = (int)CorpseFlag::Hang;
			item.Animation.AnimNumber = object.animIndex + CORPSE_ANIM_HANG;
			item.Animation.ActiveState = CORPSE_STATE_HANG;
		}
		else
		{
			item.ItemFlags[1] = (int)CorpseFlag::Grounded;
			item.Animation.AnimNumber = object.animIndex + CORPSE_ANIM_GROUNDED;
			item.Animation.ActiveState = CORPSE_STATE_GROUNDED;
		}

		AddActiveItem(itemNumber);
		item.Status = ITEM_ACTIVE;
	}

	void ControlCorpse(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		const auto& object = Objects[item.ObjectNumber];

		if (item.ItemFlags[1] == (int)CorpseFlag::Fall)
		{
			bool isWater = TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item.RoomNumber);
			float verticalVelCoeff = isWater ? 81.0f : 1.0f;
			
			int roomNumber = GetCollision(&item).RoomNumber;
			if (item.RoomNumber != roomNumber)
			{
				if (TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, roomNumber) &&
					!TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item.RoomNumber))
				{
					int waterHeight = GetWaterHeight(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, roomNumber);
					SplashSetup.y = waterHeight - 1;
					SplashSetup.x = item.Pose.Position.x;
					SplashSetup.z = item.Pose.Position.z;
					SplashSetup.splashPower = item.Animation.Velocity.y * 4;
					SplashSetup.innerRadius = 160.0f;

					SetupSplash(&SplashSetup, roomNumber);
					item.Animation.Velocity.y = 0.0f;
				}

				ItemNewRoom(itemNumber, roomNumber);
			}

			auto pointColl = GetCollision(&item);
			item.Animation.IsAirborne = true;

			if (pointColl.Position.Floor < item.Pose.Position.y)
			{
				if (!isWater)
				{
					item.Pose.Position.y = item.Pose.Position.y - item.Animation.Velocity.y;
					SoundEffect(SFX_TR4_CROCGOD_LAND, &item.Pose);
				}
				else
				{
					item.Pose.Position.y = item.Pose.Position.y;
				}

				item.Animation.IsAirborne = false;
				item.Animation.Velocity = Vector3::Zero;
				item.Animation.TargetState = CORPSE_STATE_LAND;
				item.Animation.AnimNumber = object.animIndex + CORPSE_ANIM_LAND;
				AlignEntityToSurface(&item, Vector2(object.radius));

				item.ItemFlags[1] = (int)CorpseFlag::Grounded;
				return;
			}
			else
			{
				if (isWater)
				{
					item.Animation.Velocity.y += 0.1f / verticalVelCoeff;
				}
				else
				{
					item.Animation.Velocity.y += GRAVITY;
				}
			}
		}

		AnimateItem(&item);

		if (!TriggerActive(&item))
			return;

		int meshCount = object.nmeshes;
		for (int i = 0; i < meshCount; i++)
		{
			if (Random::TestProbability(1 / 72.0f))
			{
				auto pos = GetJointPosition(&item, i).ToVector3();
				SpawnCorpseEffect(pos);
			}
		}
	}

	void HitCorpse(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex)
	{
		const auto& object = Objects[target.ObjectNumber];
		const auto& player = *GetLaraInfo(&source);

		if (pos.has_value() && (player.Control.Weapon.GunType == LaraWeaponType::Pistol ||
			player.Control.Weapon.GunType == LaraWeaponType::Shotgun ||
			player.Control.Weapon.GunType == LaraWeaponType::Uzi ||
			player.Control.Weapon.GunType == LaraWeaponType::HK ||
			player.Control.Weapon.GunType == LaraWeaponType::Revolver))
		{
			DoBloodSplat(pos->x, pos->y, pos->z, Random::GenerateInt(4, 8), source.Pose.Orientation.y, pos->RoomNumber);

			if (target.ItemFlags[1] == (int)CorpseFlag::Hang)
			{
				target.ItemFlags[1] = (int)CorpseFlag::Fall;
				target.Animation.AnimNumber = object.animIndex + CORPSE_ANIM_FALL;
				target.Animation.ActiveState = CORPSE_STATE_FALL;
			}
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
