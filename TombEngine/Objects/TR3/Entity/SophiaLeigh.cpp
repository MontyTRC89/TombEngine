#include "framework.h"
#include "SophiaLeigh.h"

#include "Objects/Effects/Boss.h"
#include "Game/animation.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Objects/Effects/enemy_missile.h"
#include "Game/items.h"
#include "Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "misc.h"
#include "setup.h"
#include "lara_helpers.h"
#include "tomb4fx.h"
#include "people.h"

using namespace TEN::Effects::Boss;
using namespace TEN::Entities::Effects;

namespace TEN::Entities::Creatures::TR3
{
	const auto SOPHIALEIGH_Staff = BiteInfo(Vector3(-28.0f, 56.0f, 356.0f), 10);
	const auto SOPHIALEIGH_Right = BiteInfo(Vector3(16.0f, 48.0f, 304.0f), 10);
	const auto SOPHIALEIGH_Left = BiteInfo(Vector3(-72.0f, 48.0f, 356.0f), 10);

	// Basic value.

	constexpr auto SOPHIALEIGH_VAULT_SHIFT = 96;
	constexpr auto SOPHIALEIGH_WALK_TURN_RATE_MAX = ANGLE(4);
	constexpr auto SOPHIALEIGH_RUN_TURN_RATE_MAX = ANGLE(7);
	constexpr auto SOPHIALEIGH_LASER_DECREASE_XANGLE_IF_LARA_CROUCH = ANGLE(0.25f);
	constexpr auto SOPHIALEIGH_LASER_DISPERSION_ANGLE = ANGLE(1.5f);
	constexpr auto SOPHIALEIGH_WALK_RANGE = SQUARE(BLOCK(1));
	constexpr auto SOPHIALEIGH_Y_DISTANCE_RANGE = BLOCK(1.5f);
	constexpr auto SOPHIALEIGH_REACHED_GOAL_RANGE = CLICK(2);
	constexpr auto SOPHIALEIGH_CHARGE_TIMER_DURATION = 600;
	constexpr auto SOPHIALEIGH_EXPLOSION_NUM_MAX = 60;
	constexpr auto SOPHIALEIGH_EFFECT_COLOR = Vector4(0.0f, 0.4f, 0.5f, 0.5f);

	// Additional normal value.
	constexpr auto SOPHIALEIGH_NORMAL_ATTACK_RANGE = SQUARE(BLOCK(5));
	constexpr auto SOPHIALEIGH_NORMAL_WALK_RANGE = SQUARE(BLOCK(5)); // < walk else run.


	enum SophiaState
	{
		// No state 0.
		SOPHIALEIGH_STATE_STAND = 1,
		SOPHIALEIGH_STATE_WALK = 2,
		SOPHIALEIGH_STATE_RUN = 3,
		SOPHIALEIGH_STATE_SUMMON = 4,
		SOPHIALEIGH_STATE_BIG_SHOOT = 5,
		SOPHIALEIGH_STATE_DEATH = 6,
		SOPHIALEIGH_STATE_LAUGH = 7,
		SOPHIALEIGH_STATE_SMALL_SHOOT = 8,
		SOPHIALEIGH_STATE_CLIMB2 = 9,
		SOPHIALEIGH_STATE_CLIMB3 = 10,
		SOPHIALEIGH_STATE_CLIMB4 = 11,
		SOPHIALEIGH_STATE_FALL4CLICK = 12,
	};

