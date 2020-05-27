#include "framework.h"
#include "tr4_sas.h"
#include "sphere.h"
#include "effect2.h"
#include "box.h"
#include "items.h"
#include "people.h"
#include "lara.h"
#include "setup.h"
#include "level.h"

BITE_INFO sasGun = { 0, 300, 64, 7 };



void InitialiseSas(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	ClearItem(itemNumber);

	item->animNumber = Objects[item->objectNumber].animIndex + ANIMATION_SAS_STAND;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = STATE_SAS_STOP;
	item->currentAnimState = STATE_SAS_STOP;
}

void SasControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ITEM_INFO* enemyItem = creature->enemy;

	short tilt = 0;
	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;

	// Handle SAS firing
	if (item->firedWeapon)
	{
		PHD_VECTOR pos;
		
		pos.x = sasGun.x;
		pos.y = sasGun.y;
		pos.z = sasGun.z;

		GetJointAbsPosition(item, &pos, sasGun.meshNum);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 24, 16, 4);
		item->firedWeapon--;
	}

	if (item->hitPoints > 0)
	{
		if (item->aiBits)
			GetAITarget(creature);
		else
			creature->enemy = LaraItem;

		AI_INFO info;

		int distance = 0;
		int ang = 0;
		int dx = 0;
		int dz = 0;

		CreatureAIInfo(item, &info);

		if (creature->enemy == LaraItem)
		{
			ang = info.angle;
			distance = info.distance;
		}
		else
		{
			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;
			ang = phd_atan(dz, dx) - item->pos.yRot;
			distance = dx * dx + dz * dz;
		}

		GetCreatureMood(item, &info, creature->enemy != LaraItem);

		// Vehicle handling
		if (Lara.Vehicle != NO_ITEM && info.bite)
			creature->mood == ESCAPE_MOOD;

		CreatureMood(item, &info, creature->enemy != LaraItem);
		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->hitStatus)
			AlertAllGuards(itemNumber);

		int angle1 = 0;
		int angle2 = 0;

		switch (item->currentAnimState)
		{
		case STATE_SAS_STOP:
			creature->flags = 0;
			creature->maximumTurn = 0;
			joint2 = ang;
			if (item->animNumber == Objects[item->objectNumber].animIndex + ANIMATION_SAS_WALK_TO_STAND)
			{
				if (abs(info.angle) >= ANGLE(10))
				{
					if (info.angle >= 0)
						item->pos.yRot += ANGLE(10);
					else
						item->pos.yRot -= ANGLE(10);
				}
				else
				{
					item->pos.yRot += info.angle;
				}
			}
			else if (item->aiBits & MODIFY /*|| Lara.bike*/)
			{
				if (abs(info.angle) >= ANGLE(2))
				{
					if (info.angle >= 0)
						item->pos.yRot += ANGLE(2);
					else
						item->pos.yRot -= ANGLE(2);
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
					if (item->currentAnimState == STATE_SAS_STOP)
					{
						item->goalAnimState = STATE_SAS_WAIT;
						break;
					}
					item->goalAnimState = STATE_SAS_STOP;
				}
			}
			else if (!(item->aiBits & PATROL1) || (item->aiBits & 0x1F) == MODIFY /* || Lara_Bike*/)
			{
				if (Targetable(item, &info))
				{
					if (info.distance < 9437184 || info.zoneNumber != info.enemyZone)
					{
						if (GetRandomControl() & 1)
						{
							item->goalAnimState = STATE_SAS_SIGHT_AIM;
						}
						else if (GetRandomControl() & 1)
						{
							item->goalAnimState = STATE_SAS_HOLD_AIM;
						}
						else
						{
							item->goalAnimState = STATE_SAS_KNEEL_AIM;
						}
					}
					else if (!(item->aiBits & MODIFY))
					{
						item->goalAnimState = STATE_SAS_WALK;
					}
				}
				else
				{
					if (item->aiBits & MODIFY)
					{
						item->goalAnimState = STATE_SAS_STOP;
					}
					else
					{
						if (creature->mood == ESCAPE_MOOD)
						{
							item->goalAnimState = STATE_SAS_RUN;
						}
						else
						{
							if ((creature->alerted || creature->mood != BORED_MOOD) && 
								(!item->hitStatus || !creature->reachedGoal && distance <= 0x400000))
							{
								if (creature->mood == BORED_MOOD || info.distance <= 0x400000)
								{
									item->goalAnimState = STATE_SAS_WALK;
									break;
								}
								item->goalAnimState = STATE_SAS_RUN;
							}
							else
							{
								item->goalAnimState = STATE_SAS_STOP;
							}
						}
					}
				}
			}
			else
			{
				item->goalAnimState = STATE_SAS_WALK;
				joint2 = 0;
			}
			break;

		case STATE_SAS_WAIT:
			joint2 = ang;
			creature->flags = 0;
			creature->maximumTurn = 0;

			if (item->aiBits & GUARD)
			{
				joint2 = AIGuard(creature);
				if (!GetRandomControl())
				{
					item->goalAnimState = STATE_SAS_STOP;
				}
			}
			else if (Targetable(item, &info)
				|| creature->mood != BORED_MOOD
				|| !info.ahead
				|| item->hitStatus
				/*|| Lara_Bike*/)
			{
				item->goalAnimState = STATE_SAS_STOP;
			}
			break;

		case STATE_SAS_WALK:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(5);
			joint2 = ang;

			if (item->aiBits & PATROL1)
			{
				item->goalAnimState = STATE_SAS_WALK;
			}
			else if (/*!Lara_Bike ||*/ !(item->aiBits & GUARD) && item->aiBits)
			{
				if (creature->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = STATE_SAS_RUN;
				}
				else
				{
					if (item->aiBits & GUARD || item->aiBits & FOLLOW && (creature->reachedGoal || distance > 0x400000))
					{
						item->goalAnimState = STATE_SAS_STOP;
						break;
					}
					if (Targetable(item, &info))
					{
						if (info.distance < 9437184 || info.enemyZone != info.zoneNumber)
						{
							item->goalAnimState = STATE_SAS_STOP;
							break;
						}
						item->goalAnimState = STATE_SAS_WALK_AIM;
					}
					else if (creature->mood)
					{
						if (info.distance > 0x400000)
						{
							item->goalAnimState = STATE_SAS_RUN;
						}
					}
					else if (info.ahead)
					{
						item->goalAnimState = STATE_SAS_STOP;
						break;
					}
				}
			}
			else
			{
				item->goalAnimState = STATE_SAS_STOP;
			}
			break;

		case STATE_SAS_RUN:
			if (info.ahead)
				joint2 = info.angle;
			creature->maximumTurn = ANGLE(10);
			tilt = angle / 2;
			
			/*if (Lara_Bike)
			{
				v22 = (item->MainFlags >> 9) & 0x1F;
				if (v22 == 8 || !v22)
				{
					goto LABEL_99;
				}
			}*/

			if (item->aiBits & GUARD || item->aiBits & FOLLOW && (creature->reachedGoal || distance > 0x400000))
			{
				item->goalAnimState = STATE_SAS_WALK;
				break;
			}
			if (creature->mood != ESCAPE_MOOD)
			{
				if (Targetable(item, &info))
				{
					item->goalAnimState = STATE_SAS_WALK;
				}
				else
				{
					if (creature->mood != BORED_MOOD || creature->mood == STALK_MOOD && 
						item->aiBits & FOLLOW && info.distance < 0x400000)
					{
						item->goalAnimState = STATE_SAS_WALK;
					}
				}
			}
			break;

		case STATE_SAS_SIGHT_AIM:
		case STATE_SAS_HOLD_AIM:
		case STATE_SAS_KNEEL_AIM:
			creature->flags = 0;
			if (info.ahead)
			{
				joint1 = info.xAngle;
				joint0 = info.angle; 
				if (Targetable(item, &info))
				{
					if (item->currentAnimState == STATE_SAS_SIGHT_AIM)
					{
						item->goalAnimState = STATE_SAS_SIGHT_SHOOT;
					}
					else if (item->currentAnimState == STATE_SAS_KNEEL_AIM)
					{
						item->goalAnimState = STATE_SAS_KNEEL_SHOOT;
					}
					else
					{
						if (!(GetRandomControl() & 1))
						{
							item->goalAnimState = STATE_SAS_HOLD_PREPARE_GRENADE;
							break;
						}
						item->goalAnimState = STATE_SAS_HOLD_SHOOT;
					}
				}
				else
				{
					item->goalAnimState = STATE_SAS_STOP;
				}
			}
			break;

		case STATE_SAS_WALK_AIM:
			creature->flags = 0;
			if (info.ahead)
			{
				joint1 = info.xAngle;
				joint0 = info.angle;
				if (Targetable(item, &info))
				{
					item->goalAnimState = STATE_SAS_WALK_SHOOT;
				}
				else
				{
					item->goalAnimState = STATE_SAS_WALK;
				}
			}
			break;

		case STATE_SAS_HOLD_PREPARE_GRENADE:
			if (info.ahead)
			{
				joint1 = info.xAngle;
				joint0 = info.angle;
			}
			break;

		case STATE_SAS_HOLD_SHOOT_GRENADE:
			if (info.ahead)
			{
				angle1 = info.angle;
				angle2 = info.xAngle;
				joint1 = info.xAngle;
				joint0 = info.angle;

				if (info.distance > 9437184)
				{
					joint1 = sqrt(info.distance) + info.xAngle - 1024;
				}
			}

			if (item->frameNumber == Anims[item->animNumber].frameBase + 20)
			{
				if (!creature->enemy->speed)
				{
					angle2 += (GetRandomControl() & 0x1FF) - 256;
					joint1 = angle2;
					angle1 += (GetRandomControl() & 0x1FF) - 256;
					joint0 = angle1;
				}
				//ShotGreanade((int)item, v28, v27);
				if (Targetable(item, &info))
					item->goalAnimState = STATE_SAS_HOLD_PREPARE_GRENADE;
			}
			break;

		case STATE_SAS_HOLD_SHOOT:
		case STATE_SAS_KNEEL_SHOOT:
		case STATE_SAS_SIGHT_SHOOT:
		case STATE_SAS_WALK_SHOOT:
			if (item->currentAnimState == STATE_SAS_HOLD_SHOOT || item->currentAnimState == STATE_SAS_KNEEL_SHOOT)
			{
				if (item->goalAnimState != STATE_SAS_STOP && item->goalAnimState != STATE_SAS_KNEEL_STOP && (creature->mood == ESCAPE_MOOD || !Targetable(item, &info)))
				{
					if(item->currentAnimState == STATE_SAS_HOLD_SHOOT)
						item->goalAnimState = STATE_SAS_STOP;
					else
						item->goalAnimState = STATE_SAS_KNEEL_STOP;
				}
			}
			
			if (info.ahead)
			{
				joint0 = info.angle;
				joint1 = info.xAngle;
			}

			if (creature->flags)
				creature->flags -= 1;
			else
			{
				ShotLara(item, &info, &sasGun, joint0, 15); 
				creature->flags = 5;
				item->firedWeapon = 3;
			}
			break;

		case STATE_SAS_BLIND:
			if (!WeaponEnemyTimer && !(GetRandomControl() & 0x7F))
				item->goalAnimState = STATE_SAS_WAIT;
			break;

		default:
			break;
		}

		if (WeaponEnemyTimer > 100 && item->currentAnimState != STATE_SAS_BLIND)
		{
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + ANIMATION_SAS_BLIND;
			item->frameNumber = Anims[item->animNumber].frameBase + (GetRandomControl() & 7);
			item->currentAnimState = STATE_SAS_BLIND;
		}

	}
	else if (item->currentAnimState != STATE_SAS_DEATH)
	{
		item->animNumber = Objects[item->objectNumber].animIndex + ANIMATION_SAS_DEATH;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->currentAnimState = STATE_SAS_DEATH;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);

	CreatureAnimation(itemNumber, angle, 0);
}