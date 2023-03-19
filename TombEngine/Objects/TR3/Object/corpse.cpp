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
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Ripple;

namespace TEN::Entities::TR3
{
	enum CorpseState
	{
		CORPSE_STATE_LYING = 0,
		CORPSE_STATE_HANGING = 1,
		CORPSE_STATE_FALLING = 2,
		CORPSE_STATE_LANDING = 3
	};

	enum CorpseAnim
	{
		CORPSE_ANIM_LYING = 0,
		CORPSE_ANIM_HANGING = 1,
		CORPSE_ANIM_FALLING = 2,
		CORPSE_ANIM_LANDING = 3
	};

	enum class CorpseFlags
	{
		Lying	= (1 << 0),
		Hanging = (1 << 1),
		Falling = (1 << 2)
	};

	void InitialiseCorpse(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.ItemFlags[2] = (int)EffectType::Corpse;

		if (item.TriggerFlags == 1)
		{
			item.ItemFlags[1] = (int)CorpseFlags::Hanging;
			item.Animation.AnimNumber = Objects[item.ObjectNumber].animIndex + CORPSE_ANIM_HANGING;
			item.Animation.ActiveState = CORPSE_STATE_HANGING;
		}
		else
		{
			item.ItemFlags[1] = (int)CorpseFlags::Lying;
			item.Animation.AnimNumber = Objects[item.ObjectNumber].animIndex + CORPSE_ANIM_LYING;
			item.Animation.ActiveState = CORPSE_STATE_LYING;
		}

		AddActiveItem(itemNumber);
		item.Status = ITEM_ACTIVE;
	}

	void CorpseControl(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
	
		if (TriggerActive(&item))
		{
			item.ItemFlags[2] = (int)EffectType::Corpse;
		}
		else
		{
			item.ItemFlags[2] = (int)EffectType::None;
		}

		item.Effect.Type = (EffectType)item.ItemFlags[2];

		if (item.ItemFlags[1] == (int)CorpseFlags::Falling)
		{
			bool isWater = TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item.RoomNumber);
			int VerticalVelCoeff = isWater ? 81.0f : 1.0f;
			
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
					item.Effect.Type = EffectType::Smoke;
				}
				else
				{
					item.Pose.Position.y = item.Pose.Position.y;
				}

				item.Animation.IsAirborne = false;
				item.Animation.Velocity = Vector3::Zero;
				item.Animation.TargetState = CORPSE_STATE_LANDING;
				item.Animation.AnimNumber = Objects[item.ObjectNumber].animIndex + CORPSE_ANIM_LANDING;
				AlignEntityToSurface(&item, Vector2(Objects[item.ObjectNumber].radius));

				item.ItemFlags[1] = (int)CorpseFlags::Lying;
				return;
			}
			else
			{
				if (isWater)

				{
					item.Animation.Velocity.y += 0.1f / VerticalVelCoeff;
				}
				else
				{
					item.Animation.Velocity.y += GRAVITY;
				}
			}
		}

		AnimateItem(&item);
	}

	void CorpseHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex)
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

			if (target.ItemFlags[1] == (int)CorpseFlags::Hanging)
			{
				target.ItemFlags[1] = (int)CorpseFlags::Falling;
				target.Animation.AnimNumber = Objects[target.ObjectNumber].animIndex + CORPSE_ANIM_FALLING;
				target.Animation.ActiveState = CORPSE_STATE_FALLING;
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
