#include "framework.h"
#include "Objects/TR1/Entity/tr1_centaur.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

namespace TEN::Entities::TR1
{
	BITE_INFO CentaurRocketBite = { 11, 415, 41, 13 };
	BITE_INFO CentaurRearBite = { 50, 30, 0, 5 };
	const vector<int> CentaurAttackJoints = { 0, 3, 4, 7, 8, 16, 17 };

	constexpr auto CENTAUR_REAR_DAMAGE = 200;

	constexpr auto CENTAUR_PROJECTILE_SPEED = CLICK(1);

	constexpr auto CENTAUR_REAR_RANGE = SECTOR(1.5f);

	constexpr auto CENTAUR_REAR_CHANCE = 0x60;

	#define CENTAUR_TURN_ANGLE ANGLE(4.0f)

	enum CentaurState
	{
		CENTAUR_STATE_NONE = 0,
		CENTAUR_STATE_IDLE = 1,
		CENTAUR_PROJECTILE_ATTACK = 2,
		CENTAUR_STATE_RUN_FORWARD = 3,
		CENTAUR_STATE_AIM = 4,
		CENTAUR_STATE_DEATH = 5,
		CENTAUR_STATE_WARNING = 6
	};

	// TODO
	enum CentaurAnim
	{
		CENTAUR_ANIM_DEATH = 8,
	};

	void ControlCentaurBomb(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		bool aboveWater = false;
		auto oldPos = item->Pose.Position;

		item->Pose.Orientation.z += ANGLE(35.0f);
		if (!TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
		{
			item->Pose.Orientation.x -= ANGLE(1.0f);
			if (item->Pose.Orientation.x < -ANGLE(90.0f))
				item->Pose.Orientation.x = -ANGLE(90.0f);

			item->Animation.Velocity = CENTAUR_PROJECTILE_SPEED * phd_cos(item->Pose.Orientation.x);
			item->Animation.VerticalVelocity = -CENTAUR_PROJECTILE_SPEED * phd_sin(item->Pose.Orientation.x);
			aboveWater = true;
		}
		else
		{
			item->Animation.VerticalVelocity += 3;
			aboveWater = true;

			if (item->Animation.Velocity)
			{
				item->Pose.Orientation.z += ((item->Animation.Velocity / 4) + 7) * ANGLE(1.0f);

				if (item->Animation.RequiredState)
					item->Pose.Orientation.y += ((item->Animation.Velocity / 2) + 7) * ANGLE(1.0f);
				else
					item->Pose.Orientation.x += ((item->Animation.Velocity / 2) + 7) * ANGLE(1.0f);
			}
		}

		TranslateItem(item, item->Pose.Orientation, item->Animation.Velocity);

		auto probe = GetCollision(item);

		if (probe.Position.Floor < item->Pose.Position.y ||
			probe.Position.Ceiling > item->Pose.Position.y)
		{
			item->Pose.Position = oldPos;

			if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
				TriggerUnderwaterExplosion(item, 0);
			else
			{
				item->Pose.Position.y -= CLICK(0.5f);
				TriggerShockwave(&item->Pose, 48, 304, 96, 0, 96, 128, 24, 0, 0);

				TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -2, 0, item->RoomNumber);
				for (int x = 0; x < 2; x++)
					TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -1, 0, item->RoomNumber);
			}

			return;
		}

		if (item->RoomNumber != probe.RoomNumber)
			ItemNewRoom(itemNumber, probe.RoomNumber);

