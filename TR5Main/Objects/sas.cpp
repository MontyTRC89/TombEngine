#include "newobjects.h"
#include "../Game/Box.h"
#include "../Game/items.h"
#include "../Game/lot.h"
#include "../Game/control.h"
#include "../Game/effects.h"
#include "../Game/draw.h"
#include "../Game/sphere.h"
#include "../Game/effect2.h"
#include "../Game/people.h"
#include "../Game/lara.h"

BITE_INFO sasGun = { 0, 300, 64, 7 };

void __cdecl InitialiseSas(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[item->objectNumber].animIndex + 12;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
}

void __cdecl SasControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
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
		TriggerDynamics(pos.x, pos.y, pos.z, 2 * item->firedWeapon + 8, 24, 16, 4);
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
			ang = ATAN(dz, dx) - item->pos.yRot;
			distance = dx * dx + dz * dz;
		}

		GetCreatureMood(item, &info, creature->enemy != LaraItem);

		// Vehicle handling
		if (g_LaraExtra.Vehicle != NO_ITEM && info.bite)
			creature->mood == ESCAPE_MOOD;

		CreatureMood(item, &info, creature->enemy != LaraItem);
		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->hitStatus)
			AlertAllGuards(itemNum);

		int angle1 = 0;
		int angle2 = 0;

		switch (item->currentAnimState)
		{
		case 1:
			creature->flags = 0;
			creature->maximumTurn = 0;
			joint2 = ang;
			if (item->animNumber == Objects[item->objectNumber].animIndex + 17)
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
					if (item->currentAnimState == 1)
					{
						item->goalAnimState = 4;
						break;
					}
					item->goalAnimState = 1;
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
							item->goalAnimState = 8;
						}
						else if (GetRandomControl() & 1)
						{
							item->goalAnimState = 10;
						}
						else
						{
							item->goalAnimState = 12;
						}
					}
					else if (!(item->aiBits & MODIFY))
					{
						item->goalAnimState = 2;
					}
				}
				else
				{
					if (item->aiBits & MODIFY)
					{
						item->goalAnimState = 1;
					}
					else
					{
						if (creature->mood == ESCAPE_MOOD)
						{
							item->goalAnimState = 3;
						}
						else
						{
							if ((creature->alerted || creature->mood != BORED_MOOD) && 
								(!item->hitStatus || !creature->reachedGoal && distance <= 0x400000))
							{
								if (creature->mood == BORED_MOOD || info.distance <= 0x400000)
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
					}
				}
			}
			else
			{
				item->goalAnimState = 2;
				joint2 = 0;
			}
			break;

		case 4:
			joint2 = ang;
			creature->flags = 0;
			creature->maximumTurn = 0;

			if (item->aiBits & GUARD)
			{
				joint2 = AIGuard(creature);
				if (!GetRandomControl())
				{
					item->goalAnimState = 1;
				}
			}
			else if (Targetable(item, &info)
				|| creature->mood != BORED_MOOD
				|| !info.ahead
				|| item->hitStatus
				/*|| Lara_Bike*/)
			{
				item->goalAnimState = 1;
			}
			break;

		case 2:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(5);
			joint2 = ang;

			if (item->aiBits & PATROL1)
			{
				item->goalAnimState = 2;
			}
			else if (/*!Lara_Bike ||*/ !(item->aiBits & GUARD) && item->aiBits)
			{
				if (creature->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = 3;
				}
				else
				{
					if (item->aiBits & GUARD || item->aiBits & FOLLOW && (creature->reachedGoal || distance > 0x400000))
					{
						item->goalAnimState = 1;
						break;
					}
					if (Targetable(item, &info))
					{
						if (info.distance < 9437184 || info.enemyZone != info.zoneNumber)
						{
							item->goalAnimState = 1;
							break;
						}
						item->goalAnimState = 9;
					}
					else if (creature->mood)
					{
						if (info.distance > 0x400000)
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
			}
			else
			{
				item->goalAnimState = 1;
			}
			break;

		case 3:
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
				item->goalAnimState = 2;
				break;
			}
			if (creature->mood != ESCAPE_MOOD)
			{
				if (Targetable(item, &info))
				{
					item->goalAnimState = 2;
				}
				else
				{
					if (creature->mood != BORED_MOOD || creature->mood == STALK_MOOD && 
						item->aiBits & FOLLOW && info.distance < 0x400000)
					{
						item->goalAnimState = 2;
					}
				}
			}
			break;

		case 8:
		case 10:
		case 12:
			creature->flags = 0;
			if (info.ahead)
			{
				joint1 = info.xAngle;
				joint0 = info.angle; 
				if (Targetable(item, &info))
				{
					if (item->currentAnimState == 8)
					{
						item->goalAnimState = 5;
					}
					else if (item->currentAnimState == 12)
					{
						item->goalAnimState = 13;
					}
					else
					{
						if (!(GetRandomControl() & 1))
						{
							item->goalAnimState = 15;
							break;
						}
						item->goalAnimState = 11;
					}
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

		case 15:
			if (info.ahead)
			{
				joint1 = info.xAngle;
				joint0 = info.angle;
			}
			break;

		case 16:
			if (info.ahead)
			{
				angle1 = info.angle;
				angle2 = info.xAngle;
				joint1 = info.xAngle;
				joint0 = info.angle;

				if (info.distance > 9437184)
				{
					joint1 = SQRT_ASM(info.distance) + info.xAngle - 1024;
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
					item->goalAnimState = 15;
			}
			break;

		case 11:
		case 13:
		case 5:
		case 6:
			if (item->currentAnimState == 11 || item->currentAnimState == 13)
			{
				if (item->goalAnimState != 1 && item->goalAnimState != 14 && (creature->mood == ESCAPE_MOOD || !Targetable(item, &info)))
				{
					item->goalAnimState = item->currentAnimState != 11 ? 14 : 1;
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

		case 17:
			if (!WeaponEnemyTimer && !(GetRandomControl() & 0x7F))
				item->goalAnimState = 4;
			break;

		default:
			break;
		}

		if ((unsigned __int8)WeaponEnemyTimer > 0x64u && item->currentAnimState != 17)
		{
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 28;
			item->frameNumber = Anims[item->animNumber].frameBase + (GetRandomControl() & 7);
			item->currentAnimState = 17;
		}
	}
	else if (item->currentAnimState != 7)
	{
		item->animNumber = Objects[item->objectNumber].animIndex + 19;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->currentAnimState = 7;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);

	CreatureAnimation(itemNum, angle, 0);
}