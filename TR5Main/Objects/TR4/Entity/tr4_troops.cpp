#include "framework.h"
#include "tr4_troops.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Specific/setup.h"
#include "Game/control/lot.h"
#include "Specific/level.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"
#include "Game/animation.h"

BITE_INFO TroopsBite1 = { 0, 300, 64, 7 };

#define STATE_TROOPS_STOP						1
#define STATE_TROOPS_WALK						2
#define STATE_TROOPS_RUN						3
#define STATE_TROOPS_GUARD						4
#define STATE_TROOPS_ATTACK1					5
#define STATE_TROOPS_ATTACK2					6
#define STATE_TROOPS_DEATH						7
#define STATE_TROOPS_AIM1						8
#define STATE_TROOPS_AIM2						9
#define STATE_TROOPS_AIM3						10
#define STATE_TROOPS_ATTACK3					11
#define STATE_TROOPS_KILLED_BY_SCORPION			15
#define STATE_TROOPS_ATTACKED_BY_SCORPION		16
#define STATE_TROOPS_FLASHED					17

void InitialiseTroops(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	if (item->TriggerFlags == 1)
	{
		item->TargetState = item->ActiveState = STATE_TROOPS_ATTACKED_BY_SCORPION;
		item->AnimNumber = Objects[item->ObjectNumber].animIndex + 27;
	}
	else
	{
		item->TargetState = item->ActiveState = STATE_TROOPS_STOP;
		item->AnimNumber = Objects[item->ObjectNumber].animIndex + 12;
	}

	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
}

void TroopsControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	OBJECT_INFO* obj = &Objects[item->ObjectNumber];
	
	short angle = 0;
	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short rot = 0;
	
	int dx = 0;
	int dy = 0;
	int dz = 0;
	
	int distance = 0;

	if (item->FiredWeapon)
	{
		PHD_VECTOR pos;

		pos.x = TroopsBite1.x;
		pos.y = TroopsBite1.y;
		pos.z = TroopsBite1.z;

		GetJointAbsPosition(item, &pos, TroopsBite1.meshNum);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * item->FiredWeapon + 8, 24, 16, 4);

		item->FiredWeapon--;
	}

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != STATE_TROOPS_DEATH 
			&& item->ActiveState != STATE_TROOPS_KILLED_BY_SCORPION)
		{
			if (creature->enemy 
				&& creature->enemy->ObjectNumber == ID_BIG_SCORPION 
				&& item->ItemFlags[0] < 80)
			{
				if (creature->enemy->AnimNumber == Objects[ID_BIG_SCORPION].animIndex + 6)
				{
					item->AnimNumber = Objects[item->ObjectNumber].animIndex + 23;
					if (item->ActiveState == STATE_TROOPS_ATTACKED_BY_SCORPION)
					{
						item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase + 37;
					}
					else
					{
						item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
					}
					item->TargetState = STATE_TROOPS_KILLED_BY_SCORPION;
					item->ActiveState = STATE_TROOPS_KILLED_BY_SCORPION;

					angle = 0;

					item->Position.xPos = creature->enemy->Position.xPos;
					item->Position.yPos = creature->enemy->Position.yPos;
					item->Position.zPos = creature->enemy->Position.zPos;

					item->Position.xRot = creature->enemy->Position.xRot;
					item->Position.yRot = creature->enemy->Position.yRot;
					item->Position.zRot = creature->enemy->Position.zRot;

					creature->enemy->TriggerFlags = 99;
				}
				else
				{
					angle = 0;
					item->ItemFlags[0]++;
				}
			}
			else
			{
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 19;
				item->ActiveState = STATE_TROOPS_DEATH;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			}
		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(creature);
		else
		{
			// Search for active troops
			creature->enemy = NULL;
			CREATURE_INFO* baddy = ActiveCreatures[0];
			int minDistance = 0x7FFFFFFF;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				baddy = ActiveCreatures[i];

				if (baddy->itemNum != NO_ITEM && baddy->itemNum != itemNumber)
				{
					ITEM_INFO* currentItem = &g_Level.Items[baddy->itemNum];

					if (currentItem->ObjectNumber != ID_LARA)
					{
						if (currentItem->ObjectNumber != ID_TROOPS &&
							(currentItem != LaraItem || creature->hurtByLara))
						{
							dx = currentItem->Position.xPos - item->Position.xPos;
							dy = currentItem->Position.yPos - item->Position.yPos;
							dz = currentItem->Position.zPos - item->Position.zPos;

							distance = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);

							if (distance < minDistance)
							{
								minDistance = distance;
								creature->enemy = currentItem;
							}
						}
					}
				}
			}
		}
		
		if (creature->hurtByLara && item->ActiveState != STATE_TROOPS_ATTACKED_BY_SCORPION)
			creature->enemy = LaraItem;

		AI_INFO info;	
		CreatureAIInfo(item, &info);

		int distance = 0;
		if (creature->enemy == LaraItem)
		{
			distance = info.distance;
			rot = info.angle;
		}
		else
		{
			dx = LaraItem->Position.xPos - item->Position.xPos;
			dz = LaraItem->Position.zPos - item->Position.zPos;
			distance = SQUARE(dx) + SQUARE(dz);
			rot = phd_atan(dz, dx) - item->Position.yRot;
		}

		if (!creature->hurtByLara && creature->enemy == LaraItem)
			creature->enemy = NULL;

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		// Vehicle handling
		if (Lara.Vehicle != NO_ITEM && info.bite)
			creature->mood = ESCAPE_MOOD;

		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->HitStatus)
			AlertAllGuards(itemNumber);

		switch (item->ActiveState)
		{
		case STATE_TROOPS_STOP:
			creature->flags = 0;
			creature->maximumTurn = 0;
			joint2 = rot;

			if (item->AnimNumber == obj->animIndex + 17)
			{
				if (abs(info.angle) >= ANGLE(10))
				{
					if (info.angle >= 0)
					{
						item->Position.yRot += ANGLE(10);
					}
					else
					{
						item->Position.yRot -= ANGLE(10);
					}
				}
				else
				{
					item->Position.yRot += info.angle;
				}
			}

			if (item->AIBits & GUARD)
			{
				joint2 = AIGuard(creature);
				if (!GetRandomControl())
				{
					if (item->ActiveState == STATE_TROOPS_STOP)
					{
						item->TargetState = STATE_TROOPS_GUARD;
					}
					else
					{
						item->TargetState = STATE_TROOPS_STOP;
					}
				}
			}
			else if (item->AIBits & PATROL1)
			{
				item->TargetState = STATE_TROOPS_WALK;
				joint2 = 0;
			}
			else if (creature->mood == ESCAPE_MOOD)
			{
				item->TargetState = STATE_TROOPS_RUN;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < SQUARE(3072) || info.zoneNumber != info.enemyZone)
				{
					if (GetRandomControl() >= 16384)
					{
						item->TargetState = STATE_TROOPS_AIM3;
					}
					else
					{
						item->TargetState = STATE_TROOPS_AIM1;
					}
				}
				else
				{
					item->TargetState = STATE_TROOPS_WALK;
				}
			}
			else
			{
				if ((creature->alerted 
					|| creature->mood != BORED_MOOD)
					&& (!(item->AIBits & FOLLOW) 
						|| !(item->AIBits & MODIFY) 
						&& distance <= SQUARE(2048)))
				{
					if (!creature->mood || info.distance <= SQUARE(2048))
					{
						item->TargetState = STATE_TROOPS_WALK;
						break;
					}
					item->TargetState = STATE_TROOPS_RUN;
				}
				else
				{
					item->TargetState = STATE_TROOPS_STOP;
				}
			}

			break;

		case STATE_TROOPS_WALK:
			creature->flags = 0;
			joint2 = rot;
			creature->maximumTurn = ANGLE(5);

			if (item->AIBits & PATROL1)
			{
				item->TargetState = STATE_TROOPS_WALK;
			}
			else if (creature->mood == ESCAPE_MOOD)
			{
				item->TargetState = STATE_TROOPS_RUN;
			}
			else
			{
				if ((item->AIBits & GUARD)
					|| (item->AIBits & FOLLOW) 
					&& (creature->reachedGoal 
						|| distance > SQUARE(2048)))
				{
					item->TargetState = STATE_TROOPS_STOP;
				}
				else if (Targetable(item, &info))
				{
					if (info.distance < SQUARE(3072) || info.enemyZone != info.zoneNumber)
					{
						item->TargetState = STATE_TROOPS_STOP;
					}
					else
					{
						item->TargetState = STATE_TROOPS_AIM2;
					}
				}
				else if (creature->mood != BORED_MOOD)
				{
					if (info.distance > SQUARE(2048))
					{
						item->TargetState = STATE_TROOPS_RUN;
					}
				}
				else if (info.ahead)
				{
					item->TargetState = STATE_TROOPS_STOP;
				}
			}

			break;

		case STATE_TROOPS_RUN:
			if (info.ahead)
			{
				joint2 = info.angle;
			}
			creature->maximumTurn = ANGLE(10);
			tilt = angle / 2;

			if ((item->AIBits & GUARD) 
				|| (item->AIBits & FOLLOW) 
				&& (creature->reachedGoal 
					|| distance > SQUARE(2048)))
			{
				item->TargetState = STATE_TROOPS_WALK;
			}
			else if (creature->mood != ESCAPE_MOOD)
			{
				if (Targetable(item, &info))
				{
					item->TargetState = STATE_TROOPS_WALK;
				}
				else if (creature->mood == BORED_MOOD 
					|| creature->mood == STALK_MOOD 
					&& !(item->AIBits & FOLLOW) 
					&& info.distance < SQUARE(2048))
				{
					item->TargetState = STATE_TROOPS_WALK;
				}
			}

			break;

		case STATE_TROOPS_GUARD:
			creature->flags = 0;
			creature->maximumTurn = 0;
			joint2 = rot;

			if (item->AIBits & GUARD)
			{
				joint2 = AIGuard(creature);
				if (!GetRandomControl())
				{
					item->TargetState = STATE_TROOPS_STOP;
				}
			}
			else if (Targetable(item, &info))
			{
				item->TargetState = STATE_TROOPS_ATTACK1;
			}
			else if (creature->mood != BORED_MOOD || !info.ahead)
			{
				item->TargetState = STATE_TROOPS_STOP;
			}

			break;

		case STATE_TROOPS_ATTACK1:
		case STATE_TROOPS_ATTACK2:
			if (info.ahead)
			{
				joint0 = info.angle;
				joint1 = info.xAngle;
			}

			if (creature->flags)
			{
				creature->flags--;
			}
			else
			{
				ShotLara(item, &info, &TroopsBite1, joint0, 23);
				creature->flags = 5;
			}

			break;

		case STATE_TROOPS_AIM1:
		case STATE_TROOPS_AIM3:
			creature->flags = 0;

			if (info.ahead)
			{
				joint1 = info.xAngle;
				joint0 = info.angle;

				if (Targetable(item, &info))
				{
					item->TargetState = item->ActiveState != STATE_TROOPS_AIM1 ? STATE_TROOPS_ATTACK3 : STATE_TROOPS_ATTACK1;
				}
				else
				{
					item->TargetState = STATE_TROOPS_STOP;
				}
			}

			break;

		case STATE_TROOPS_AIM2:
			creature->flags = 0;

			if (info.ahead)
			{
				joint1 = info.xAngle;
				joint0 = info.angle;

				if (Targetable(item, &info))
				{
					item->TargetState = STATE_TROOPS_ATTACK2;
				}
				else
				{
					item->TargetState = STATE_TROOPS_WALK;
				}
			}

			break;

		case STATE_TROOPS_ATTACK3:
			if (item->TargetState != STATE_TROOPS_STOP
				&& (creature->mood == ESCAPE_MOOD || 
					info.distance > SQUARE(3072) || 
					!Targetable(item, &info)))
			{
				item->TargetState = STATE_TROOPS_STOP;
			}

			if (info.ahead)
			{
				joint0 = info.angle;
				joint1 = info.xAngle;
			}

			if (creature->flags)
			{
				creature->flags--;
			}
			else
			{
				ShotLara(item, &info, &TroopsBite1, joint0, 23);
				creature->flags = 5;
			}

			break;

		case STATE_TROOPS_ATTACKED_BY_SCORPION:
			creature->maximumTurn = 0;
			break;

		case STATE_TROOPS_FLASHED:
			if (!WeaponEnemyTimer && !(GetRandomControl() & 0x7F))
			{
				item->TargetState = STATE_TROOPS_GUARD;
			}

			break;

		default:
			break;
		}

		if (WeaponEnemyTimer > 100)
		{
			if (item->ActiveState != STATE_TROOPS_FLASHED 
				&& item->ActiveState != STATE_TROOPS_ATTACKED_BY_SCORPION)
			{
				creature->maximumTurn = 0;
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 28;
				item->ActiveState = STATE_TROOPS_FLASHED;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase + (GetRandomControl() & 7);
			}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);
}
