#include "framework.h"
#include "Objects/TR3/Object/Corpse.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/collision/floordata.h"
#include "Game/effects/effects.h"
#include "Game/effects/Ripple.h"
#include "Game/effects/Splash.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Effects/Fireflies.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Collision::Point;
using namespace TEN::Effects::Ripple;
using namespace TEN::Effects::Splash;
using namespace TEN::Math;
using namespace TEN::Effects::Fireflys;

namespace TEN::Entities::TR3
{
	constexpr auto FLY_EFFECT_MAX_WIDTH = -1;
	constexpr auto LANDING_BUFFER = 150;

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

		item.ItemFlags[FirefliesItemFlags::TargetItemPtr] = item.Index;
		item.ItemFlags[FirefliesItemFlags::TriggerFlags] = -1;
		item.HitPoints = 16;
	}

	void ControlCorpse(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		const auto& object = Objects[item.ObjectNumber];

		if (item.ItemFlags[1] == (int)CorpseFlag::Fall)
		{
			bool isWater = TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item.RoomNumber);
			bool isSwamp = TestEnvironment(RoomEnvFlags::ENV_FLAG_SWAMP, item.RoomNumber);

			float verticalVelCoeff = isWater ? 81.0f : 1.0f;
			
			auto pointColl = GetPointCollision(item);
			if (item.RoomNumber != pointColl.GetRoomNumber())
			{
				if (TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, pointColl.GetRoomNumber()) &&
					!isWater)
				{
					int waterHeight = pointColl.GetWaterTopHeight();
					SplashSetup.Position = Vector3(item.Pose.Position.x, waterHeight - 1, item.Pose.Position.z);
					SplashSetup.SplashPower = item.Animation.Velocity.y * 4;
					SplashSetup.InnerRadius = 160.0f;

					SetupSplash(&SplashSetup, pointColl.GetRoomNumber());
					item.Animation.Velocity.y = 0.0f;
				}

				ItemNewRoom(itemNumber, pointColl.GetRoomNumber());
			}

			// Remove fly effect when in water.
			if (isWater || isSwamp)
			{
				item.ItemFlags[FirefliesItemFlags::FliesEffect] = 1;
			}
			else
			{
				item.ItemFlags[FirefliesItemFlags::FliesEffect] = 0;
			}

			pointColl = GetPointCollision(item);
			item.Animation.IsAirborne = true;

			if (pointColl.GetFloorHeight() < item.Pose.Position.y + LANDING_BUFFER)
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
					item.Animation.Velocity.y += g_GameFlow->GetSettings()->Physics.Gravity;
				}
			}
		}

		AnimateItem(&item);

		if (!TriggerActive(&item))
		return;

		// Spawn fly effect.
		if (item.HitPoints != NOT_TARGETABLE)
		{
			int fireflyCount = item.HitPoints - item.ItemFlags[FirefliesItemFlags::Spawncounter];

			if (fireflyCount < 0)
			{
				int firefliesToTurnOff = -fireflyCount;
				for (auto& firefly : FireflySwarm)
				{
					if (firefly.LeaderItemPtr == &item && firefly.Life > 0.0f)
					{
						firefly.Life = 0.0f;
						firefly.on = false;
						firefliesToTurnOff--;

						if (firefliesToTurnOff == 0)
							break;
					}
				}
			}
			else if (fireflyCount > 0)
			{
				for (int i = 0; i < fireflyCount; i++)
				{
					SpawnFireflySwarm(item, FLY_EFFECT_MAX_WIDTH);
				}
			}

			item.ItemFlags[FirefliesItemFlags::Spawncounter] = item.HitPoints;
			item.HitPoints = NOT_TARGETABLE;
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
