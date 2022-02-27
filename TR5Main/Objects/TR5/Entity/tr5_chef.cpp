#include "framework.h"
#include "tr5_chef.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/people.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

#define STATE_CHEF_COOKING			1
#define STATE_CHEF_TURN_180			2
#define STATE_CHEF_ATTACK			3
#define STATE_CHEF_AIM				4
#define STATE_CHEF_WALK				5
#define STATE_CHEF_DEATH			8

#define ANIMATION_CHEF_DEATH		16

BITE_INFO ChefBite = { 0, 200, 0 ,13 };

void InitialiseChef(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	
	ClearItem(itemNumber);

	item->animNumber = Objects[item->objectNumber].animIndex;
	item->goalAnimState = STATE_CHEF_COOKING;
	item->currentAnimState = STATE_CHEF_COOKING;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->pos.xPos += 192 * phd_sin(item->pos.yRot);
	item->pos.zPos += 192 * phd_cos(item->pos.yRot);
}

void ControlChef(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short angle = 0;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != STATE_CHEF_DEATH)
		{
			item->hitPoints = 0;
			item->currentAnimState = STATE_CHEF_DEATH;
			item->animNumber = Objects[item->objectNumber].animIndex + ANIMATION_CHEF_DEATH;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
	else
	{
		if (item->aiBits)
		{
			GetAITarget(creature);
		}
		else if (creature->hurtByLara)
		{
			creature->enemy = LaraItem;
		}

		AI_INFO info;
		AI_INFO laraInfo;
		CreatureAIInfo(item,&info);

		if (creature->enemy == LaraItem)
		{
			laraInfo.angle = info.angle;
			laraInfo.distance = info.distance;
		}
		else
		{
			int dx = LaraItem->pos.xPos - item->pos.xPos;
			int dz = LaraItem->pos.zPos - item->pos.zPos;

			laraInfo.angle = phd_atan(dz, dx) - item->pos.yRot;
			laraInfo.ahead = true;
			if (laraInfo.angle <= -ANGLE(90) || laraInfo.angle >= ANGLE(90))
				laraInfo.ahead = false;
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
		}

		GetCreatureMood(item,&info, VIOLENT);
		CreatureMood(item,&info, VIOLENT);

		short angle = CreatureTurn(item, creature->maximumTurn);

		if (info.ahead)
		{
			joint0 = info.angle / 2;
			//joint1 = info.xAngle;
			joint2 = info.angle / 2;
		}

		creature->maximumTurn = 0;

		switch (item->currentAnimState)
		{
		case STATE_CHEF_COOKING:
			if (abs(LaraItem->pos.yPos - item->pos.yPos) < 1024
				&& info.distance < SQUARE(1536)
				&& (item->touchBits
					|| LaraItem->speed > 15
					|| item->hitStatus
					|| TargetVisible(item,&laraInfo)))
			{
				item->goalAnimState = STATE_CHEF_TURN_180;
				creature->alerted = true;
				item->aiBits = 0;
			}
			break;

		case STATE_CHEF_TURN_180:
			creature->maximumTurn = 0;
			if (info.angle > 0)
				item->pos.yRot -= ANGLE(2);
			else
				item->pos.yRot += ANGLE(2);
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
				item->pos.yRot += -ANGLE(180);
			break;

		case STATE_CHEF_ATTACK:
			creature->maximumTurn = 0;
			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle > 0)
					item->pos.yRot += ANGLE(2);
				else
					item->pos.yRot -= ANGLE(2);
			}
			else
			{
				item->pos.yRot += info.angle;
			}

			if (!creature->flags)
			{
				if (item->touchBits & 0x2000)
				{
					if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 10)
					{
						LaraItem->hitPoints -= 80;
						LaraItem->hitStatus = true;
						CreatureEffect2(item,&ChefBite, 20, item->pos.yRot, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD,&item->pos, 0);
						creature->flags = 1;
					}
				}
			}
			break;

		case STATE_CHEF_AIM:
			creature->flags = 0;
			creature->maximumTurn = 364;
			if (info.distance >= SQUARE(682))
			{
				if (info.angle > 20480 || info.angle < -20480)
				{
					item->goalAnimState = STATE_CHEF_TURN_180;
				}
				else if (creature->mood == ATTACK_MOOD)
				{
					item->goalAnimState = STATE_CHEF_WALK;
				}
			}
			else if (info.bite)
			{
				item->goalAnimState = STATE_CHEF_ATTACK;
			}
			break;

		case STATE_CHEF_WALK:
			creature->maximumTurn = ANGLE(7);
			if (info.distance < SQUARE(682)
				|| info.angle > 20480
				|| info.angle < -20480
				|| creature->mood != ATTACK_MOOD)
				item->goalAnimState = STATE_CHEF_AIM;
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