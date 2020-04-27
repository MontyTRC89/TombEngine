#include "../newobjects.h"
#include "../../Game/Box.h"
#include "../../Game/items.h"
#include "../../Game/sphere.h"
#include "../../Game/effect2.h"
#include "../../Game/lara.h"
#include "../../Game/people.h"
#include "../../Specific/setup.h"
#include "../../Game/lot.h"
#include "..\..\Specific\level.h"

BITE_INFO TroopsBite1 = { 0, 300, 64, 7 };



void InitialiseTroops(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	if (item->triggerFlags == 1)
	{
		item->goalAnimState = item->currentAnimState = 16;
		item->animNumber = Objects[item->objectNumber].animIndex + 27;
	}
	else
	{
		item->goalAnimState = item->currentAnimState = 1;
		item->animNumber = Objects[item->objectNumber].animIndex + 12;
	}

	item->frameNumber = Anims[item->animNumber].frameBase;
}

void TroopsControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
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
		if (item->currentAnimState != 7 && item->currentAnimState != 15)
		{
			if (creature->enemy && creature->enemy->objectNumber == ID_BIG_SCORPION && (item->itemFlags[0] < 80))
			{
				if (creature->enemy->animNumber == Objects[ID_BIG_SCORPION].animIndex + 6)
				{
					item->animNumber = Objects[item->objectNumber].animIndex + 23;
					if (item->currentAnimState == 16)
					{
						item->frameNumber = Anims[item->animNumber].frameBase += 37;
					}
					else
					{
						item->frameNumber = Anims[item->animNumber].frameBase;
					}
					item->goalAnimState = 15;
					item->currentAnimState = 15;

					angle = 0;

					item->pos.xPos = creature->enemy->pos.xPos;
					item->pos.yPos = creature->enemy->pos.yPos;
					item->pos.zPos = creature->enemy->pos.zPos;

					item->pos.xRot = creature->enemy->pos.xRot;
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
				item->currentAnimState = 7;
				item->frameNumber = Anims[item->frameNumber].frameBase;
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
			CREATURE_INFO* baddy = &BaddieSlots[0];
			int minDistance = 0x7FFFFFFF;

			for (int i = 0; i < NUM_SLOTS; i++)
			{
				baddy = &BaddieSlots[i];

				if (baddy->itemNum != NO_ITEM && baddy->itemNum != itemNum)
				{
					ITEM_INFO* currentItem = &Items[baddy->itemNum];

					if (currentItem->objectNumber != ID_TROOPS &&
						(currentItem != LaraItem || creature->hurtByLara))
					{
						dx = currentItem->pos.xPos - item->pos.xPos;
						dy = currentItem->pos.yPos - item->pos.yPos;
						dz = currentItem->pos.zPos - item->pos.zPos;
						distance = dx * dx + dy * dy + dz * dz;

						if (distance < minDistance)
						{
							minDistance = distance;
							creature->enemy = currentItem;
						}
					}
				}
			}
		}
		
		if (creature->hurtByLara && item->currentAnimState != 16)
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
			distance = dx * dx + dz * dz;
			rot = phd_atan(dz, dx) - item->pos.yRot;
		}

		GetCreatureMood(item, &info, TIMID);

		// Vehicle handling
		if (Lara.Vehicle != NO_ITEM && info.bite)
			creature->mood == ESCAPE_MOOD;

		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->hitStatus)
			AlertAllGuards(itemNum);

		switch (item->currentAnimState)
		{
		case 1:
			creature->flags = 0;
			creature->maximumTurn = 0;
			joint2 = rot;

			if (item->animNumber == obj->animIndex + 17)
			{
				if (abs(info.angle) >= ANGLE(10))
				{
					if ((info.angle & 0x8000u) == 0)
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
				if (!(byte)GetRandomControl())
				{
					if (item->currentAnimState == 1)
					{
						item->goalAnimState = 4;
						break;
					}
					item->goalAnimState = 1;
				}
			}
			else if (item->aiBits & PATROL1)
			{
				item->goalAnimState = 2;
				joint2 = 0;
			}
			else if (creature->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < SQUARE(3072) || info.zoneNumber != info.enemyZone)
				{
					if (GetRandomControl() >= 0x4000)
					{
						item->goalAnimState = 10;
					}
					else
					{
						item->goalAnimState = 8;
					}
				}
				else
				{
					item->goalAnimState = 2;
				}
			}
			else
			{
				if ((creature->alerted || creature->mood)
					&& (!(item->aiBits & FOLLOW) || !(item->aiBits & MODIFY) && distance <= SQUARE(2048)))
				{
					if (!creature->mood || info.distance <= SQUARE(2048))
					{
						item->goalAnimState = 2;
						break;
					}
					item->goalAnimState = 3;
				}
				else
				{
					item->goalAnimState = 1;
				}
			}

			break;

		case 2:
			creature->flags = 0;
			joint2 = rot;
			creature->maximumTurn = ANGLE(5);

			if (item->aiBits & PATROL1)
			{
				item->goalAnimState = 2;
			}
			else if (creature->mood == 2)
			{
				item->goalAnimState = 3;
			}
			else
			{
				if ((item->aiBits & GUARD) || (item->aiBits & GUARD) && 
					(creature->reachedGoal || distance > SQUARE(2048)))
				{
					item->goalAnimState = 1;
					break;
				}
				if (Targetable(item, &info))
				{
					if (info.distance < SQUARE(3072) || info.enemyZone != info.zoneNumber)
					{
						item->goalAnimState = 1;
						break;
					}
					item->goalAnimState = 9;
				}
				else if (creature->mood)
				{
					if (info.distance > SQUARE(2048))
					{
						item->goalAnimState = 3;
					}
				}
				else if (info.ahead)
				{
					item->goalAnimState = 1;
					break;
				}
			}

			break;

		case 3:
			if (info.ahead)
			{
				joint2 = info.angle;
			}
			creature->maximumTurn = ANGLE(10);
			tilt = angle / 2;

			if ((item->aiBits & GUARD) || (item->aiBits & FOLLOW) && 
				(creature->reachedGoal || distance > SQUARE(2048)))
			{
				item->goalAnimState = 2;
				break;
			}
			if (creature->mood != ESCAPE_MOOD)
			{
				if (Targetable(item, &info))
				{
					item->goalAnimState = 2;
					break;
				}
				if (!creature->mood || creature->mood == STALK_MOOD && 
					!(item->aiBits & FOLLOW) && info.distance < SQUARE(2048))
				{
					item->goalAnimState = 2;
				}
			}

			break;

		case 4:
			creature->flags = 0;
			creature->maximumTurn = 0;
			joint2 = rot;

			if (item->aiBits & GUARD)
			{
				joint2 = AIGuard(creature);
				if (!(byte)GetRandomControl())
				{
					item->goalAnimState = 1;
				}
			}
			else if (Targetable(item, &info))
			{
				item->goalAnimState = 5;
			}
			else if (creature->mood || !info.ahead)
			{
				item->goalAnimState = 1;
			}

			break;

		case 5:
		case 6:
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

		case 8:
		case 10:
			creature->flags = 0;

			if (info.ahead)
			{
				joint1 = info.xAngle;
				joint0 = info.angle;

				if (Targetable(item, &info))
				{
					item->goalAnimState = item->currentAnimState != 8 ? 11 : 5;
				}
				else
				{
					item->goalAnimState = 1;
				}
			}

			break;

		case 9:
			creature->flags = 0;

			if (info.ahead)
			{
				joint1 = info.xAngle;
				joint0 = info.angle;

				if (Targetable(item, &info))
				{
					item->goalAnimState = 6;
				}
				else
				{
					item->goalAnimState = 2;
				}
			}

			break;

		case 11:
			if (item->goalAnimState != 1
				&& (creature->mood == ESCAPE_MOOD || 
					info.distance > SQUARE(3072) || 
					!Targetable(item, &info)))
			{
				item->goalAnimState = 1;
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

		case 16:
			creature->maximumTurn = 0;
			break;

		case 17u:
			if (!WeaponEnemyTimer && !(GetRandomControl() & 0x7F))
			{
				item->goalAnimState = 4;
			}

			break;

		default:
			break;
		}
	}

	CreatureTilt(item, tilt);
	
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	
	CreatureAnimation(itemNum, angle, 0);
}