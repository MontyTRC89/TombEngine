#include "framework.h"
#include "Objects/TR1/Entity/Kold.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/misc.h"
#include "Game/people.h"

namespace TEN::Entities::Creatures::TR1
{
	constexpr auto KOLD_WALK_TURN_RATE = ANGLE(3.0f);
	constexpr auto KOLD_RUN_TURN_RATE = ANGLE(6.0f);
	constexpr auto KOLD_WALK_RANGE = SQUARE(BLOCK(4.0f));
	constexpr auto KOLD_DAMAGE = 150;
	constexpr auto KOLD_SHOTGUN_SHELL_COUNT = 6;

	const auto KoldGunBite = CreatureBiteInfo(Vector3i(-20, 440, 20), 9);

	enum KoldState
	{
		KOLD_STATE_DEATH = 0,
		KOLD_STATE_STOP = 1,
		KOLD_STATE_WALK = 2,
		KOLD_STATE_RUN = 3,
		KOLD_STATE_AIM = 4,
		KOLD_STATE_SHOOT = 6
	};

	enum KoldAnim
	{
		KOLD_ANIM_RUN_STOP_START = 0,
		KOLD_ANIM_RUN_STOP,
		KOLD_ANIM_IDLE_START_AIM,
		KOLD_ANIM_RUN_START_1,
		KOLD_ANIM_RUN_START_2,
		KOLD_ANIM_IDLE_AIM,
		KOLD_ANIM_IDLE_AIM_STOP,
		KOLD_ANIM_IDLE,
		KOLD_ANIM_RUN,
		KOLD_ANIM_WALK,
		KOLD_ANIM_RELOAD,
		KOLD_ANIM_SHOOT,
		KOLD_ANIM_WALK_START,
		KOLD_ANIM_WALK_STOP,
		KOLD_ANIM_DEATH
	};

	void InitializeKold(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		InitializeCreature(itemNumber);
		SetAnimation(item, KOLD_ANIM_IDLE);
	}

	void KoldControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& creature = *GetCreatureInfo(&item);
		short angle = 0, tilt = 0, headY = 0, torsoY = 0, torsoX = 0;

		if (creature.MuzzleFlash[0].Delay != 0)
			creature.MuzzleFlash[0].Delay--;

		if (item.HitPoints <= 0)
		{
			if (item.Animation.ActiveState != KOLD_STATE_DEATH)
				SetAnimation(item, KOLD_ANIM_DEATH);
		}
		else
		{
			AI_INFO ai;
			CreatureAIInfo(&item, &ai);
			if (ai.ahead)
			{
				headY = ai.angle / 2;
				torsoY = ai.angle / 2;
				torsoX = ai.xAngle / 2;
			}
			GetCreatureMood(&item, &ai, true);
			CreatureMood(&item, &ai, true);
			angle = CreatureTurn(&item, creature.MaxTurn);

			switch (item.Animation.ActiveState)
			{
			case KOLD_STATE_STOP:
				creature.MaxTurn = 0;

				if (item.Animation.RequiredState != NO_STATE)
					item.Animation.TargetState = item.Animation.RequiredState;
				else if (Targetable(&item, &ai))
					item.Animation.TargetState = KOLD_STATE_AIM;
				else if (creature.Mood == MoodType::Bored)
					item.Animation.TargetState = KOLD_STATE_WALK;
				else
					item.Animation.TargetState = KOLD_STATE_RUN;
				break;
			case KOLD_STATE_WALK:
				creature.MaxTurn = KOLD_WALK_TURN_RATE;

				if (creature.Mood == MoodType::Escape || !ai.ahead)
				{
					item.Animation.RequiredState = KOLD_STATE_RUN;
					item.Animation.TargetState = KOLD_STATE_STOP;
				}
				else if (Targetable(&item, &ai))
				{
					item.Animation.RequiredState = KOLD_STATE_AIM;
					item.Animation.TargetState = KOLD_STATE_STOP;
				}
				else if (ai.distance > KOLD_WALK_RANGE)
				{
					item.Animation.RequiredState = KOLD_STATE_RUN;
					item.Animation.TargetState = KOLD_STATE_STOP;
				}
				break;
			case KOLD_STATE_RUN:
				creature.MaxTurn = KOLD_RUN_TURN_RATE;
				tilt = angle / 2;

				if (creature.Mood == MoodType::Escape && !ai.ahead)
					break;

				if (Targetable(&item, &ai))
				{
					item.Animation.RequiredState = KOLD_STATE_AIM;
					item.Animation.TargetState = KOLD_STATE_STOP;
				}
				else if (ai.ahead && ai.distance < KOLD_WALK_RANGE)
				{
					item.Animation.RequiredState = KOLD_STATE_WALK;
					item.Animation.TargetState = KOLD_STATE_STOP;
				}

				break;
			case KOLD_STATE_AIM:
				creature.MaxTurn = 0;
				creature.Flags = 0;

				if (item.Animation.RequiredState != NO_STATE)
					item.Animation.TargetState = KOLD_STATE_STOP;
				else if (Targetable(&item, &ai))
					item.Animation.TargetState = KOLD_STATE_SHOOT;
				else
					item.Animation.TargetState = KOLD_STATE_STOP;
				break;
			case KOLD_STATE_SHOOT:
				creature.MaxTurn = 0;

				if (creature.Flags == 0)
				{
					creature.MuzzleFlash[0].Bite = KoldGunBite;
					creature.MuzzleFlash[0].Delay = 2;
					ShotLara(&item, &ai, KoldGunBite, headY, KOLD_DAMAGE);
					creature.Flags = 1;
				}

				if (creature.Mood == MoodType::Escape)
					item.Animation.RequiredState = KOLD_STATE_RUN;
				break;
			}
		}

		CreatureTilt(&item, tilt);
		CreatureJoint(&item, 0, headY);
		CreatureJoint(&item, 1, torsoX);
		CreatureJoint(&item, 2, torsoY);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
