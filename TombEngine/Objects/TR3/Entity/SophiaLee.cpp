#include "framework.h"
#include "SophiaLee.h"

#include "Objects/Effects/Boss.h"
#include "Game/animation.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "misc.h"
#include <setup.h>

using namespace TEN::Effects::Boss;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto SOPHIALEE_VAULT_SHIFT = 96;
	constexpr auto SOPHIALEE_WALK_TURN_RATE_MAX = ANGLE(4);
	constexpr auto SOPHIALEE_RUN_TURN_RATE_MAX = ANGLE(7);
	constexpr auto SOPHIALEE_Y_DISTANCE_RANGE = CLICK(2);
	constexpr auto SOPHIALEE_REACHED_GOAL_RANGE = CLICK(2);
	constexpr auto SOPHIALEE_CHARGE_TIMER_DURATION = 600;

	const auto SOPHIALEE_Staff = BiteInfo(Vector3(-28.0f, 56.0f, 356.0f), 10);
	const auto SOPHIALEE_Right = BiteInfo(Vector3(16.0f, 48.0f, 304.0f), 10);
	const auto SOPHIALEE_Left = BiteInfo(Vector3(-72.0f, 48.0f, 356.0f), 10);

	enum SophiaState
	{
		// No state 0.
		SOPHIALEE_STATE_STAND = 1,
		SOPHIALEE_STATE_WALK = 2,
		SOPHIALEE_STATE_RUN = 3,
		SOPHIALEE_STATE_SUMMON = 4,
		SOPHIALEE_STATE_BIG_SHOOT = 5,
		SOPHIALEE_STATE_DEATH = 6,
		SOPHIALEE_STATE_LAUGH = 7,
		SOPHIALEE_STATE_SMALL_SHOOT = 8,
		SOPHIALEE_STATE_CLIMB2 = 9,
		SOPHIALEE_STATE_CLIMB3 = 10,
		SOPHIALEE_STATE_CLIMB4 = 11,
		SOPHIALEE_STATE_FALL4CLICK = 12,
	};

	enum SophiaAnim
	{
		SOPHIALEE_ANIM_WALK = 0,
		SOPHIALEE_ANIM_SUMMON_START = 1,
		SOPHIALEE_ANIM_SUMMON = 2,
		SOPHIALEE_ANIM_SUMMON_END = 3,
		SOPHIALEE_ANIM_SCEPTER_AIM = 4,
		SOPHIALEE_ANIM_SCEPTER_SHOOT = 5,
		SOPHIALEE_ANIM_SCEPTER_AIM_TO_IDLE = 6,
		SOPHIALEE_ANIM_IDLE = 7,
		SOPHIALEE_ANIM_LAUGH = 8,
		SOPHIALEE_ANIM_CLIMB2CLICK = 9,
		SOPHIALEE_ANIM_CLIMB2CLICK_END = 10,
		SOPHIALEE_ANIM_WALK_STOP = 11,
		SOPHIALEE_ANIM_RUN = 12,
		SOPHIALEE_ANIM_RUN_TO_STAND_LEFT = 13,
		SOPHIALEE_ANIM_RUN_TO_WALK_RIGHT = 14,
		SOPHIALEE_ANIM_CLIMB4CLICK = 15,
		SOPHIALEE_ANIM_WALK_START = 16,
		SOPHIALEE_ANIM_DEATH = 17,
		SOPHIALEE_ANIM_CLIMB3CLICK = 18,
		SOPHIALEE_ANIM_WALK_TO_RUN_RIGHT = 19,
		SOPHIALEE_ANIM_RUN_START = 20,
		SOPHIALEE_ANIM_FALL4CLICK = 21,
		SOPHIALEE_ANIM_WALK_STOP_LEFT = 22,
		SOPHIALEE_ANIM_RUN_TO_WALK_LEFT = 23,
		SOPHIALEE_ANIM_RUN_TO_STAND_RIGHT = 24,
		SOPHIALEE_ANIM_SCEPTER_SMALL_SHOOT = 25
	};

	enum class SophiaMode : int
	{
		Normal = 0, // Like other entity, move/climb/attack and chase lara.
		Tower = 1, // TR3 one, which only climb, can't be killed unless a trigger say otherwise (electrical box for example).
	};

	struct SophiaData
	{
		short angle;
		short tilt;
		short headAngle;
		short torsoXAngle;
		short torsoYAngle;
	};

	void InitialiseLondonBoss(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		InitialiseCreature(itemNumber);
		item->SetFlagField(0, 1); // Immortal state.
		item->SetFlagField(4, 0); // Charged state. 1 = full charged.
		item->SetFlagField((int)BossItemFlags::DeathCount, 0);
		item->SetFlagField((int)BossItemFlags::ExplodeCount, 0);
		SetAnimation(item, SOPHIALEE_ANIM_SUMMON_START); // She always start with charging.
	}

	static Vector3i GetAIPosition(ItemInfo* item, int objectNumber, int currentFlagToSearch)
	{
		for (auto& aiObj : g_Level.AIObjects)
		{
			if (aiObj.objectNumber == objectNumber && aiObj.triggerFlags == currentFlagToSearch)
				return aiObj.pos.Position;
		}
		return Vector3i::Zero;
	}

	static void LondonBossTowerControl(ItemInfo* item, CreatureInfo* creature, SophiaData* data)
	{
		AI_INFO ai;
		CreatureAIInfo(item, &ai);

		// Check the old and next position of ai object
		// This will allow sophia to go down or up based on enemy y pos.
		auto oldPosition = GetAIPosition(item, ID_AI_X1, item->ItemFlags[1]);
		auto nextPosition = GetAIPosition(item, ID_AI_X2, item->ItemFlags[1]);
		if (Vector3i::Distance(item->Pose.Position, nextPosition) < SOPHIALEE_REACHED_GOAL_RANGE)
		{
			creature->ReachedGoal = TRUE;
		}
		else
		{
			creature->ReachedGoal = FALSE;
			if (item->ItemFlags[2] == 1) // enemy is ahead
				creature->Target = nextPosition;
			else if (item->ItemFlags[2] == -1)
				creature->Target = oldPosition;
		}
		
		if (item->Timer > 0) // Used for charge count, if 0, sophia will be able to do a new charge animation.
			item->Timer--;

		if (ai.ahead)
		{
			data->headAngle = ai.angle;
		}
		else
		{
			data->torsoXAngle = 0;
			data->torsoYAngle = 0;
		}

		auto* enemy = creature->Enemy;
		bool isEnemyAhead = ai.verticalDistance >= SOPHIALEE_Y_DISTANCE_RANGE; // If enemy is up, then sophia need to get to next AI_X2
		bool isSophiaAhead = ai.verticalDistance <= -SOPHIALEE_Y_DISTANCE_RANGE; // If enemy is down, then sophia need to get to previous AI_X1
		item->ItemFlags[2] = isEnemyAhead ? 1 : isSophiaAhead ? -1 : 0;

		data->angle = CreatureTurn(item, creature->MaxTurn);
		switch (item->Animation.ActiveState)
		{
		case SOPHIALEE_STATE_LAUGH:
			creature->MaxTurn = 0;

			break;
		case SOPHIALEE_STATE_STAND:
			creature->MaxTurn = 0;
			creature->Flags = 0;

			if (item->AIBits & GUARD)
			{
				TENLog("AI_GUARD found.");
			}
			/*else if (enemy->HitPoints <= 0)
			{
				item->Animation.TargetState = SOPHIALEE_STATE_LAUGH;
			}*/
			else if (creature->ReachedGoal)
			{
				if (item->TestFlagField(4, 1))
					item->Animation.TargetState = SOPHIALEE_STATE_BIG_SHOOT;
				else if (!item->Timer)
					item->Animation.TargetState = SOPHIALEE_STATE_SUMMON;
				else
					item->Animation.TargetState = SOPHIALEE_STATE_SMALL_SHOOT;
			}
			else
				item->Animation.TargetState = SOPHIALEE_STATE_RUN;
			break;
		case SOPHIALEE_STATE_WALK:
			creature->MaxTurn = SOPHIALEE_WALK_TURN_RATE_MAX;
			if (creature->ReachedGoal)
			{
				item->Animation.TargetState = SOPHIALEE_STATE_STAND;
				break;
			}

			break;
		case SOPHIALEE_STATE_RUN:
			creature->MaxTurn = SOPHIALEE_RUN_TURN_RATE_MAX;
			if (creature->ReachedGoal)
			{
				item->Animation.TargetState = SOPHIALEE_STATE_STAND;
				break;
			}
			data->tilt = data->angle / 2;

			break;
		case SOPHIALEE_STATE_SUMMON:
			creature->MaxTurn = 0;

			if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + SOPHIALEE_ANIM_SUMMON_START)
			{
				if (item->Animation.FrameNumber == GetFrameNumber(item, 0))
				{
					item->Timer = SOPHIALEE_CHARGE_TIMER_DURATION;
				}
				else if (item->HitStatus && item->Animation.TargetState != SOPHIALEE_STATE_STAND)
				{
					StopSoundEffect(SFX_TR3_SOFIALEE_SUMMON);
					SoundEffect(SFX_TR3_SOFIALEE_SUMMON_FAIL, &item->Pose);
					SoundEffect(SFX_TR3_SOFIALEE_TAKE_HIT, &item->Pose);
					item->Animation.TargetState = SOPHIALEE_STATE_STAND;
				}
			}
			else if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + SOPHIALEE_ANIM_SUMMON && item->Animation.FrameNumber == GetFrameCount(item->Animation.AnimNumber))
			{
				item->SetFlagField(4, 1);
			}

			break;
		case SOPHIALEE_STATE_BIG_SHOOT:
			creature->MaxTurn = 0;
			if (ai.ahead)
			{
				data->torsoYAngle = ai.angle;
				data->torsoXAngle = ai.xAngle;
			}
			item->SetFlagField(4, 0);
			break;
		case SOPHIALEE_STATE_SMALL_SHOOT:
			creature->MaxTurn = 0;
			if (ai.ahead)
			{
				data->torsoYAngle = ai.angle;
				data->torsoXAngle = ai.xAngle;
			}

			break;
		}
	}

	static void LondonBossNormalControl(ItemInfo* item, CreatureInfo* creature, SophiaData* data)
	{

	}

	void LondonBossControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		SophiaData data{};

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != SOPHIALEE_STATE_DEATH)
				SetAnimation(item, SOPHIALEE_ANIM_DEATH);
			AnimateItem(item);
			return;
		}
		else
		{
			if (item->TestOcb((int)SophiaMode::Tower))
				LondonBossTowerControl(item, creature, &data);
			else
				LondonBossNormalControl(item, creature, &data);
		}

		CreatureTilt(item, data.tilt);
		CreatureJoint(item, 0, data.torsoYAngle);
		CreatureJoint(item, 1, data.torsoXAngle);
		CreatureJoint(item, 2, data.headAngle);

		if ((item->Animation.ActiveState < SOPHIALEE_STATE_CLIMB2 || item->Animation.ActiveState > SOPHIALEE_STATE_FALL4CLICK) &&
			 item->Animation.ActiveState != SOPHIALEE_STATE_DEATH)
		{
			switch (CreatureVault(itemNumber, data.angle, 2, SOPHIALEE_VAULT_SHIFT))
			{
			case 2:
				creature->MaxTurn = 0;
				SetAnimation(item, SOPHIALEE_ANIM_CLIMB2CLICK);
				break;
			case 3:
				creature->MaxTurn = 0;
				SetAnimation(item, SOPHIALEE_ANIM_CLIMB3CLICK);
				break;
			case 4:
				creature->MaxTurn = 0;
				SetAnimation(item, SOPHIALEE_ANIM_CLIMB4CLICK);
				break;
			case -4:
				creature->MaxTurn = 0;
				SetAnimation(item, SOPHIALEE_ANIM_FALL4CLICK);
				break;
			}
		}
		else
		{
			CreatureAnimation(itemNumber, data.angle, 0);
		}
	}
}