	enum SophiaAnim
	{
		SOPHIALEIGH_ANIM_WALK = 0,
		SOPHIALEIGH_ANIM_SUMMON_START = 1,
		SOPHIALEIGH_ANIM_SUMMON = 2,
		SOPHIALEIGH_ANIM_SUMMON_END = 3,
		SOPHIALEIGH_ANIM_SCEPTER_AIM = 4,
		SOPHIALEIGH_ANIM_SCEPTER_SHOOT = 5,
		SOPHIALEIGH_ANIM_SCEPTER_AIM_TO_IDLE = 6,
		SOPHIALEIGH_ANIM_IDLE = 7,
		SOPHIALEIGH_ANIM_LAUGH = 8,
		SOPHIALEIGH_ANIM_CLIMB2CLICK = 9,
		SOPHIALEIGH_ANIM_CLIMB2CLICK_END = 10,
		SOPHIALEIGH_ANIM_WALK_STOP = 11,
		SOPHIALEIGH_ANIM_RUN = 12,
		SOPHIALEIGH_ANIM_RUN_TO_STAND_LEFT = 13,
		SOPHIALEIGH_ANIM_RUN_TO_WALK_RIGHT = 14,
		SOPHIALEIGH_ANIM_CLIMB4CLICK = 15,
		SOPHIALEIGH_ANIM_WALK_START = 16,
		SOPHIALEIGH_ANIM_DEATH = 17,
		SOPHIALEIGH_ANIM_CLIMB3CLICK = 18,
		SOPHIALEIGH_ANIM_WALK_TO_RUN_RIGHT = 19,
		SOPHIALEIGH_ANIM_RUN_START = 20,
		SOPHIALEIGH_ANIM_FALL4CLICK = 21,
		SOPHIALEIGH_ANIM_WALK_STOP_LEFT = 22,
		SOPHIALEIGH_ANIM_RUN_TO_WALK_LEFT = 23,
		SOPHIALEIGH_ANIM_RUN_TO_STAND_RIGHT = 24,
		SOPHIALEIGH_ANIM_SCEPTER_SMALL_SHOOT = 25
	};

	enum class SophiaOCB : int
	{
		OCB_Normal = 1, // Like other entity, move/climb/attack and chase lara.
		OCB_Tower = 2, // TR3 one, which only climb, can't be killed unless a trigger say otherwise (electrical box for example).
		OCB_LuaToMoveUpDown = 4, // TR3 one but use volume to move her instead of height check, they need to increase or decrease creature->LocationAI for her to go up/down.
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
		if (item->TriggerFlags == 0) // if no mode is set then set it to normal by default.
			item->TriggerFlags = (short)SophiaOCB::OCB_Normal;
		item->SetFlagField((int)BossItemFlags::ImmortalState, TRUE); // Immortal state.
		item->SetFlagField((int)BossItemFlags::ChargedState, FALSE); // Charged state. 1 = full charged.
		item->SetFlagField((int)BossItemFlags::DeathCount, 0);
		item->SetFlagField((int)BossItemFlags::ExplodeCount, 0);
		SetAnimation(item, SOPHIALEIGH_ANIM_SUMMON_START); // She always start with charging.
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

	static void RotateToTarget(ItemInfo* item, AI_INFO* ai, short angleRate)
	{
		if (abs(ai->angle) < angleRate)
			item->Pose.Orientation.y += ai->angle;
		else if (ai->angle < 0)
			item->Pose.Orientation.y -= angleRate;
		else
			item->Pose.Orientation.y += angleRate;
	}

	static void TriggerLaserBolt(ItemInfo* item, ItemInfo* enemy, const BiteInfo& bite, SophiaData* data, bool isBigLaser, short angleAdd)
	{
		short fxNumber = CreateNewEffect(item->RoomNumber);
		if (fxNumber == -1)
			return;

		auto pos = GetJointPosition(item, bite.meshNum, bite.Position);
		auto& fx = EffectList[fxNumber];
		auto laserType = isBigLaser ? (short)MissileType::SophiaLeigh_Big : (short)MissileType::SophiaLeigh_Small;
		fx.pos.Position.x = pos.x;
		fx.pos.Position.y = pos.y;
		fx.pos.Position.z = pos.z;
		fx.pos.Orientation.x = item->Pose.Orientation.x + data->torsoXAngle;

		if (enemy->IsLara())
		{
			auto lara = GetLaraInfo(enemy);
			if (lara->Control.IsLow)
				fx.pos.Orientation.x -= SOPHIALEIGH_LASER_DECREASE_XANGLE_IF_LARA_CROUCH;
		}

		fx.pos.Orientation.y = item->Pose.Orientation.y + data->torsoYAngle + angleAdd;
		fx.pos.Orientation.z = 0;
		fx.roomNumber = item->RoomNumber;
		fx.counter = 0;
		fx.flag1 = laserType;
		fx.flag2 = isBigLaser ? 10 : 2; // damage
		fx.speed = Random::GenerateInt(120, 160);
		fx.objectNumber = ID_ENERGY_BUBBLES;
		fx.frameNumber = Objects[fx.objectNumber].meshIndex + (laserType - 1);
	}

