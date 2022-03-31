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
	auto* creature = GetCreatureInfo(item);

	if (creature->Flags & APE_FLAG_TURN_LEFT)
	{
		item->Position.yRot -= ANGLE(90.0f);
		creature->Flags -= APE_FLAG_TURN_LEFT;
	}
	else if (item->Flags & APE_FLAG_TURN_RIGHT)
	{
		item->Position.yRot += ANGLE(90.0f);
		creature->Flags -= APE_FLAG_TURN_RIGHT;
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
		item->Animation.AnimNumber = Objects[ID_APE].animIndex + APE_ANIM_VAULT;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = APE_STATE_VAULT;
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
		if (item->Animation.ActiveState != APE_STATE_DEATH)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + APE_ANIM_DEATH + (short)(GetRandomControl() / 0x4000);
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = APE_STATE_DEATH;
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (AI.ahead)
			head = AI.angle;

		GetCreatureMood(item, &AI, TIMID);
		CreatureMood(item, &AI, TIMID);

		angle = CreatureTurn(item, creatureInfo->MaxTurn);

		if (item->HitStatus || AI.distance < PANIC_RANGE)
			creatureInfo->Flags |= APE_FLAG_ATTACK;

		short random;

		switch (item->Animation.ActiveState)
		{
		case APE_STATE_IDLE:
			if (creatureInfo->Flags & APE_FLAG_TURN_LEFT)
			{
				item->Position.yRot -= ANGLE(90);
				creatureInfo->Flags -= APE_FLAG_TURN_LEFT;
			}
			else if (item->Flags & APE_FLAG_TURN_RIGHT)
			{
				item->Position.yRot += ANGLE(90);
				creatureInfo->Flags -= APE_FLAG_TURN_RIGHT;
			}

			if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;
			else if (AI.bite && AI.distance < ATTACK_RANGE)
				item->Animation.TargetState = APE_STATE_ATTACK;
			else if (!(creatureInfo->Flags & APE_FLAG_ATTACK) &&
				AI.zoneNumber == AI.enemyZone && AI.ahead)
			{
				random = (short)(GetRandomControl() / 32);
				if (random < JUMP_CHANCE)
					item->Animation.TargetState = APE_STATE_JUMP;
				else if (random < WARNING_1_CHANCE)
					item->Animation.TargetState = APE_STATE_WARNING_1;
				else if (random < WARNING_2_CHANCE)
					item->Animation.TargetState = APE_STATE_WARNING_2;
				else if (random < RUNLEFT_CHANCE)
				{
					item->Animation.TargetState = APE_STATE_RUN_LEFT;
					creatureInfo->MaxTurn = 0;
				}
				else
				{
					item->Animation.TargetState = APE_STATE_RUN_RIGHT;
					creatureInfo->MaxTurn = 0;
				}
			}
			else
				item->Animation.TargetState = APE_STATE_RUN;

			break;

		case APE_STATE_RUN:
			creatureInfo->MaxTurn = RUN_TURN;

			if (creatureInfo->Flags == 0 &&
				AI.angle > -DISPLAY_ANGLE &&
				AI.angle < DISPLAY_ANGLE)
			{
				item->Animation.TargetState = APE_STATE_IDLE;
			}
			else if (AI.ahead && item->TouchBits & TOUCH)
			{
				item->Animation.RequiredState = APE_STATE_ATTACK;
				item->Animation.TargetState = APE_STATE_IDLE;
			}
			else if (creatureInfo->Mood != MoodType::Escape)
			{
				random = (short)GetRandomControl();
				if (random < JUMP_CHANCE)
				{
					item->Animation.RequiredState = APE_STATE_JUMP;
					item->Animation.TargetState = APE_STATE_IDLE;
				}
				else if (random < WARNING_1_CHANCE)
				{
					item->Animation.RequiredState = APE_STATE_WARNING_1;
					item->Animation.TargetState = APE_STATE_IDLE;
				}
				else if (random < WARNING_2_CHANCE)
				{
					item->Animation.RequiredState = APE_STATE_WARNING_2;
					item->Animation.TargetState = APE_STATE_IDLE;
				}
			}

			break;

		case APE_STATE_RUN_LEFT:
			if (!(creatureInfo->Flags & APE_FLAG_TURN_RIGHT))
			{
				item->Position.yRot -= ANGLE(90);
				creatureInfo->Flags |= APE_FLAG_TURN_RIGHT;
			}

			item->Animation.TargetState = APE_STATE_IDLE;
			break;

		case APE_STATE_RUN_RIGHT:
			if (!(creatureInfo->Flags & APE_FLAG_TURN_LEFT))
			{
				item->Position.yRot += ANGLE(90);
				creatureInfo->Flags |= APE_FLAG_TURN_LEFT;
			}

			item->Animation.TargetState = APE_STATE_IDLE;
			break;

		case APE_STATE_ATTACK:
			if (!item->Animation.RequiredState && item->TouchBits & TOUCH)
			{
				CreatureEffect(item, &ApeBite, DoBloodSplat);
				item->Animation.RequiredState = APE_STATE_IDLE;

				LaraItem->HitPoints -= ATTACK_DAMAGE;
				LaraItem->HitStatus = true;
			}

			break;
		}
	}

	CreatureJoint(item, 0, head);

	if (item->Animation.ActiveState != APE_STATE_VAULT)
		ApeVault(itemNumber, angle);
	else
		CreatureAnimation(itemNumber, angle, 0);
}
