#include "framework.h"
#include "Objects/TR3/Entity/Winston.h"

#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Specific/clock.h"
#include "Specific/level.h"

using namespace TEN::Math;

// NOTES:
// ItemFlags[0]: defeat timer.

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto WINSTON_IDLE_RANGE = SQUARE(BLOCK(1.5f));
	constexpr auto WINSTON_TURN_RATE_MAX = ANGLE(2.0f);

	constexpr auto WINSTON_RECOVER_HIT_POINTS = 16;
	constexpr auto WINSTON_DEFEAT_TIMER_MAX = 5 * FPS;

	enum WinstonState
	{
		// No state 0.
		WINSTON_STATE_IDLE = 1,
		WINSTON_STATE_WALK_FORWARD = 2,
		WINSTON_STATE_GUARD_1 = 3,
		WINSTON_STATE_GUARD_2 = 4, // Unused.
		WINSTON_STATE_GUARD_3 = 5, // Unused.
		WINSTON_STATE_RECOIL_1 = 6,
		WINSTON_STATE_RECOIL_2 = 7, // Unused.
		WINSTON_STATE_RECOIL_3 = 8, // Unused.
		WINSTON_STATE_DEFEAT_CONTINUE = 9,
		WINSTON_STATE_DEFEAT_START = 10,
		WINSTON_STATE_DEFEAT_RECOVER = 11,
		WINSTON_STATE_BRUSH_OFF = 12,
		WINSTON_STATE_DEFEAT_END = 13
	};

	enum WinstonAnim
	{

	};

	void OldControlWinston(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& creature = *GetCreatureInfo(&item);

		AI_INFO ai;
		CreatureAIInfo(&item, &ai);
		GetCreatureMood(&item, &ai, 1);
		CreatureMood(&item, &ai, 1);

		short headingAngle = CreatureTurn(&item, creature.MaxTurn);

		if (item.Animation.ActiveState == WINSTON_STATE_IDLE)
		{
			if ((ai.distance > WINSTON_IDLE_RANGE || !ai.ahead) && item.Animation.TargetState != WINSTON_STATE_WALK_FORWARD)
			{
				item.Animation.TargetState = WINSTON_STATE_WALK_FORWARD;
				//SoundEffect(SFX_WILARD_STAB, &item.Pose, SFX_DEFAULT);
			}
		}
		else if (ai.distance < WINSTON_IDLE_RANGE)
		{
			if (ai.ahead)
			{
				item.Animation.TargetState = WINSTON_STATE_IDLE;

				if (creature.Flags & 1)
					creature.Flags--;
			}
			else if (!(creature.Flags & 1))
			{
				//SoundEffect(SFX_WILARD_ODD_NOISE, &item.Pose, SFX_DEFAULT);
				//SoundEffect(SFX_LITTLE_SUB_START, &item.Pose, SFX_DEFAULT);
				creature.Flags |= 1;
			}
		}

		if (item.TouchBits.TestAny())
		{
			if (!(creature.Flags & 2))
			{
				//SoundEffect(SFX_LITTLE_SUB_LOOP, &item.Pose, SFX_DEFAULT);
				//SoundEffect(SFX_LITTLE_SUB_START, &item.Pose, SFX_DEFAULT);
				creature.Flags |= 2;
			}
		}
		else if (creature.Flags & 2)
		{
			creature.Flags -= 2;
		}

		//if (Random::TestProbability(1 / 128.0f))
			//SoundEffect(SFX_LITTLE_SUB_START, &item.Pose, SFX_DEFAULT);

		CreatureAnimation(itemNumber, headingAngle, 0);
	}

	void ControlWinston(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;
		
		auto& item = g_Level.Items[itemNumber];
		auto& creature = *GetCreatureInfo(&item);
		const auto& player = Lara;

		short& defeatTimer = item.ItemFlags[0];

		AI_INFO ai;
		CreatureAIInfo(&item, &ai);
		GetCreatureMood(&item, &ai, 1);
		CreatureMood(&item, &ai, 1);

		short headingAngle = CreatureTurn(&item, creature.MaxTurn);

		if (item.HitPoints <= 0)
		{
			creature.MaxTurn = 0;

			switch (item.Animation.ActiveState)
			{
			case WINSTON_STATE_DEFEAT_CONTINUE:
			case WINSTON_STATE_DEFEAT_START:
				if (item.HitStatus)
				{
					item.Animation.TargetState = WINSTON_STATE_DEFEAT_CONTINUE;
				}
				else
				{
					defeatTimer--;
					if (defeatTimer < 0)
						item.Animation.TargetState = WINSTON_STATE_DEFEAT_END;
				}

				break;

			case WINSTON_STATE_DEFEAT_RECOVER:
				item.HitPoints = WINSTON_RECOVER_HIT_POINTS;

				if (Random::TestProbability(1 / 2.0f))
					creature.Flags = 999;

				break;

			case WINSTON_STATE_DEFEAT_END:
				if (item.HitStatus)
				{
					item.Animation.TargetState = WINSTON_STATE_DEFEAT_CONTINUE;
				}
				else
				{
					defeatTimer--;
					if (defeatTimer < 0)
						item.Animation.TargetState = WINSTON_STATE_DEFEAT_RECOVER;
				}

				break;

			default:
				SetAnimation(item, ID_ARMY_WINSTON, 16);
				defeatTimer = WINSTON_DEFEAT_TIMER_MAX;
				break;
			}
		}
		else
		{
			switch (item.Animation.ActiveState)
			{
			case WINSTON_STATE_IDLE:
				creature.MaxTurn = WINSTON_TURN_RATE_MAX;

				if (creature.Flags == 999)
				{
					item.Animation.TargetState = WINSTON_STATE_BRUSH_OFF;
				}
				else if (player.TargetEntity == &item)
				{
					item.Animation.TargetState = WINSTON_STATE_GUARD_1;
				}
				else if ((ai.distance > WINSTON_IDLE_RANGE || !ai.ahead) && item.Animation.TargetState != WINSTON_STATE_WALK_FORWARD)
				{
					item.Animation.TargetState = WINSTON_STATE_WALK_FORWARD;
					SoundEffect(SFX_TR2_WINSTON_CUPS, &item.Pose);
				}

				break;

			case WINSTON_STATE_WALK_FORWARD:
				creature.MaxTurn = WINSTON_TURN_RATE_MAX;

				if (player.TargetEntity == &item)
				{
					item.Animation.TargetState = WINSTON_STATE_IDLE;
				}
				else if (ai.distance < WINSTON_IDLE_RANGE)
				{
					if (ai.ahead)
					{
						item.Animation.TargetState = WINSTON_STATE_IDLE;

						if (creature.Flags & 1)
							creature.Flags--;
					}
					else if ((creature.Flags & 1) == 0)
					{
						creature.Flags |= 1;

						SoundEffect(SFX_TR2_WINSTON_SURPRISED, &item.Pose);
						SoundEffect(SFX_TR2_WINSTON_SHUFFLE, &item.Pose);
					}
				}

				break;

			case WINSTON_STATE_GUARD_1:
				creature.MaxTurn = WINSTON_TURN_RATE_MAX;

				if (item.Animation.RequiredState != NO_STATE)
					item.Animation.TargetState = item.Animation.RequiredState;

				if (item.HitStatus)
				{
					item.Animation.TargetState = WINSTON_STATE_RECOIL_1;
				}
				else if (player.TargetEntity != &item)
				{
					item.Animation.TargetState = WINSTON_STATE_IDLE;
				}

				break;

			case WINSTON_STATE_GUARD_2:
				creature.MaxTurn = WINSTON_TURN_RATE_MAX;

				if (item.Animation.RequiredState != NO_STATE)
					item.Animation.TargetState = item.Animation.RequiredState;

				if (item.HitStatus)
					item.Animation.TargetState = WINSTON_STATE_RECOIL_2;

				break;

			case WINSTON_STATE_GUARD_3:
				creature.MaxTurn = WINSTON_TURN_RATE_MAX;

				if (item.Animation.RequiredState != NO_STATE)
					item.Animation.TargetState = item.Animation.RequiredState;

				if (item.HitStatus)
				{
					item.Animation.TargetState = WINSTON_STATE_RECOIL_3;
				}
				else if (player.TargetEntity == &item)
				{
					item.Animation.TargetState = WINSTON_STATE_GUARD_1;
				}

				break;

			case WINSTON_STATE_RECOIL_1:
				item.Animation.RequiredState = Random::TestProbability(1 / 2.0f) ? WINSTON_STATE_GUARD_3 : WINSTON_STATE_GUARD_2;
				break;

			case WINSTON_STATE_RECOIL_2:
			case WINSTON_STATE_RECOIL_3:
				item.Animation.RequiredState = WINSTON_STATE_GUARD_1;
				break;

			case WINSTON_STATE_BRUSH_OFF:
				creature.MaxTurn = 0;
				creature.Flags = 0;
				break;
			}
		}

		if (Random::TestProbability(1 / 128.0f))
			SoundEffect(SFX_TR2_WINSTON_SHUFFLE, &item.Pose);

		CreatureAnimation(itemNumber, headingAngle, 0);
	}
}
