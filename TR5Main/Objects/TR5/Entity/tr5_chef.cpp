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

	item->AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->TargetState = STATE_CHEF_COOKING;
	item->ActiveState = STATE_CHEF_COOKING;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->Position.xPos += 192 * phd_sin(item->Position.yRot);
	item->Position.zPos += 192 * phd_cos(item->Position.yRot);
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
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != STATE_CHEF_DEATH)
		{
			item->HitPoints = 0;
			item->ActiveState = STATE_CHEF_DEATH;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + ANIMATION_CHEF_DEATH;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}
	}
	else
	{
		if (item->AIBits)
		{
			GetAITarget(creature);
		}
		else if (creature->hurtByLara)
		{
			creature->enemy = LaraItem;
		}

		AI_INFO info;
		AI_INFO laraInfo;
		CreatureAIInfo(item, &info);

		if (creature->enemy == LaraItem)
		{
			laraInfo.angle = info.angle;
			laraInfo.distance = info.distance;
		}
		else
		{
			int dx = LaraItem->Position.xPos - item->Position.xPos;
			int dz = LaraItem->Position.zPos - item->Position.zPos;

			laraInfo.angle = phd_atan(dz, dx) - item->Position.yRot;
			laraInfo.ahead = true;
			if (laraInfo.angle <= -ANGLE(90) || laraInfo.angle >= ANGLE(90))
				laraInfo.ahead = false;
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		short angle = CreatureTurn(item, creature->maximumTurn);

		if (info.ahead)
		{
			joint0 = info.angle / 2;
			//joint1 = info.xAngle;
			joint2 = info.angle / 2;
		}

		creature->maximumTurn = 0;

		switch (item->ActiveState)
		{
		case STATE_CHEF_COOKING:
			if (abs(LaraItem->Position.yPos - item->Position.yPos) < 1024
				&& info.distance < SQUARE(1536)
				&& (item->TouchBits
					|| LaraItem->Velocity> 15
					|| item->HitStatus
					|| TargetVisible(item, &laraInfo)))
			{
				item->TargetState = STATE_CHEF_TURN_180;
				creature->alerted = true;
				item->AIBits = 0;
			}
			break;

		case STATE_CHEF_TURN_180:
			creature->maximumTurn = 0;
			if (info.angle > 0)
				item->Position.yRot -= ANGLE(2);
			else
				item->Position.yRot += ANGLE(2);
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
				item->Position.yRot += -ANGLE(180);
			break;

		case STATE_CHEF_ATTACK:
			creature->maximumTurn = 0;
			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle > 0)
					item->Position.yRot += ANGLE(2);
				else
					item->Position.yRot -= ANGLE(2);
			}
			else
			{
				item->Position.yRot += info.angle;
			}

			if (!creature->flags)
			{
				if (item->TouchBits & 0x2000)
				{
					if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 10)
					{
						LaraItem->HitPoints -= 80;
						LaraItem->HitStatus = true;
						CreatureEffect2(item, &ChefBite, 20, item->Position.yRot, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Position, 0);
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
					item->TargetState = STATE_CHEF_TURN_180;
				}
				else if (creature->mood == ATTACK_MOOD)
				{
					item->TargetState = STATE_CHEF_WALK;
				}
			}
			else if (info.bite)
			{
				item->TargetState = STATE_CHEF_ATTACK;
			}
			break;

		case STATE_CHEF_WALK:
			creature->maximumTurn = ANGLE(7);
			if (info.distance < SQUARE(682)
				|| info.angle > 20480
				|| info.angle < -20480
				|| creature->mood != ATTACK_MOOD)
				item->TargetState = STATE_CHEF_AIM;
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