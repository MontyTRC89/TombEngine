#include "framework.h"
#include "Objects/TR1/Entity/tr1_wolf.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO WolfBite = { 0, -14, 174, 6 };

enum WolfState
{
	WOLF_STATE_NONE = 0, 
	WOLF_STATE_IDLE = 1,
	WOLF_STATE_WALK = 2,
	WOLF_STATE_RUN = 3,
	WOLF_STATE_JUMP = 4,
	WOLF_STATE_STALK = 5,
	WOLF_STATE_ATTACK = 6,
	WOLF_STATE_HOWL = 7,
	WOLF_STATE_SLEEP = 8,
	WOLF_STATE_CROUCH = 9,
	WOLF_STATE_FAST_TURN = 10,
	WOLF_STATE_DEATH = 11,
	WOLF_STATE_BITE = 12
};

// TODO
enum WolfAnim
{
	WOLF_ANIM_DEATH = 20,
};

#define TOUCH 0x774f

#define SLEEP_FRAME 96

#define ATTACK_RANGE pow(SECTOR(1.5f), 2)
#define STALK_RANGE  pow(SECTOR(2), 2)

#define BITE_DAMAGE  100
#define LUNGE_DAMAGE 50

#define WAKE_CHANCE  0x20
#define SLEEP_CHANCE 0x20
#define HOWL_CHANCE  0x180

#define WALK_TURN  ANGLE(2.0f)
#define RUN_TURN   ANGLE(5.0f)
#define STALK_TURN ANGLE(2.0f)

void InitialiseWolf(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);
	item->FrameNumber = SLEEP_FRAME;
}

void WolfControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != WOLF_STATE_DEATH)
		{
			item->AnimNumber = Objects[ID_WOLF].animIndex + WOLF_ANIM_DEATH + (short)(GetRandomControl() / 11000);
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = WOLF_STATE_DEATH;
		}
	}
	else
	{
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		if (aiInfo.ahead)
			head = aiInfo.angle;

		GetCreatureMood(item, &aiInfo, TIMID);
		CreatureMood(item, &aiInfo, TIMID);

		angle = CreatureTurn(item, info->maximumTurn);

		switch (item->ActiveState)
		{
		case WOLF_STATE_SLEEP:
			head = 0;

			if (info->mood == ESCAPE_MOOD || aiInfo.zoneNumber == aiInfo.enemyZone)
			{
				item->RequiredState = WOLF_STATE_CROUCH;
				item->TargetState = WOLF_STATE_IDLE;
			}
			else if (GetRandomControl() < WAKE_CHANCE)
			{
				item->RequiredState = WOLF_STATE_WALK;
				item->TargetState = WOLF_STATE_IDLE;
			}

			break;

		case WOLF_STATE_IDLE:
			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else
				item->TargetState = WOLF_STATE_WALK;
			break;

		case WOLF_STATE_WALK:
			info->maximumTurn = WALK_TURN;

			if (info->mood != BORED_MOOD)
			{
				item->TargetState = WOLF_STATE_STALK;
				item->RequiredState = WOLF_STATE_NONE;
			}
			else if (GetRandomControl() < SLEEP_CHANCE)
			{
				item->RequiredState = WOLF_STATE_SLEEP;
				item->TargetState = WOLF_STATE_IDLE;
			}

			break;

		case WOLF_STATE_CROUCH:
			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (info->mood == ESCAPE_MOOD)
				item->TargetState = WOLF_STATE_RUN;
			else if (aiInfo.distance < pow(345, 2) && aiInfo.bite)
				item->TargetState = WOLF_STATE_BITE;
			else if (info->mood == STALK_MOOD)
				item->TargetState = WOLF_STATE_STALK;
			else if (info->mood == BORED_MOOD)
				item->TargetState = WOLF_STATE_IDLE;
			else
				item->TargetState = WOLF_STATE_RUN;

			break;

		case WOLF_STATE_STALK:
			info->maximumTurn = STALK_TURN;

			if (info->mood == ESCAPE_MOOD)
				item->TargetState = WOLF_STATE_RUN;
			else if (aiInfo.distance < pow(345, 2) && aiInfo.bite)
				item->TargetState = WOLF_STATE_BITE;
			else if (aiInfo.distance > pow(SECTOR(3), 2))
				item->TargetState = WOLF_STATE_RUN;
			else if (info->mood == ATTACK_MOOD)
			{
				if (!aiInfo.ahead || aiInfo.distance > pow(SECTOR(1.5f), 2) ||
					(aiInfo.enemyFacing < FRONT_ARC && aiInfo.enemyFacing > -FRONT_ARC))
				{
					item->TargetState = WOLF_STATE_RUN;
				}
			}
			else if (GetRandomControl() < HOWL_CHANCE)
			{
				item->RequiredState = WOLF_STATE_HOWL;
				item->TargetState = WOLF_STATE_CROUCH;
			}
			else if (info->mood == BORED_MOOD)
				item->TargetState = WOLF_STATE_CROUCH;

			break;

		case WOLF_STATE_RUN:
			info->maximumTurn = RUN_TURN;
			tilt = angle;

			if (aiInfo.ahead && aiInfo.distance < ATTACK_RANGE)
			{
				if (aiInfo.distance > (ATTACK_RANGE / 2) &&
					(aiInfo.enemyFacing > FRONT_ARC || aiInfo.enemyFacing < -FRONT_ARC))
				{
					item->RequiredState = WOLF_STATE_STALK;
					item->TargetState = WOLF_STATE_CROUCH;
				}
				else
				{
					item->TargetState = WOLF_STATE_ATTACK;
					item->RequiredState = WOLF_STATE_NONE;
				}
			}
			else if (info->mood == STALK_MOOD && aiInfo.distance < STALK_RANGE)
			{
				item->RequiredState = WOLF_STATE_STALK;
				item->TargetState = WOLF_STATE_CROUCH;
			}
			else if (info->mood == BORED_MOOD)
				item->TargetState = WOLF_STATE_CROUCH;

			break;

		case WOLF_STATE_ATTACK:
			tilt = angle;

			if (!item->RequiredState && item->TouchBits & TOUCH)
			{
				CreatureEffect(item, &WolfBite, DoBloodSplat);
				LaraItem->HitPoints -= LUNGE_DAMAGE;
				LaraItem->HitStatus = true;
				item->RequiredState = WOLF_STATE_RUN;
			}

			item->TargetState = WOLF_STATE_RUN;
			break;

		case WOLF_STATE_BITE:
			if (!item->RequiredState &&
				item->TouchBits & TOUCH && aiInfo.ahead)
			{
				CreatureEffect(item, &WolfBite, DoBloodSplat);
				LaraItem->HitPoints -= BITE_DAMAGE;
				LaraItem->HitStatus = true;
				item->RequiredState = WOLF_STATE_CROUCH;
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNumber, angle, tilt);
}
