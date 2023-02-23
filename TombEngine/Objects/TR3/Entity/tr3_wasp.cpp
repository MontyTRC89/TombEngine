#include "framework.h"
#include "Objects/TR3/Entity/tr3_wasp.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/spark.h"
#include "Game/misc.h"
#include "Math/Math.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Spark;
using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto WASP_DAMAGE = 50;

	constexpr auto WASP_TAKE_OFF_RANGE = SQUARE(BLOCK(3));
	constexpr auto WASP_ATTACK_RANGE   = SQUARE(BLOCK(0.5f));

	constexpr auto WASP_LAND_CHANCE = 1 / 256.0f;

	constexpr auto WASP_LAND_TURN_RATE_MAX = ANGLE(1.0f);
	constexpr auto WASP_AIR_TURN_RATE_MAX  = ANGLE(3.0f);

	constexpr auto WASP_IDLE_TO_FLY_VELOCITY = BLOCK(1 / 20.0f);

	constexpr auto WaspVenomSackLightColor = Vector4(0.0f, 0.35f, 0.0f, 1.0f);

	const auto WaspBite          = BiteInfo(Vector3::Zero, 12);
	const auto WaspVenomSackBite = BiteInfo(Vector3::Zero, 10);

	enum WaspState
	{
		WASP_STATE_FLY_IDLE,
		WASP_STATE_FLY_IDLE_TO_IDLE, // Floor
		WASP_STATE_IDLE,			 // Floor
		WASP_STATE_IDLE_TO_FLY_IDLE,
		WASP_STATE_ATTACK,			 // Flying
		WASP_STATE_FALL,
		WASP_STATE_DEATH,
		WASP_STATE_FLY_FORWARD
	};

	enum WaspAnim
	{
		WASP_ANIM_FLY_IDLE,
		WASP_ANIM_FLY_IDLE_TO_IDLE,
		WASP_ANIM_IDLE,
		WASP_ANIM_IDLE_TO_FLY_IDLE,
		WASP_ANIM_ATTACK,
		WASP_ANIM_FALL,
		WASP_ANIM_DEATH,
		WASP_ANIM_FLY_FORWARD
	};

	static void SpawnWaspParticle(short itemNumber)
	{
		auto& particle = *GetFreeParticle();

		particle.on = true;
		particle.sG = Random::GenerateInt(32, 96);
		particle.sB = particle.sG >> 1;
		particle.sR = particle.sG >> 2;
		particle.dG = Random::GenerateInt(224, 256);
		particle.dB = particle.dG >> 1;
		particle.dR = particle.dG >> 2;
		
		particle.colFadeSpeed = 4;
		particle.fadeToBlack = 2;
		particle.sLife =
		particle.life = 8;

		particle.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;

		particle.extras = 0;
		particle.dynamic = -1;

		particle.x = Random::GenerateInt(-8, 8);
		particle.y = Random::GenerateInt(-8, 8);
		particle.z = Random::GenerateInt(-64, 64);

		particle.xVel = Random::GenerateInt(-32, 32);
		particle.yVel = Random::GenerateInt(-32, 32);
		particle.zVel = Random::GenerateInt(-32, 32);
		particle.friction = 2 | (2 << 4);

		particle.flags = SP_SCALE | SP_ITEM | SP_NODEATTACH | SP_DEF;
		particle.gravity = particle.maxYvel = 0;

		particle.fxObj = itemNumber;
		particle.nodeNumber = ParticleNodeOffsetIDs::NodeWasp;

		particle.spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
		particle.scalar = 3;

		int size = Random::GenerateInt(4, 8);

		particle.size =
		particle.sSize = size;
		particle.dSize = size >> 1;
	}

	static void DoWaspEffects(short itemNumber, ItemInfo& item)
	{
		constexpr auto PARTICLE_EFFECT_COUNT = 2;

		// Spawn light.
		auto pos = GetJointPosition(&item, WaspVenomSackBite.meshNum, WaspVenomSackBite.Position);
		TriggerDynamicLight(
			pos.x, pos.y, pos.z, 10, 
			WaspVenomSackLightColor.x * 255,
			WaspVenomSackLightColor.y * 255,
			WaspVenomSackLightColor.z * 255);

		// Spawn wasp effect twice.
		for (int i = 0; i < PARTICLE_EFFECT_COUNT; i++)
			SpawnWaspParticle(itemNumber);
	}

	void InitialiseWaspMutant(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		InitialiseCreature(itemNumber);
		SetAnimation(&item, WASP_STATE_IDLE);
	}

	// NOTE: AI_MODIFY doesn't allow the wasp to land.
	// If it spawns in the land state (set by default), it will be forced to fly.
	void WaspMutantControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& creature = *GetCreatureInfo(&item);

		short headingAngle = 0;

		if (item.HitPoints <= 0)
		{
			switch (item.Animation.ActiveState)
			{
			case WASP_STATE_FALL:
				if (item.Pose.Position.y >= item.Floor)
				{
					item.Animation.TargetState = WASP_STATE_DEATH;
					item.Animation.IsAirborne = false;
					item.Animation.Velocity.y = 0.0f;
					item.Pose.Position.y = item.Floor;
				}

				break;

			case WASP_STATE_DEATH:
				item.Pose.Position.y = item.Floor;
				break;

			default:
				SetAnimation(&item, WASP_STATE_FALL);
				item.Animation.IsAirborne = true;
				item.Animation.Velocity = Vector3::Zero;
				break;
			}

			item.Pose.Orientation.x = 0;
		}
		else
		{
			if (item.AIBits)
				GetAITarget(&creature);

			AI_INFO ai;
			CreatureAIInfo(&item, &ai);

			GetCreatureMood(&item, &ai, true);
			CreatureMood(&item, &ai, true);

			headingAngle = CreatureTurn(&item, creature.MaxTurn);

			switch (item.Animation.ActiveState)
			{
			case WASP_STATE_IDLE:
				creature.MaxTurn = WASP_LAND_TURN_RATE_MAX;
				item.Pose.Position.y = item.Floor;

				if (item.HitStatus ||
					ai.distance < WASP_TAKE_OFF_RANGE ||
					creature.HurtByLara ||
					item.AIBits == MODIFY)
				{
					item.Animation.TargetState = WASP_STATE_IDLE_TO_FLY_IDLE;
				}

				break;

			case WASP_STATE_FLY_IDLE_TO_IDLE:
				creature.MaxTurn = WASP_LAND_TURN_RATE_MAX;

				item.Pose.Position.y += WASP_IDLE_TO_FLY_VELOCITY;
				if (item.Pose.Position.y >= item.Floor)
					item.Pose.Position.y = item.Floor;

				break;

			case WASP_STATE_FLY_IDLE:
				creature.MaxTurn = WASP_AIR_TURN_RATE_MAX;
				creature.Flags = 0;

				if (item.Animation.RequiredState)
				{
					item.Animation.TargetState = item.Animation.RequiredState;
				}
				// NOTE: This causes the wasp to wait until probability is valid or
				// the player has hit the wasp to move forward, which is conceptually bad.
				else if (item.HitStatus || Random::TestProbability(WASP_LAND_CHANCE) || item.AIBits == MODIFY)
				{
					item.Animation.TargetState = WASP_STATE_FLY_FORWARD;
				}
				else if ((creature.Mood == MoodType::Bored || GetRandomControl() < WASP_LAND_CHANCE) &&
					!creature.HurtByLara && item.AIBits != MODIFY)
				{
					item.Animation.TargetState = WASP_STATE_FLY_IDLE_TO_IDLE;
				}
				else if (ai.ahead && ai.distance < WASP_ATTACK_RANGE)
				{
					item.Animation.TargetState = WASP_STATE_ATTACK;
				}

				break;

			case WASP_STATE_FLY_FORWARD:
				creature.MaxTurn = WASP_AIR_TURN_RATE_MAX;
				creature.Flags = 0;

				if (item.Animation.RequiredState)
				{
					item.Animation.TargetState = item.Animation.RequiredState;
				}
				else if ((creature.Mood == MoodType::Bored || GetRandomControl() < WASP_LAND_CHANCE) &&
					!creature.HurtByLara && item.AIBits != MODIFY)
				{
					item.Animation.TargetState = WASP_STATE_FLY_IDLE;
				}
				else if (ai.ahead && ai.distance < WASP_ATTACK_RANGE)
				{
					item.Animation.TargetState = WASP_STATE_ATTACK;
				}

				break;

			case WASP_STATE_ATTACK:
				creature.MaxTurn = WASP_AIR_TURN_RATE_MAX;

				if (ai.ahead && ai.distance < WASP_ATTACK_RANGE)
				{
					item.Animation.TargetState = WASP_STATE_ATTACK;
				}
				else if (ai.distance < WASP_ATTACK_RANGE)
				{
					item.Animation.TargetState = WASP_STATE_FLY_IDLE;
				}
				else
				{
					item.Animation.TargetState = WASP_STATE_FLY_IDLE;
					item.Animation.RequiredState = WASP_STATE_FLY_FORWARD;
				}

				if (!creature.Flags && item.TouchBits.Test(WaspBite.meshNum))
				{
					DoDamage(creature.Enemy, WASP_DAMAGE);
					CreatureEffect(&item, WaspBite, DoBloodSplat);
					creature.Flags = 1;
				}

				break;
			}

			// Avoid spawning dynamic light when dead.
			DoWaspEffects(itemNumber, item);
		}

		CreatureAnimation(itemNumber, headingAngle, 0);
	}
}
