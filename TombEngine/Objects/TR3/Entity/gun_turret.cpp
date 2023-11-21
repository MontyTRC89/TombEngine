#include "framework.h"
#include "Objects/TR3/Entity/gun_turret.h"

#include "Game/control/lot.h"
#include "Game/control/los.h"
#include "Game/control/box.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Game/Setup.h"
#include "Specific/level.h"
#include "Objects/TR5/Entity/AutoGun.h"

// NOTES:
// item.ItemFlags[0]: flag to duplicate the shooting light diffusion.
// item.ItemFlags[3]: flag to indicate if the turret is hostile or neutral.


namespace TEN::Entities::Creatures::TR3
{
	constexpr auto GUN_TURRET_SHOT_DAMAGE = 5;
	constexpr auto GUN_TURRET_MAX_HITPOINTS = 28;

	const auto GunTurretLeftBite = CreatureBiteInfo(110, -30, 530, 2);
	const auto GunTurretRightBite = CreatureBiteInfo(-110, -30, 530, 2);

	enum GunTurretAnim
	{
		AUTOGUN_ANIM_FIRE = 0,
		AUTOGUN_ANIM_STILL = 1
	};

	enum GunTurretState
	{
		AUTOGUN_STATE_FIRE = 0,
		AUTOGUN_STATE_STILL = 1
	};

	void InitialiseGunTurret(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		SetAnimation(item, AUTOGUN_ANIM_STILL);

		item.ItemFlags[0] = 0;
		item.ItemFlags[3] = 0;		
	}

