#include "framework.h"
#include "Objects/TR3/Entity/SophiaLeigh.h"

#include "Game/animation.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Objects/Effects/Boss.h"
#include "Objects/Effects/enemy_missile.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "misc.h"
#include "setup.h"
#include "lara_helpers.h"
#include "tomb4fx.h"
#include "people.h"
#include "Game/effects/spark.h"

using namespace TEN::Effects::Boss;
using namespace TEN::Entities::Effects;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto SOPHIALEIGH_WALK_RANGE		   = SQUARE(BLOCK(1));
	constexpr auto SOPHIALEIGH_Y_DISTANCE_RANGE	   = BLOCK(1.5f);
	constexpr auto SOPHIALEIGH_REACHED_GOAL_RANGE  = CLICK(2);
	constexpr auto SOPHIALEIGH_NORMAL_ATTACK_RANGE = SQUARE(BLOCK(5));
	constexpr auto SOPHIALEIGH_NORMAL_WALK_RANGE   = SQUARE(BLOCK(5));

	constexpr auto SOPHIALEIGH_CHARGE_TIMER_DURATION = 600;
	constexpr auto SOPHIALEIGH_EXPLOSION_NUM_MAX	 = 60;

	constexpr auto SOPHIALEIGH_EFFECT_COLOR			  = Vector4(0.0f, 0.7f, 0.3f, 0.5f);
	constexpr auto SOPHIALEIGH_SHOCKWAVE_COLOR		  = Vector4(0.0f, 0.7f, 0.3f, 0.5f);
	constexpr auto SOPHIALEIGH_EXPLOSION_MAIN_COLOR   = Vector4(0.0f, 0.7f, 0.2f, 0.5f);
	constexpr auto SOPHIALEIGH_EXPLOSION_SECOND_COLOR = Vector4(0.0f, 0.7f, 0.0f, 0.5f);

	constexpr auto SOPHIALEIGH_WALK_TURN_RATE_MAX					= ANGLE(4.0f);
	constexpr auto SOPHIALEIGH_RUN_TURN_RATE_MAX					= ANGLE(7.0f);
	constexpr auto SOPHIALEIGH_LASER_DECREASE_XANGLE_IF_LARA_CROUCH = ANGLE(0.25f);
	constexpr auto SOPHIALEIGH_LASER_DISPERSION_ANGLE				= ANGLE(1.5f);

	constexpr auto SOPHIALEIGH_LIGHTNING_GLOW_SIZE	   = 8;
	constexpr auto SOPHIALEIGH_MAX_LIGHTNING_GLOW_SIZE = 10;
	constexpr auto SOPHIALEIGH_SHOCKWAVE_SPEED		   = -184;
	constexpr auto SOPHIALEIGH_SHOCKWAVE_INNER_SIZE	   = 2700;
	constexpr auto SOPHIALEIGH_SHOCKWAVE_OUTER_SIZE	   = 2300;

	constexpr auto SOPHIALEIGH_VAULT_SHIFT = 96;

	const auto SophiaLeighStaffBite = BiteInfo(Vector3(-28.0f, 56.0f, 356.0f), 10);
	const auto SophiaLeighLeftBite	= BiteInfo(Vector3(-72.0f, 48.0f, 356.0f), 10);
	const auto SophiaLeighRightBite = BiteInfo(Vector3(16.0f, 48.0f, 304.0f), 10);

	struct SophiaData
	{
		short angle;
		short tilt;
		short headAngle;
		short torsoXAngle;
		short torsoYAngle;
		short shockwaveCount;
		short shockwaveTimer;
	};

	enum SophiaLeighState
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

	enum SophiaLeighAnim
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

	enum class SophiaOCB
	{
		OCB_Normal = 1,			 // Move, climb, attack, and chase player.
		OCB_Tower = 2,			 // TR3 one, which only climb, can't be killed unless a trigger say otherwise (electrical box for example).
		OCB_LuaToMoveUpDown = 6, // TR3 one but use volume to move her instead of height check, they need to increase or decrease creature->LocationAI for her to go up/down.
	};

	void InitialiseSophiaLeigh(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);
		CheckForRequiredObjects(item);

		// Set to normal mode by default if none is set
		if (item.TriggerFlags == 0)
			item.TriggerFlags = (short)SophiaOCB::OCB_Normal;
		item.SetFlagField((int)BossItemFlags::ChargedState, false); // Charged state. 1 = fully charged.
		item.SetFlagField((int)BossItemFlags::DeathCount, 0);
		item.SetFlagField((int)BossItemFlags::ExplodeCount, 0);
		item.SetFlagField((int)BossItemFlags::Rotation, 0);
		SetAnimation(&item, SOPHIALEIGH_ANIM_SUMMON_START);			// Always start with projectile attack.
	}

	static Vector3i GetAIPosition(ItemInfo* item, int objectNumber, int currentFlagToSearch)
	{
		for (auto& aiObject : g_Level.AIObjects)
		{
			if (aiObject.objectNumber == objectNumber && aiObject.triggerFlags == currentFlagToSearch)
				return aiObject.pos.Position;
		}

		return Vector3i::Zero;
	}

	static void RotateTowardTarget(ItemInfo& item, AI_INFO* ai, short angleRate)
	{
		if (abs(ai->angle) < angleRate)
		{
			item.Pose.Orientation.y += ai->angle;
		}
		else if (ai->angle < 0)
		{
			item.Pose.Orientation.y -= angleRate;
		}
		else
		{
			item.Pose.Orientation.y += angleRate;
		}
	}

	static void TriggerSophiaLeightLight(ItemInfo& item, Pose& shockwavePos)
	{
		if ((item.Animation.AnimNumber == GetAnimNumber(item, SOPHIALEIGH_ANIM_SUMMON_START) && item.Animation.FrameNumber > GetFrameNumber(&item, 6)) ||
			item.Animation.AnimNumber == GetAnimNumber(item, SOPHIALEIGH_ANIM_SUMMON) ||
			(item.Animation.AnimNumber == GetAnimNumber(item, SOPHIALEIGH_ANIM_SUMMON_END) && item.Animation.FrameNumber < GetFrameNumber(&item, 3)) ||
			(item.Animation.AnimNumber == GetAnimNumber(item, SOPHIALEIGH_ANIM_SCEPTER_SHOOT) && item.Animation.FrameNumber > GetFrameNumber(&item, 39) && item.Animation.FrameNumber < GetFrameNumber(&item, 47)) ||
			(item.Animation.AnimNumber == GetAnimNumber(item, SOPHIALEIGH_ANIM_SCEPTER_SMALL_SHOOT) && item.Animation.FrameNumber > GetFrameNumber(&item, 14) && item.Animation.FrameNumber < GetFrameNumber(&item, 18)))
		{

			TriggerDynamicLight(shockwavePos.Position.x, shockwavePos.Position.y, shockwavePos.Position.z,
				item.ItemFlags[(int)BossItemFlags::Rotation] + SOPHIALEIGH_LIGHTNING_GLOW_SIZE,
				SOPHIALEIGH_EFFECT_COLOR.x * UCHAR_MAX, SOPHIALEIGH_EFFECT_COLOR.y * UCHAR_MAX, SOPHIALEIGH_EFFECT_COLOR.z * UCHAR_MAX);

			if (item.ItemFlags[(int)BossItemFlags::Rotation] < SOPHIALEIGH_MAX_LIGHTNING_GLOW_SIZE)
				item.ItemFlags[(int)BossItemFlags::Rotation]++;
		}
		else if (item.Animation.AnimNumber == GetAnimNumber(item, SOPHIALEIGH_ANIM_SUMMON_END) && item.Animation.FrameNumber >= GetFrameNumber(&item, 3) && item.ItemFlags[1] > 0)
		{
			TriggerDynamicLight(shockwavePos.Position.x, shockwavePos.Position.y, shockwavePos.Position.z,
				item.ItemFlags[(int)BossItemFlags::Rotation] + SOPHIALEIGH_LIGHTNING_GLOW_SIZE,
				SOPHIALEIGH_EFFECT_COLOR.x * UCHAR_MAX, SOPHIALEIGH_EFFECT_COLOR.y * UCHAR_MAX, SOPHIALEIGH_EFFECT_COLOR.z * UCHAR_MAX);

			item.ItemFlags[(int)BossItemFlags::Rotation]--;

		}
		else
			item.ItemFlags[(int)BossItemFlags::AttackType] = 0;

	}

	static void SpawnSophiaLeighProjectileBolt(ItemInfo& item, ItemInfo* enemy, const BiteInfo& bite, SophiaData* data, bool isBigLaser, short angleAdd)
	{
		int fxNumber = CreateNewEffect(item.RoomNumber);
		if (fxNumber == NO_ITEM)
			return;

		auto& fx = EffectList[fxNumber];

		auto laserType = isBigLaser ? (short)MissileType::SophiaLeighLarge : (short)MissileType::SophiaLeighNormal;

		fx.pos.Position = GetJointPosition(&item, bite.meshNum, bite.Position);
		fx.pos.Orientation.x = item.Pose.Orientation.x + data->torsoXAngle;

		if (enemy->IsLara())
		{
			const auto& player = *GetLaraInfo(enemy);
			if (player.Control.IsLow)
				fx.pos.Orientation.x -= SOPHIALEIGH_LASER_DECREASE_XANGLE_IF_LARA_CROUCH;
		}

		fx.pos.Orientation.y = (item.Pose.Orientation.y + data->torsoYAngle) + angleAdd;
		fx.pos.Orientation.z = 0;
		fx.roomNumber = item.RoomNumber;
		fx.counter = 0;
		fx.flag1 = laserType;
		fx.flag2 = isBigLaser ? 10 : 2; // Damage value.
		fx.speed = Random::GenerateInt(120, 160);
		fx.objectNumber = ID_ENERGY_BUBBLES;
		fx.frameNumber = Objects[fx.objectNumber].meshIndex + (laserType - 1);
	}

	static void TriggerSophiaShockwave(ItemInfo* item, const BiteInfo& bite)
	{
		static constexpr auto LIFE_MAX = 64.0f;
		static constexpr auto VELOCITY = -400.0f;
		static constexpr auto COLOR	   = Vector4(0.0f, 1.0f, 1.0f, 1.0f);

		int fxNumber = GetFreeShockwave();
		if (fxNumber == NO_ITEM)
			return;

		auto* ringEffect = &ShockWaves[fxNumber];

		auto pos = GetJointPosition(item, bite.meshNum, bite.Position);

		ringEffect->x = pos.x;
		ringEffect->y = pos.y;
		ringEffect->z = pos.z;
		ringEffect->innerRad = 620;
		ringEffect->outerRad = 640;
		ringEffect->xRot = Random::GenerateAngle(-ANGLE(0.25f), ANGLE(0.25f));
		ringEffect->damage = 0;
		ringEffect->r = COLOR.x;
		ringEffect->g = COLOR.y * UCHAR_MAX;
		ringEffect->b = COLOR.z * UCHAR_MAX;
		ringEffect->speed = VELOCITY;
		ringEffect->life = LIFE_MAX;
	}

	// TODO: Rename. What even is "tower"?
	static void SophiaLeighTowerControl(ItemInfo& item, CreatureInfo* creature, SophiaData* data)
	{
		if (item.AIBits)
			GetAITarget(creature);

		// Immortal in tower mode.
		auto& object = Objects[item.ObjectNumber];
		item.HitPoints = object.HitPoints;

		auto sphere = BoundingSphere(item.Pose.Position.ToVector3() + Vector3(0.0f, -CLICK(2), 0.0f), BLOCK(1 / 16.0f));
		auto shockwavePos = Pose(Random::GeneratePointInSphere(sphere), item.Pose.Orientation);
		TriggerSophiaLeightLight(item, shockwavePos);

		AI_INFO ai;
		CreatureAIInfo(&item, &ai);

		// Check the previous and next position of AI object to
		// allow Sophia to go up or down based on enemy's vertical position.
		FindAITargetObject(creature, ID_AI_X1, creature->LocationAI, false);

		if (Vector3i::Distance(item.Pose.Position, creature->Enemy->Pose.Position) < SOPHIALEIGH_REACHED_GOAL_RANGE)
		{
			creature->ReachedGoal = true;
			creature->Enemy = LaraItem; // TODO: Deal with LaraItem global.

			if (!(item.TriggerFlags & (short)SophiaOCB::OCB_LuaToMoveUpDown))
			{
				// If enemy is above, get to next AI_X1.
				if (ai.verticalDistance >= SOPHIALEIGH_Y_DISTANCE_RANGE)
					creature->LocationAI++;

				// If enemy is below, get to previous AI_X1.
				else if (ai.verticalDistance <= -SOPHIALEIGH_Y_DISTANCE_RANGE)
					creature->LocationAI--;
			}
		}
		else
		{
			creature->ReachedGoal = false;
		}

		// Charge count. Sophia can start the charge animation again when at 0.
		if (item.Timer > 0)
			item.Timer--;

		if (ai.ahead)
		{
			data->headAngle = ai.angle;
		}
		else
		{
			data->torsoXAngle = 0;
			data->torsoYAngle = 0;
		}

		GetCreatureMood(&item, &ai, true);
		CreatureMood(&item, &ai, true);

		data->angle = CreatureTurn(&item, creature->MaxTurn);
		switch (item.Animation.ActiveState)
		{
		case SOPHIALEIGH_STATE_LAUGH:
			creature->MaxTurn = 0;
			RotateTowardTarget(item, &ai, SOPHIALEIGH_WALK_TURN_RATE_MAX);
			break;

		case SOPHIALEIGH_STATE_STAND:
			creature->MaxTurn = 0;
			creature->Flags = 0;

			if (creature->Enemy->IsLara() && creature->Enemy->HitPoints <= 0)
			{
				item.Animation.TargetState = SOPHIALEIGH_STATE_LAUGH;
			}
			else if (creature->ReachedGoal)
			{
				if (item.TestFlagField((int)BossItemFlags::ChargedState, true))
				{
					item.Animation.TargetState = SOPHIALEIGH_STATE_BIG_SHOOT;
				}
				else if (item.Timer <= 0)
				{
					item.Animation.TargetState = SOPHIALEIGH_STATE_SUMMON;
				}
				else
				{
					item.Animation.TargetState = SOPHIALEIGH_STATE_SMALL_SHOOT;
				}
			}
			else
			{
				item.Animation.TargetState = SOPHIALEIGH_STATE_RUN;
			}

			break;

		case SOPHIALEIGH_STATE_WALK:
			creature->MaxTurn = SOPHIALEIGH_WALK_TURN_RATE_MAX;

			if (creature->ReachedGoal)
			{
				item.Animation.TargetState = SOPHIALEIGH_STATE_STAND;
			}
			else if (ai.distance > SOPHIALEIGH_WALK_RANGE)
			{
				item.Animation.TargetState = SOPHIALEIGH_STATE_RUN;
			}

			break;

		case SOPHIALEIGH_STATE_RUN:
			creature->MaxTurn = SOPHIALEIGH_RUN_TURN_RATE_MAX;
			data->tilt = data->angle / 2;

			if (creature->ReachedGoal)
			{
				item.Animation.TargetState = SOPHIALEIGH_STATE_STAND;
				break;
			}
			break;

		case SOPHIALEIGH_STATE_SUMMON:
			creature->MaxTurn = 0;
			RotateTowardTarget(item, &ai, SOPHIALEIGH_WALK_TURN_RATE_MAX);

			if (item.Animation.AnimNumber == (Objects[item.ObjectNumber].animIndex + SOPHIALEIGH_ANIM_SUMMON_START))
			{
				if (item.Animation.FrameNumber == GetFrameNumber(&item, 0))
				{
					item.Timer = SOPHIALEIGH_CHARGE_TIMER_DURATION;
					data->shockwaveTimer = 0;
					data->shockwaveCount = 0;
				}
				else if (item.HitStatus && item.Animation.TargetState != SOPHIALEIGH_STATE_STAND)
				{
					item.Animation.TargetState = SOPHIALEIGH_STATE_STAND;

					StopSoundEffect(SFX_TR3_SOFIALEIGH_SUMMON);
					SoundEffect(SFX_TR3_SOFIALEIGH_SUMMON_FAIL, &item.Pose);
					SoundEffect(SFX_TR3_SOFIALEIGH_TAKE_HIT, &item.Pose);
				}
			}
			else if (item.Animation.AnimNumber == GetAnimNumber(item, SOPHIALEIGH_ANIM_SUMMON) &&
					item.Animation.FrameNumber >= (GetFrameCount(item.Animation.AnimNumber) - 2))
			{
				item.SetFlagField((int)BossItemFlags::ChargedState, true);
			}

			if (!data->shockwaveTimer && data->shockwaveCount < 4)
			{
				sphere = BoundingSphere(item.Pose.Position.ToVector3() + Vector3(0.0f, -CLICK(2), 0.0f), BLOCK(1 / 16.0f));
				shockwavePos = Pose(Random::GeneratePointInSphere(sphere), item.Pose.Orientation);

				SpawnSophiaSparks(shockwavePos.Position.ToVector3(), Vector3(SOPHIALEIGH_EFFECT_COLOR.x * UCHAR_MAX, SOPHIALEIGH_EFFECT_COLOR.y * UCHAR_MAX, SOPHIALEIGH_EFFECT_COLOR.z * UCHAR_MAX), 5, 2);
				TriggerShockwave(
					&shockwavePos, SOPHIALEIGH_SHOCKWAVE_OUTER_SIZE, SOPHIALEIGH_SHOCKWAVE_INNER_SIZE, SOPHIALEIGH_SHOCKWAVE_SPEED,
					SOPHIALEIGH_EFFECT_COLOR.x * UCHAR_MAX, SOPHIALEIGH_EFFECT_COLOR.y * UCHAR_MAX, SOPHIALEIGH_EFFECT_COLOR.z * UCHAR_MAX,
					36, EulerAngles(Random::GenerateInt(0, 180), 30, Random::GenerateInt(0, 180)), 0, false, true, (int)ShockwaveStyle::Sophia);

				data->shockwaveTimer = 2;
				data->shockwaveCount++;
				break;
			}

			if (data->shockwaveCount == 4)
			{
				data->shockwaveCount = 0;
				data->shockwaveTimer = 15;
				break;
			}

			data->shockwaveTimer--;
			break;

		case SOPHIALEIGH_STATE_BIG_SHOOT:
			item.SetFlagField((int)BossItemFlags::ChargedState, false);
			creature->MaxTurn = 0;
			RotateTowardTarget(item, &ai, SOPHIALEIGH_WALK_TURN_RATE_MAX);

			if (ai.ahead)
			{
				data->torsoYAngle = ai.angle;
				data->torsoXAngle = ai.xAngle;
			}

			if (item.Animation.FrameNumber == GetFrameNumber(&item, 36))
			{
				SpawnSophiaLeighProjectileBolt(item, creature->Enemy, SophiaLeighRightBite, data, false, SOPHIALEIGH_LASER_DISPERSION_ANGLE);
				SpawnSophiaLeighProjectileBolt(item, creature->Enemy, SophiaLeighStaffBite, data, true, 0);
				SpawnSophiaLeighProjectileBolt(item, creature->Enemy, SophiaLeighLeftBite, data, false, -SOPHIALEIGH_LASER_DISPERSION_ANGLE);
			}
			
			break;

		case SOPHIALEIGH_STATE_SMALL_SHOOT:
			creature->MaxTurn = 0;
			RotateTowardTarget(item, &ai, SOPHIALEIGH_WALK_TURN_RATE_MAX);

			if (ai.ahead)
			{
				data->torsoYAngle = ai.angle;
				data->torsoXAngle = ai.xAngle;
			}

			if (item.Animation.FrameNumber == GetFrameNumber(&item, 14))
			{
				SpawnSophiaLeighProjectileBolt(item, creature->Enemy, SophiaLeighRightBite, data, false, SOPHIALEIGH_LASER_DISPERSION_ANGLE);
				SpawnSophiaLeighProjectileBolt(item, creature->Enemy, SophiaLeighLeftBite, data, false, -SOPHIALEIGH_LASER_DISPERSION_ANGLE);
			}

			break;
		}
	}

	static void SophiaLeighNormalControl(ItemInfo& item, CreatureInfo* creature, SophiaData* data)
	{
		AI_INFO ai;
		CreatureAIInfo(&item, &ai);
	
		auto sphere = BoundingSphere(item.Pose.Position.ToVector3() + Vector3(0.0f, -CLICK(2), 0.0f), BLOCK(1 / 16.0f));
		auto shockwavePos = Pose(Random::GeneratePointInSphere(sphere), item.Pose.Orientation);
		TriggerSophiaLeightLight(item, shockwavePos);

		// Charge count. Sophia can start the charge animation again when at 0.
		if (item.Timer > 0)
			item.Timer--;

		if (ai.ahead)
		{
			data->headAngle = ai.angle;
		}
		else
		{
			data->torsoXAngle = 0;
			data->torsoYAngle = 0;
		}

		GetCreatureMood(&item, &ai, true);
		CreatureMood(&item, &ai, true);

		data->angle = CreatureTurn(&item, creature->MaxTurn);
		switch (item.Animation.ActiveState)
		{
		case SOPHIALEIGH_STATE_LAUGH:
			creature->MaxTurn = 0;
			RotateTowardTarget(item, &ai, SOPHIALEIGH_WALK_TURN_RATE_MAX);
			break;

		case SOPHIALEIGH_STATE_STAND:
			creature->MaxTurn = 0;
			creature->Flags = 0;

			if (creature->Enemy->IsLara() && creature->Enemy->HitPoints <= 0)
			{
				item.Animation.TargetState = SOPHIALEIGH_STATE_LAUGH;
			}
			else if (ai.distance < SOPHIALEIGH_NORMAL_ATTACK_RANGE && Targetable(&item, &ai))
			{
				if (item.TestFlagField((int)BossItemFlags::ChargedState, true))
				{
					item.Animation.TargetState = SOPHIALEIGH_STATE_BIG_SHOOT;
				}
				else if (item.Timer <= 0)
				{
					item.Animation.TargetState = SOPHIALEIGH_STATE_SUMMON;
				}
				else
				{
					item.Animation.TargetState = SOPHIALEIGH_STATE_SMALL_SHOOT;
				}
			}
			else if (ai.distance < SOPHIALEIGH_NORMAL_WALK_RANGE && abs(ai.verticalDistance) <= STEPUP_HEIGHT)
			{
				item.Animation.TargetState = SOPHIALEIGH_STATE_WALK;
			}
			else
			{
				item.Animation.TargetState = SOPHIALEIGH_STATE_RUN;
			}

			break;

		case SOPHIALEIGH_STATE_WALK:
			creature->MaxTurn = SOPHIALEIGH_WALK_TURN_RATE_MAX;

			if (ai.distance > SOPHIALEIGH_NORMAL_WALK_RANGE)
			{
				item.Animation.TargetState = SOPHIALEIGH_STATE_RUN;
			}
			else if (Targetable(&item, &ai) && ai.distance < SOPHIALEIGH_NORMAL_ATTACK_RANGE)
			{
				item.Animation.TargetState = SOPHIALEIGH_STATE_STAND;
			}

			break;

		case SOPHIALEIGH_STATE_RUN:
			creature->MaxTurn = SOPHIALEIGH_RUN_TURN_RATE_MAX;
			data->tilt = data->angle / 2;

			if (Targetable(&item, &ai) && ai.distance < SOPHIALEIGH_NORMAL_ATTACK_RANGE)
			{
				item.Animation.TargetState = SOPHIALEIGH_STATE_STAND;
			}
			else if (ai.distance < SOPHIALEIGH_NORMAL_WALK_RANGE && abs(ai.verticalDistance) <= STEPUP_HEIGHT)
			{
				item.Animation.TargetState = SOPHIALEIGH_STATE_WALK;
			}

			break;

		case SOPHIALEIGH_STATE_SUMMON:
			creature->MaxTurn = 0;

			if (item.Animation.AnimNumber == (Objects[item.ObjectNumber].animIndex + SOPHIALEIGH_ANIM_SUMMON_START))
			{
				if (item.Animation.FrameNumber == GetFrameNumber(&item, 0))
				{
					item.Timer = SOPHIALEIGH_CHARGE_TIMER_DURATION;
					data->shockwaveTimer = 0;
					data->shockwaveCount = 0;
				}
				else if (item.HitStatus &&
					item.Animation.TargetState != SOPHIALEIGH_STATE_STAND &&
					Random::TestProbability(1.0f / 50.0f)) // Avoid cancellation every time.
				{
					item.Animation.TargetState = SOPHIALEIGH_STATE_STAND;

					StopSoundEffect(SFX_TR3_SOFIALEIGH_SUMMON);
					SoundEffect(SFX_TR3_SOFIALEIGH_SUMMON_FAIL, &item.Pose);
					SoundEffect(SFX_TR3_SOFIALEIGH_TAKE_HIT, &item.Pose);
				}
			}
			else if (item.Animation.AnimNumber == GetAnimNumber(item, SOPHIALEIGH_ANIM_SUMMON) &&
					item.Animation.FrameNumber >= (GetFrameCount(item.Animation.AnimNumber) - 2))
			{
				item.SetFlagField((int)BossItemFlags::ChargedState, true);
			}

			if (!data->shockwaveTimer && data->shockwaveCount < 4)
			{
				int position = -512;
				sphere = BoundingSphere(item.Pose.Position.ToVector3() + Vector3(0.0f, position, 0.0f), BLOCK(1 / 16.0f));
				shockwavePos = Pose(Random::GeneratePointInSphere(sphere), item.Pose.Orientation);

				auto pos = Pose(item.Pose.Position, 0, 0, 0);

				SpawnSophiaSparks(shockwavePos.Position.ToVector3(), Vector3(SOPHIALEIGH_EFFECT_COLOR.x * UCHAR_MAX, SOPHIALEIGH_EFFECT_COLOR.y * UCHAR_MAX, SOPHIALEIGH_EFFECT_COLOR.z * UCHAR_MAX), 5, 2);
				TriggerShockwave(&shockwavePos, SOPHIALEIGH_SHOCKWAVE_INNER_SIZE, SOPHIALEIGH_SHOCKWAVE_OUTER_SIZE, SOPHIALEIGH_SHOCKWAVE_SPEED,
					SOPHIALEIGH_EFFECT_COLOR.x * UCHAR_MAX, SOPHIALEIGH_EFFECT_COLOR.y * UCHAR_MAX, SOPHIALEIGH_EFFECT_COLOR.z * UCHAR_MAX,
					36, EulerAngles(Random::GenerateInt(0, 180), 30, Random::GenerateInt(0, 180)), 0, false, true, (int)ShockwaveStyle::Sophia);

				data->shockwaveTimer = 2;
				data->shockwaveCount++;
				break;
			}

			if (data->shockwaveCount == 4)
			{
				data->shockwaveCount = 0;
				data->shockwaveTimer = 15;
				break;
			}

			data->shockwaveTimer--;
			break;

		case SOPHIALEIGH_STATE_BIG_SHOOT:
			item.SetFlagField((int)BossItemFlags::ChargedState, false);
			creature->MaxTurn = 0;

			if (ai.ahead)
			{
				data->torsoYAngle = ai.angle;
				data->torsoXAngle = ai.xAngle;
			}

			if (item.Animation.FrameNumber == GetFrameNumber(&item, 36))
			{
				SpawnSophiaLeighProjectileBolt(item, creature->Enemy, SophiaLeighRightBite, data, false, SOPHIALEIGH_LASER_DISPERSION_ANGLE);
				SpawnSophiaLeighProjectileBolt(item, creature->Enemy, SophiaLeighStaffBite, data, true, 0);
				SpawnSophiaLeighProjectileBolt(item, creature->Enemy, SophiaLeighLeftBite, data, false, -SOPHIALEIGH_LASER_DISPERSION_ANGLE);
			}

			break;

		case SOPHIALEIGH_STATE_SMALL_SHOOT:
			creature->MaxTurn = 0;
			RotateTowardTarget(item, &ai, SOPHIALEIGH_WALK_TURN_RATE_MAX);

			if (ai.ahead)
			{
				data->torsoYAngle = ai.angle;
				data->torsoXAngle = ai.xAngle;
			}

			if (item.Animation.FrameNumber == GetFrameNumber(&item, 14))
			{
				SpawnSophiaLeighProjectileBolt(item, creature->Enemy, SophiaLeighRightBite, data, false, SOPHIALEIGH_LASER_DISPERSION_ANGLE);
				SpawnSophiaLeighProjectileBolt(item, creature->Enemy, SophiaLeighLeftBite, data, false, -SOPHIALEIGH_LASER_DISPERSION_ANGLE);
			}

			break;
		}
	}

	void SophiaLeighControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& object = Objects[item.ObjectNumber];
		auto& creature = *GetCreatureInfo(&item);

		SophiaData data{};

		if (item.HitPoints <= 0)
		{
			if (item.Animation.ActiveState != SOPHIALEIGH_STATE_DEATH)
				SetAnimation(&item, SOPHIALEIGH_ANIM_DEATH);

			int frameEnd = g_Level.Anims[object.animIndex + SOPHIALEIGH_ANIM_DEATH].frameEnd;
			if (item.Animation.FrameNumber >= frameEnd)
			{
				// Avoid having the object stop working.
				item.Animation.FrameNumber = frameEnd;
				item.MeshBits.ClearAll();

				if (item.GetFlagField((int)BossItemFlags::ExplodeCount) < SOPHIALEIGH_EXPLOSION_NUM_MAX)
					item.ItemFlags[(int)BossItemFlags::ExplodeCount]++;

				// Do explosion effect.
				ExplodeBoss(itemNumber, item, SOPHIALEIGH_EXPLOSION_NUM_MAX, SOPHIALEIGH_SHOCKWAVE_COLOR, SOPHIALEIGH_EXPLOSION_MAIN_COLOR, SOPHIALEIGH_EXPLOSION_SECOND_COLOR, false);
				return;
			}
		}
		else
		{

			if (item.TriggerFlags & (int)SophiaOCB::OCB_Tower)
				SophiaLeighTowerControl(item, &creature, &data);
			else
				SophiaLeighNormalControl(item, &creature, &data);
		}

		CreatureTilt(&item, data.tilt);
		CreatureJoint(&item, 0, data.torsoYAngle);
		CreatureJoint(&item, 1, data.torsoXAngle);
		CreatureJoint(&item, 2, data.headAngle);

		if ((item.Animation.ActiveState < SOPHIALEIGH_STATE_CLIMB2 || item.Animation.ActiveState > SOPHIALEIGH_STATE_FALL4CLICK) &&
			 item.Animation.ActiveState != SOPHIALEIGH_STATE_DEATH)
		{
			switch (CreatureVault(itemNumber, data.angle, 2, SOPHIALEIGH_VAULT_SHIFT))
			{
			case 2:
				creature.MaxTurn = 0;
				SetAnimation(&item, SOPHIALEIGH_ANIM_CLIMB2CLICK);
				break;
			case 3:
				creature.MaxTurn = 0;
				SetAnimation(&item, SOPHIALEIGH_ANIM_CLIMB3CLICK);
				break;
			case 4:
				creature.MaxTurn = 0;
				SetAnimation(&item, SOPHIALEIGH_ANIM_CLIMB4CLICK);
				break;
			case -4:
				creature.MaxTurn = 0;
				SetAnimation(&item, SOPHIALEIGH_ANIM_FALL4CLICK);
				break;
			}
		}
		else
		{
			CreatureAnimation(itemNumber, data.angle, 0);
		}
	}


	void SpawnSophiaSparks(const Vector3& pos, const Vector3& color, unsigned int count, int unk)
	{
		for (int i = 0; i < count; i++)
		{
			auto* spark = GetFreeParticle();

			auto sphere = BoundingSphere(Vector3::Zero, BLOCK(2));
			auto vel = Random::GeneratePointInSphere(sphere) * SQUARE(unk);

			spark->on = true;
			spark->sR = color.x;
			spark->sG = color.y;
			spark->sB = color.z;
			spark->dB = 0;
			spark->dG = 0;
			spark->dR = 0;
			spark->colFadeSpeed = 9 * SQUARE(unk);
			spark->fadeToBlack = 0;
			spark->life = 9 * SQUARE(unk);
			spark->sLife = 9 * SQUARE(unk);
			spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
			spark->x = pos.x;
			spark->y = pos.y;
			spark->z = pos.z;
			spark->gravity = (GetRandomControl() / 128) & 0x1F;
			spark->yVel = vel.x;
			spark->xVel = vel.y;
			spark->zVel = vel.z;
			spark->flags = SP_NONE;
			spark->maxYvel = 0;
			spark->friction = 34 * SQUARE(unk);
			spark->scalar = 3;
			spark->dSize = spark->sSize = spark->size = Random::GenerateInt(84, 98);
		}
	}

}
