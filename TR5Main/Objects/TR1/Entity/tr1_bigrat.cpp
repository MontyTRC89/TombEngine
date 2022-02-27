#include "framework.h"
#include "Objects/TR1/Entity/tr1_bigrat.h"

#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"

static BITE_INFO BigRatBite = { 0, -11, 108, 3 };

#define BIG_RAT_RUN_TURN  ANGLE(6.0f)
#define BIG_RAT_SWIM_TURN ANGLE(3.0f)

constexpr auto DEFAULT_SWIM_UPDOWN_SPEED = 32;
constexpr auto BIG_RAT_TOUCH = 0x300018f;
constexpr auto BIG_RAT_ALERT_RANGE = SQUARE(SECTOR(1) + CLICK(2));
constexpr auto BIG_RAT_VISIBILITY_RANGE = SQUARE(SECTOR(5));
constexpr auto BIG_RAT_BITE_RANGE = SQUARE(CLICK(1) + CLICK(1) / 3);
constexpr auto BIG_RAT_CHARGE_RANGE = SQUARE(SECTOR(1) / 2);
constexpr auto BIG_RAT_POSE_CHANCE = 0x100;
constexpr auto BIG_RAT_WATER_BITE_RANGE = SQUARE(CLICK(1) + CLICK(1) / 6);
constexpr auto BIG_RAT_BITE_DAMAGE = 20;
constexpr auto BIG_RAT_CHARGE_DAMAGE = 25;

enum BigRatState
{
	BIG_RAT_STATE_EMPTY = 0,
	BIG_RAT_STATE_IDLE = 1,
	BIG_RAT_STATE_CHARGE_ATTACK = 2,
	BIG_RAT_STATE_RUN = 3,
	BIG_RAT_STATE_BITE_ATTACK = 4,
	BIG_RAT_STATE_LAND_DEATH = 5,
	BIG_RAT_STATE_POSE = 6,
	BIG_RAT_STATE_SWIM = 7,
	BIG_RAT_STATE_SWIM_ATTACK = 8,
	BIG_RAT_STATE_WATER_DEATH = 9
};

enum BigRatAnim
{
	BIG_RAT_ANIM_EMPTY = 0,
	BIG_RAT_ANIM_STOP_TO_RUN = 1,
	BIG_RAT_ANIM_RUN = 2,
	BIG_RAT_ANIM_RUN_TO_STOP = 3,
	BIG_RAT_ANIM_POSE = 4,
	BIG_RAT_ANIM_POSE_TO_STOP = 5,
	BIG_RAT_ANIM_LAND_BITE_ATTACK = 6,
	BIG_RAT_ANIM_CHARGE_ATTACK = 7,
	BIG_RAT_ANIM_LAND_DEATH = 8,
	BIG_RAT_ANIM_SWIM = 9,
	BIG_RAT_ANIM_WATER_BITE = 10,
	BIG_RAT_ANIM_WATER_DEATH = 11,
	BIG_RAT_ANIM_RUN_TO_SWIM = 12,
	BIG_RAT_ANIM_SWIM_TO_RUN = 13
};

void InitialiseBigRat(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	InitialiseCreature(itemNumber);

	if (TestEnvironment(ENV_FLAG_WATER, item))
	{
		item->AnimNumber = Objects[item->ObjectNumber].animIndex + BIG_RAT_ANIM_SWIM;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->ActiveState = BIG_RAT_STATE_SWIM;
		item->TargetState = BIG_RAT_STATE_SWIM;
	}
	else
	{
		item->AnimNumber = Objects[item->ObjectNumber].animIndex + BIG_RAT_ANIM_EMPTY;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->ActiveState = BIG_RAT_STATE_IDLE;
		item->TargetState = BIG_RAT_STATE_IDLE;
	}
}

static bool RatIsInWater(ITEM_INFO* item)
{
	auto* info = GetCreatureInfo(item);

	EntityStoringInfo storingInfo;
	storingInfo.x = item->Position.xPos;
	storingInfo.y = item->Position.yPos;
	storingInfo.z = item->Position.zPos;
	storingInfo.roomNumber = item->RoomNumber;

	GetFloor(storingInfo.x, storingInfo.y, storingInfo.z, &storingInfo.roomNumber);
	storingInfo.waterDepth = GetWaterSurface(storingInfo.x, storingInfo.y, storingInfo.z, storingInfo.roomNumber);

	if (storingInfo.waterDepth != NO_HEIGHT)
	{
		info->LOT.step = SECTOR(20);
		info->LOT.drop = -SECTOR(20);
		info->LOT.fly = DEFAULT_SWIM_UPDOWN_SPEED;
		return true;
	}
	else
	{
		info->LOT.step = CLICK(1);
		info->LOT.drop = -CLICK(1);
		info->LOT.fly = NO_FLYING;
		return false;
	}
}

void BigRatControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);
	auto* objectInfo = &Objects[item->ObjectNumber];

	short head = 0;
	short angle = 0;
	int waterHeight = GetWaterHeight(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber);

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != BIG_RAT_STATE_LAND_DEATH &&
			item->ActiveState != BIG_RAT_STATE_WATER_DEATH)
		{
			if (TestEnvironment(ENV_FLAG_WATER, item))
			{
				item->AnimNumber = objectInfo->animIndex + BIG_RAT_ANIM_WATER_DEATH;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				item->ActiveState = BIG_RAT_STATE_WATER_DEATH;
				item->TargetState = BIG_RAT_STATE_WATER_DEATH;
			}
			else
			{
				item->AnimNumber = objectInfo->animIndex + BIG_RAT_ANIM_LAND_DEATH;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				item->ActiveState = BIG_RAT_STATE_LAND_DEATH;
				item->TargetState = BIG_RAT_STATE_LAND_DEATH;
			}
		}

		if (TestEnvironment(ENV_FLAG_WATER, item))
			CreatureFloat(itemNumber);
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

		if (item->AIBits & ALL_AIOBJ)
			GetAITarget(info);
		else if (info->hurtByLara)
			info->enemy = LaraItem;

		if ((item->HitStatus || aiInfo.distance < BIG_RAT_ALERT_RANGE) ||
			(TargetVisible(item, &aiInfo) && aiInfo.distance < BIG_RAT_VISIBILITY_RANGE))
		{
			if (!info->alerted)
				info->alerted = true;

			AlertAllGuards(itemNumber);
		}

		switch (item->ActiveState)
		{
		case BIG_RAT_STATE_IDLE:
			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (aiInfo.bite && aiInfo.distance < BIG_RAT_BITE_RANGE)
				item->TargetState = BIG_RAT_STATE_BITE_ATTACK;
			else
				item->TargetState = BIG_RAT_STATE_RUN;

			break;

		case BIG_RAT_STATE_RUN:
			info->maximumTurn = BIG_RAT_RUN_TURN;

			if (RatIsInWater(item))
			{
				item->RequiredState = BIG_RAT_STATE_SWIM;
				item->TargetState = BIG_RAT_STATE_SWIM;

				break;
			}

			if (aiInfo.ahead && (item->TouchBits & BIG_RAT_TOUCH))
				item->TargetState = BIG_RAT_STATE_IDLE;
			else if (aiInfo.bite && aiInfo.distance < BIG_RAT_CHARGE_RANGE)
				item->TargetState = BIG_RAT_STATE_CHARGE_ATTACK;
			else if (aiInfo.ahead && GetRandomControl() < BIG_RAT_POSE_CHANCE)
			{
				item->RequiredState = BIG_RAT_STATE_POSE;
				item->TargetState = BIG_RAT_STATE_IDLE;
			}

			break;

		case BIG_RAT_STATE_BITE_ATTACK:
			if (!item->RequiredState && aiInfo.ahead && (item->TouchBits & BIG_RAT_TOUCH))
			{
				CreatureEffect(item, &BigRatBite, DoBloodSplat);
				item->RequiredState = BIG_RAT_STATE_IDLE;

				LaraItem->HitPoints -= BIG_RAT_BITE_DAMAGE;
				LaraItem->HitStatus = true;
			}

			break;

		case BIG_RAT_STATE_CHARGE_ATTACK:
			if (!item->RequiredState && aiInfo.ahead && (item->TouchBits & BIG_RAT_TOUCH))
			{
				CreatureEffect(item, &BigRatBite, DoBloodSplat);
				item->RequiredState = BIG_RAT_STATE_RUN;

				LaraItem->HitPoints -= BIG_RAT_CHARGE_DAMAGE;
				LaraItem->HitStatus = true;
			}

			break;

		case BIG_RAT_STATE_POSE:
			if (info->mood != BORED_MOOD || GetRandomControl() < BIG_RAT_POSE_CHANCE)
				item->TargetState = BIG_RAT_STATE_IDLE;

			break;

		case BIG_RAT_STATE_SWIM:
			info->maximumTurn = BIG_RAT_SWIM_TURN;

			if (!RatIsInWater(item))
			{
				item->RequiredState = BIG_RAT_STATE_RUN;
				item->TargetState = BIG_RAT_STATE_RUN;
				break;
			}

			if (aiInfo.ahead && item->TouchBits & BIG_RAT_TOUCH)
				item->TargetState = BIG_RAT_STATE_SWIM_ATTACK;

			break;

		case BIG_RAT_STATE_SWIM_ATTACK:
			if (!item->RequiredState && aiInfo.ahead && item->TouchBits & BIG_RAT_TOUCH)
			{
				CreatureEffect(item, &BigRatBite, DoBloodSplat);

				LaraItem->HitPoints -= BIG_RAT_BITE_DAMAGE;
				LaraItem->HitStatus = true;
			}

			item->TargetState = BIG_RAT_STATE_SWIM;
			break;
		}

	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNumber, angle, 0);

	if (RatIsInWater(item))
	{
		CreatureUnderwater(item, 0);
		item->Position.yPos = waterHeight;
	}
	else
		item->Position.yPos = item->Floor;
}
