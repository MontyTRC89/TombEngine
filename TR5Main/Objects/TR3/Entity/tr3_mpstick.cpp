#include "framework.h"
#include "Objects/TR3/Entity/tr3_mpstick.h"

#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO mpstickBite1 = { 247, 10, 11, 13 };
BITE_INFO mpstickBite2 = { 0, 0, 100, 6 };

enum MPSTICK_STATES {
	BATON_EMPTY,
	BATON_STOP,
	BATON_WALK, 
	BATON_PUNCH2,
	BATON_AIM2, 
	BATON_WAIT, 
	BATON_AIM1,
	BATON_AIM0,
	BATON_PUNCH1,
	BATON_PUNCH0,
	BATON_RUN, 
	BATON_DEATH, 
	BATON_KICK, 
	BATON_CLIMB3, 
	BATON_CLIMB1,
	BATON_CLIMB2, 
	BATON_FALL3
};

void InitialiseMPStick(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	ClearItem(itemNumber);

	item->animNumber = Objects[ID_MP_WITH_STICK].animIndex + 6;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->currentAnimState = item->goalAnimState = BATON_STOP;
}

void MPStickControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	short torsoY = 0;
	short torsoX = 0;
	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->boxNumber != NO_BOX && (g_Level.Boxes[item->boxNumber].flags & BLOCKED))
	{
		DoLotsOfBlood(item->pos.xPos, item->pos.yPos - (GetRandomControl() & 255) - 32, item->pos.zPos, (GetRandomControl() & 127) + 128, GetRandomControl() * 2, item->roomNumber, 3);
		item->hitPoints -= 20;
	}

	int dx;
	int dz;
	int x;
	int y;
	int z;
	AI_INFO info;
	AI_INFO laraInfo;
	ITEM_INFO* target;
	ITEM_INFO* enemy;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != BATON_DEATH)
		{
			item->animNumber = Objects[ID_MP_WITH_STICK].animIndex + 26;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = BATON_DEATH;
			creature->LOT.step = 256;
		}
	}
	else
	{
		if (item->aiBits)
			GetAITarget(creature);
		else
		{
			creature->enemy = LaraItem;

			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;

			laraInfo.distance = SQUARE(dx) + SQUARE(dx);

			int bestDistance = 0x7fffffff;
			for (int slot = 0; slot < ActiveCreatures.size(); slot++)
			{
				CREATURE_INFO* currentCreature = ActiveCreatures[slot];
				if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNumber)
					continue;

				target = &g_Level.Items[currentCreature->itemNum];
				if (target->objectNumber != ID_LARA)
					continue;

				dx = target->pos.xPos - item->pos.xPos;
				dz = target->pos.zPos - item->pos.zPos;

				if (dz > 32000 || dz < -32000 || dx > 32000 || dx < -32000)
					continue;

				int distance = SQUARE(dx) + SQUARE(dz);
				if (distance < bestDistance && distance < laraInfo.distance)
				{
					bestDistance = distance;
					creature->enemy = target;
				}
			}
		}

		CreatureAIInfo(item, &info);

		if (creature->enemy == LaraItem)
		{
			laraInfo.angle = info.angle;
			laraInfo.distance = info.distance;
		}
		else
		{
			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;
			laraInfo.angle = phd_atan(dz, dx) - item->pos.yRot;
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		enemy = creature->enemy;
		creature->enemy = LaraItem;
		if (item->hitStatus || ((laraInfo.distance < SQUARE(1024) || TargetVisible(item, &laraInfo)) && (abs(LaraItem->pos.yPos - item->pos.yPos) < 1024)))
		{
			if (!creature->alerted)
				SoundEffect(SFX_TR3_AMERCAN_HOY, &item->pos, 0);
			AlertAllGuards(itemNumber);
		}
		creature->enemy = enemy;

		switch (item->currentAnimState)
		{
		case BATON_WAIT:
			if (creature->alerted || item->goalAnimState == BATON_RUN)
			{
				item->goalAnimState = BATON_STOP;
				break;
			}

		case BATON_STOP:
			creature->flags = 0;
			creature->maximumTurn = 0;
			head = laraInfo.angle;

			if (item->aiBits & GUARD)
			{
				head = AIGuard(creature);
				if (!(GetRandomControl() & 0xFF))
				{
					if (item->currentAnimState == BATON_STOP)
						item->goalAnimState = BATON_WAIT;
					else
						item->goalAnimState = BATON_STOP;
				}
				break;
			}

			else if (item->aiBits & PATROL1)
				item->goalAnimState = BATON_WALK;

			else if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead && !item->hitStatus)
					item->goalAnimState = BATON_STOP;
				else
					item->goalAnimState = BATON_RUN;
			}
			else if (creature->mood == BORED_MOOD || ((item->aiBits & FOLLOW) && (creature->reachedGoal || laraInfo.distance > SQUARE(2048))))
			{
				if (item->requiredAnimState)
					item->goalAnimState = item->requiredAnimState;
				else if (info.ahead)
					item->goalAnimState = BATON_STOP;
				else
					item->goalAnimState = BATON_RUN;
			}
			else if (info.bite && info.distance < SQUARE(512))
				item->goalAnimState = BATON_AIM0;
			else if (info.bite && info.distance < SQUARE(1024))
				item->goalAnimState = BATON_AIM1;
			else if (info.bite && info.distance < SQUARE(1024))
				item->goalAnimState = BATON_WALK;
			else
				item->goalAnimState = BATON_RUN;
			break;
		case BATON_WALK:
			head = laraInfo.angle;
			creature->flags = 0;

			creature->maximumTurn = ANGLE(6);

			if (item->aiBits & PATROL1)
			{
				item->goalAnimState = BATON_WALK;
				head = 0;
			}
			else if (creature->mood == ESCAPE_MOOD)
				item->goalAnimState = BATON_RUN;
			else if (creature->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100)
				{
					item->requiredAnimState = BATON_WAIT;
					item->goalAnimState = BATON_STOP;
				}
			}
			else if (info.bite && info.distance < SQUARE(1536) && info.xAngle < 0)
				item->goalAnimState = BATON_KICK;
			else if (info.bite && info.distance < SQUARE(512))
				item->goalAnimState = BATON_STOP;
			else if (info.bite && info.distance < SQUARE(1280))
				item->goalAnimState = BATON_AIM2;
			else
				item->goalAnimState = BATON_RUN;
			break;

		case BATON_RUN:
			if (info.ahead)
				head = info.angle;

			creature->maximumTurn = ANGLE(7);
			tilt = angle / 2;

			if (item->aiBits & GUARD)
				item->goalAnimState = BATON_WAIT;
			else if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->goalAnimState = BATON_STOP;
				break;
			}
			else if ((item->aiBits & FOLLOW) && (creature->reachedGoal || laraInfo.distance > SQUARE(2048)))
				item->goalAnimState = BATON_STOP;
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = BATON_WALK;
			else if (info.ahead && info.distance < SQUARE(1024))
				item->goalAnimState = BATON_WALK;
			break;

		case BATON_AIM0:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->maximumTurn = ANGLE(6);

			creature->flags = 0;
			if (info.bite && info.distance < SQUARE(512))
				item->goalAnimState = BATON_PUNCH0;
			else
				item->goalAnimState = BATON_STOP;
			break;

		case BATON_AIM1:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->maximumTurn = ANGLE(6);

			creature->flags = 0;
			if (info.ahead && info.distance < SQUARE(1024))
				item->goalAnimState = BATON_PUNCH1;
			else
				item->goalAnimState = BATON_STOP;
			break;

		case BATON_AIM2:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->maximumTurn = ANGLE(6);

			creature->flags = 0;
			if (info.bite && info.distance < SQUARE(1280))
				item->goalAnimState = BATON_PUNCH2;
			else
				item->goalAnimState = BATON_WALK;
			break;

		case BATON_PUNCH0:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->maximumTurn = ANGLE(6);

			if (enemy == LaraItem)
			{
				if (!creature->flags && (item->touchBits & 0x2400))
				{
					LaraItem->hitPoints -= 80;
					LaraItem->hitStatus = 1;
					CreatureEffect(item, &mpstickBite1, DoBloodSplat);
					SoundEffect(SFX_TR4_LARA_THUD, &item->pos, 0);

					creature->flags = 1;
				}
			}
			else
			{
				if (!creature->flags && enemy)
				{
					if (abs(enemy->pos.xPos - item->pos.xPos) < 256 &&
						abs(enemy->pos.yPos - item->pos.yPos) <= 256 &&
						abs(enemy->pos.zPos - item->pos.zPos) < 256)
					{
						enemy->hitPoints -= 5;
						enemy->hitStatus = 1;
						creature->flags = 1;
						CreatureEffect(item, &mpstickBite1, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->pos, 0);
					}
				}
			}

			break;

		case BATON_PUNCH1:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->maximumTurn = ANGLE(6);

			if (enemy == LaraItem)
			{
				if (!creature->flags && (item->touchBits & 0x2400))
				{
					LaraItem->hitPoints -= 80;
					LaraItem->hitStatus = 1;
					CreatureEffect(item, &mpstickBite1, DoBloodSplat);
					SoundEffect(SFX_TR4_LARA_THUD, &item->pos, 0);

					creature->flags = 1;
				}
			}
			else
			{
				if (!creature->flags && enemy)
				{
					if (abs(enemy->pos.xPos - item->pos.xPos) < 256 &&
						abs(enemy->pos.yPos - item->pos.yPos) <= 256 &&
						abs(enemy->pos.zPos - item->pos.zPos) < 256)
					{
						enemy->hitPoints -= 5;
						enemy->hitStatus = 1;
						creature->flags = 1;
						CreatureEffect(item, &mpstickBite1, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->pos, 0);

					}
				}
			}


			if (info.ahead && info.distance > SQUARE(1024) && info.distance < SQUARE(1280))
				item->goalAnimState = BATON_PUNCH2;
			break;

		case BATON_PUNCH2:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->maximumTurn = ANGLE(6);

			if (enemy == LaraItem)
			{
				if (creature->flags != 2 && (item->touchBits & 0x2400))
				{
					LaraItem->hitPoints -= 100;
					LaraItem->hitStatus = 1;
					CreatureEffect(item, &mpstickBite1, DoBloodSplat);
					creature->flags = 2;
					SoundEffect(70, &item->pos, 0);

				}
			}
			else
			{
				if (creature->flags != 2 && enemy)
				{
					if (abs(enemy->pos.xPos - item->pos.xPos) < 256 &&
						abs(enemy->pos.yPos - item->pos.yPos) <= 256 &&
						abs(enemy->pos.zPos - item->pos.zPos) < 256)
					{
						enemy->hitPoints -= 6;
						enemy->hitStatus = 1;
						creature->flags = 2;
						CreatureEffect(item, &mpstickBite1, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->pos, 0);
					}
				}
			}
			break;
		case BATON_KICK:
			if (info.ahead)
			{
				torsoY = info.angle;
			}
			creature->maximumTurn = ANGLE(6);

			if (enemy == LaraItem)
			{
				if (creature->flags != 1 && (item->touchBits & 0x60) && (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 8))
				{
					LaraItem->hitPoints -= 150;
					LaraItem->hitStatus = 1;
					CreatureEffect(item, &mpstickBite2, DoBloodSplat);
					SoundEffect(SFX_TR4_LARA_THUD, &item->pos, 0);

					creature->flags = 1;
				}
			}
			else
			{
				if (!creature->flags != 1 && enemy && (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 8))
				{
					if (abs(enemy->pos.xPos - item->pos.xPos) < 256 &&
						abs(enemy->pos.yPos - item->pos.yPos) <= 256 &&
						abs(enemy->pos.zPos - item->pos.zPos) < 256)
					{
						enemy->hitPoints -= 9;
						enemy->hitStatus = 1;
						creature->flags = 1;
						CreatureEffect(item, &mpstickBite2, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->pos, 0);
					}
				}
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torsoY);
	CreatureJoint(item, 1, torsoX);
	CreatureJoint(item, 2, head);

	if (item->currentAnimState < BATON_DEATH)
	{
		switch (CreatureVault(itemNumber, angle, 2, 260))
		{
		case 2:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MP_WITH_STICK].animIndex + 28;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = BATON_CLIMB1;
			break;

		case 3:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MP_WITH_STICK].animIndex + 29;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = BATON_CLIMB2;
			break;

		case 4:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MP_WITH_STICK].animIndex + 27;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = BATON_CLIMB3;
			break;
		case -4:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MP_WITH_STICK].animIndex + 30;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = BATON_FALL3;
			break;
		}
	}
	else
	{
		creature->maximumTurn = 0;
		CreatureAnimation(itemNumber, angle, tilt);
	}
}