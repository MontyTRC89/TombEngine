#include "framework.h"
#include "Objects/TR2/Entity/tr2_knife_thrower.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;

namespace TEN::Entities::TR2
{
	constexpr auto KNIFE_PROJECTILE_DAMAGE = 50;

	// TODO: Ranges.

	const auto KnifeBiteLeft  = BiteInfo(Vector3::Zero, 5);
	const auto KnifeBiteRight = BiteInfo(Vector3::Zero, 8);

	enum KnifeThrowerState
	{
		KTHROWER_STATE_NONE = 0,
		KTHROWER_STATE_IDLE = 1,
		KTHROWER_STATE_WALK_FORWARD = 2,
		KTHROWER_STATE_RUN_FORWARD = 3,
		KTHROWER_STATE_WALK_KNIFE_ATTACK_LEFT_START = 4,
		KTHROWER_STATE_WALK_KNIFE_ATTACK_LEFT_CONTINUE = 5,
		KTHROWER_STATE_WALK_KNIFE_ATTACK_RIGHT_START = 6,
		KTHROWER_STATE_WALK_KNIFE_ATTACK_RIGHT_CONTINUE = 7,
		KTHROWER_STATE_IDLE_KNIFE_ATTACK_START = 8,
		KTHROWER_STATE_IDLE_KNIFE_ATTACK_CONTINUE = 9,
		KTHROWER_STATE_DEATH = 10
	};

	enum KnifeThrowerAnim
	{
		KTHROWER_ANIM_IDLE = 0,
		KTHROWER_ANIM_WALK_KNIFE_ATTACK_LEFT_START = 1,
		KTHROWER_ANIM_WALK_KNIFE_ATTACK_LEFT_CANCEL = 2,
		KTHROWER_ANIM_WALK_KNIFE_ATTACK_RIGHT_START = 3,
		KTHROWER_ANIM_WALK_KNIFE_ATTACK_RIGHT_CANCEL = 4,
		KTHROWER_ANIM_IDLE_KNIFE_ATTACK_START = 5,
		KTHROWER_ANIM_RUN_FORWARD_TO_IDLE = 6,
		KTHROWER_ANIM_RUN_FORWARD_TO_WALK_FORWARD = 7,
		KTHROWER_ANIM_RUN_FORWARD = 8,
		KTHROWER_ANIM_WALK_KNIFE_ATTACK_LEFT_CONTINUE = 9,
		KTHROWER_ANIM_WALK_KNIFE_ATTACK_RIGHT_CONTINUE = 10,
		KTHROWER_ANIM_IDLE_KNIFE_ATTACK_CONTINUE = 11,
		KTHROWER_ANIM_WALK_KNIFE_ATTACK_LEFT_END = 12,
		KTHROWER_ANIM_IDLE_KNIFE_ATTACK_END_TO_IDLE = 13,
		KTHROWER_ANIM_IDLE_KNIFE_ATTACK_END_TO_START = 14,
		KTHROWER_ANIM_IDLE_TO_RUN_FORWARD = 15,
		KTHROWER_ANIM_IDLE_TO_WALK_FORWARD = 16,
		KTHROWER_ANIM_WALK_FORWARD = 17,
		KTHROWER_ANIM_WALK_FORWARD_TO_RUN_FORWARD = 18,
		KTHROWER_ANIM_WALK_FORWARD_TO_IDLE_START = 19,
		KTHROWER_ANIM_WALK_FORWARD_TO_IDLE_END = 20,
		KTHROWER_ANIM_WALK_KNIFE_ATTACK_RIGHT_END = 21,
		KTHROWER_ANIM_IDLE_KNIFE_ATTACK_CANCEL = 22,
		KTHROWER_ANIM_DEATH = 23
	};

	void KnifeControl(short fxNumber)
	{
		auto* fx = &EffectList[fxNumber];

		if (fx->counter <= 0)
		{
			KillEffect(fxNumber);
			return;
		}
		else
			fx->counter--;

		int speed = fx->speed * cos(fx->pos.Orientation.x);
		fx->pos.Position.z += speed * cos(fx->pos.Orientation.y);
		fx->pos.Position.x += speed * sin(fx->pos.Orientation.y);
		fx->pos.Position.y += fx->speed * sin(-fx->pos.Orientation.x);

		auto probe = GetCollision(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z, fx->roomNumber);

		if (fx->pos.Position.y >= probe.Position.Floor ||
			fx->pos.Position.y <= probe.Position.Ceiling)
		{
			KillEffect(fxNumber);
			return;
		}

		if (probe.RoomNumber != fx->roomNumber)
			EffectNewRoom(fxNumber, probe.RoomNumber);

		fx->pos.Orientation.z += Angle::DegToRad(30.0f);

		if (ItemNearLara(&fx->pos, 200))
		{
			DoDamage(LaraItem, KNIFE_PROJECTILE_DAMAGE);

			fx->pos.Orientation.y = LaraItem->Pose.Orientation.y;
			fx->speed = LaraItem->Animation.Velocity.z;
			fx->frameNumber = fx->counter = 0;

			DoBloodSplat(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z, 80, fx->pos.Orientation.y, fx->roomNumber);
			SoundEffect(SFX_TR2_CRUNCH2, &fx->pos);
			KillEffect(fxNumber);
		}
	}

	short ThrowKnife(int x, int y, int z, short velocity, short yRot, short roomNumber)
	{
		short fxNumber = 0;
		// TODO: add fx parameters
		return fxNumber;
	}

