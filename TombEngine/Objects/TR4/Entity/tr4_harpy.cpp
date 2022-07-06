#include "framework.h"
#include "tr4_harpy.h"
#include "Game/people.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/animation.h"
#include "Specific/setup.h"
#include "Game/control/lot.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"
#include "Renderer/Renderer11Enums.h"

using std::vector;

namespace TEN::Entities::TR4
{
	BITE_INFO HarpyBite1 = { 0, 0, 0, 4 };
	BITE_INFO HarpyBite2 = { 0, 0, 0, 2 };
	BITE_INFO HarpyBite3 = { 0, 0, 0, 21 };
	BITE_INFO HarpyAttack1 = { 0, 128, 0, 2 };
	BITE_INFO HarpyAttack2 = { 0, 128, 0, 4 };
	const vector<int> HarpySwoopAttackJoints = { 2, 4 };
	const vector<int> HarpyStingerAttackJoints = { 20, 21 };

	constexpr auto HARPY_STINGER_ATTACK_DAMAGE = 100;
	constexpr auto HARPY_SWOOP_ATTACK_DAMAGE = 10;
	constexpr auto HARPY_STINGER_POISON_POTENCY = 8;

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

	static void TriggerHarpyMissile(PHD_3DPOS* pose, short roomNumber, int count)
	{
		short fxNumber = CreateNewEffect(roomNumber);
		if (fxNumber != -1)
		{
			auto* fx = &EffectList[fxNumber];

			fx->pos.Position.x = pose->Position.x;
			fx->pos.Position.y = pose->Position.y - (GetRandomControl() & 0x3F) - 32;
			fx->pos.Position.z = pose->Position.z;
			fx->pos.Orientation.x = pose->Orientation.x;
			fx->pos.Orientation.y = pose->Orientation.y;
			fx->pos.Orientation.z = 0;
			fx->roomNumber = roomNumber;
			fx->counter = 2 * GetRandomControl() + -32768;
			fx->objectNumber = ID_ENERGY_BUBBLES;
			fx->speed = (GetRandomControl() & 0x1F) + 96;
			fx->flag1 = count;
			fx->frameNumber = Objects[fx->objectNumber].meshIndex + 2 * count;
		}
	}

