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

namespace TEN::Entities::TR4
{
	BITE_INFO HarpyBite1 = { 0, 0, 0, 4 };
	BITE_INFO HarpyBite2 = { 0, 0, 0, 2 };
	BITE_INFO HarpyBite3 = { 0, 0, 0, 21 };
	BITE_INFO HarpyAttack1 = { 0, 128, 0, 2 };
	BITE_INFO HarpyAttack2 = { 0, 128, 0, 4 };

	// TODO
	enum HarpyState
	{
		STATE_HARPY_STOP = 1,

		STATE_HARPY_ATTACK = 5,
		STATE_HARPY_POISON_ATTACK = 6,

		STATE_HARPY_FLAME_ATTACK = 8,

		STATE_HARPY_FALLING = 10,
		STATE_HARPY_DEATH = 11
	};

	// TODO
	enum HarpyAnim
	{

	};

	static void TriggerHarpyMissile(PHD_3DPOS* pos, short roomNumber, int count)
	{
		short fxNumber = CreateNewEffect(roomNumber);
		if (fxNumber != -1)
		{
			auto* fx = &EffectList[fxNumber];

			fx->pos.Position.x = pos->Position.x;
			fx->pos.Position.y = pos->Position.y - (GetRandomControl() & 0x3F) - 32;
			fx->pos.Position.z = pos->Position.z;
			fx->pos.Orientation.x = pos->Orientation.x;
			fx->pos.Orientation.y = pos->Orientation.y;
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
			auto* spark = &Sparks[GetFreeSpark()];

			spark->on = true;
			spark->sR = 0;
			spark->sG = 0;
			spark->sB = 0;
			spark->dB = 0;
			spark->dG = spark->dR = (GetRandomControl() & 0x7F) + 32;
			spark->fadeToBlack = 8;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
			spark->transType = TransTypeEnum::COLADD;
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

		if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
		{
			auto* spark = &Sparks[GetFreeSpark()];

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
			spark->transType = TransTypeEnum::COLADD;
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

		int something = 2 * item->ItemFlags[0];
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

				auto pose = PHD_3DPOS(pos1);

				auto angles = GetVectorAngles(pos3.x - pos1.x, pos3.y - pos1.y, pos3.z - pos1.z);
				pose.Orientation = angles;
				TriggerHarpyMissile(&pose, item->RoomNumber, 2);
			}

			if (item->ItemFlags[0] >= 61 && item->ItemFlags[0] <= 65 && !(GlobalCounter & 1))
			{
				auto pos3 = Vector3Int(HarpyAttack2.x, HarpyAttack2.y * 2, HarpyAttack2.z);
				GetJointAbsPosition(item, &pos3, HarpyAttack2.meshNum);

				auto pos = PHD_3DPOS(pos1.x, pos1.y, pos1.z);

				auto angles = GetVectorAngles(pos3.x - pos1.x, pos3.y - pos1.y, pos3.z - pos1.z);
				pos.Orientation = angles;
				TriggerHarpyMissile(&pos, item->RoomNumber, 2);
			}
		}
	}

