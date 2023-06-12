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

	const auto KoldGunBite = CreatureBiteInfo(Vector3i(-20, 440, 20), 9);

	enum KoldState
	{
		// No state 0.
		KOLD_STATE_STOP = 1,
		KOLD_STATE_WALK,
		KOLD_STATE_RUN,
		KOLD_STATE_AIM,
		KOLD_STATE_DEATH,
		KOLD_STATE_SHOOT
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

		if (item.HitPoints <= 0)
		{
			if (item.Animation.ActiveState == KOLD_STATE_DEATH)
				SetAnimation(item, KOLD_ANIM_DEATH);
		}
		else
		{
			AI_INFO ai;
			CreatureAIInfo(&item, &ai);
			GetCreatureMood(&item, &ai, true);
			CreatureMood(&item, &ai, true);
			

		}

		CreatureTilt(&item, tilt);
		CreatureJoint(&item, 0, headY);
		CreatureJoint(&item, 1, torsoX);
		CreatureJoint(&item, 2, torsoY);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