	static void TriggerHarpyFlame(short itemNumber, byte number, int size)
	{
		auto* item = &g_Level.Items[itemNumber];

		int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
		int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;

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
			spark->xVel = (byte)GetRandomControl() - 128;
			spark->yVel = 0;
			spark->zVel = (byte)GetRandomControl() - 128;
			spark->friction = 5;
			spark->flags = 4762;
			spark->rotAng = GetRandomControl() & 0xFFF;

			if (GetRandomControl() & 1)
				spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
			else
				spark->rotAdd = (GetRandomControl() & 0x1F) + 32;

			spark->maxYvel = 0;
			spark->gravity = (GetRandomControl() & 0x1F) + 16;
			spark->fxObj = itemNumber;
			spark->nodeNumber = number;
			spark->scalar = 2;
			spark->sSize = spark->size = GetRandomControl() & 0xF + size;
			spark->dSize = spark->size / 8;
		}
	}

	static void TriggerHarpySparks(int x, int y, int z, int xv, int yv, int zv)
	{
		int dx = LaraItem->Pose.Position.x - x;
		int dz = LaraItem->Pose.Position.z - z;

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
			spark->y = y;
			spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
			spark->fadeToBlack = 4;
			spark->x = x;
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

	static void DoHarpyEffects(ItemInfo* item, short itemNumber)
	{
		item->ItemFlags[0]++;

		auto pos1 = Vector3Int(HarpyAttack1.x, HarpyAttack1.y, HarpyAttack1.z);
		GetJointAbsPosition(item, &pos1, HarpyAttack1.meshNum);

		auto pos2 = Vector3Int(HarpyAttack2.x, HarpyAttack2.y, HarpyAttack2.z);
		GetJointAbsPosition(item, &pos2, HarpyAttack2.meshNum);

		if (item->ItemFlags[0] >= 24 &&
			item->ItemFlags[0] <= 47 &&
			(GetRandomControl() & 0x1F) < item->ItemFlags[0])
		{
			for (int i = 0; i < 2; i++)
			{
				int dx = (GetRandomControl() & 0x7FF) + pos1.x - 1024;
				int dy = (GetRandomControl() & 0x7FF) + pos1.y - 1024;
				int dz = (GetRandomControl() & 0x7FF) + pos1.z - 1024;

				TriggerHarpySparks(dx, dy, dz, 8 * (pos1.x - dx), 8 * (pos1.y - dy), 8 * (pos1.z - dz));

				dx = (GetRandomControl() & 0x7FF) + pos2.x - 1024;
				dy = (GetRandomControl() & 0x7FF) + pos2.y - 1024;
				dz = (GetRandomControl() & 0x7FF) + pos2.z - 1024;

				TriggerHarpySparks(dx, dy, dz, 8 * (pos2.x - dx), 8 * (pos2.y - dy), 8 * (pos2.z - dz));
			}
		}

		int something = item->ItemFlags[0] * 2;
		if (something > 64)
			something = 64;
		if (something < 80)
		{
			if ((Wibble & 0xF) == 8)
				TriggerHarpyFlame(itemNumber, 4, something);
			else if (!(Wibble & 0xF))
				TriggerHarpyFlame(itemNumber, 5, something);
		}

		if (item->ItemFlags[0] >= 61)
		{
			if (item->ItemFlags[0] <= 65 && GlobalCounter & 1)
			{
				auto pos3 = Vector3Int(HarpyAttack1.x, HarpyAttack1.y * 2, HarpyAttack1.z);
				GetJointAbsPosition(item, &pos3, HarpyAttack1.meshNum);

				auto angles = GetVectorAngles(pos3.x - pos1.x, pos3.y - pos1.y, pos3.z - pos1.z);
				auto pose = PHD_3DPOS(pos1, angles);
				TriggerHarpyMissile(&pose, item->RoomNumber, 2);
			}

			if (item->ItemFlags[0] >= 61 && item->ItemFlags[0] <= 65 && !(GlobalCounter & 1))
			{
				auto pos3 = Vector3Int(HarpyAttack2.x, HarpyAttack2.y * 2, HarpyAttack2.z);
				GetJointAbsPosition(item, &pos3, HarpyAttack2.meshNum);

				auto angles = GetVectorAngles(pos3.x - pos1.x, pos3.y - pos1.y, pos3.z - pos1.z);
				auto pose = PHD_3DPOS(pos1, angles);
				TriggerHarpyMissile(&pose, item->RoomNumber, 2);
			}
		}
	}

	void InitialiseHarpy(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + HARPY_ANIM_IDLE;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = HARPY_STATE_IDLE;
		item->Animation.TargetState = HARPY_STATE_IDLE;
	}

	void HarpyControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (!CreatureActive(itemNumber))
			return;

		auto* creature = GetCreatureInfo(item);
		auto* object = &Objects[item->ObjectNumber];

		short angle = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;

		if (item->HitPoints <= 0)
		{
			int state = item->Animation.ActiveState - 9;
			item->HitPoints = 0;

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
						item->Pose.Orientation.x = 0;
						item->Animation.AnimNumber = object->animIndex + 5;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
						item->Animation.ActiveState = HARPY_STATE_DEATH_START;
						item->Animation.IsAirborne = true;
						item->Animation.Velocity = 0;
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
				item->Pose.Position.y = item->Floor;
				item->Animation.TargetState = HARPY_STATE_DEATH_END;
				item->Animation.IsAirborne = false;
				item->Animation.VerticalVelocity = 0;
			}

			item->Pose.Orientation.x = 0;
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);

			int minDistance = INT_MAX;

			creature->Enemy = nullptr;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				auto* currentCreature = ActiveCreatures[i];

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

			if (creature->Enemy != LaraItem)
				phd_atan(LaraItem->Pose.Position.z - item->Pose.Position.z, LaraItem->Pose.Position.x - item->Pose.Position.x);

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

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
				creature->MaxTurn = ANGLE(7.0f);
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
					!(GetRandomControl() & 1))
				{
					item->Animation.TargetState = HARPY_STATE_FLY_FORWARD;
					break;
				}

				item->Animation.TargetState = HARPY_STATE_FLAME_ATTACK;
				item->ItemFlags[0] = 0;
				break;

			case HARPY_STATE_FLY_FORWARD:
				creature->MaxTurn = ANGLE(7.0f);
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
						if (AI.ahead &&
							AI.distance >= pow(SECTOR(2), 2) &&
							AI.distance > pow(SECTOR(3.5f), 2) &&
							GetRandomControl() & 1)
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

				if (GetRandomControl() & 1)
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
						AI.distance > pow(SECTOR(3.5f), 2) && GetRandomControl() & 1)
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
				creature->MaxTurn = ANGLE(2.0f);

				if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
					item->Animation.TargetState = HARPY_STATE_SWOOP_ATTACK;
				else
					item->Animation.TargetState = HARPY_STATE_GLIDE;

				break;

			case HARPY_STATE_SWOOP_ATTACK:
				item->Animation.TargetState = HARPY_STATE_FLY_FORWARD;
				creature->MaxTurn = ANGLE(2.0f);

				if (item->TestBits(JointBitType::Touch, HarpySwoopAttackJoints) ||
					creature->Enemy && !creature->Enemy->IsLara() &&
					abs(creature->Enemy->Pose.Position.y - item->Pose.Position.y) <= SECTOR(1) &&
					AI.distance < pow(SECTOR(2), 2))
				{
					DoDamage(creature->Enemy, HARPY_SWOOP_ATTACK_DAMAGE);

					if (item->TouchBits & 0x10)
					{
						CreatureEffect2(
							item,
							&HarpyBite1,
							5,
							-1,
							DoBloodSplat);
					}
					else
					{
						CreatureEffect2(
							item,
							&HarpyBite2,
							5,
							-1,
							DoBloodSplat);
					}
				}

				break;

			case HARPY_STATE_STINGER_ATTACK:
				creature->MaxTurn = ANGLE(2.0f);

				if (creature->Flags == 0 &&
					(item->TestBits(JointBitType::Touch, HarpyStingerAttackJoints) ||
						creature->Enemy && !creature->Enemy->IsLara() &&
						abs(creature->Enemy->Pose.Position.y - item->Pose.Position.y) <= SECTOR(1) &&
						AI.distance < pow(SECTOR(2), 2)))
				{
					DoDamage(creature->Enemy, HARPY_STINGER_ATTACK_DAMAGE);

					CreatureEffect2(
						item,
						&HarpyBite3,
						10,
						-1,
						DoBloodSplat);

					if (creature->Enemy == LaraItem)
						Lara.PoisonPotency += HARPY_STINGER_POISON_POTENCY;

					creature->Flags = 1;
				}

				break;

			case HARPY_STATE_FLAME_ATTACK:
				DoHarpyEffects(item, itemNumber);
				break;

			case HARPY_STATE_FLY_BACK:
				if (AI.ahead && AI.distance > pow(SECTOR(3.5f), 2))
				{
					item->Animation.TargetState = HARPY_STATE_FLY_FORWARD;
					item->Animation.RequiredState = HARPY_STATE_FLAME_ATTACK;
				}
				else if (GetRandomControl() & 1)
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