	void KnifeThrowerControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short torso = 0;
		short head = 0;
		short tilt = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != KTHROWER_STATE_DEATH)
				SetAnimation(item, KTHROWER_ANIM_DEATH);
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case KTHROWER_STATE_IDLE:
				creature->MaxTurn = 0;

				if (AI.ahead)
					head = AI.angle;

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = KTHROWER_STATE_RUN_FORWARD;
				else if (Targetable(item, &AI))
					item->Animation.TargetState = KTHROWER_STATE_IDLE_KNIFE_ATTACK_START;
				else if (creature->Mood == MoodType::Bored)
				{
					if (!AI.ahead || AI.distance > pow(SECTOR(6), 2))
						item->Animation.TargetState = KTHROWER_STATE_WALK_FORWARD;
				}
				else if (AI.ahead && AI.distance < pow(SECTOR(4), 2))
					item->Animation.TargetState = KTHROWER_STATE_WALK_FORWARD;
				else
					item->Animation.TargetState = KTHROWER_STATE_RUN_FORWARD;

				break;

			case KTHROWER_STATE_WALK_FORWARD:
				creature->MaxTurn = Angle::DegToRad(3.0f);

				if (AI.ahead)
					head = AI.angle;

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = KTHROWER_STATE_RUN_FORWARD;
				else if (Targetable(item, &AI))
				{
					if (AI.distance < pow(SECTOR(2.5f), 2) || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = KTHROWER_STATE_IDLE;
					else if (TestProbability(0.5f))
						item->Animation.TargetState = KTHROWER_STATE_WALK_KNIFE_ATTACK_LEFT_START;
					else
						item->Animation.TargetState = KTHROWER_STATE_WALK_KNIFE_ATTACK_RIGHT_START;
				}
				else if (creature->Mood == MoodType::Bored)
				{
					if (AI.ahead && AI.distance < pow(SECTOR(6), 2))
						item->Animation.TargetState = KTHROWER_STATE_IDLE;
				}
				else if (!AI.ahead || AI.distance > pow(SECTOR(4), 2))
					item->Animation.TargetState = KTHROWER_STATE_RUN_FORWARD;

				break;

			case KTHROWER_STATE_RUN_FORWARD:
				tilt = angle / 3;
				creature->MaxTurn = Angle::DegToRad(6.0f);

				if (AI.ahead)
					head = AI.angle;

				if (Targetable(item, &AI))
					item->Animation.TargetState = KTHROWER_STATE_WALK_FORWARD;
				else if (creature->Mood == MoodType::Bored)
				{
					if (AI.ahead && AI.distance < pow(SECTOR(6), 2))
						item->Animation.TargetState = KTHROWER_STATE_IDLE;
					else
						item->Animation.TargetState = KTHROWER_STATE_WALK_FORWARD;
				}
				else if (AI.ahead && AI.distance < pow(SECTOR(4), 2))
					item->Animation.TargetState = KTHROWER_STATE_WALK_FORWARD;

				break;

			case KTHROWER_STATE_WALK_KNIFE_ATTACK_LEFT_START:
				creature->Flags = 0;

				if (AI.ahead)
					torso = AI.angle;

				if (Targetable(item, &AI))
					item->Animation.TargetState = KTHROWER_STATE_WALK_KNIFE_ATTACK_LEFT_CONTINUE;
				else
					item->Animation.TargetState = KTHROWER_STATE_WALK_FORWARD;

				break;

			case KTHROWER_STATE_WALK_KNIFE_ATTACK_RIGHT_START:
				creature->Flags = 0;

				if (AI.ahead)
					torso = AI.angle;

				if (Targetable(item, &AI))
					item->Animation.TargetState = KTHROWER_STATE_WALK_KNIFE_ATTACK_RIGHT_CONTINUE;
				else
					item->Animation.TargetState = KTHROWER_STATE_WALK_FORWARD;

				break;

			case KTHROWER_STATE_IDLE_KNIFE_ATTACK_START:
				creature->Flags = 0;

				if (AI.ahead)
					torso = AI.angle;

				if (Targetable(item, &AI))
					item->Animation.TargetState = KTHROWER_STATE_IDLE_KNIFE_ATTACK_CONTINUE;
				else
					item->Animation.TargetState = KTHROWER_STATE_IDLE;

				break;

			case KTHROWER_STATE_WALK_KNIFE_ATTACK_LEFT_CONTINUE:
				if (AI.ahead)
					torso = AI.angle;

				if (!creature->Flags)
				{
					CreatureEffect(item, KnifeBiteLeft, ThrowKnife);
					creature->Flags = 1;
				}

				break;

			case KTHROWER_STATE_WALK_KNIFE_ATTACK_RIGHT_CONTINUE:
				if (AI.ahead)
					torso = AI.angle;

				if (!creature->Flags)
				{
					CreatureEffect(item, KnifeBiteRight, ThrowKnife);
					creature->Flags = 1;
				}

				break;

			case KTHROWER_STATE_IDLE_KNIFE_ATTACK_CONTINUE:
				if (AI.ahead)
					torso = AI.angle;

				if (!creature->Flags)
				{
					CreatureEffect(item, KnifeBiteLeft, ThrowKnife);
					CreatureEffect(item, KnifeBiteRight, ThrowKnife);
					creature->Flags = 1;
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, torso);
		CreatureJoint(item, 1, head);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