	void GunTurretControl(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		auto& laraItem = *LaraItem;
		auto& gun = *GetCreatureInfo(&item);
		short tilt, diff;
		Vector3i pos;

		if (!CreatureActive(itemNumber))
			return;

		if (item.HitStatus || item.HitPoints < GUN_TURRET_MAX_HITPOINTS)
			item.ItemFlags[3] = 1;

		if (gun.MuzzleFlash[0].Delay >= 1)
		{
			gun.MuzzleFlash[0].Delay--;
		}

		if (gun.MuzzleFlash[1].Delay >= 1)
		{
			gun.MuzzleFlash[1].Delay--;
		}

		if (!&gun)
			return;

		if (item.HitPoints <= 0)
		{
			ExplodingDeath(itemNumber, BODY_DO_EXPLOSION | BODY_NO_BOUNCE);
			DisableEntityAI(itemNumber);
			KillItem(itemNumber);

			item.Flags |= 1u;
			item.Status = ITEM_DEACTIVATED;

			TriggerExplosionSparks(item.Pose.Position.x, item.Pose.Position.y - CLICK(3), item.Pose.Position.z, 3, -2, 0, item.RoomNumber);
			for (int i = 0; i < 2; i++)
				TriggerExplosionSparks(item.Pose.Position.x, item.Pose.Position.y - CLICK(3), item.Pose.Position.z, 3, -1, 0, item.RoomNumber);

			SoundEffect(SFX_TR4_EXPLOSION1, &item.Pose, SoundEnvironment::Land, 1.5f);
			SoundEffect(SFX_TR4_EXPLOSION2, &item.Pose);
		}

		if (!item.ItemFlags[3])
			return;

		AI_INFO AI;
		CreatureAIInfo(&item, &AI);
		tilt = AI.xAngle;

		auto origin = GameVector(GetJointPosition(&item, 2), item.RoomNumber);
		auto target = GameVector(GetJointPosition(&laraItem, LM_TORSO), laraItem.RoomNumber);
		bool los = LOS(&origin, &target);

		switch (item.Animation.ActiveState)
		{
		case AUTOGUN_STATE_FIRE:

			if (!los)
				item.Animation.TargetState = AUTOGUN_STATE_STILL;
			else if (item.Animation.FrameNumber == GetAnimData(item).frameBase)
			{
				item.ItemFlags[0] = 1;

				if (!(gun.MuzzleFlash[0].Delay) )
				{
					DoDamage(&laraItem, GUN_TURRET_SHOT_DAMAGE);

					auto bloodPos = GetJointPosition(&laraItem, Random::GenerateInt(0, NUM_LARA_MESHES - 1));
					float bloodVel = Random::GenerateFloat(4.0f, 8.0f);
					DoBloodSplat(bloodPos.x, bloodPos.y, bloodPos.z, bloodVel, Random::GenerateAngle(), laraItem.RoomNumber);
					

					pos = GetJointPosition(item, GunTurretLeftBite);
					auto colorRed =		Random::GenerateFloat(0.75f, 1.0f) * UCHAR_MAX;
					auto colorGreen =	Random::GenerateFloat(0.5f, 0.6f) * UCHAR_MAX;
					auto colorBlue =	Random::GenerateFloat(0.0f, 0.25f) * UCHAR_MAX;
					TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * item.ItemFlags[0] + 8, colorRed, colorGreen, colorBlue);

					gun.MuzzleFlash[0].Bite = GunTurretLeftBite;
					gun.MuzzleFlash[0].SwitchToMuzzle2 = true;
					gun.MuzzleFlash[0].ApplyXRotation = false;
					gun.MuzzleFlash[0].ApplyZRotation = true;
					gun.MuzzleFlash[0].UseSmoke = true;
					gun.MuzzleFlash[0].Delay = 1;
				}

				if (!(gun.MuzzleFlash[1].Delay))
				{
					DoDamage(&laraItem, GUN_TURRET_SHOT_DAMAGE);

					auto bloodPos = GetJointPosition(&laraItem, Random::GenerateInt(0, NUM_LARA_MESHES - 1));
					float bloodVel = Random::GenerateFloat(4.0f, 8.0f);
					DoBloodSplat(bloodPos.x, bloodPos.y, bloodPos.z, bloodVel, Random::GenerateAngle(), laraItem.RoomNumber);

					pos = GetJointPosition(item, GunTurretRightBite);
					auto colorRed =		Random::GenerateFloat(0.75f, 1.0f) * UCHAR_MAX;
					auto colorGreen =	Random::GenerateFloat(0.5f, 0.6f) * UCHAR_MAX;
					auto colorBlue =	Random::GenerateFloat(0.0f, 0.25f) * UCHAR_MAX;
					TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * item.ItemFlags[0] + 8, colorRed, colorGreen, colorBlue);

					gun.MuzzleFlash[1].Bite = GunTurretRightBite;
					gun.MuzzleFlash[1].SwitchToMuzzle2 = true;
					gun.MuzzleFlash[1].ApplyXRotation = false;
					gun.MuzzleFlash[1].ApplyZRotation = true;
					gun.MuzzleFlash[1].UseSmoke = true;					
					gun.MuzzleFlash[1].Delay = 1;
				}

				SoundEffect(SFX_TR4_HK_FIRE, &item.Pose, SoundEnvironment::Land, 0.8f);				
			}

			break;

		case AUTOGUN_STATE_STILL:

			if (los && !item.ItemFlags[0])
				item.Animation.TargetState = AUTOGUN_STATE_FIRE;

			else if (item.ItemFlags[0])
			{
				if (item.AIBits == MODIFY)
				{
					item.ItemFlags[0] = 1;
					item.Animation.TargetState = AUTOGUN_STATE_FIRE;
				}
				else
				{
					item.ItemFlags[1] = 0;
					item.ItemFlags[0] = 0;
					gun.Flags |= 0;
				}
			}

			break;
		}

		diff = AI.angle - gun.JointRotation[0];

		if (diff > 1820)
			diff = 1820;
		else if (diff < -1820)
			diff = -1820;

		gun.JointRotation[0] += diff;
		CreatureJoint(&item, 1, tilt);
		AnimateItem(&item);
	}

	void GunTurretHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex)
	{
		DefaultItemHit(target, source, pos, damage, isExplosive, jointIndex);
	}
}