	static void TriggerSophiaShockwave(ItemInfo* item, const BiteInfo& bite)
	{
		short shockwaveID = GetFreeShockwave();
		if (shockwaveID != NO_ITEM)
		{
			auto* ringEffect = &ShockWaves[shockwaveID];
			auto pos = GetJointPosition(item, bite.meshNum, bite.Position);
			ringEffect->x = pos.x;
			ringEffect->y = pos.y;
			ringEffect->z = pos.z;
			ringEffect->innerRad = 620;
			ringEffect->outerRad = 640;
			ringEffect->xRot = Random::GenerateAngle(-45, 45);
			ringEffect->damage = 0;
			ringEffect->r = 0;
			ringEffect->g = 255;
			ringEffect->b = 255;
			ringEffect->speed = -400;
			ringEffect->life = 64;
		}
	}

	static void LondonBossTowerControl(ItemInfo* item, CreatureInfo* creature, SophiaData* data)
	{
		if (item->AIBits)
			GetAITarget(creature);

		AI_INFO ai;
		CreatureAIInfo(item, &ai);

		// Check the old and next position of ai object
		// This will allow sophia to go down or up based on enemy y pos.
		FindAITargetObject(creature, ID_AI_X1, creature->LocationAI, false);
		// Avoid increasing the value when a enemy shoot her.
		if (Vector3i::Distance(item->Pose.Position, creature->Enemy->Pose.Position) < SOPHIALEIGH_REACHED_GOAL_RANGE)
		{
			creature->ReachedGoal = TRUE;
			creature->Enemy = LaraItem; // TODO: deal with LaraItem global.
			if (!(item->TriggerFlags & (short)SophiaOCB::OCB_LuaToMoveUpDown))
			{
				// If enemy is up, then sophia need to get to next AI_X2
				if (ai.verticalDistance >= SOPHIALEIGH_Y_DISTANCE_RANGE)
					creature->LocationAI++;
				// If enemy is down, then sophia need to get to previous AI_X1
				else if (ai.verticalDistance <= -SOPHIALEIGH_Y_DISTANCE_RANGE)
					creature->LocationAI--;
			}
		}
		else
		{
			creature->ReachedGoal = FALSE;
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

		GetCreatureMood(item, &ai, true);
		CreatureMood(item, &ai, true);

		data->angle = CreatureTurn(item, creature->MaxTurn);
		switch (item->Animation.ActiveState)
		{
		case SOPHIALEIGH_STATE_LAUGH:
			creature->MaxTurn = 0;
			RotateToTarget(item, &ai, SOPHIALEIGH_WALK_TURN_RATE_MAX);
			break;
		case SOPHIALEIGH_STATE_STAND:
			creature->MaxTurn = 0;
			creature->Flags = 0;

			if (item->AIBits & GUARD)
			{
				TENLog("AI_GUARD found.");
			}
			else if (creature->Enemy->IsLara() && creature->Enemy->HitPoints <= 0)
			{
				item->Animation.TargetState = SOPHIALEIGH_STATE_LAUGH;
			}
			else if (creature->ReachedGoal)
			{
				if (item->TestFlagField((int)BossItemFlags::ChargedState, TRUE))
					item->Animation.TargetState = SOPHIALEIGH_STATE_BIG_SHOOT;
				else if (item->Timer <= 0)
					item->Animation.TargetState = SOPHIALEIGH_STATE_SUMMON;
				else
					item->Animation.TargetState = SOPHIALEIGH_STATE_SMALL_SHOOT;
			}
			else
				item->Animation.TargetState = SOPHIALEIGH_STATE_RUN;
			break;
		case SOPHIALEIGH_STATE_WALK:
			creature->MaxTurn = SOPHIALEIGH_WALK_TURN_RATE_MAX;
			if (creature->ReachedGoal)
				item->Animation.TargetState = SOPHIALEIGH_STATE_STAND;
			else if (ai.distance > SOPHIALEIGH_WALK_RANGE)
				item->Animation.TargetState = SOPHIALEIGH_STATE_RUN;
			break;
		case SOPHIALEIGH_STATE_RUN:
			creature->MaxTurn = SOPHIALEIGH_RUN_TURN_RATE_MAX;
			if (creature->ReachedGoal)
			{
				item->Animation.TargetState = SOPHIALEIGH_STATE_STAND;
				break;
			}
			data->tilt = data->angle / 2;

			break;
		case SOPHIALEIGH_STATE_SUMMON:
			creature->MaxTurn = 0;
			RotateToTarget(item, &ai, SOPHIALEIGH_WALK_TURN_RATE_MAX);

			if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + SOPHIALEIGH_ANIM_SUMMON_START)
			{
				if (item->Animation.FrameNumber == GetFrameNumber(item, 0))
				{
					item->Timer = SOPHIALEIGH_CHARGE_TIMER_DURATION;
				}
				else if (item->HitStatus && item->Animation.TargetState != SOPHIALEIGH_STATE_STAND)
				{
					StopSoundEffect(SFX_TR3_SOFIALEE_SUMMON);
					SoundEffect(SFX_TR3_SOFIALEE_SUMMON_FAIL, &item->Pose);
					SoundEffect(SFX_TR3_SOFIALEE_TAKE_HIT, &item->Pose);
					item->Animation.TargetState = SOPHIALEIGH_STATE_STAND;
				}
			}
			else if (item->Animation.AnimNumber == (Objects[item->ObjectNumber].animIndex + SOPHIALEIGH_ANIM_SUMMON) && item->Animation.FrameNumber >= GetFrameCount(item->Animation.AnimNumber) - 1)
			{
				item->SetFlagField((int)BossItemFlags::ChargedState, TRUE);
			}

			//TriggerSophiaShockwave(item, SOPHIALEIGH_Staff); // SHITTY EFFECT

			break;
		case SOPHIALEIGH_STATE_BIG_SHOOT:
			creature->MaxTurn = 0;
			RotateToTarget(item, &ai, SOPHIALEIGH_WALK_TURN_RATE_MAX);
			if (ai.ahead)
			{
				data->torsoYAngle = ai.angle;
				data->torsoXAngle = ai.xAngle;
			}

			item->SetFlagField((int)BossItemFlags::ChargedState, FALSE);

			if (item->Animation.FrameNumber == GetFrameNumber(item, 36))
			{
				TriggerLaserBolt(item, creature->Enemy, SOPHIALEIGH_Right, data, false, SOPHIALEIGH_LASER_DISPERSION_ANGLE);
				TriggerLaserBolt(item, creature->Enemy, SOPHIALEIGH_Staff, data, true, 0);
				TriggerLaserBolt(item, creature->Enemy, SOPHIALEIGH_Left, data, false, -SOPHIALEIGH_LASER_DISPERSION_ANGLE);
			}
			
			break;
		case SOPHIALEIGH_STATE_SMALL_SHOOT:
			creature->MaxTurn = 0;
			RotateToTarget(item, &ai, SOPHIALEIGH_WALK_TURN_RATE_MAX);
			if (ai.ahead)
			{
				data->torsoYAngle = ai.angle;
				data->torsoXAngle = ai.xAngle;
			}

			if (item->Animation.FrameNumber == GetFrameNumber(item, 14))
			{
				TriggerLaserBolt(item, creature->Enemy, SOPHIALEIGH_Right, data, false, SOPHIALEIGH_LASER_DISPERSION_ANGLE);
				TriggerLaserBolt(item, creature->Enemy, SOPHIALEIGH_Left, data, false, -SOPHIALEIGH_LASER_DISPERSION_ANGLE);
			}

			break;
		}
	}

