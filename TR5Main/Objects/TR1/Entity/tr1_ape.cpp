#include "framework.h"
#include "Objects/TR1/Entity/tr1_ape.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO apeBite = { 0, -19, 75, 15 };

#define ATTACK_DAMAGE 200

#define TOUCH (0xFF00)

#define DIE_ANIM 7
#define VAULT_ANIM 19

#define RUN_TURN ANGLE(5)

#define DISPLAY_ANGLE ANGLE(45)

#define ATTACK_RANGE SQUARE(430)
#define PANIC_RANGE SQUARE(WALL_SIZE*2)

#define JUMP_CHANCE 0xa0
#define WARN1_CHANCE (JUMP_CHANCE + 0xA0)
#define WARN2_CHANCE (WARN1_CHANCE + 0xA0)
#define RUNLEFT_CHANCE (WARN2_CHANCE + 0x110)

#define ATTACK_FLAG 1
#define TURNL_FLAG 2
#define TURNR_FLAG 4

#define SHIFT 75

enum ape_anims {
	APE_EMPTY, 
	APE_STOP, 
	APE_WALK, 
	APE_RUN, 
	APE_ATTACK1, 
	APE_DEATH,
	APE_WARNING, 
	APE_WARNING2, 
	APE_RUNLEFT, 
	APE_RUNRIGHT, 
	APE_JUMP, 
	APE_VAULT
};

void ApeVault(short item_number, short angle)
{
	ITEM_INFO *item;
	CREATURE_INFO *ape;
	long long y, xx, yy, x_floor, y_floor;
	short room_number;

	item = &g_Level.Items[item_number];

	ape = (CREATURE_INFO *)item->data;
	if (ape->flags & TURNL_FLAG)
	{
		item->pos.yRot -= 0x4000;
		ape->flags -= TURNL_FLAG;
	}
	else if (item->flags & TURNR_FLAG)
	{
		item->pos.yRot += 0x4000;
		ape->flags -= TURNR_FLAG;
	}

	xx = item->pos.zPos / SECTOR(1);
	yy = item->pos.xPos / SECTOR(1);
	y = item->pos.yPos;
	room_number = item->roomNumber;

	CreatureAnimation(item_number, angle, 0);

	if (item->pos.yPos > y - STEP_SIZE * 3 / 2)
		return;

	x_floor = item->pos.zPos / SECTOR(1);
	y_floor = item->pos.xPos / SECTOR(1);
	if (xx == x_floor)
	{
		if (yy == y_floor)
			return;

		if (yy < y_floor)
		{
			item->pos.xPos = (y_floor * SECTOR(1)) - SHIFT;
			item->pos.yRot = 0x4000;
		}
		else
		{
			item->pos.xPos = (yy * SECTOR(1)) + SHIFT;
			item->pos.yRot = -0x4000;
		}
	}
	else if (yy == y_floor)
	{
		if (xx < x_floor)
		{
			item->pos.zPos = (x_floor * SECTOR(1)) - SHIFT;
			item->pos.yRot = 0;
		}
		else
		{
			item->pos.zPos = (xx * SECTOR(1)) + SHIFT;
			item->pos.yRot = -0x8000;
		}
	}
	else
	{
		// diagonal
	}

	switch (CreatureVault(item_number, angle, 2, SHIFT))
	{
	case 2:
		item->pos.yPos = y;
		item->animNumber = Objects[ID_APE].animIndex + VAULT_ANIM;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->currentAnimState = APE_VAULT;
		break;

	default:
		return;
	}
}

void ApeControl(short itemNum)
{
	short head, angle, random;

	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	AI_INFO info;
	head = angle = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != APE_DEATH)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + DIE_ANIM + (short)(GetRandomControl() / 0x4000);
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = APE_DEATH;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->hitStatus || info.distance < PANIC_RANGE)
			creature->flags |= ATTACK_FLAG;

		switch (item->currentAnimState)
		{
		case APE_STOP:
			if (creature->flags & TURNL_FLAG)
			{
				item->pos.yRot -= ANGLE(90);
				creature->flags -= TURNL_FLAG;
			}
			else if (item->flags & TURNR_FLAG)
			{
				item->pos.yRot += ANGLE(90);
				creature->flags -= TURNR_FLAG;
			}

			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (info.bite && info.distance < ATTACK_RANGE)
				item->goalAnimState = APE_ATTACK1;
			else if (!(creature->flags & ATTACK_FLAG) &&
				info.zoneNumber == info.enemyZone && info.ahead)
			{
				random = (short)(GetRandomControl() / 32);
				if (random < JUMP_CHANCE)
					item->goalAnimState = APE_JUMP;
				else if (random < WARN1_CHANCE)
					item->goalAnimState = APE_WARNING;
				else if (random < WARN2_CHANCE)
					item->goalAnimState = APE_WARNING2;
				else if (random < RUNLEFT_CHANCE)
				{
					item->goalAnimState = APE_RUNLEFT;
					creature->maximumTurn = 0;
				}
				else
				{
					item->goalAnimState = APE_RUNRIGHT;
					creature->maximumTurn = 0;
				}
			}
			else
				item->goalAnimState = APE_RUN;
			break;

		case APE_RUN:
			creature->maximumTurn = RUN_TURN;

			if (creature->flags == 0 && info.angle > -DISPLAY_ANGLE && info.angle < DISPLAY_ANGLE)
				item->goalAnimState = APE_STOP;
			else if (info.ahead && (item->touchBits & TOUCH))
			{
				item->requiredAnimState = APE_ATTACK1;
				item->goalAnimState = APE_STOP;
			}
			else if (creature->mood != ESCAPE_MOOD)
			{
				random = (short)GetRandomControl();
				if (random < JUMP_CHANCE)
				{
					item->requiredAnimState = APE_JUMP;
					item->goalAnimState = APE_STOP;
				}
				else if (random < WARN1_CHANCE)
				{
					item->requiredAnimState = APE_WARNING;
					item->goalAnimState = APE_STOP;
				}
				else if (random < WARN2_CHANCE)
				{
					item->requiredAnimState = APE_WARNING2;
					item->goalAnimState = APE_STOP;
				}
			}
			break;

		case APE_RUNLEFT:
			if (!(creature->flags & TURNR_FLAG))
			{
				item->pos.yRot -= ANGLE(90);
				creature->flags |= TURNR_FLAG;
			}

			item->goalAnimState = APE_STOP;
			break;

		case APE_RUNRIGHT:
			if (!(creature->flags & TURNL_FLAG))
			{
				item->pos.yRot += ANGLE(90);
				creature->flags |= TURNL_FLAG;
			}

			item->goalAnimState = APE_STOP;
			break;

		case APE_ATTACK1:
			if (!item->requiredAnimState && (item->touchBits & TOUCH))
			{
				CreatureEffect(item, &apeBite, DoBloodSplat);

				LaraItem->hitPoints -= ATTACK_DAMAGE;
				LaraItem->hitStatus = true;

				item->requiredAnimState = APE_STOP;
			}
			break;
		}
	}

	CreatureJoint(item, 0, head);

	if (item->currentAnimState != APE_VAULT)
	{
		ApeVault(itemNum, angle);
	}
	else
		CreatureAnimation(itemNum, angle, 0);
}
