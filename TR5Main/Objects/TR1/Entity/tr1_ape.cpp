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

#define RUN_TURN ANGLE(5.0f)

#define DISPLAY_ANGLE ANGLE(45.0f)

#define ATTACK_RANGE pow(430, 2)
#define PANIC_RANGE pow(SECTOR(2), 2)

#define JUMP_CHANCE 0xa0
#define WARN1_CHANCE (JUMP_CHANCE + 0xA0)
#define WARN2_CHANCE (WARN1_CHANCE + 0xA0)
#define RUNLEFT_CHANCE (WARN2_CHANCE + 0x110)

#define ATTACK_FLAG 1
#define TURNL_FLAG 2
#define TURNR_FLAG 4

#define SHIFT 75

enum ApeAnims
{
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

void ApeVault(short itemNumber, short angle)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* creatureInfo = (CREATURE_INFO*)item->Data;

	if (creatureInfo->flags & TURNL_FLAG)
	{
		item->Position.yRot -= 0x4000;
		creatureInfo->flags -= TURNL_FLAG;
	}
	else if (item->Flags & TURNR_FLAG)
	{
		item->Position.yRot += 0x4000;
		creatureInfo->flags -= TURNR_FLAG;
	}

	long long xx = item->Position.zPos / SECTOR(1);
	long long yy = item->Position.xPos / SECTOR(1);
	long long y = item->Position.yPos;

	CreatureAnimation(itemNumber, angle, 0);

	if (item->Position.yPos > (y - CLICK(1.5f)))
		return;

	long long x_floor = item->Position.zPos / SECTOR(1);
	long long y_floor = item->Position.xPos / SECTOR(1);
	if (xx == x_floor)
	{
		if (yy == y_floor)
			return;

		if (yy < y_floor)
		{
			item->Position.xPos = (y_floor * SECTOR(1)) - SHIFT;
			item->Position.yRot = 0x4000;
		}
		else
		{
			item->Position.xPos = (yy * SECTOR(1)) + SHIFT;
			item->Position.yRot = -0x4000;
		}
	}
	else if (yy == y_floor)
	{
		if (xx < x_floor)
		{
			item->Position.zPos = (x_floor * SECTOR(1)) - SHIFT;
			item->Position.yRot = 0;
		}
		else
		{
			item->Position.zPos = (xx * SECTOR(1)) + SHIFT;
			item->Position.yRot = -0x8000;
		}
	}
	else
	{
		// diagonal
	}

	switch (CreatureVault(itemNumber, angle, 2, SHIFT))
	{
	case 2:
		item->Position.yPos = y;
		item->AnimNumber = Objects[ID_APE].animIndex + VAULT_ANIM;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->ActiveState = APE_VAULT;
		break;

	default:
		return;
	}
}

void ApeControl(short itemNumber)
{
	short head, angle, random;

	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creatureInfo = (CREATURE_INFO*)item->Data;
	AI_INFO info;
	head = angle = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != APE_DEATH)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + DIE_ANIM + (short)(GetRandomControl() / 0x4000);
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = APE_DEATH;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, creatureInfo->maximumTurn);

		if (item->HitStatus || info.distance < PANIC_RANGE)
			creatureInfo->flags |= ATTACK_FLAG;

		switch (item->ActiveState)
		{
		case APE_STOP:
			if (creatureInfo->flags & TURNL_FLAG)
			{
				item->Position.yRot -= ANGLE(90);
				creatureInfo->flags -= TURNL_FLAG;
			}
			else if (item->Flags & TURNR_FLAG)
			{
				item->Position.yRot += ANGLE(90);
				creatureInfo->flags -= TURNR_FLAG;
			}

			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (info.bite && info.distance < ATTACK_RANGE)
				item->TargetState = APE_ATTACK1;
			else if (!(creatureInfo->flags & ATTACK_FLAG) &&
				info.zoneNumber == info.enemyZone && info.ahead)
			{
				random = (short)(GetRandomControl() / 32);
				if (random < JUMP_CHANCE)
					item->TargetState = APE_JUMP;
				else if (random < WARN1_CHANCE)
					item->TargetState = APE_WARNING;
				else if (random < WARN2_CHANCE)
					item->TargetState = APE_WARNING2;
				else if (random < RUNLEFT_CHANCE)
				{
					item->TargetState = APE_RUNLEFT;
					creatureInfo->maximumTurn = 0;
				}
				else
				{
					item->TargetState = APE_RUNRIGHT;
					creatureInfo->maximumTurn = 0;
				}
			}
			else
				item->TargetState = APE_RUN;
			break;

		case APE_RUN:
			creatureInfo->maximumTurn = RUN_TURN;

			if (creatureInfo->flags == 0 && info.angle > -DISPLAY_ANGLE && info.angle < DISPLAY_ANGLE)
				item->TargetState = APE_STOP;
			else if (info.ahead && (item->TouchBits & TOUCH))
			{
				item->RequiredState = APE_ATTACK1;
				item->TargetState = APE_STOP;
			}
			else if (creatureInfo->mood != ESCAPE_MOOD)
			{
				random = (short)GetRandomControl();
				if (random < JUMP_CHANCE)
				{
					item->RequiredState = APE_JUMP;
					item->TargetState = APE_STOP;
				}
				else if (random < WARN1_CHANCE)
				{
					item->RequiredState = APE_WARNING;
					item->TargetState = APE_STOP;
				}
				else if (random < WARN2_CHANCE)
				{
					item->RequiredState = APE_WARNING2;
					item->TargetState = APE_STOP;
				}
			}
			break;

		case APE_RUNLEFT:
			if (!(creatureInfo->flags & TURNR_FLAG))
			{
				item->Position.yRot -= ANGLE(90);
				creatureInfo->flags |= TURNR_FLAG;
			}

			item->TargetState = APE_STOP;
			break;

		case APE_RUNRIGHT:
			if (!(creatureInfo->flags & TURNL_FLAG))
			{
				item->Position.yRot += ANGLE(90);
				creatureInfo->flags |= TURNL_FLAG;
			}

			item->TargetState = APE_STOP;
			break;

		case APE_ATTACK1:
			if (!item->RequiredState && (item->TouchBits & TOUCH))
			{
				CreatureEffect(item, &apeBite, DoBloodSplat);

				LaraItem->HitPoints -= ATTACK_DAMAGE;
				LaraItem->HitStatus = true;

				item->RequiredState = APE_STOP;
			}
			break;
		}
	}

	CreatureJoint(item, 0, head);

	if (item->ActiveState != APE_VAULT)
	{
		ApeVault(itemNumber, angle);
	}
	else
		CreatureAnimation(itemNumber, angle, 0);
}
