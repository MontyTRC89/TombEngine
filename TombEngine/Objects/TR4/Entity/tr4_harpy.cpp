#include "framework.h"
#include "Objects/TR4/Entity/tr4_harpy.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Renderer/Renderer11Enums.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::TR4
{
	constexpr auto HARPY_STINGER_ATTACK_DAMAGE	= 100;
	constexpr auto HARPY_SWOOP_ATTACK_DAMAGE	= 10;
	constexpr auto HARPY_STINGER_POISON_POTENCY = 8;

	const auto HarpyBite1	= BiteInfo(Vector3::Zero, 4);
	const auto HarpyBite2	= BiteInfo(Vector3::Zero, 2);
	const auto HarpyBite3	= BiteInfo(Vector3::Zero, 15);
	const auto HarpyAttack1 = BiteInfo(Vector3(0.0f, 128.0f, 0.0f), 2);
	const auto HarpyAttack2 = BiteInfo(Vector3(0.0f, 128.0f, 0.0f), 4);
	const vector<int> HarpySwoopAttackJoints   = { HarpyBite2.meshNum, HarpyBite1.meshNum, HarpyBite3.meshNum };
	const vector<int> HarpyStingerAttackJoints = { HarpyAttack1.meshNum, HarpyAttack2.meshNum };

	enum HarpyState
	{
		HARPY_STATE_NONE = 0,
		HARPY_STATE_IDLE = 1,
		HARPY_STATE_FLY_FORWARD = 2,
		HARPY_STATE_FLY_DOWN = 3,
		HARPY_STATE_FLY_FORWARD_DOWN = 4,
		HARPY_STATE_SWOOP_ATTACK = 5,
		HARPY_STATE_STINGER_ATTACK = 6,
		HARPY_STATE_FLY_FORWARD_SPIN = 7,
		HARPY_STATE_FLAME_ATTACK = 8,
		HARPY_STATE_DEATH_START = 9,
		HARPY_STATE_DEATH_FALL = 10,
		HARPY_STATE_DEATH_END = 11,
		HARPY_STATE_FLY_BACK = 12,
		HARPY_STATE_GLIDE = 13
	};

	enum HarpyAnim
	{
		HARPY_ANIM_FLY_FORWARD = 0,
		HARPY_ANIM_FLAME_ATTACK_START = 1,
		HARPY_ANIM_FLAME_ATTACK_CONTINUE = 2,
		HARPY_ANIM_FLAME_ATTACK_END = 3,
		HARPY_ANIM_IDLE = 4,
		HARPY_ANIM_DEATH_START = 5,
		HARPY_ANIM_DEATH_FALL = 6,
		HARPY_ANIM_DEATH_END = 7,
		HARPY_ANIM_STINGER_ATTACK = 8,
		HARPY_ANIM_FLY_BACK = 9,
		HARPY_ANIM_FLY_FORWARD_DOWN_START = 10,
		HARPY_ANIM_FLY_FORWARD_DOWN_CONTINUE = 11,
		HARPY_ANIM_FLY_FORWARD_DOWN_END = 12,
		HARPY_ANIM_SWOOP_ATTACK = 13,
		HARPY_ANIM_FLY_DOWN_START = 14,
		HARPY_ANIM_FLY_DOWN_CONTINUE = 15,
		HARPY_ANIM_FLY_DOWN_END = 16,
		HARPY_ANIM_FLY_FORWARD_SPIN = 17,
		HARPY_ANIM_GLIDE = 18
	};

	void TriggerHarpyMissile(PHD_3DPOS* pose, short roomNumber, short mesh)
	{
		short fxNumber = CreateNewEffect(roomNumber);
		if (fxNumber == -1)
			return;

		auto* fx = &EffectList[fxNumber];

		fx->pos.Position.x = pose->Position.x;
		fx->pos.Position.y = pose->Position.y - (GetRandomControl() & 0x3F) - 32;
		fx->pos.Position.z = pose->Position.z;
		fx->pos.Orientation.x = pose->Orientation.x;
		fx->pos.Orientation.y = pose->Orientation.y;
		fx->pos.Orientation.z = 0;
		fx->roomNumber = roomNumber;
		fx->counter = short(2 * GetRandomControl() + 0x8000);
		fx->objectNumber = ID_ENERGY_BUBBLES;
		fx->speed = (GetRandomControl() & 0x1F) + 96;
		fx->flag1 = mesh;
		fx->frameNumber = Objects[fx->objectNumber].meshIndex + mesh * 2;
	}

	void TriggerHarpyFlame(short itemNumber, ItemInfo* target, byte nodeNumber, short size)
	{
		auto* item = &g_Level.Items[itemNumber];

		int dx = target->Pose.Position.x - item->Pose.Position.x;
		int dz = target->Pose.Position.z - item->Pose.Position.z;

		if (dx >= -SECTOR(16) && dx <= SECTOR(16) &&
			dz >= -SECTOR(16) && dz <= SECTOR(16))
		{
			auto* spark = GetFreeParticle();

			spark->on = true;
			spark->sR = 0;
			spark->sG = 0;
			spark->sB = 0;
			spark->dB = 0;
			spark->dG = spark->dR = (GetRandomControl() & 0x7F) + 32;
			spark->fadeToBlack = 8;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
			spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
			spark->life = spark->sLife = (GetRandomControl() & 7) + 20;
			spark->x = (GetRandomControl() & 0xF) - 8;
			spark->y = 0;
			spark->z = (GetRandomControl() & 0xF) - 8;
			spark->xVel = (GetRandomControl() & 0xFF) - 128;
			spark->yVel = 0;
			spark->zVel = (GetRandomControl() & 0xFF) - 128;
			spark->friction = 5;
			spark->flags = SP_SCALE | SP_ROTATE | SP_ITEM | SP_EXPDEF | SP_NODEATTACH;
			spark->rotAng = GetRandomControl() & 0xFFF;

			if (TestProbability(0.5f))
				spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
			else
				spark->rotAdd = (GetRandomControl() & 0x1F) + 32;

			spark->maxYvel = 0;
			spark->gravity = (GetRandomControl() & 0x1F) + 16;
			spark->fxObj = byte(itemNumber);
			spark->nodeNumber = nodeNumber;
			spark->scalar = 2;
			spark->sSize = spark->size = GetRandomControl() & 0xF + size;
			spark->dSize = spark->size / 8;
		}
	}

	void TriggerHarpySparks(ItemInfo* target, int x, int y, int z, short xv, short yv, short zv)
	{
		int dx = target->Pose.Position.x - x;
		int dz = target->Pose.Position.z - z;

		if (dx >= -SECTOR(16) && dx <= SECTOR(16) &&
			dz >= -SECTOR(16) && dz <= SECTOR(16))
		{
			auto* spark = GetFreeParticle();

			spark->on = true;
			spark->sR = 0;
			spark->sG = 0;
			spark->sB = 0;
			spark->dR = spark->dG = (GetRandomControl() & 0x7F) + 64;
			spark->dB = 0;
			spark->life = 16;
			spark->sLife = 16;
			spark->colFadeSpeed = 4;
			spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
			spark->fadeToBlack = 4;
			spark->x = x;
			spark->y = y;
			spark->z = z;
			spark->xVel = xv;
			spark->yVel = yv;
			spark->zVel = zv;
			spark->friction = 34;
			spark->scalar = 1;
			spark->sSize = spark->size = (GetRandomControl() & 3) + 4;
			spark->maxYvel = 0;
			spark->gravity = 0;
			spark->dSize = (GetRandomControl() & 1) + 1;
			spark->flags = SP_NONE;
		}
	}

	void DoHarpyEffects(ItemInfo* item, CreatureInfo* creature, short itemNumber)
	{
		item->ItemFlags[0]++;

		auto rh = Vector3Int(HarpyAttack1.Position);
		GetJointAbsPosition(item, &rh, HarpyAttack1.meshNum);

		auto lr = Vector3Int(HarpyAttack2.Position);
		GetJointAbsPosition(item, &lr, HarpyAttack2.meshNum);

		if (item->ItemFlags[0] >= 24 &&
			item->ItemFlags[0] <= 47 &&
			(GetRandomControl() & 0x1F) < item->ItemFlags[0])
		{
			for (int i = 0; i < 2; i++)
			{
				int dx = (GetRandomControl() & 0x7FF) + rh.x - 1024;
				int dy = (GetRandomControl() & 0x7FF) + rh.y - 1024;
				int dz = (GetRandomControl() & 0x7FF) + rh.z - 1024;

				TriggerHarpySparks(creature->Enemy, dx, dy, dz, 8 * (rh.x - dx), 8 * (rh.y - dy), 8 * (rh.z - dz));

				dx = (GetRandomControl() & 0x7FF) + lr.x - 1024;
				dy = (GetRandomControl() & 0x7FF) + lr.y - 1024;
				dz = (GetRandomControl() & 0x7FF) + lr.z - 1024;

				TriggerHarpySparks(creature->Enemy, dx, dy, dz, 8 * (lr.x - dx), 8 * (lr.y - dy), 8 * (lr.z - dz));
			}
		}

		int something = item->ItemFlags[0] * 2;
		if (something > 64)
			something = 64;
		if (something < 80)
		{
			if ((Wibble & 0xF) == 8)
				TriggerHarpyFlame(itemNumber, creature->Enemy, 4, something);
			else if (!(Wibble & 0xF))
				TriggerHarpyFlame(itemNumber, creature->Enemy, 5, something);
		}

		if (item->ItemFlags[0] >= 61)
		{
			if (item->ItemFlags[0] <= 65 && GlobalCounter & 1)
			{
				auto pos3 = Vector3Int(HarpyAttack1.Position);
				pos3.y *= 2;
				GetJointAbsPosition(item, &pos3, HarpyAttack1.meshNum);

				auto angles = GetOrientTowardPoint(rh.ToVector3(), pos3.ToVector3());
				auto pose = PHD_3DPOS(rh, angles);
				TriggerHarpyMissile(&pose, item->RoomNumber, 2);
			}

			if (item->ItemFlags[0] >= 61 && item->ItemFlags[0] <= 65 && !(GlobalCounter & 1))
			{
				auto pos3 = Vector3Int(HarpyAttack2.Position);
				pos3.y *= 2;
				GetJointAbsPosition(item, &pos3, HarpyAttack2.meshNum);

				auto angles = GetOrientTowardPoint(rh.ToVector3(), pos3.ToVector3());
				auto pose = PHD_3DPOS(rh, angles);
				TriggerHarpyMissile(&pose, item->RoomNumber, 2);
			}
		}
	}

	void InitialiseHarpy(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		InitialiseCreature(itemNumber);
		SetAnimation(item, HARPY_ANIM_IDLE);
	}

	void HarpyControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* object = &Objects[item->ObjectNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;

		if (item->HitPoints <= 0)
		{
			item->HitPoints = 0;

			int state = item->Animation.ActiveState - 9;
			if (state)
			{
				state--;
				if (state)
				{
					if (state == HARPY_STATE_IDLE)
					{
						item->Pose.Position.y = item->Floor;
						item->Pose.Orientation.x = 0;
					}
					else
					{
						SetAnimation(item, HARPY_ANIM_DEATH_START);
						item->Animation.IsAirborne = true;
						item->Animation.Velocity.z = 0;
						item->Pose.Orientation.x = 0;
					}

					CreatureTilt(item, 0);

					CreatureJoint(item, 0, joint0);
					CreatureJoint(item, 1, joint1);
					CreatureJoint(item, 2, joint2);

					CreatureAnimation(itemNumber, angle, 0);
					return;
				}
			}
			else
				item->Animation.TargetState = HARPY_STATE_DEATH_FALL;

			if (item->Pose.Position.y >= item->Floor)
			{
				item->Animation.TargetState = HARPY_STATE_DEATH_END;
				item->Animation.IsAirborne = false;
				item->Animation.Velocity.y = 0.0f;
				item->Pose.Position.y = item->Floor;
			}

			item->Pose.Orientation.x = 0;
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);

			int minDistance = INT_MAX;

			creature->Enemy = nullptr;

			for (auto& currentCreature : ActiveCreatures)
			{
				if (currentCreature->ItemNumber == NO_ITEM || currentCreature->ItemNumber == itemNumber)
					continue;

				auto* target = &g_Level.Items[currentCreature->ItemNumber];

				if (target->ObjectNumber == ID_LARA_DOUBLE)
				{
					int dx = target->Pose.Position.x - item->Pose.Position.x;
					int dz = target->Pose.Position.z - item->Pose.Position.z;
					int distance = pow(dx, 2) + pow(dz, 2);

					if (distance < minDistance)
					{
						creature->Enemy = target;
						minDistance = distance;
					}
				}
			}

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (!creature->Enemy->IsLara())
				phd_atan(LaraItem->Pose.Position.z - item->Pose.Position.z, LaraItem->Pose.Position.x - item->Pose.Position.x);

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			if (AI.ahead)
			{
				joint0 = AI.angle / 2;
				joint2 = AI.angle / 2;
				joint1 = AI.xAngle;
			}

			int height = 0;
			int dy = 0;

			switch (item->Animation.ActiveState)
			{
			case HARPY_STATE_IDLE:
				creature->MaxTurn = Angle::DegToRad(7.0f);
				creature->Flags = 0;

				if (creature->Enemy)
				{
					height = (item->Pose.Position.y + SECTOR(2));
					if (creature->Enemy->Pose.Position.y > height && item->Floor > height)
					{
						item->Animation.TargetState = HARPY_STATE_FLY_DOWN;
						break;
					}
				}

				if (AI.ahead)
				{
					dy = abs(creature->Enemy->Pose.Position.y - item->Pose.Position.y);
					if (dy <= SECTOR(1))
					{
						if (AI.distance < pow(341, 2))
						{
							item->Animation.TargetState = HARPY_STATE_STINGER_ATTACK;
							break;
						}

						if (dy <= SECTOR(1) && AI.distance < pow(SECTOR(2), 2))
						{
							item->Animation.TargetState = HARPY_STATE_FLY_FORWARD_DOWN;
							break;
						}
					}
				}

				if (creature->Enemy != LaraItem ||
					!Targetable(item, &AI) ||
					AI.distance <= pow(SECTOR(3.5f), 2) ||
					TestProbability(0.5f))
				{
					item->Animation.TargetState = HARPY_STATE_FLY_FORWARD;
					break;
				}

				item->Animation.TargetState = HARPY_STATE_FLAME_ATTACK;
				item->ItemFlags[0] = 0;
				break;

			case HARPY_STATE_FLY_FORWARD:
				creature->MaxTurn = Angle::DegToRad(7.0f);
				creature->Flags = 0;

				if (item->Animation.RequiredState)
				{
					item->Animation.TargetState = item->Animation.RequiredState;
					if (item->Animation.RequiredState == HARPY_STATE_FLAME_ATTACK)
						item->ItemFlags[0] = 0;

					break;
				}

				if (item->HitStatus)
				{
					item->Animation.TargetState = HARPY_STATE_FLY_FORWARD_SPIN;
					break;
				}

				if (AI.ahead)
				{
					if (AI.distance >= pow(341, 2))
					{
						if (AI.ahead && TestProbability(0.5f) &&
							AI.distance >= pow(SECTOR(2), 2) &&
							AI.distance > pow(SECTOR(3.5f), 2))
						{
							item->Animation.TargetState = HARPY_STATE_FLAME_ATTACK;
							item->ItemFlags[0] = 0;
						}
						else
							item->Animation.TargetState = HARPY_STATE_FLY_FORWARD_DOWN;
					}
					else
						item->Animation.TargetState = HARPY_STATE_STINGER_ATTACK;

					break;
				}

				if (TestProbability(0.5f))
				{
					item->Animation.TargetState = HARPY_STATE_FLY_FORWARD_SPIN;
					break;
				}

				if (!AI.ahead)
				{
					item->Animation.TargetState = HARPY_STATE_FLY_FORWARD_DOWN;
					break;
				}

				if (AI.distance >= pow(341, 2))
				{
					if (AI.ahead && AI.distance >= pow(SECTOR(2), 2) &&
						AI.distance > pow(SECTOR(3.5f), 2) &&
						TestProbability(0.5f))
					{
						item->Animation.TargetState = HARPY_STATE_FLAME_ATTACK;
						item->ItemFlags[0] = 0;
					}
					else
						item->Animation.TargetState = HARPY_STATE_FLY_FORWARD_DOWN;
				}
				else
					item->Animation.TargetState = HARPY_STATE_STINGER_ATTACK;

				break;

			case HARPY_STATE_FLY_DOWN:
				if (!creature->Enemy ||
					creature->Enemy->Pose.Position.y < (item->Pose.Position.y + SECTOR(2)))
				{
					item->Animation.TargetState = HARPY_STATE_IDLE;
				}

				break;

			case HARPY_STATE_FLY_FORWARD_DOWN:
				creature->MaxTurn = Angle::DegToRad(2.0f);

				if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
					item->Animation.TargetState = HARPY_STATE_SWOOP_ATTACK;
				else
					item->Animation.TargetState = HARPY_STATE_GLIDE;

				break;

			case HARPY_STATE_SWOOP_ATTACK:
				item->Animation.TargetState = HARPY_STATE_FLY_FORWARD;
				creature->MaxTurn = Angle::DegToRad(2.0f);

				if (item->TestBits(JointBitType::Touch, HarpySwoopAttackJoints) ||
					creature->Enemy != nullptr && !creature->Enemy->IsLara() &&
					abs(creature->Enemy->Pose.Position.y - item->Pose.Position.y) <= SECTOR(1) &&
					AI.distance < pow(SECTOR(2), 2))
				{
					DoDamage(creature->Enemy, HARPY_SWOOP_ATTACK_DAMAGE);

					if (item->TouchBits & 0x10)
						CreatureEffect2(item, HarpyBite1, 5, -1, DoBloodSplat);
					else
						CreatureEffect2(item, HarpyBite2, 5, -1, DoBloodSplat);
				}

				break;

			case HARPY_STATE_STINGER_ATTACK:
				creature->MaxTurn = Angle::DegToRad(2.0f);

				if (creature->Flags == 0 &&
					(item->TestBits(JointBitType::Touch, HarpyStingerAttackJoints) ||
						creature->Enemy != nullptr && !creature->Enemy->IsLara() &&
						abs(creature->Enemy->Pose.Position.y - item->Pose.Position.y) <= SECTOR(1) &&
						AI.distance < pow(SECTOR(2), 2)))
				{
					if (creature->Enemy->IsLara())
						GetLaraInfo(creature->Enemy)->PoisonPotency += HARPY_STINGER_POISON_POTENCY;

					DoDamage(creature->Enemy, HARPY_STINGER_ATTACK_DAMAGE);
					CreatureEffect2(item, HarpyBite3, 10, -1, DoBloodSplat);
					creature->Flags = 1;
				}

				break;

			case HARPY_STATE_FLAME_ATTACK:
				DoHarpyEffects(item, creature, itemNumber);
				break;

			case HARPY_STATE_FLY_BACK:
				if (AI.ahead && AI.distance > pow(SECTOR(3.5f), 2))
				{
					item->Animation.TargetState = HARPY_STATE_FLY_FORWARD;
					item->Animation.RequiredState = HARPY_STATE_FLAME_ATTACK;
				}
				else if (TestProbability(0.5f))
					item->Animation.TargetState = HARPY_STATE_IDLE;

				break;

			case HARPY_STATE_GLIDE:
				item->Animation.TargetState = HARPY_STATE_FLY_FORWARD;
				break;

			default:
				break;
			}
		}

		CreatureTilt(item, 0);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
