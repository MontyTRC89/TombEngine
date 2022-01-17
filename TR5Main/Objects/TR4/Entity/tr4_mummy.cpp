#include "framework.h"
#include "tr4_mummy.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Game/control/control.h"
#include "Specific/level.h"
#include "Game/itemdata/creature_info.h"

enum MUMMY_STATES {
	STATE_MUMMY_ARMS_CROSSED = 0,
	STATE_MUMMY_STOP = 1,
	STATE_MUMMY_WALK = 2,
	STATE_MUMMY_WALK_ARMS_UP = 3,
	STATE_MUMMY_WALK_HIT = 4,
	STATE_MUMMY_PUSHED_BACK = 5,
	STATE_MUMMY_ARMS_UP_PUSHED_BACK = 6,
	STATE_MUMMY_COLLAPSE = 7,
	STATE_MUMMY_LYING_DOWN = 8,
	STATE_MUMMY_GET_UP = 9,
	STATE_MUMMY_HIT = 10
};

enum MUMMY_ANIM {
	ANIMATION_MUMMY_STAND = 0,
	ANIMATION_MUMMY_WALK = 1,
	ANIMATION_MUMMY_WALK_ARMS_UP = 2,
	ANIMATION_MUMMY_PUSHED_BACK = 3,
	ANIMATION_MUMMY_WALK_TO_WALK_ARMS_UP_RIGHT = 4,
	ANIMATION_MUMMY_WALK_ARMS_UP_TO_WALK_LEFT = 5,
	ANIMATION_MUMMY_WALK_ARMS_UP_TO_STAND = 6,
	ANIMATION_MUMMY_STAND_TO_WALK_ARMS_UP = 7,
	ANIMATION_MUMMY_STAND_TO_WALK = 8,
	ANIMATION_MUMMY_WALK_TO_STAND = 9,
	ANIMATION_MUMMY_COLLAPSE_START = 10,
	ANIMATION_MUMMY_COLLAPSE_END = 11,
	ANIMATION_MUMMY_LYING_DOWN = 12,
	ANIMATION_MUMMY_GET_UP = 13,
	ANIMATION_MUMMY_HIT_RIGHT = 14,
	ANIMATION_MUMMY_HIT_LEFT = 15,
	ANIMATION_MUMMY_WALK_HIT = 16,
	ANIMATION_MUMMY_ARMS_CROSSED_TO_STAND_START = 17,
	ANIMATION_MUMMY_ARMS_CROSSED_TO_STAND_END = 18,
	ANIMATION_MUMMY_ARMS_CROSSED = 19,
	ANIMATION_MUMMY_ARMS_UP_PUSHED_BACK = 20
};

BITE_INFO mummyBite1 = { 0, 0, 0, 11 };
BITE_INFO mummyBite2 = { 0, 0, 0, 14 };

void InitialiseMummy(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	if (item->triggerFlags == 2)
	{
		item->animNumber = Objects[item->objectNumber].animIndex + ANIMATION_MUMMY_LYING_DOWN;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = STATE_MUMMY_LYING_DOWN;
		item->currentAnimState = STATE_MUMMY_LYING_DOWN;
		item->status = ITEM_INVISIBLE;
	}
	else
	{
		item->animNumber = Objects[item->objectNumber].animIndex + ANIMATION_MUMMY_ARMS_CROSSED;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = STATE_MUMMY_ARMS_CROSSED;
		item->currentAnimState = STATE_MUMMY_ARMS_CROSSED;
	}
}

void MummyControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	short tilt = 0;
	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;

	if (item->aiBits)
		GetAITarget(creature);
	else if (creature->hurtByLara)
		creature->enemy = LaraItem;

	AI_INFO info;
	CreatureAIInfo(item, &info);

	if (item->hitStatus)
	{
		if (info.distance < SQUARE(3072))
		{
			if (item->currentAnimState != ANIMATION_MUMMY_STAND_TO_WALK_ARMS_UP 
				&& item->currentAnimState != ANIMATION_MUMMY_WALK_ARMS_UP_TO_WALK_LEFT
				&& item->currentAnimState != ANIMATION_MUMMY_STAND_TO_WALK)
			{
				if (GetRandomControl() & 3 
					|| Lara.gunType != WEAPON_SHOTGUN 
					&& Lara.gunType != WEAPON_HK 
					&& Lara.gunType != WEAPON_REVOLVER)
				{
					if (!(GetRandomControl() & 7) 
						|| Lara.gunType == WEAPON_SHOTGUN 
						|| Lara.gunType == WEAPON_HK
						|| Lara.gunType == WEAPON_REVOLVER)
					{
						if (item->currentAnimState == STATE_MUMMY_WALK_ARMS_UP
							|| item->currentAnimState == STATE_MUMMY_WALK_HIT)
						{
							item->currentAnimState = STATE_MUMMY_ARMS_UP_PUSHED_BACK;
							item->animNumber = Objects[item->objectNumber].animIndex + ANIMATION_MUMMY_ARMS_UP_PUSHED_BACK;
						}
						else
						{
							item->currentAnimState = STATE_MUMMY_PUSHED_BACK;
							item->animNumber = Objects[item->objectNumber].animIndex + ANIMATION_MUMMY_PUSHED_BACK;
						}
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->pos.yRot += info.angle;
					}
				}
				else
				{
					item->animNumber = Objects[item->objectNumber].animIndex + ANIMATION_MUMMY_COLLAPSE_START;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->currentAnimState = STATE_MUMMY_COLLAPSE;
					item->pos.yRot += info.angle;
					creature->maximumTurn = 0;
				}
			}
		}
	}
	else
	{
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (info.ahead)
		{
			joint0 = info.angle / 2;
			joint1 = info.angle / 2;
			joint2 = info.xAngle;
		}

		switch (item->currentAnimState)
		{
		case STATE_MUMMY_STOP:
			creature->flags = 0;
			creature->maximumTurn = 0;

			if (info.distance <= SQUARE(512) 
				|| info.distance >= SQUARE(7168))
			{
				if (info.distance - SQUARE(512) <= 0)
					item->goalAnimState = STATE_MUMMY_HIT;
				else
				{
					item->goalAnimState = STATE_MUMMY_STOP;
					joint0 = 0;
					joint1 = 0;
					joint2 = 0;
					if (item->triggerFlags > -100 && item->triggerFlags & 0x8000 < 0)
						item->triggerFlags++;
				}
			}
			else
				item->goalAnimState = STATE_MUMMY_WALK;
			break;

		case STATE_MUMMY_WALK:
			if (item->triggerFlags == 1)
			{
				creature->maximumTurn = 0;
				if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
					item->triggerFlags = 0;
			}
			else
			{
				creature->maximumTurn = ANGLE(7);
				if (info.distance >= SQUARE(3072))
				{
					if (info.distance > SQUARE(7168))
					{
						item->goalAnimState = STATE_MUMMY_STOP;
					}
				}
				else
				{
					item->goalAnimState = STATE_MUMMY_WALK_ARMS_UP;
				}
			}
			break;

		case STATE_MUMMY_WALK_ARMS_UP:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(7);
			if (info.distance < SQUARE(512))
			{
				item->goalAnimState = STATE_MUMMY_STOP;
				break;
			}
			if (info.distance > SQUARE(3072) && info.distance < SQUARE(7168))
			{
				item->goalAnimState = STATE_MUMMY_WALK;
				break;
			}
			if (info.distance <= SQUARE(682))
				item->goalAnimState = STATE_MUMMY_WALK_HIT;
			else if (info.distance > SQUARE(7168))
				item->goalAnimState = STATE_MUMMY_STOP;
			break;

		case STATE_MUMMY_ARMS_CROSSED:
			creature->maximumTurn = 0;
			if (info.distance < SQUARE(1024) || item->triggerFlags > -1)
				item->goalAnimState = STATE_MUMMY_WALK;
			break;

		case STATE_MUMMY_LYING_DOWN:
			joint0 = 0;
			joint1 = 0;
			joint2 = 0;
			creature->maximumTurn = 0;
			item->hitPoints = 0;
			if (info.distance < SQUARE(1024) || !(GetRandomControl() & 0x7F))
			{
				item->goalAnimState = STATE_MUMMY_GET_UP;
				item->hitPoints = Objects[item->objectNumber].hitPoints;
			}
			break;

		case STATE_MUMMY_WALK_HIT:
		case STATE_MUMMY_HIT:
			creature->maximumTurn = 0;
			if (abs(info.angle) >= ANGLE(7))
			{
				if (info.angle >= 0)
				{
					item->pos.yRot += ANGLE(7);
				}
				else
				{
					item->pos.yRot -= ANGLE(7);
				}
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (!creature->flags)
			{

				if (item->touchBits & 0x4800)
				{
					if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase && item->frameNumber < g_Level.Anims[item->animNumber].frameEnd)
					{
						LaraItem->hitPoints -= 100;
						LaraItem->hitStatus = true;

						if (item->animNumber == Objects[item->objectNumber].animIndex + ANIMATION_MUMMY_HIT_LEFT)
						{
							CreatureEffect2(
								item,
								&mummyBite1,
								5,
								-1,
								DoBloodSplat);
						}
						else
						{
							CreatureEffect2(
								item,
								&mummyBite2,
								5,
								-1,
								DoBloodSplat);
						}
						creature->flags = 1;
					}
				}
			}
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