	void InitialiseHarpy(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = STATE_HARPY_STOP;
		item->Animation.ActiveState = STATE_HARPY_STOP;
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
			short state = item->Animation.ActiveState - 9;
			item->HitPoints = 0;

			if (state)
			{
				state--;
				if (state)
				{
					if (state == 1)
					{
						item->Pose.Orientation.x = 0;
						item->Pose.Position.y = item->Floor;
					}
					else
					{
						item->Animation.AnimNumber = object->animIndex + 5;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
						item->Animation.ActiveState = 9;
						item->Animation.Velocity = 0;
						item->Animation.Airborne = true;
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
				item->Animation.TargetState = STATE_HARPY_FALLING;

			if (item->Pose.Position.y >= item->Floor)
			{
				item->Pose.Position.y = item->Floor;
				item->Animation.VerticalVelocity = 0;
				item->Animation.TargetState = STATE_HARPY_DEATH;
				item->Animation.Airborne = false;
			}

			item->Pose.Orientation.x = 0;
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);

			int minDistance = INT_MAX;

			creature->Enemy = NULL;

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
					int distance = dx * dx + dz * dz;

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
			case STATE_HARPY_STOP:
				creature->MaxTurn = ANGLE(7.0f);
				creature->Flags = 0;

				if (creature->Enemy)
				{
					height = (item->Pose.Position.y + SECTOR(2));
					if (creature->Enemy->Pose.Position.y > height && item->Floor > height)
					{
						item->Animation.TargetState = 3;
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
							item->Animation.TargetState = STATE_HARPY_POISON_ATTACK;
							break;
						}
						if (dy <= SECTOR(1) && AI.distance < pow(SECTOR(2), 2))
						{
							item->Animation.TargetState = 4;
							break;
						}
					}
				}

				if (creature->Enemy != LaraItem ||
					!Targetable(item, &AI) ||
					AI.distance <= pow(SECTOR(3.5f), 2) ||
					!(GetRandomControl() & 1))
				{
					item->Animation.TargetState = 2;
					break;
				}

				item->Animation.TargetState = STATE_HARPY_FLAME_ATTACK;
				item->ItemFlags[0] = 0;
				break;

			case 2:
				creature->MaxTurn = ANGLE(7.0f);
				creature->Flags = 0;

				if (item->Animation.RequiredState)
				{
					item->Animation.TargetState = item->Animation.RequiredState;
					if (item->Animation.RequiredState == 8)
						item->ItemFlags[0] = 0;

					break;
				}
				if (item->HitStatus)
				{
					item->Animation.TargetState = 7;
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
							item->Animation.TargetState = STATE_HARPY_FLAME_ATTACK;
							item->ItemFlags[0] = 0;
						}
						else
							item->Animation.TargetState = 4;
					}
					else
						item->Animation.TargetState = STATE_HARPY_POISON_ATTACK;

					break;
				}
				if (GetRandomControl() & 1)
				{
					item->Animation.TargetState = 7;
					break;
				}
				if (!AI.ahead)
				{
					item->Animation.TargetState = 4;
					break;
				}
				if (AI.distance >= pow(341, 2))
				{
					if (AI.ahead && AI.distance >= pow(SECTOR(2), 2) &&
						AI.distance > pow(SECTOR(3.5f), 2) && GetRandomControl() & 1)
					{
						item->Animation.TargetState = STATE_HARPY_FLAME_ATTACK;
						item->ItemFlags[0] = 0;
					}
					else
						item->Animation.TargetState = 4;
				}
				else
					item->Animation.TargetState = STATE_HARPY_POISON_ATTACK;

				break;

			case 3:
				if (!creature->Enemy ||
					creature->Enemy->Pose.Position.y < (item->Pose.Position.y + SECTOR(2)))
				{
					item->Animation.TargetState = STATE_HARPY_STOP;
				}

				break;

			case 4:
				creature->MaxTurn = ANGLE(2.0f);

				if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
					item->Animation.TargetState = STATE_HARPY_ATTACK;
				else
					item->Animation.TargetState = 13;

				break;

			case STATE_HARPY_ATTACK:
				item->Animation.TargetState = 2;
				creature->MaxTurn = ANGLE(2.0f);

				if (item->TouchBits & 0x14 ||
					creature->Enemy && creature->Enemy != LaraItem &&
					abs(creature->Enemy->Pose.Position.y - item->Pose.Position.y) <= SECTOR(1) &&
					AI.distance < pow(SECTOR(2), 2))
				{
					LaraItem->HitPoints -= 10;
					LaraItem->HitStatus = true;

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

			case STATE_HARPY_POISON_ATTACK:
				creature->MaxTurn = ANGLE(2.0f);

				if (creature->Flags == 0 &&
					(item->TouchBits & 0x300000 ||
						creature->Enemy && creature->Enemy != LaraItem &&
						abs(creature->Enemy->Pose.Position.y - item->Pose.Position.y) <= SECTOR(1) &&
						AI.distance < pow(SECTOR(2), 2)))
				{
					LaraItem->HitPoints -= 100;
					LaraItem->HitStatus = true;

					CreatureEffect2(
						item,
						&HarpyBite3,
						10,
						-1,
						DoBloodSplat);

					if (creature->Enemy == LaraItem)
						Lara.PoisonPotency += 8;

					creature->Flags = 1;
				}

				break;

			case STATE_HARPY_FLAME_ATTACK:
				DoHarpyEffects(item, itemNumber);
				break;

			case 12:
				if (AI.ahead && AI.distance > pow(SECTOR(3.5f), 2))
				{
					item->Animation.TargetState = 2;
					item->Animation.RequiredState = STATE_HARPY_FLAME_ATTACK;
				}
				else if (GetRandomControl() & 1)
					item->Animation.TargetState = STATE_HARPY_STOP;

				break;

			case 13:
				item->Animation.TargetState = 2;
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
