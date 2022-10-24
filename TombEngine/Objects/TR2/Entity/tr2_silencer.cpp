#include "framework.h"
#include "Objects/TR2/Entity/tr2_silencer.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto SILENCER_SHOOT_ATTACK_DAMAGE = 50;
	constexpr auto SILENCER_RUN_RANGE = SQUARE(SECTOR(2));

	const auto SilencerGunBite = BiteInfo(Vector3(3.0f, 331.0f, 56.0f), 10);

	#define SILENCER_WALK_TURN_RATE_MAX ANGLE(5.0f)
	#define SILENCER_RUN_TURN_RATE_MAX	ANGLE(5.0f)

	enum SilencerState
	{
		// No state 0.
		SILENCER_STATE_WALK_FORWARD = 1,
		SILENCER_STATE_RUN_FORWARD = 2,
		SILENCER_STATE_IDLE_FRAME = 3,
		SILENCER_STATE_IDLE = 4,
		SILENCER_STATE_POSE = 5,
		SILENCER_STATE_AIM_1 = 6,
		SILENCER_STATE_SHOOT_1 = 7,
		// No state 8.
		SILENCER_STATE_RUN_SHOOT = 9,
		SILENCER_STATE_AIM_2 = 10,
		SILENCER_STATE_SHOOT_2 = 11,
		SILENCER_STATE_DEATH_1 = 12,
		SILENCER_STATE_DEATH_2 = 13
	};

	enum SilencerAnim
	{
		SILENCER_ANIM_IDLE_FRAME = 0,
		SILENCER_ANIM_IDLE_TO_WALK_FORWARD = 1,
		SILENCER_ANIM_WALK_FORWARD = 2,
		SILENCER_ANIM_WALK_FORWARD_TO_IDLE = 3,
		SILENCER_ANIM_WALK_FORWARD_TO_POSE = 4,
		SILENCER_ANIM_POSE = 5,
		SILENCER_ANIM_WALK_FORWARD_TO_RUN_FORWARD = 6,
		SILENCER_ANIM_RUN_FORWARD = 7,
		SILENCER_ANIM_RUN_FORWARD_TO_IDLE = 8,
		SILENCER_ANIM_IDLE_TO_RUN_FORWARD = 9,
		SILENCER_ANIM_POSE_TO_IDLE = 10,
		SILENCER_ANIM_RUN_FORWARD_AIM_LEFT = 11,
		SILENCER_ANIM_RUN_FORWARD_SHOOT_LEFT = 12,
		SILENCER_ANIM_RUN_FORWARD_UNAIM_LEFT = 13,
		SILENCER_ANIM_AIM_1_START = 14,
		SILENCER_ANIM_AIM_1_CONTINUE = 15,
		SILENCER_ANIM_SHOOT_1 = 16,
		SILENCER_ANIM_UNAIM_1 = 17,
		SILENCER_ANIM_POSE_TO_AIM_1 = 18,
		SILENCER_ANIM_IDLE = 19,
		SILENCER_ANIM_DEATH_1 = 20,
		SILENCER_ANIM_DEATH_2 = 21, // Unused.
		SILENCER_ANIM_AIM_2_START = 22,
		SILENCER_ANIM_AIM_2_CONTINUE = 23,
		SILENCER_ANIM_SHOOT_2 = 24,
		SILENCER_ANIM_UNAIM_2 = 25,
		SILENCER_ANIM_RUN_FORWARD_AIM_RIGHT = 26,
		SILENCER_ANIM_RUN_FORWARD_SHOOT_RIGHT = 27,
		SILENCER_ANIM_RUN_FORWARD_UNAIM_RIGHT = 28
	};

	void SilencerControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		auto extraHeadRot = EulerAngles::Zero;
		auto extraTorsoRot = EulerAngles::Zero;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != SILENCER_STATE_DEATH_1 &&
				item->Animation.ActiveState != SILENCER_STATE_DEATH_2)
			{
				SetAnimation(item, SILENCER_ANIM_DEATH_1);
			}
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
			case SILENCER_STATE_IDLE_FRAME:
				creature->MaxTurn = 0;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;

				break;

			case SILENCER_STATE_IDLE:
				creature->MaxTurn = 0;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (creature->Mood == MoodType::Escape)
				{
					item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;
					item->Animation.RequiredState = SILENCER_STATE_RUN_FORWARD;
				}
				else
				{
					if (Targetable(item, &AI))
					{
						item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;
						item->Animation.RequiredState = TestProbability(0.5f) ? SILENCER_STATE_AIM_1 : SILENCER_STATE_AIM_2;
					}

					if (creature->Mood == MoodType::Attack || !AI.ahead)
					{
						if (AI.distance >= SILENCER_RUN_RANGE)
						{
							item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;
							item->Animation.RequiredState = SILENCER_STATE_RUN_FORWARD;
						}
						else
						{
							item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;
							item->Animation.RequiredState = SILENCER_STATE_WALK_FORWARD;
						}
					}
					else
					{
						if (TestProbability(0.96f))
						{
							if (TestProbability(0.08f))
							{
								item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;
								item->Animation.RequiredState = SILENCER_STATE_WALK_FORWARD;
							}
						}
						else
						{
							item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;
							item->Animation.RequiredState = SILENCER_STATE_POSE;
						}
					}
				}

				break;

			case SILENCER_STATE_WALK_FORWARD:
				creature->MaxTurn = SILENCER_WALK_TURN_RATE_MAX;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = SILENCER_STATE_RUN_FORWARD;
				else if (Targetable(item, &AI))
				{
					item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;
					item->Animation.RequiredState = TestProbability(0.5f) ? SILENCER_STATE_AIM_1 : SILENCER_STATE_AIM_2;
				}
				else
				{
					if (AI.distance > SILENCER_RUN_RANGE || !AI.ahead)
						item->Animation.TargetState = SILENCER_STATE_RUN_FORWARD;
					if (creature->Mood == MoodType::Bored && TestProbability(0.025f))
						item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;
				}

				break;

			case SILENCER_STATE_RUN_FORWARD:
				creature->MaxTurn = SILENCER_RUN_TURN_RATE_MAX;
				creature->Flags = 0;
				tilt = angle / 4;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (creature->Mood == MoodType::Escape)
				{
					if (Targetable(item, &AI))
						item->Animation.TargetState = SILENCER_STATE_RUN_SHOOT;

					break;
				}

				if (Targetable(item, &AI))
				{
					if (AI.distance >= SILENCER_RUN_RANGE && AI.zoneNumber == AI.enemyZone)
						item->Animation.TargetState = SILENCER_STATE_RUN_SHOOT;

					break;
				}
				else if (creature->Mood == MoodType::Attack)
					item->Animation.TargetState = TestProbability(0.5f) ? SILENCER_STATE_RUN_FORWARD : SILENCER_STATE_IDLE_FRAME;
				else
					item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;

				break;

			case SILENCER_STATE_POSE:
				creature->MaxTurn = 0;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (Targetable(item, &AI))
				{
					item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;
					item->Animation.RequiredState = SILENCER_STATE_AIM_1;
				}
				else
				{
					if (creature->Mood == MoodType::Attack || TestProbability(1.0f / 128))
						item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;

					if (!AI.ahead)
						item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;
				}

				break;

			case SILENCER_STATE_AIM_1:
			case SILENCER_STATE_AIM_2:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}
				else
					extraHeadRot.y = AI.angle;

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;
				else if (Targetable(item, &AI))
					item->Animation.TargetState = (item->Animation.ActiveState != SILENCER_STATE_AIM_1) ? SILENCER_STATE_SHOOT_2 : SILENCER_STATE_SHOOT_1;
				else
					item->Animation.TargetState = SILENCER_STATE_IDLE_FRAME;

				break;

			case SILENCER_STATE_SHOOT_1:
			case SILENCER_STATE_SHOOT_2:
				creature->MaxTurn = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}
				else
					extraHeadRot.y = AI.angle;

				if (!creature->Flags)
				{
					ShotLara(item, &AI, SilencerGunBite, extraTorsoRot.y, SILENCER_SHOOT_ATTACK_DAMAGE);
					creature->Flags = 1;
				}

				break;

			case SILENCER_STATE_RUN_SHOOT:
				creature->MaxTurn = SILENCER_RUN_TURN_RATE_MAX;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}
				else
					extraHeadRot.y = AI.angle;

				if (!item->Animation.RequiredState)
				{
					if (!ShotLara(item, &AI, SilencerGunBite, extraTorsoRot.y, SILENCER_SHOOT_ATTACK_DAMAGE))
						item->Animation.TargetState = SILENCER_STATE_RUN_FORWARD;

					item->Animation.RequiredState = SILENCER_STATE_RUN_SHOOT;
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, extraTorsoRot.y);
		CreatureJoint(item, 1, extraTorsoRot.x);
		CreatureJoint(item, 2, extraHeadRot.y);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
