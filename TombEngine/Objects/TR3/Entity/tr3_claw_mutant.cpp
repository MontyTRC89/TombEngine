#include "framework.h"
#include "Objects/TR3/Entity/tr3_claw_mutant.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Objects/Effects/enemy_missile.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/setup.h"

using namespace TEN::Entities::Effects;
using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto CLAW_MUTANT_WALK_TURN_RATE_MAX = ANGLE(3.0f);
	constexpr auto CLAW_MUTANT_RUN_TURN_RATE_MAX = ANGLE(4.0f);

	constexpr auto CLAW_MUTANT_SLASH_RANGE = SQUARE(BLOCK(1));
	constexpr auto CLAW_MUTANT_RUN_ATTACK_RANGE = SQUARE(BLOCK(2));
	constexpr auto CLAW_MUTANT_CLAW_RANGE = SQUARE(BLOCK(1));
	constexpr auto CLAW_MUTANT_SHOOT_RANGE = SQUARE(BLOCK(3));

	constexpr auto CLAW_MUTANT_DAMAGE = 100;
	constexpr auto CLAW_MUTANT_PLASMA_DAMAGE = 200;
	constexpr auto CLAW_MUTANT_PLASMA_SPEED = 250;

	constexpr auto CLAW_MUTANT_ROAR_CHANCE = 0x60;
	constexpr auto CLAW_MUTANT_WALK_CHANCE = CLAW_MUTANT_ROAR_CHANCE + 0x400;

	const auto ClawMutantLeftBite  = BiteInfo(Vector3(19.0f, -13.0f, 3.0f), 7);
	const auto ClawMutantRightBite = BiteInfo(Vector3(19.0f, -13.0f, 3.0f), 4);
	const auto ClawMutantTailBite  = BiteInfo(Vector3(-32.0f, -16.0f, -119.0f), 13);

	enum ClawMutantState
	{
		CLAW_MUTANT_STATE_IDLE,
		CLAW_MUTANT_STATE_WALK,
		CLAW_MUTANT_STATE_RUN,
		CLAW_MUTANT_STATE_RUN_ATTACK,
		CLAW_MUTANT_STATE_WALK_ATTACK_LEFT,
		CLAW_MUTANT_STATE_WALK_ATTACK_RIGHT,
		CLAW_MUTANT_STATE_SLASH_LEFT,
		CLAW_MUTANT_STATE_SLASH_RIGHT,
		CLAW_MUTANT_STATE_DEATH,
		CLAW_MUTANT_STATE_CLAW_ATTACK,
		CLAW_MUTANT_STATE_PLASMA_ATTACK
	};

	enum ClawMutantAnim
	{
		CLAW_MUTANT_ANIM_IDLE,
		CLAW_MUTANT_ANIM_START_WALK_LEFT,
		CLAW_MUTANT_ANIM_START_RUN_LEFT,
		CLAW_MUTANT_ANIM_STOP_WALK_RIGHT,
		CLAW_MUTANT_ANIM_STOP_WALK_LEFT,
		CLAW_MUTANT_ANIM_STOP_RUN_RIGHT,
		CLAW_MUTANT_ANIM_STOP_RUN_LEFT,
		CLAW_MUTANT_ANIM_WALK,
		CLAW_MUTANT_ANIM_UNKNOWN,
		CLAW_MUTANT_ANIM_RUN,
		CLAW_MUTANT_ANIM_RUN_TO_WALK_LEFT,
		CLAW_MUTANT_ANIM_RUN_TO_WALK_RIGHT,
		CLAW_MUTANT_ANIM_RUN_ATTACK,
		CLAW_MUTANT_ANIM_RUN_ATTACK_CANCEL,
		CLAW_MUTANT_ANIM_IDLE_ATTACK_LEFT,
		CLAW_MUTANT_ANIM_IDLE_ATTACK_RIGHT,
		CLAW_MUTANT_ANIM_IDLE_DOUBLE_ATTACK,
		CLAW_MUTANT_ANIM_WALK_ATTACK_LEFT,
		CLAW_MUTANT_ANIM_WALK_ATTACK_RIGHT,
		CLAW_MUTANT_ANIM_SHOOT,
		CLAW_MUTANT_ANIM_DEATH
	};

	static void SpawnMutantPlasma(int itemNumber)
	{
		auto* sptr = GetFreeParticle();

		sptr->on = 1;
		sptr->sB = 255;
		sptr->sG = 48 + (GetRandomControl() & 31);
		sptr->sR = 48;

		sptr->dB = 192 + (GetRandomControl() & 63);
		sptr->dG = 128 + (GetRandomControl() & 63);
		sptr->dR = 32;

		sptr->colFadeSpeed = 12 + (GetRandomControl() & 3);
		sptr->fadeToBlack = 8;
		sptr->sLife = sptr->life = (GetRandomControl() & 7) + 24;

		sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;

		sptr->extras = 0;
		sptr->dynamic = -1;

		sptr->x = ((GetRandomControl() & 15) - 8);
		sptr->y = 0;
		sptr->z = ((GetRandomControl() & 15) - 8);

		sptr->xVel = ((GetRandomControl() & 31) - 16);
		sptr->yVel = (GetRandomControl() & 15) + 16;
		sptr->zVel = ((GetRandomControl() & 31) - 16);
		sptr->friction = 3;

		if (GetRandomControl() & 1)
		{
			sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_ITEM | SP_NODEATTACH;
			sptr->rotAng = GetRandomControl() & 4095;
			if (GetRandomControl() & 1)
				sptr->rotAdd = -(GetRandomControl() & 15) - 16;
			else
				sptr->rotAdd = (GetRandomControl() & 15) + 16;
		}
		else
		{
			sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTACH;
		}

		sptr->gravity = (GetRandomControl() & 31) + 16;
		sptr->maxYvel = (GetRandomControl() & 7) + 16;

		sptr->fxObj = itemNumber;
		sptr->nodeNumber = ParticleNodeOffsetIDs::NodeClawMutantPlasma;

		sptr->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
		sptr->scalar = 1;
		int size = (GetRandomControl() & 31) + 64;
		sptr->size = sptr->sSize = size;
		sptr->dSize = size >> 2;
	}

	void SpawnPlasmaBallFlame(int fxNumber, const Vector3& vel, const Vector3& offset, float life)
	{
		auto* sptr = GetFreeParticle();

		sptr->on = 1;
		sptr->sB = 255;
		sptr->sG = 48 + (GetRandomControl() & 31);
		sptr->sR = 48;

		sptr->dB = 192 + (GetRandomControl() & 63);
		sptr->dG = 128 + (GetRandomControl() & 63);
		sptr->dR = 32;

		sptr->colFadeSpeed = 12 + (GetRandomControl() & 3);
		sptr->fadeToBlack = 8;
		sptr->sLife = sptr->life = (GetRandomControl() & 7) + life;

		sptr->blendMode = BLENDMODE_ADDITIVE;

		sptr->extras = 0;
		sptr->dynamic = -1;

		sptr->x = offset.x + ((GetRandomControl() & 15) - 8);
		sptr->y = 0;
		sptr->z = offset.z + ((GetRandomControl() & 15) - 8);

		sptr->xVel = vel.x;
		sptr->yVel = vel.y;
		sptr->zVel = vel.z;
		sptr->friction = 5;

		if (GetRandomControl() & 1)
		{
			sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX;
			sptr->rotAng = GetRandomControl() & 4095;

			if (GetRandomControl() & 1)
			{
				sptr->rotAdd = -(GetRandomControl() & 15) - 16;
			}
			else
			{
				sptr->rotAdd = (GetRandomControl() & 15) + 16;
			}
		}
		else
		{
			sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX;
		}

		sptr->fxObj = fxNumber;
		sptr->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
		sptr->scalar = 1;
		sptr->gravity = sptr->maxYvel = 0;

		int size = (GetRandomControl() & 31) + 64;
		sptr->size = sptr->sSize = size;
		sptr->dSize = size >> 4;

		sptr->yVel = (GetRandomControl() & 511) - 256;
		sptr->xVel <<= 1;
		sptr->zVel <<= 1;
		sptr->scalar = 2;
		sptr->friction = 85;
		sptr->gravity = 22;
	}

	static void SpawnMutantPlasmaBall(ItemInfo* item, CreatureInfo* creature)
	{
		short fxNumber = CreateNewEffect(item->RoomNumber);
		if (fxNumber != NO_ITEM)
		{
			auto jointPos = GetJointPosition(item, ClawMutantTailBite.meshNum, ClawMutantTailBite.Position);

			auto enemyPos = creature->Enemy->Pose.Position;
			if (creature->Enemy->IsLara() && GetLaraInfo(creature->Enemy)->Control.IsLow)
			{
				enemyPos.y -= CLICK(1);
			}
			else
			{
				enemyPos.y -= CLICK(2);
			}

			auto angle = Geometry::GetOrientToPoint(jointPos.ToVector3(), enemyPos.ToVector3());
			
			auto* fx = &EffectList[fxNumber];
			fx->pos.Position = jointPos;
			fx->pos.Orientation.y = angle.y;
			fx->pos.Orientation.x = angle.x;
			fx->objectNumber = ID_ENERGY_BUBBLES;
			fx->color = Vector4::Zero;
			fx->speed = CLAW_MUTANT_PLASMA_SPEED;
			fx->flag2 = CLAW_MUTANT_PLASMA_DAMAGE;
			fx->flag1 = (int)MissileType::ClawMutantPlasma;
			fx->fallspeed = 0;
		}
	}

	static void SpawnMutantPlasmaLight(ItemInfo* item)
	{
		int bright = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
		if (bright > 16)
		{
			bright = g_Level.Anims[item->Animation.AnimNumber].frameBase + 28 + 16 - item->Animation.FrameNumber;
			if (bright > 16)
				bright = 16;
		}

		if (bright > 0)
		{
			auto pos = GetJointPosition(item, 13, Vector3i(-32, -16, -192));
			int rnd = GetRandomControl();
			byte r, g, b;

			b = 31 - ((rnd >> 4) & 3);
			g = 24 - ((rnd >> 6) & 3);
			r = rnd & 7;

			r = (r * bright) >> 4;
			g = (g * bright) >> 4;
			b = (b * bright) >> 4;
			TriggerDynamicLight(pos.x, pos.y, pos.z, bright, r, g, b);
		}
	}

	static void DamageTargetWithClaw(ItemInfo* source, ItemInfo* target)
	{
		if (source->ItemFlags[5] == 0 && source->TouchBits.Test(ClawMutantLeftBite.meshNum))
		{
			DoDamage(target, CLAW_MUTANT_DAMAGE / 2);
			CreatureEffect2(source, ClawMutantLeftBite, 10, source->Pose.Orientation.y, DoBloodSplat);
			source->ItemFlags[5] = 1;
		}

		if (source->ItemFlags[6] == 0 && source->TouchBits.Test(ClawMutantRightBite.meshNum))
		{
			DoDamage(target, CLAW_MUTANT_DAMAGE / 2);
			CreatureEffect2(source, ClawMutantRightBite, 10, source->Pose.Orientation.y, DoBloodSplat);
			source->ItemFlags[6] = 1;
		}
	}

	// source->ItemFlags[5] is flag to enable damage left (0 is enabled, 1 is disabled)
	// source->ItemFlags[6] is flag to enable damage right (0 is enabled, 1 is disabled)
	void ClawMutantControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* object = &Objects[item->ObjectNumber];
		auto* creature = GetCreatureInfo(item);

		short head = 0;
		short torsoX = 0;
		short torsoY = 0;
		short headingAngle = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != CLAW_MUTANT_STATE_DEATH)
				SetAnimation(item, CLAW_MUTANT_ANIM_DEATH);

			int frameEnd = g_Level.Anims[object->animIndex + CLAW_MUTANT_ANIM_DEATH].frameEnd;
			if (item->Animation.FrameNumber >= frameEnd)
				CreatureDie(itemNumber, true);
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);

			AI_INFO ai;
			CreatureAIInfo(item, &ai);
			GetCreatureMood(item, &ai, ai.zoneNumber == ai.enemyZone ? true : false);
			CreatureMood(item, &ai, ai.zoneNumber == ai.enemyZone ? true : false);

			headingAngle = CreatureTurn(item, creature->MaxTurn);
			bool canShoot = Targetable(item, &ai) && ((ai.distance > CLAW_MUTANT_SHOOT_RANGE && !item->ItemFlags[0]) || ai.zoneNumber != ai.enemyZone);

			switch (item->Animation.ActiveState)
			{
			case CLAW_MUTANT_STATE_IDLE:
				item->ItemFlags[5] = 0;
				item->ItemFlags[6] = 0;
				creature->MaxTurn = 0;

				if (item->AIBits & GUARD)
				{
					head = AIGuard(creature);
					item->Animation.TargetState = CLAW_MUTANT_STATE_IDLE;
					break;
				}
				else if (item->AIBits & PATROL1)
				{
					head = 0;
					item->Animation.TargetState = CLAW_MUTANT_STATE_WALK;
				}
				else if (creature->Mood == MoodType::Escape)
				{
					item->Animation.TargetState = CLAW_MUTANT_STATE_RUN;
				}
				else if (ai.bite && ai.distance < CLAW_MUTANT_SLASH_RANGE)
				{
					torsoY = ai.angle;
					torsoX = ai.xAngle;

					if (ai.angle < 0)
					{
						item->Animation.TargetState = CLAW_MUTANT_STATE_SLASH_LEFT;
					}
					else
					{
						item->Animation.TargetState = CLAW_MUTANT_STATE_SLASH_RIGHT;
					}
				}
				else if (ai.bite && ai.distance < CLAW_MUTANT_CLAW_RANGE)
				{
					torsoY = ai.angle;
					torsoX = ai.xAngle;
					item->Animation.TargetState = CLAW_MUTANT_STATE_CLAW_ATTACK;
				}
				else if (canShoot)
				{
					item->Animation.TargetState = CLAW_MUTANT_STATE_PLASMA_ATTACK;
				}
				else if (creature->Mood == MoodType::Bored)
				{
					if (GetRandomControl() < CLAW_MUTANT_WALK_CHANCE)
						item->Animation.TargetState = CLAW_MUTANT_STATE_WALK;
				}
				else if (item->Animation.RequiredState != NO_STATE)
				{
					item->Animation.TargetState = item->Animation.RequiredState;
				}
				else
				{
					item->Animation.TargetState = CLAW_MUTANT_STATE_RUN;
				}

				break;

			case CLAW_MUTANT_STATE_WALK:
				item->ItemFlags[5] = 0;
				item->ItemFlags[6] = 0;
				creature->MaxTurn = CLAW_MUTANT_WALK_TURN_RATE_MAX;

				if (ai.ahead)
					head = ai.angle;

				if (item->AIBits & PATROL1)
				{
					item->Animation.TargetState = CLAW_MUTANT_STATE_WALK;
					head = 0;
				}
				else if (ai.bite && ai.distance < CLAW_MUTANT_CLAW_RANGE)
				{
					if (ai.angle < 0)
					{
						item->Animation.TargetState = CLAW_MUTANT_STATE_WALK_ATTACK_LEFT;
					}
					else
					{
						item->Animation.TargetState = CLAW_MUTANT_STATE_WALK_ATTACK_RIGHT;
					}
				}
				else if (canShoot)
				{
					item->Animation.TargetState = CLAW_MUTANT_STATE_IDLE;
				}
				else if (creature->Mood == MoodType::Escape || creature->Mood == MoodType::Attack)
				{
					item->Animation.TargetState = CLAW_MUTANT_STATE_RUN;
				}

				break;

			case CLAW_MUTANT_STATE_RUN:
				item->ItemFlags[5] = 0;
				item->ItemFlags[6] = 0;
				creature->MaxTurn = CLAW_MUTANT_RUN_TURN_RATE_MAX;

				if (ai.ahead)
					head = ai.angle;

				if (item->AIBits & GUARD)
				{
					item->Animation.TargetState = CLAW_MUTANT_STATE_IDLE;
				}
				else if (creature->Mood == MoodType::Bored)
				{
					item->Animation.TargetState = CLAW_MUTANT_STATE_IDLE;
				}
				else if (creature->Flags != 0 && ai.ahead)
				{
					item->Animation.TargetState = CLAW_MUTANT_STATE_IDLE;
				}
				else if (ai.bite && ai.distance < CLAW_MUTANT_RUN_ATTACK_RANGE)
				{
					if (creature->Enemy != nullptr && creature->Enemy->Animation.Velocity.z == 0.0f)
					{
						item->Animation.TargetState = CLAW_MUTANT_STATE_IDLE;
					}
					else
					{
						item->Animation.TargetState = CLAW_MUTANT_STATE_RUN_ATTACK;
					}
				}
				else if (canShoot)
				{
					item->Animation.TargetState = CLAW_MUTANT_STATE_IDLE;
				}

				break;

			case CLAW_MUTANT_STATE_WALK_ATTACK_LEFT:
			case CLAW_MUTANT_STATE_WALK_ATTACK_RIGHT:
			case CLAW_MUTANT_STATE_RUN_ATTACK:
			case CLAW_MUTANT_STATE_CLAW_ATTACK:
			case CLAW_MUTANT_STATE_SLASH_LEFT:
			case CLAW_MUTANT_STATE_SLASH_RIGHT:
				if (ai.ahead)
				{
					torsoY = ai.angle;
					torsoX = ai.xAngle;
				}

				DamageTargetWithClaw(item, creature->Enemy);
				break;

			case CLAW_MUTANT_STATE_PLASMA_ATTACK:
				if (ai.ahead)
				{
					torsoY = ai.angle;
					torsoX = ai.xAngle;
				}

				if (item->Animation.FrameNumber == GetFrameNumber(item, 0) && Random::TestProbability(1 / 4.0f) == 0)
					item->ItemFlags[0] = 1;

				if (item->Animation.FrameNumber < GetFrameNumber(item, 28))
				{
					SpawnMutantPlasma(itemNumber);
				}
				else if (item->Animation.FrameNumber == GetFrameNumber(item, 28))
				{
					SpawnMutantPlasmaBall(item, creature);
				}

				SpawnMutantPlasmaLight(item);
				break;
			}
		}

		CreatureJoint(item, 0, torsoX);
		CreatureJoint(item, 1, torsoY);
		CreatureJoint(item, 2, head);
		CreatureAnimation(itemNumber, headingAngle, 0);
	}
}