		if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber) && aboveWater)
			SetupRipple(item->Pose.Position.x, g_Level.Rooms[item->RoomNumber].minfloor, item->Pose.Position.z, (GetRandomControl() & 7) + 8, 0);

		GetCollidedObjects(item, HARPOON_HIT_RADIUS, true, &CollidedItems[0], &CollidedMeshes[0], 0);

		if (!CollidedItems[0] && !CollidedMeshes[0])
			return;

		if (CollidedItems[0])
		{
			auto* currentItem = CollidedItems[0];

			int k = 0;
			do
			{
				auto* currentObject = &Objects[currentItem->ObjectNumber];

				if (currentItem->Status == ITEM_ACTIVE &&
					currentObject->intelligent && !currentObject->undead &&
					currentObject->collision)
				{
					DoExplosiveDamageOnBaddy(LaraItem, currentItem, item, LaraWeaponType::Crossbow);
				}

				k++;
				currentItem = CollidedItems[k];

			} while (currentItem);
		}
	}

	static void RocketGun(ItemInfo* centaurItem)
	{
		short itemNumber;
		itemNumber = CreateItem();

		if (itemNumber != NO_ITEM)
		{
			auto* projectileItem = &g_Level.Items[itemNumber];

			projectileItem->ObjectNumber = ID_PROJ_BOMB;
			projectileItem->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
			projectileItem->RoomNumber = centaurItem->RoomNumber;

			auto pos = Vector3Int(11, 415, 41);
			GetJointAbsPosition(centaurItem, &pos, 13);

			projectileItem->Pose.Position = pos;
			InitialiseItem(itemNumber);

			projectileItem->Pose.Orientation = Vector3Shrt(0, centaurItem->Pose.Orientation.y, 0 );

			projectileItem->Animation.Velocity = CENTAUR_PROJECTILE_SPEED * phd_cos(projectileItem->Pose.Orientation.x);
			projectileItem->Animation.VerticalVelocity = -CENTAUR_PROJECTILE_SPEED * phd_cos(projectileItem->Pose.Orientation.x);
			projectileItem->ItemFlags[0] = 1;

			AddActiveItem(itemNumber);
		}
	}

	void CentaurControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		if (!CreatureActive(itemNumber))
			return;

		short head = 0;
		short angle = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != CENTAUR_STATE_DEATH)
			{
				item->Animation.AnimNumber = Objects[ID_CENTAUR_MUTANT].animIndex + CENTAUR_ANIM_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = CENTAUR_STATE_DEATH;
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, CENTAUR_TURN_ANGLE);

			switch (item->Animation.ActiveState)
			{
			case CENTAUR_STATE_IDLE:
				CreatureJoint(item, 17, 0);
				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (AI.bite && AI.distance < pow(CENTAUR_REAR_RANGE, 2))
					item->Animation.TargetState = CENTAUR_STATE_RUN_FORWARD;
				else if (Targetable(item, &AI))
					item->Animation.TargetState = CENTAUR_STATE_AIM;
				else
					item->Animation.TargetState = CENTAUR_STATE_RUN_FORWARD;

				break;

			case CENTAUR_STATE_RUN_FORWARD:
				if (AI.bite && AI.distance < pow(CENTAUR_REAR_RANGE, 2))
				{
					item->Animation.RequiredState = CENTAUR_STATE_WARNING;
					item->Animation.TargetState = CENTAUR_STATE_IDLE;
				}
				else if (Targetable(item, &AI))
				{
					item->Animation.RequiredState = CENTAUR_STATE_AIM;
					item->Animation.TargetState = CENTAUR_STATE_IDLE;
				}
				else if (GetRandomControl() < CENTAUR_REAR_CHANCE)
				{
					item->Animation.RequiredState = CENTAUR_STATE_WARNING;
					item->Animation.TargetState = CENTAUR_STATE_IDLE;
				}

				break;

			case CENTAUR_STATE_AIM:
				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (Targetable(item, &AI))
					item->Animation.TargetState = CENTAUR_PROJECTILE_ATTACK;
				else
					item->Animation.TargetState = CENTAUR_STATE_IDLE;

				break;

			case CENTAUR_PROJECTILE_ATTACK:
				if (!item->Animation.RequiredState)
				{
					item->Animation.RequiredState = CENTAUR_STATE_AIM;
					RocketGun(item);
				}

				break;

			case CENTAUR_STATE_WARNING:
				if (!item->Animation.RequiredState &&
					item->TestBits(JointBitType::Touch, CentaurAttackJoints))
				{
					CreatureEffect(item, &CentaurRearBite, DoBloodSplat);
					DoDamage(creature->Enemy, CENTAUR_REAR_DAMAGE);
					item->Animation.RequiredState = CENTAUR_STATE_IDLE;
				}

				break;
			}
		}

		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, 0);

		if (item->Status == ITEM_DEACTIVATED)
		{
			SoundEffect(SFX_TR1_ATLANTEAN_DEATH, &item->Pose);
			ExplodingDeath(itemNumber, BODY_EXPLODE);
			KillItem(itemNumber);
			item->Status = ITEM_DEACTIVATED;
		}
	}
}
