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

	if (item->triggerFlags == 1)
	{
		item->goalAnimState = item->currentAnimState = STATE_TROOPS_ATTACKED_BY_SCORPION;
		item->animNumber = Objects[item->objectNumber].animIndex + 27;
	}
	else
	{
		item->goalAnimState = item->currentAnimState = STATE_TROOPS_STOP;
		item->animNumber = Objects[item->objectNumber].animIndex + 12;
	}

	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
}

void TroopsControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];
	
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

	if (item->firedWeapon)
	{
		PHD_VECTOR pos;

		pos.x = TroopsBite1.x;
		pos.y = TroopsBite1.y;
		pos.z = TroopsBite1.z;

		GetJointAbsPosition(item, &pos, TroopsBite1.meshNum);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * item->firedWeapon + 8, 24, 16, 4);

		item->firedWeapon--;
	}

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != STATE_TROOPS_DEATH 
			&& item->currentAnimState != STATE_TROOPS_KILLED_BY_SCORPION)
		{
			if (creature->enemy 
				&& creature->enemy->objectNumber == ID_BIG_SCORPION 
				&& item->itemFlags[0] < 80)
			{
				if (creature->enemy->animNumber == Objects[ID_BIG_SCORPION].animIndex + 6)
				{
					item->animNumber = Objects[item->objectNumber].animIndex + 23;
					if (item->currentAnimState == STATE_TROOPS_ATTACKED_BY_SCORPION)
					{
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 37;
					}
					else
					{
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					}
					item->goalAnimState = STATE_TROOPS_KILLED_BY_SCORPION;
					item->currentAnimState = STATE_TROOPS_KILLED_BY_SCORPION;

					angle = 0;

					item->pos.xPos = creature->enemy->pos.xPos;
					item->pos.yPos = creature->enemy->pos.yPos;
					item->pos.zPos = creature->enemy->pos.zPos;

					item->pos.xRot = creature->enemy->pos.xRot;
					item->pos.yRot = creature->enemy->pos.yRot;
					item->pos.zRot = creature->enemy->pos.zRot;

					creature->enemy->triggerFlags = 99;
				}
				else
				{
					angle = 0;
					item->itemFlags[0]++;
				}
			}
			else
			{
				item->animNumber = Objects[item->objectNumber].animIndex + 19;
				item->currentAnimState = STATE_TROOPS_DEATH;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			}
		}
	}
	else
	{
		if (item->aiBits)
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

					if (currentItem->objectNumber != ID_LARA)
					{
						if (currentItem->objectNumber != ID_TROOPS &&
							(currentItem != LaraItem || creature->hurtByLara))
						{
							dx = currentItem->pos.xPos - item->pos.xPos;
							dy = currentItem->pos.yPos - item->pos.yPos;
							dz = currentItem->pos.zPos - item->pos.zPos;

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
		
		if (creature->hurtByLara && item->currentAnimState != STATE_TROOPS_ATTACKED_BY_SCORPION)
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
			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;
			distance = SQUARE(dx) + SQUARE(dz);
			rot = phd_atan(dz, dx) - item->pos.yRot;
		}

		if (!creature->hurtByLara && creature->enemy == LaraItem)
			creature->enemy = NULL;

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		// Vehicle handling
		if (Lara.Vehicle != NO_ITEM && info.bite)
			creature->mood = ESCAPE_MOOD;

		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->hitStatus)
			AlertAllGuards(itemNumber);

		switch (item->currentAnimState)
		{
		case STATE_TROOPS_STOP:
			creature->flags = 0;
			creature->maximumTurn = 0;
			joint2 = rot;

			if (item->animNumber == obj->animIndex + 17)
			{
				if (abs(info.angle) >= ANGLE(10))
				{
					if (info.angle >= 0)
					{
						item->pos.yRot += ANGLE(10);
					}
					else
					{
						item->pos.yRot -= ANGLE(10);
					}
				}
				else
				{
					item->pos.yRot += info.angle;
				}
			}

			if (item->aiBits & GUARD)
			{
				joint2 = AIGuard(creature);
				if (!GetRandomControl())
				{
					if (item->currentAnimState == STATE_TROOPS_STOP)
					{
						item->goalAnimState = STATE_TROOPS_GUARD;
					}
					else
					{
						item->goalAnimState = STATE_TROOPS_STOP;
					}
				}
			}
			else if (item->aiBits & PATROL1)
			{
				item->goalAnimState = STATE_TROOPS_WALK;
				joint2 = 0;
			}
			else if (creature->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = STATE_TROOPS_RUN;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < SQUARE(3072) || info.zoneNumber != info.enemyZone)
				{
					if (GetRandomControl() >= 16384)
					{
						item->goalAnimState = STATE_TROOPS_AIM3;
					}
					else
					{
						item->goalAnimState = STATE_TROOPS_AIM1;
					}
				}
				else
				{
					item->goalAnimState = STATE_TROOPS_WALK;
				}
			}
			else
			{
				if ((creature->alerted 
					|| creature->mood != BORED_MOOD)
					&& (!(item->aiBits & FOLLOW) 
						|| !(item->aiBits & MODIFY) 
						&& distance <= SQUARE(2048)))
				{
					if (!creature->mood || info.distance <= SQUARE(2048))
					{
						item->goalAnimState = STATE_TROOPS_WALK;
						break;
					}
					item->goalAnimState = STATE_TROOPS_RUN;
				}
				else
				{
					item->goalAnimState = STATE_TROOPS_STOP;
				}
			}

			break;

		case STATE_TROOPS_WALK:
			creature->flags = 0;
			joint2 = rot;
			creature->maximumTurn = ANGLE(5);

			if (item->aiBits & PATROL1)
			{
				item->goalAnimState = STATE_TROOPS_WALK;
			}
			else if (creature->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = STATE_TROOPS_RUN;
			}
			else
			{
				if ((item->aiBits & GUARD)
					|| (item->aiBits & FOLLOW) 
					&& (creature->reachedGoal 
						|| distance > SQUARE(2048)))
				{
					item->goalAnimState = STATE_TROOPS_STOP;
				}
				else if (Targetable(item, &info))
				{
					if (info.distance < SQUARE(3072) || info.enemyZone != info.zoneNumber)
					{
						item->goalAnimState = STATE_TROOPS_STOP;
					}
					else
					{
						item->goalAnimState = STATE_TROOPS_AIM2;
					}
				}
				else if (creature->mood != BORED_MOOD)
				{
					if (info.distance > SQUARE(2048))
					{
						item->goalAnimState = STATE_TROOPS_RUN;
					}
				}
				else if (info.ahead)
				{
					item->goalAnimState = STATE_TROOPS_STOP;
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

			if ((item->aiBits & GUARD) 
				|| (item->aiBits & FOLLOW) 
				&& (creature->reachedGoal 
					|| distance > SQUARE(2048)))
			{
				item->goalAnimState = STATE_TROOPS_WALK;
			}
			else if (creature->mood != ESCAPE_MOOD)
			{
				if (Targetable(item, &info))
				{
					item->goalAnimState = STATE_TROOPS_WALK;
				}
				else if (creature->mood == BORED_MOOD 
					|| creature->mood == STALK_MOOD 
					&& !(item->aiBits & FOLLOW) 
					&& info.distance < SQUARE(2048))
				{
					item->goalAnimState = STATE_TROOPS_WALK;
				}
			}

			break;

		case STATE_TROOPS_GUARD:
			creature->flags = 0;
			creature->maximumTurn = 0;
			joint2 = rot;

			if (item->aiBits & GUARD)
			{
				joint2 = AIGuard(creature);
				if (!GetRandomControl())
				{
					item->goalAnimState = STATE_TROOPS_STOP;
				}
			}
			else if (Targetable(item, &info))
			{
				item->goalAnimState = STATE_TROOPS_ATTACK1;
			}
			else if (creature->mood != BORED_MOOD || !info.ahead)
			{
				item->goalAnimState = STATE_TROOPS_STOP;
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
					item->goalAnimState = item->currentAnimState != STATE_TROOPS_AIM1 ? STATE_TROOPS_ATTACK3 : STATE_TROOPS_ATTACK1;
				}
				else
				{
					item->goalAnimState = STATE_TROOPS_STOP;
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
					item->goalAnimState = STATE_TROOPS_ATTACK2;
				}
				else
				{
					item->goalAnimState = STATE_TROOPS_WALK;
				}
			}

			break;

		case STATE_TROOPS_ATTACK3:
			if (item->goalAnimState != STATE_TROOPS_STOP
				&& (creature->mood == ESCAPE_MOOD || 
					info.distance > SQUARE(3072) || 
					!Targetable(item, &info)))
			{
				item->goalAnimState = STATE_TROOPS_STOP;
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
				item->goalAnimState = STATE_TROOPS_GUARD;
			}

			break;

		default:
			break;
		}

		if (WeaponEnemyTimer > 100)
		{
			if (item->currentAnimState != STATE_TROOPS_FLASHED 
				&& item->currentAnimState != STATE_TROOPS_ATTACKED_BY_SCORPION)
			{
				creature->maximumTurn = 0;
				item->animNumber = Objects[item->objectNumber].animIndex + 28;
				item->currentAnimState = STATE_TROOPS_FLASHED;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase + (GetRandomControl() & 7);
			}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);
}