	static void LondonBossNormalControl(ItemInfo* item, CreatureInfo* creature, SophiaData* data)
	{
		AI_INFO ai;

		CreatureAIInfo(item, &ai);
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
		GetCreatureMood(item, &ai, true);
		CreatureMood(item, &ai, true);

		data->angle = CreatureTurn(item, creature->MaxTurn);
		switch (item->Animation.ActiveState)
		{
		case SOPHIALEIGH_STATE_LAUGH:
			creature->MaxTurn = 0;
			RotateToTarget(item, &ai, SOPHIALEIGH_WALK_TURN_RATE_MAX);
			break;
		case SOPHIALEIGH_STATE_STAND:
			creature->MaxTurn = 0;
			creature->Flags = 0;

			if (creature->Enemy->IsLara() && creature->Enemy->HitPoints <= 0)
			{
				item->Animation.TargetState = SOPHIALEIGH_STATE_LAUGH;
			}
			else if (ai.distance < SOPHIALEIGH_NORMAL_ATTACK_RANGE && Targetable(item, &ai))
			{
				if (item->TestFlagField((int)BossItemFlags::ChargedState, TRUE))
					item->Animation.TargetState = SOPHIALEIGH_STATE_BIG_SHOOT;
				else if (item->Timer <= 0)
					item->Animation.TargetState = SOPHIALEIGH_STATE_SUMMON;
				else
					item->Animation.TargetState = SOPHIALEIGH_STATE_SMALL_SHOOT;
			}
			else if (ai.distance < SOPHIALEIGH_NORMAL_WALK_RANGE && abs(ai.verticalDistance) <= STEPUP_HEIGHT)
				item->Animation.TargetState = SOPHIALEIGH_STATE_WALK;
			else
				item->Animation.TargetState = SOPHIALEIGH_STATE_RUN;
			break;
		case SOPHIALEIGH_STATE_WALK:
			creature->MaxTurn = SOPHIALEIGH_WALK_TURN_RATE_MAX;

			if (ai.distance > SOPHIALEIGH_NORMAL_WALK_RANGE)
				item->Animation.TargetState = SOPHIALEIGH_STATE_RUN;
			else if (Targetable(item, &ai) && ai.distance < SOPHIALEIGH_NORMAL_ATTACK_RANGE)
				item->Animation.TargetState = SOPHIALEIGH_STATE_STAND;
			break;
		case SOPHIALEIGH_STATE_RUN:
			creature->MaxTurn = SOPHIALEIGH_RUN_TURN_RATE_MAX;
			data->tilt = data->angle / 2;

			if (Targetable(item, &ai) && ai.distance < SOPHIALEIGH_NORMAL_ATTACK_RANGE)
				item->Animation.TargetState = SOPHIALEIGH_STATE_STAND; 
			else if (ai.distance < SOPHIALEIGH_NORMAL_WALK_RANGE && abs(ai.verticalDistance) <= STEPUP_HEIGHT)
				item->Animation.TargetState = SOPHIALEIGH_STATE_WALK;
			break;
		case SOPHIALEIGH_STATE_SUMMON:
			creature->MaxTurn = 0;

			if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + SOPHIALEIGH_ANIM_SUMMON_START)
			{
				if (item->Animation.FrameNumber == GetFrameNumber(item, 0))
				{
					item->Timer = SOPHIALEIGH_CHARGE_TIMER_DURATION;
				}
				else if (item->HitStatus && item->Animation.TargetState != SOPHIALEIGH_STATE_STAND && Random::TestProbability(1.0f / 50.0f)) // Avoid her being canceled every time.
				{
					StopSoundEffect(SFX_TR3_SOFIALEE_SUMMON);
					SoundEffect(SFX_TR3_SOFIALEE_SUMMON_FAIL, &item->Pose);
					SoundEffect(SFX_TR3_SOFIALEE_TAKE_HIT, &item->Pose);
					item->Animation.TargetState = SOPHIALEIGH_STATE_STAND;
				}
			}
			else if (item->Animation.AnimNumber == (Objects[item->ObjectNumber].animIndex + SOPHIALEIGH_ANIM_SUMMON) && item->Animation.FrameNumber >= GetFrameCount(item->Animation.AnimNumber) - 1)
			{
				item->SetFlagField((int)BossItemFlags::ChargedState, TRUE);
			}

			//TriggerSophiaShockwave(item, SOPHIALEIGH_Staff); // SHITTY EFFECT

			break;
		case SOPHIALEIGH_STATE_BIG_SHOOT:
			creature->MaxTurn = 0;
			if (ai.ahead)
			{
				data->torsoYAngle = ai.angle;
				data->torsoXAngle = ai.xAngle;
			}

			item->SetFlagField((int)BossItemFlags::ChargedState, FALSE);

			if (item->Animation.FrameNumber == GetFrameNumber(item, 36))
			{
				TriggerLaserBolt(item, creature->Enemy, SOPHIALEIGH_Right, data, false, SOPHIALEIGH_LASER_DISPERSION_ANGLE);
				TriggerLaserBolt(item, creature->Enemy, SOPHIALEIGH_Staff, data, true, 0);
				TriggerLaserBolt(item, creature->Enemy, SOPHIALEIGH_Left, data, false, -SOPHIALEIGH_LASER_DISPERSION_ANGLE);
			}

			break;
		case SOPHIALEIGH_STATE_SMALL_SHOOT:
			creature->MaxTurn = 0;
			RotateToTarget(item, &ai, SOPHIALEIGH_WALK_TURN_RATE_MAX);
			if (ai.ahead)
			{
				data->torsoYAngle = ai.angle;
				data->torsoXAngle = ai.xAngle;
			}

			if (item->Animation.FrameNumber == GetFrameNumber(item, 14))
			{
				TriggerLaserBolt(item, creature->Enemy, SOPHIALEIGH_Right, data, false, SOPHIALEIGH_LASER_DISPERSION_ANGLE);
				TriggerLaserBolt(item, creature->Enemy, SOPHIALEIGH_Left, data, false, -SOPHIALEIGH_LASER_DISPERSION_ANGLE);
			}

			break;
		}
	}

