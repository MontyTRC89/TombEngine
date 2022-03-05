#include "framework.h"
#include "tr4_demigod.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/people.h"
#include "Game/effects/effects.h"
#include "Game/animation.h"
#include "Game/effects/tomb4fx.h"
#include "Game/camera.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/control/control.h"
#include "Game/itemdata/creature_info.h"
#include "Game/misc.h"

namespace TEN::Entities::TR4
{
	enum DemigodState
	{
		STATE_DEMIGOD_IDLE = 0,
		STATE_DEMIGOD_WALK = 1,
		STATE_DEMIGOD_RUN = 2,
		STATE_DEMIGOD_AIM = 3,
		STATE_DEMIGOD_ATTACK = 4,
		STATE_DEMIGOD_START_FLY = 5,
		STATE_DEMIGOD_FLY = 6,
		STATE_DEMIGOD_IDLE_FLY = 7,
		STATE_DEMIGOD_DEATH_1 = 8,
		STATE_DEMIGOD_CIRCLE_AIM = 9,
		STATE_DEMIGOD_CIRCLE_ATTACK = 10,
		STATE_DEMIGOD_GROUND_AIM = 11,
		STATE_DEMIGOD_GROUND_ATTACK = 12,
		STATE_DEMIGOD_HAMMER_AIM = 13,
		STATE_DEMIGOD_HAMMER_ATTACK = 14,
		STATE_DEMIGOD_DEATH_2 = 15
	};

	// TODO
	enum DemigodAnim
	{

	};

	void TriggerDemigodMissileFlame(short fxNumber, short xVel, short yVel, short zVel)
	{
		auto* fx = &EffectList[fxNumber];

		int dx = LaraItem->Position.xPos - fx->pos.xPos;
		int dz = LaraItem->Position.zPos - fx->pos.zPos;

		if (dx >= -SECTOR(16) && dx <= SECTOR(16) &&
			dz >= -SECTOR(16) && dz <= SECTOR(16))
		{
			auto* spark = &Sparks[GetFreeSpark()];

			spark->on = 1;
			if (fx->flag1 == 3 || fx->flag1 == 4)
			{
				spark->sR = 0;
				spark->dR = 0;
				spark->sB = (GetRandomControl() & 0x7F) + 32;
				spark->sG = spark->sB + 64;
				spark->dG = (GetRandomControl() & 0x7F) + 32;
				spark->dB = spark->dG + 64;
			}
			else
			{
				spark->sR = (GetRandomControl() & 0x7F) + 32;
				spark->sG = spark->sR - (GetRandomControl() & 0x1F);
				spark->sB = 0;
				spark->dR = (GetRandomControl() & 0x7F) + 32;
				spark->dB = 0;
				spark->dG = spark->dR - (GetRandomControl() & 0x1F);
			}

			spark->fadeToBlack = 8;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
			spark->transType = TransTypeEnum::COLADD;
			spark->life = spark->sLife = (GetRandomControl() & 3) + 16;
			spark->y = 0;
			spark->x = (GetRandomControl() & 0xF) - 8;
			spark->yVel = yVel;
			spark->zVel = zVel;
			spark->z = (GetRandomControl() & 0xF) - 8;
			spark->xVel = xVel;
			spark->friction = 68;
			spark->flags = 602;
			spark->rotAng = GetRandomControl() & 0xFFF;

			if (GetRandomControl() & 1)
				spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
			else
				spark->rotAdd = (GetRandomControl() & 0x1F) + 32;

			spark->gravity = 0;
			spark->maxYvel = 0;
			spark->fxObj = fxNumber;
			spark->scalar = 2;
			spark->sSize = spark->size = (GetRandomControl() & 7) + 64;
			spark->dSize = spark->size / 32;
		}
	}

	void TriggerDemigodMissile(PHD_3DPOS* pos, short roomNumber, int flags)
	{
		short fxNumber = CreateNewEffect(roomNumber);
		if (fxNumber != -1)
		{
			FX_INFO* fx = &EffectList[fxNumber];

			fx->pos.xPos = pos->xPos;
			fx->pos.yPos = pos->yPos - (GetRandomControl() & 0x3F) - 32;
			fx->pos.zPos = pos->zPos;

			fx->pos.xRot = pos->xRot;

			if (flags < 4)
				fx->pos.yRot = pos->yRot;
			else
				fx->pos.yRot = pos->yRot + (GetRandomControl() & 0x7FF) - 1024;

			fx->pos.zRot = 0;

			fx->roomNumber = roomNumber;
			fx->counter = 2 * GetRandomControl() + -ANGLE(180.0f);
			fx->flag1 = flags;
			fx->speed = (GetRandomControl() & 0x1F) + 96;
			fx->objectNumber = ID_ENERGY_BUBBLES;
			fx->frameNumber = Objects[ID_ENERGY_BUBBLES].meshIndex + (flags >= 4, flags - 1, flags);
		}
	}

