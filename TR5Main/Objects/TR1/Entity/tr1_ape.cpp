#include "framework.h"
#include "Objects/TR1/Entity/tr1_ape.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO ApeBite = { 0, -19, 75, 15 };

#define ATTACK_DAMAGE 200

#define TOUCH (0xFF00)

#define RUN_TURN ANGLE(5.0f)

#define DISPLAY_ANGLE ANGLE(45.0f)

#define ATTACK_RANGE pow(430, 2)
#define PANIC_RANGE  pow(SECTOR(2), 2)

#define JUMP_CHANCE 0xa0
#define WARNING_1_CHANCE (JUMP_CHANCE + 0xA0)
#define WARNING_2_CHANCE (WARNING_1_CHANCE + 0xA0)
#define RUNLEFT_CHANCE   (WARNING_2_CHANCE + 0x110)

#define SHIFT 75

enum ApeState
{
	APE_STATE_NONE = 0, 
	APE_STATE_IDLE = 1,
	APE_STATE_WALK = 2,
	APE_STATE_RUN = 3,
	APE_STATE_ATTACK = 4,
	APE_STATE_DEATH = 5,
	APE_STATE_WARNING_1 = 6,
	APE_STATE_WARNING_2 = 7,
	APE_STATE_RUN_LEFT = 8,
	APE_STATE_RUN_RIGHT = 9,
	APE_STATE_JUMP = 10,
	APE_STATE_VAULT = 11
};

// TODO
enum ApeAnim
{
	APE_ANIM_DEATH = 7,

	APE_ANIM_VAULT = 19,
};

enum ApeFlags
{
	APE_FLAG_ATTACK = 1,
	APE_FLAG_TURN_LEFT = 2,
	APE_FLAG_TURN_RIGHT = 4
};

void ApeVault(short itemNumber, short angle)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	if (info->flags & APE_FLAG_TURN_LEFT)
	{
		item->Position.yRot -= ANGLE(90.0f);
		info->flags -= APE_FLAG_TURN_LEFT;
	}
	else if (item->Flags & APE_FLAG_TURN_RIGHT)
	{
		item->Position.yRot += ANGLE(90.0f);
		info->flags -= APE_FLAG_TURN_RIGHT;
	}

	long long xx = item->Position.zPos / SECTOR(1);
	long long yy = item->Position.xPos / SECTOR(1);
	long long y = item->Position.yPos;

	CreatureAnimation(itemNumber, angle, 0);

	if (item->Position.yPos > (y - CLICK(1.5f)))
		return;

	long long xFloor = item->Position.zPos / SECTOR(1);
	long long yFloor = item->Position.xPos / SECTOR(1);
	if (xx == xFloor)
	{
		if (yy == yFloor)
			return;

		if (yy < yFloor)
		{
			item->Position.xPos = (yFloor * SECTOR(1)) - SHIFT;
			item->Position.yRot = ANGLE(90.0f);
		}
		else
		{
			item->Position.xPos = (yy * SECTOR(1)) + SHIFT;
			item->Position.yRot = -ANGLE(90.0f);
		}
	}
	else if (yy == yFloor)
	{
		if (xx < xFloor)
		{
			item->Position.zPos = (xFloor * SECTOR(1)) - SHIFT;
			item->Position.yRot = 0;
		}
		else
		{
			item->Position.zPos = (xx * SECTOR(1)) + SHIFT;
			item->Position.yRot = -ANGLE(180.0f);
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
		item->AnimNumber = Objects[ID_APE].animIndex + APE_ANIM_VAULT;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->ActiveState = APE_STATE_VAULT;
		break;

	default:
		return;
	}
}

void ApeControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creatureInfo = GetCreatureInfo(item);

	short head = 0;
	short angle = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != APE_STATE_DEATH)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + APE_ANIM_DEATH + (short)(GetRandomControl() / 0x4000);
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = APE_STATE_DEATH;
		}
	}
	else
	{
		AI_INFO AIInfo;
		CreatureAIInfo(item, &AIInfo);

		if (AIInfo.ahead)
			head = AIInfo.angle;

		GetCreatureMood(item, &AIInfo, TIMID);
		CreatureMood(item, &AIInfo, TIMID);

		angle = CreatureTurn(item, creatureInfo->maximumTurn);

		if (item->HitStatus || AIInfo.distance < PANIC_RANGE)
			creatureInfo->flags |= APE_FLAG_ATTACK;

		short random;

		switch (item->ActiveState)
		{
		case APE_STATE_IDLE:
			if (creatureInfo->flags & APE_FLAG_TURN_LEFT)
			{
				item->Position.yRot -= ANGLE(90);
				creatureInfo->flags -= APE_FLAG_TURN_LEFT;
			}
			else if (item->Flags & APE_FLAG_TURN_RIGHT)
			{
				item->Position.yRot += ANGLE(90);
				creatureInfo->flags -= APE_FLAG_TURN_RIGHT;
			}

			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (AIInfo.bite && AIInfo.distance < ATTACK_RANGE)
				item->TargetState = APE_STATE_ATTACK;
			else if (!(creatureInfo->flags & APE_FLAG_ATTACK) &&
				AIInfo.zoneNumber == AIInfo.enemyZone && AIInfo.ahead)
			{
				random = (short)(GetRandomControl() / 32);
				if (random < JUMP_CHANCE)
					item->TargetState = APE_STATE_JUMP;
				else if (random < WARNING_1_CHANCE)
					item->TargetState = APE_STATE_WARNING_1;
				else if (random < WARNING_2_CHANCE)
					item->TargetState = APE_STATE_WARNING_2;
				else if (random < RUNLEFT_CHANCE)
				{
					item->TargetState = APE_STATE_RUN_LEFT;
					creatureInfo->maximumTurn = 0;
				}
				else
				{
					item->TargetState = APE_STATE_RUN_RIGHT;
					creatureInfo->maximumTurn = 0;
				}
			}
			else
				item->TargetState = APE_STATE_RUN;

			break;

		case APE_STATE_RUN:
			creatureInfo->maximumTurn = RUN_TURN;

			if (creatureInfo->flags == 0 &&
				AIInfo.angle > -DISPLAY_ANGLE &&
				AIInfo.angle < DISPLAY_ANGLE)
			{
				item->TargetState = APE_STATE_IDLE;
			}
			else if (AIInfo.ahead && item->TouchBits & TOUCH)
			{
				item->RequiredState = APE_STATE_ATTACK;
				item->TargetState = APE_STATE_IDLE;
			}
			else if (creatureInfo->mood != ESCAPE_MOOD)
			{
				random = (short)GetRandomControl();
				if (random < JUMP_CHANCE)
				{
					item->RequiredState = APE_STATE_JUMP;
					item->TargetState = APE_STATE_IDLE;
				}
				else if (random < WARNING_1_CHANCE)
				{
					item->RequiredState = APE_STATE_WARNING_1;
					item->TargetState = APE_STATE_IDLE;
				}
				else if (random < WARNING_2_CHANCE)
				{
					item->RequiredState = APE_STATE_WARNING_2;
					item->TargetState = APE_STATE_IDLE;
				}
			}

			break;

		case APE_STATE_RUN_LEFT:
			if (!(creatureInfo->flags & APE_FLAG_TURN_RIGHT))
			{
				item->Position.yRot -= ANGLE(90);
				creatureInfo->flags |= APE_FLAG_TURN_RIGHT;
			}

			item->TargetState = APE_STATE_IDLE;
			break;

		case APE_STATE_RUN_RIGHT:
			if (!(creatureInfo->flags & APE_FLAG_TURN_LEFT))
			{
				item->Position.yRot += ANGLE(90);
				creatureInfo->flags |= APE_FLAG_TURN_LEFT;
			}

			item->TargetState = APE_STATE_IDLE;
			break;

		case APE_STATE_ATTACK:
			if (!item->RequiredState && item->TouchBits & TOUCH)
			{
				CreatureEffect(item, &ApeBite, DoBloodSplat);
				item->RequiredState = APE_STATE_IDLE;

				LaraItem->HitPoints -= ATTACK_DAMAGE;
				LaraItem->HitStatus = true;
			}

			break;
		}
	}

	CreatureJoint(item, 0, head);

	if (item->ActiveState != APE_STATE_VAULT)
		ApeVault(itemNumber, angle);
	else
		CreatureAnimation(itemNumber, angle, 0);
}