	void LondonBossControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto& object = Objects[item->ObjectNumber];
		auto* creature = GetCreatureInfo(item);
		SophiaData data{};

		// She is immortal in tower mode !
		if (item->TestFlagField((int)BossItemFlags::ImmortalState, TRUE) && item->TriggerFlags & (int)SophiaOCB::OCB_Tower)
			item->HitPoints = object.HitPoints;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != SOPHIALEIGH_STATE_DEATH)
				SetAnimation(item, SOPHIALEIGH_ANIM_DEATH);

			int frameEnd = g_Level.Anims[object.animIndex + SOPHIALEIGH_ANIM_DEATH].frameEnd;
			if (item->Animation.FrameNumber >= frameEnd)
			{
				// Avoid having the object stop working.
				item->Animation.FrameNumber = frameEnd;

				if (item->GetFlagField((int)BossItemFlags::ExplodeCount) < SOPHIALEIGH_EXPLOSION_NUM_MAX)
					item->ItemFlags[(int)BossItemFlags::ExplodeCount]++;

				// Do explosion effect.
				ExplodeBoss(itemNumber, *item, SOPHIALEIGH_EXPLOSION_NUM_MAX, SOPHIALEIGH_EFFECT_COLOR, false);
				AnimateItem(item);
				return;
			}
		}
		else
		{
			if (item->TriggerFlags & (int)SophiaOCB::OCB_Tower)
				LondonBossTowerControl(item, creature, &data);
			else
				LondonBossNormalControl(item, creature, &data);
		}

		CreatureTilt(item, data.tilt);
		CreatureJoint(item, 0, data.torsoYAngle);
		CreatureJoint(item, 1, data.torsoXAngle);
		CreatureJoint(item, 2, data.headAngle);

		if ((item->Animation.ActiveState < SOPHIALEIGH_STATE_CLIMB2 || item->Animation.ActiveState > SOPHIALEIGH_STATE_FALL4CLICK) &&
			 item->Animation.ActiveState != SOPHIALEIGH_STATE_DEATH)
		{
			switch (CreatureVault(itemNumber, data.angle, 2, SOPHIALEIGH_VAULT_SHIFT))
			{
			case 2:
				creature->MaxTurn = 0;
				SetAnimation(item, SOPHIALEIGH_ANIM_CLIMB2CLICK);
				break;
			case 3:
				creature->MaxTurn = 0;
				SetAnimation(item, SOPHIALEIGH_ANIM_CLIMB3CLICK);
				break;
			case 4:
				creature->MaxTurn = 0;
				SetAnimation(item, SOPHIALEIGH_ANIM_CLIMB4CLICK);
				break;
			case -4:
				creature->MaxTurn = 0;
				SetAnimation(item, SOPHIALEIGH_ANIM_FALL4CLICK);
				break;
			}
		}
		else
		{
			CreatureAnimation(itemNumber, data.angle, 0);
		}
	}
}