	void DoDemigodEffects(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		short animIndex = item->AnimNumber - Objects[item->ObjectNumber].animIndex;

		if (animIndex == 8)
		{
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
			{
				PHD_VECTOR pos1 = { -544, 96, 0 };
				GetJointAbsPosition(item, &pos1, 16);

				PHD_VECTOR pos2 = { -900, 96, 0 };
				GetJointAbsPosition(item, &pos2, 16);

				short angles[2];
				phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);

				PHD_3DPOS pos;
				pos.xPos = pos1.x;
				pos.yPos = pos1.y;
				pos.zPos = pos1.z;
				pos.xRot = angles[1];
				pos.yRot = angles[0];
				pos.zRot = 0;

				if (item->ObjectNumber == ID_DEMIGOD3)
					TriggerDemigodMissile(&pos, item->RoomNumber, 3);
				else
					TriggerDemigodMissile(&pos, item->RoomNumber, 5);
			}
		}
		else if (animIndex == 19)
		{

			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
			{
				PHD_VECTOR pos1 = { -544, 96, 0 };
				GetJointAbsPosition(item, &pos1, 16);

				PHD_VECTOR pos2 = { -900, 96, 0 };
				GetJointAbsPosition(item, &pos2, 16);

				short angles[2];
				phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);

				PHD_3DPOS pos;
				pos.xPos = pos1.x;
				pos.yPos = pos1.y;
				pos.zPos = pos1.z;
				pos.xRot = angles[1];
				pos.yRot = angles[0];
				pos.zRot = 0;

				if (item->ObjectNumber == ID_DEMIGOD3)
					TriggerDemigodMissile(&pos, item->RoomNumber, 3);
				else
					TriggerDemigodMissile(&pos, item->RoomNumber, 5);
			}
		}
		else if (animIndex == 16)
		{
			// Animation 16 (State 10) is the big circle attack of DEMIGOD_3
			int frameNumber = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;

			if (frameNumber >= 8 && frameNumber <= 64)
			{
				PHD_VECTOR pos1 = { 0, 0, 192 };
				PHD_VECTOR pos2 = { 0, 0, 384 };

				if (GlobalCounter & 1)
				{
					GetJointAbsPosition(item, &pos1, 18);
					GetJointAbsPosition(item, &pos2, 18);
				}
				else
				{
					GetJointAbsPosition(item, &pos1, 17);
					GetJointAbsPosition(item, &pos2, 17);
				}

				short angles[2];
				phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);

				PHD_3DPOS pos;
				pos.xPos = pos1.x;
				pos.yPos = pos1.y;
				pos.zPos = pos1.z;
				pos.xRot = angles[1];
				pos.yRot = angles[0];
				pos.zRot = 0;

				TriggerDemigodMissile(&pos, item->RoomNumber, 4);
			}
		}
	}

	void TriggerHammerSmoke(int x, int y, int z, int something)
	{
		int angle = 2 * GetRandomControl();
		int deltaAngle = 0x10000 / something;

		if (something > 0)
		{
			for (int i = 0; i < something; i++)
			{
				auto* spark = &SmokeSparks[GetFreeSmokeSpark()];

				spark->on = true;
				spark->sShade = 0;
				spark->colFadeSpeed = 4;
				spark->dShade = (GetRandomControl() & 0x1F) + 96;
				spark->fadeToBlack = 24 - (GetRandomControl() & 7);
				spark->transType = TransTypeEnum::COLADD;
				spark->life = spark->sLife = (GetRandomControl() & 7) + 48;
				spark->x = (GetRandomControl() & 0x1F) + x - 16;
				spark->y = (GetRandomControl() & 0x1F) + y - 16;
				spark->z = (GetRandomControl() & 0x1F) + z - 16;
				spark->xVel = (byte)(GetRandomControl() + 256) * phd_sin(angle);
				spark->yVel = -32 - (GetRandomControl() & 0x3F);
				spark->zVel = (byte)(GetRandomControl() + 256) * phd_cos(angle);
				spark->friction = 9;

				if (GetRandomControl() & 1)
				{
					spark->flags = 16;
					spark->rotAng = GetRandomControl() & 0xFFF;

					if (GetRandomControl() & 1)
						spark->rotAdd = -64 - (GetRandomControl() & 0x3F);
					else
						spark->rotAdd = (GetRandomControl() & 0x3F) + 64;
				}
				else if (g_Level.Rooms[LaraItem->RoomNumber].flags & ENV_FLAG_WIND)
					spark->flags = 256;
				else
					spark->flags = SP_NONE;
				
				spark->gravity = -4 - (GetRandomControl() & 3);
				spark->maxYvel = -4 - (GetRandomControl() & 3);
				spark->dSize = ((GetRandomControl() & 0x3F) + 64);
				spark->sSize = spark->dSize / 8;
				spark->size = spark->dSize / 8;

				angle += deltaAngle;
			}
		}
	}

	void InitialiseDemigod(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->AnimNumber = Objects[item->ObjectNumber].animIndex;
		item->TargetState = STATE_DEMIGOD_IDLE;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->ActiveState = STATE_DEMIGOD_IDLE;

		/*if (g_Level.NumItems > 0)
		{
			ITEM_INFO* currentItem = &g_Level.Items[0];
			int k = 0;

			while (item == currentItem || currentItem->objectNumber != ID_DEMIGOD3 || currentItem->itemFlags[0])
			{
				k++;
				currentItem++;
				if (k >= g_Level.NumItems)
					return;
			}

			item->itemFlags[0] = k;
		}*/
	}

	void DemigodControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];

		int someItemNumber = item->ItemFlags[0];
		if (someItemNumber &&
			g_Level.Items[someItemNumber].Status == ITEM_ACTIVE &&
			g_Level.Items[someItemNumber].Active)
		{
			item->HitPoints = Objects[item->ObjectNumber].HitPoints;
			return;
		}

		auto* creature = GetCreatureInfo(item);

		short tilt = 0;
		short angle = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;
		short joint3 = 0;

		if (item->HitPoints <= 0)
		{
			item->HitPoints = 0;

			if (item->ActiveState != STATE_DEMIGOD_DEATH_1 &&
				item->ActiveState != STATE_DEMIGOD_DEATH_2)
			{
				if (item->ActiveState == STATE_DEMIGOD_WALK ||
					item->ActiveState == STATE_DEMIGOD_RUN)
				{
					item->AnimNumber = Objects[item->ObjectNumber].animIndex + 27;
					item->ActiveState = STATE_DEMIGOD_DEATH_2;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				}
				else
				{
					item->AnimNumber = Objects[item->ObjectNumber].animIndex + 12;
					item->ActiveState = STATE_DEMIGOD_DEATH_1;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				}
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);

			int dx = 0;
			int dy = 0;
			int dz = 0;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			AI_INFO laraAI;
			if (creature->enemy == LaraItem)
			{
				laraAI.ahead = AI.ahead;
				laraAI.angle = AI.angle;
				laraAI.xAngle = 0;
			}
			else
			{
				dx = LaraItem->Position.xPos - item->Position.xPos;
				dz = LaraItem->Position.zPos - item->Position.zPos;
				laraAI.angle = phd_atan(dz, dx) - item->Position.yRot;
				laraAI.xAngle = 0;

				laraAI.ahead = true;
				if (laraAI.angle <= -ANGLE(90.0f) || laraAI.angle >= ANGLE(90.0f))
					laraAI.ahead = false;

				dx = abs(dx);
				dy = item->Position.yPos - LaraItem->Position.yPos;
				dz = abs(dz);

				if (dx <= dz)
					laraAI.xAngle = phd_atan(dz + (dx >> 1), dy);
				else
					laraAI.xAngle = phd_atan(dx + (dz >> 1), dy);
			}

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, creature->maximumTurn);

			if (laraAI.ahead)
			{
				joint0 = laraAI.angle >> 1;
				joint1 = -laraAI.xAngle;
				joint2 = laraAI.angle >> 1;
				joint3 = laraAI.angle >> 1;
			}
			else if (AI.ahead)
			{
				joint0 = AI.angle >> 1;
				joint1 = -AI.xAngle;
				joint2 = AI.angle >> 1;
				joint3 = AI.angle >> 1;
			}

			switch (item->ActiveState)
			{
			case STATE_DEMIGOD_IDLE:
				creature->maximumTurn = 0;

				if (AI.ahead)
					joint1 = -AI.xAngle;

				if (item->ObjectNumber == ID_DEMIGOD1)
				{
					if (AI.distance >= pow(SECTOR(3), 2))
					{
						item->TargetState = STATE_DEMIGOD_WALK;
						break;
					}
					if (AI.bite ||
						LaraItem->ActiveState >= LS_LADDER_IDLE &&
						LaraItem->ActiveState <= LS_LADDER_DOWN &&
						!Lara.location)
					{
						item->TargetState = STATE_DEMIGOD_HAMMER_AIM;
						break;
					}
				}
				else
				{
					if (Targetable(item, &AI))
					{
						creature->flags = 1;

						if (item->ObjectNumber == ID_DEMIGOD2)
							item->TargetState = STATE_DEMIGOD_AIM;
						else
							item->TargetState = STATE_DEMIGOD_GROUND_AIM;

						break;
					}

					if (item->ObjectNumber == ID_DEMIGOD3)
					{
						if (AI.distance <= pow(SECTOR(2), 2) || AI.distance >= pow(SECTOR(5), 2))
						{
							item->TargetState = STATE_DEMIGOD_WALK;
							break;
						}

						if (!(GetRandomControl() & 3))
						{
							item->TargetState = STATE_DEMIGOD_CIRCLE_AIM;
							break;
						}
					}
				}

				if (AI.distance <= pow(SECTOR(3), 2) || item->ObjectNumber != ID_DEMIGOD2)
				{
					item->TargetState = STATE_DEMIGOD_WALK;
					break;
				}

				item->TargetState = STATE_DEMIGOD_FLY;
				break;

			case STATE_DEMIGOD_WALK:
				creature->maximumTurn = ANGLE(7.0f);

				if (AI.distance < pow(SECTOR(2), 2))
				{
					item->TargetState = STATE_DEMIGOD_IDLE;
					break;
				}

				if (item->ObjectNumber == ID_DEMIGOD1)
				{
					if (AI.distance < pow(SECTOR(3), 2))
					{
						item->TargetState = STATE_DEMIGOD_IDLE;
						break;
					}
				}
				else
				{
					if (Targetable(item, &AI))
					{
						item->TargetState = STATE_DEMIGOD_IDLE;
						break;
					}

				}

				if (AI.distance > pow(SECTOR(3), 2))
				{
					if (item->ObjectNumber == ID_DEMIGOD2)
						item->TargetState = STATE_DEMIGOD_FLY;
					else
						item->TargetState = STATE_DEMIGOD_RUN;
				}

				break;

			case STATE_DEMIGOD_RUN:
				creature->maximumTurn = ANGLE(7.0f);

				if (AI.distance < pow(SECTOR(2), 2))
				{
					item->TargetState = STATE_DEMIGOD_IDLE;
					break;
				}
				if (item->ObjectNumber == ID_DEMIGOD1)
				{
					if (AI.distance < pow(SECTOR(3), 2))
					{
						item->TargetState = STATE_DEMIGOD_IDLE;
						break;
					}
				}
				else
				{
					if (Targetable(item, &AI) || item->ObjectNumber == ID_DEMIGOD3 && AI.distance > pow(SECTOR(2), 2))
					{
						item->TargetState = STATE_DEMIGOD_IDLE;
						break;
					}

					if (AI.distance < pow(SECTOR(3), 2))
						item->TargetState = STATE_DEMIGOD_WALK;
				}

				break;

			case STATE_DEMIGOD_AIM:
				creature->maximumTurn = 0;

				if (AI.ahead)
					joint1 = -AI.xAngle;

				if (item->AnimNumber == Objects[item->ObjectNumber].animIndex + 6)
				{
					if (AI.angle >= ANGLE(7.0f))
						item->Position.yRot += ANGLE(7.0f);
					else if (AI.angle <= -ANGLE(7))
						item->Position.yRot += -ANGLE(7.0f);
					else
						item->Position.yRot += AI.angle;
				}

				if (Targetable(item, &AI) || creature->flags)
				{
					item->TargetState = STATE_DEMIGOD_ATTACK;
					creature->flags = 0;
				}
				else
				{
					item->TargetState = STATE_DEMIGOD_IDLE;
					creature->flags = 0;
				}

				break;

			case STATE_DEMIGOD_ATTACK:
			case STATE_DEMIGOD_GROUND_ATTACK:
				DoDemigodEffects(itemNumber);
				break;

			case STATE_DEMIGOD_FLY:
				creature->maximumTurn = ANGLE(7.0f);

				if (Targetable(item, &AI))
					item->TargetState = STATE_DEMIGOD_IDLE_FLY;

				break;

			case STATE_DEMIGOD_CIRCLE_AIM:
				creature->maximumTurn = ANGLE(7.0f);
				if (!Targetable(item, &AI) && AI.distance < pow(SECTOR(5), 2))
					item->TargetState = STATE_DEMIGOD_CIRCLE_ATTACK;

				break;

			case STATE_DEMIGOD_CIRCLE_ATTACK:
				creature->maximumTurn = ANGLE(7.0f);

				DoDemigodEffects(itemNumber);

				if (!Targetable(item, &AI) || AI.distance < pow(SECTOR(5), 2) || !GetRandomControl())
				{
					item->TargetState = STATE_DEMIGOD_IDLE;
					break;
				}

				break;

			case STATE_DEMIGOD_GROUND_AIM:
				creature->maximumTurn = 0;
				joint2 = joint0;
				joint0 = 0;

				if (AI.ahead)
					joint1 = -AI.xAngle;

				if (item->AnimNumber == Objects[(signed short)item->ObjectNumber].animIndex + 6)
				{
					if (AI.angle >= ANGLE(7.0f))
						item->Position.yRot += ANGLE(7.0f);
					else if (AI.angle <= -ANGLE(7.0f))
						item->Position.yRot += -ANGLE(7.0f);
					else
						item->Position.yRot += AI.angle;
				}

				if (Targetable(item, &AI) || creature->flags)
				{
					item->TargetState = STATE_DEMIGOD_GROUND_ATTACK;
					creature->flags = 0;
				}
				else
				{
					item->TargetState = STATE_DEMIGOD_IDLE;
					creature->flags = 0;
				}

				break;

			case STATE_DEMIGOD_HAMMER_AIM:
				creature->maximumTurn = 0;
				joint2 = joint0;
				joint0 = 0;

				if (AI.angle >= ANGLE(7.0f))
					item->Position.yRot += ANGLE(7.0f);
				else if (AI.angle <= -ANGLE(7.0f))
					item->Position.yRot += -ANGLE(7.0f);
				else
					item->Position.yRot += AI.angle;

				if (AI.distance >= pow(SECTOR(3), 2) ||
					!AI.bite &&
					(LaraItem->ActiveState < LS_LADDER_IDLE ||
						LaraItem->ActiveState > LS_LADDER_DOWN ||
						Lara.location))
				{
					item->TargetState = STATE_DEMIGOD_IDLE;
				}
				else
					item->TargetState = STATE_DEMIGOD_HAMMER_ATTACK;

				break;

			case STATE_DEMIGOD_HAMMER_ATTACK:
				if (item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase == 26)
				{
					PHD_VECTOR pos = { 80, -8, -40 };
					GetJointAbsPosition(item, &pos, 17);

					short roomNumber = item->RoomNumber;
					FLOOR_INFO* floor = GetFloor(pos.x, pos.y, pos.z, &roomNumber);
					int height = GetFloorHeight(floor, pos.x, pos.y, pos.z);
					if (height == NO_HEIGHT)
						pos.y = pos.y - 128;
					else
						pos.y = height - 128;

					TriggerShockwave((PHD_3DPOS*)&pos, 24, 88, 256, 128, 128, 128, 32, 0, 2);
					TriggerHammerSmoke(pos.x, pos.y + 128, pos.z, 8);

					Camera.bounce = -128;

					if (LaraItem->ActiveState >= 56 && LaraItem->ActiveState <= 61 && !Lara.location)
					{
						ResetLaraFlex(LaraItem);
						LaraItem->TargetState = 3;
						LaraItem->ActiveState = 3;
						LaraItem->AnimNumber = 34;
						LaraItem->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
						LaraItem->HitStatus = true;
						LaraItem->Velocity = 2;
						LaraItem->VerticalVelocity = 1;
						Lara.Control.HandStatus = HandStatus::Free;
					}
				}

			default:
				break;
			}
		}

		CreatureTilt(item, 0);

		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);
		CreatureJoint(item, 3, joint3);

		CreatureAnimation(itemNumber, angle, 0);
	}
}
