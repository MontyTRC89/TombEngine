#include "framework.h"
#include "Objects/TR1/Entity/tr1_bigrat.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#define BIG_RAT_RUN_TURN ANGLE(6.0f)
#define BIG_RAT_SWIM_TURN ANGLE(3.0f)

constexpr auto DEFAULT_SWIM_UPDOWN_SPEED = 32;

constexpr auto BIG_RAT_TOUCH = (0x300018f);
constexpr auto BIG_RAT_ALERT_RANGE = SQUARE(SECTOR(1) + CLICK(2));
constexpr auto BIG_RAT_VISIBILITY_RANGE = SQUARE(SECTOR(5));
constexpr auto BIG_RAT_BITE_RANGE = SQUARE(CLICK(1) + CLICK(1) / 3);
constexpr auto BIG_RAT_CHARGE_RANGE = SQUARE(SECTOR(1) / 2);
constexpr auto BIG_RAT_POSE_CHANCE = 0x100;
constexpr auto BIG_RAT_WATER_BITE_RANGE = SQUARE(CLICK(1) + CLICK(1) / 6);
constexpr auto BIG_RAT_BITE_DAMAGE = 20;
constexpr auto BIG_RAT_CHARGE_DAMAGE = 25;

enum big_rat_state
{
    BIG_RAT_EMPTY,
    BIG_RAT_STOP,
    BIG_RAT_CHARGE_ATTACK,
    BIG_RAT_RUN,
    BIG_RAT_BITE_ATTACK,
    BIG_RAT_LAND_DEATH,
    BIG_RAT_POSE,
    BIG_RAT_SWIM,
    BIG_RAT_SWIM_ATTACK,
    BIG_RAT_WATER_DEATH
};

enum big_rat_anims
{
    BIG_RAT_ANIM_EMPTY,
    BIG_RAT_ANIM_STOP_TO_RUN,
    BIG_RAT_ANIM_RUN,
    BIG_RAT_ANIM_RUN_TO_STOP,
    BIG_RAT_ANIM_POSE,
    BIG_RAT_ANIM_POSE_TO_STOP,
    BIG_RAT_ANIM_LAND_BITE_ATTACK,
    BIG_RAT_ANIM_CHARGE_ATTACK,
    BIG_RAT_ANIM_LAND_DEATH,
    BIG_RAT_ANIM_SWIM,
    BIG_RAT_ANIM_WATER_BITE,
    BIG_RAT_ANIM_WATER_DEATH,
    BIG_RAT_ANIM_RUN_TO_SWIM,
    BIG_RAT_ANIM_SWIM_TO_RUN
};

static BITE_INFO big_ratBite = { 0, -11, 108, 3 };

static bool RatIsInWater(ITEM_INFO* item, CREATURE_INFO* big_rat)
{
    EntityStoringInfo info;
    info.x = item->pos.xPos;
    info.y = item->pos.yPos;
    info.z = item->pos.zPos;
    info.roomNumber = item->roomNumber;
    GetFloor(info.x, info.y, info.z, &info.roomNumber);
    info.waterDepth = GetWaterSurface(info.x, info.y, info.z, info.roomNumber);
    if (info.waterDepth != NO_HEIGHT)
    {
        big_rat->LOT.step = SECTOR(20);
        big_rat->LOT.drop = -SECTOR(20);
        big_rat->LOT.fly = DEFAULT_SWIM_UPDOWN_SPEED;
        return true;
    }
    else
    {
        big_rat->LOT.step = CLICK(1);
        big_rat->LOT.drop = -CLICK(1);
        big_rat->LOT.fly = NO_FLYING;
        return false;
    }
}

void InitialiseBigRat(short itemNumber)
{
    ITEM_INFO* item = &g_Level.Items[itemNumber];
    InitialiseCreature(itemNumber);

    if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
    {
        item->animNumber = Objects[item->objectNumber].animIndex + BIG_RAT_ANIM_SWIM;
        item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
        item->currentAnimState = BIG_RAT_SWIM;
        item->goalAnimState = BIG_RAT_SWIM;
    }
    else
    {
        item->animNumber = Objects[item->objectNumber].animIndex + BIG_RAT_ANIM_EMPTY;
        item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
        item->currentAnimState = BIG_RAT_STOP;
        item->goalAnimState = BIG_RAT_STOP;
    }
}

void BigRatControl(short itemNumber)
{
    if (!CreatureActive(itemNumber))
        return;

    ITEM_INFO* item;
    OBJECT_INFO* obj;
    CREATURE_INFO* big_rat;
    AI_INFO info;
    short head, angle;
    int WaterHeight;

    item = &g_Level.Items[itemNumber];
    obj = &Objects[item->objectNumber];
    big_rat = GetCreatureInfo(item);
    head = angle = 0;
    WaterHeight = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);

    if (item->hitPoints <= 0)
    {
        if (item->currentAnimState != BIG_RAT_LAND_DEATH && item->currentAnimState != BIG_RAT_WATER_DEATH)
        {
            if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
            {
                item->animNumber = obj->animIndex + BIG_RAT_ANIM_WATER_DEATH;
                item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
                item->currentAnimState = BIG_RAT_WATER_DEATH;
                item->goalAnimState = BIG_RAT_WATER_DEATH;
            }
            else
            {
                item->animNumber = obj->animIndex + BIG_RAT_ANIM_LAND_DEATH;
                item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
                item->currentAnimState = BIG_RAT_LAND_DEATH;
                item->goalAnimState = BIG_RAT_LAND_DEATH;
            }
        }

        if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
            CreatureFloat(itemNumber);
    }
    else
    {
        AI_INFO info;
        CreatureAIInfo(item, &info);

        if (info.ahead)
            head = info.angle;

        GetCreatureMood(item, &info, TIMID);
        CreatureMood(item, &info, TIMID);
        angle = CreatureTurn(item, big_rat->maximumTurn);

        if (item->aiBits & ALL_AIOBJ)
            GetAITarget(big_rat);
        else if (big_rat->hurtByLara)
            big_rat->enemy = LaraItem;

        if ((item->hitStatus || info.distance < BIG_RAT_ALERT_RANGE) || (TargetVisible(item, &info) && info.distance < BIG_RAT_VISIBILITY_RANGE))
        {
            if (!big_rat->alerted)
                big_rat->alerted = true;
            AlertAllGuards(itemNumber);
        }

        switch (item->currentAnimState)
        {
        case BIG_RAT_STOP:
            if (item->requiredAnimState)
                item->goalAnimState = item->requiredAnimState;
            else if (info.bite && info.distance < BIG_RAT_BITE_RANGE)
                item->goalAnimState = BIG_RAT_BITE_ATTACK;
            else
                item->goalAnimState = BIG_RAT_RUN;
            break;

        case BIG_RAT_RUN:
            big_rat->maximumTurn = BIG_RAT_RUN_TURN;

            if (RatIsInWater(item, big_rat))
            {
                item->requiredAnimState = BIG_RAT_SWIM;
                item->goalAnimState = BIG_RAT_SWIM;
                break;
            }

            if (info.ahead && (item->touchBits & BIG_RAT_TOUCH))
                item->goalAnimState = BIG_RAT_STOP;
            else if (info.bite && info.distance < BIG_RAT_CHARGE_RANGE)
                item->goalAnimState = BIG_RAT_CHARGE_ATTACK;
            else if (info.ahead && GetRandomControl() < BIG_RAT_POSE_CHANCE)
            {
                item->requiredAnimState = BIG_RAT_POSE;
                item->goalAnimState = BIG_RAT_STOP;
            }
            break;

        case BIG_RAT_BITE_ATTACK:
            if (!item->requiredAnimState && info.ahead && (item->touchBits & BIG_RAT_TOUCH))
            {
                CreatureEffect(item, &big_ratBite, DoBloodSplat);
                LaraItem->hitPoints -= BIG_RAT_BITE_DAMAGE;
                LaraItem->hitStatus = true;
                item->requiredAnimState = BIG_RAT_STOP;
            }
            break;

        case BIG_RAT_CHARGE_ATTACK:
            if (!item->requiredAnimState && info.ahead && (item->touchBits & BIG_RAT_TOUCH))
            {
                CreatureEffect(item, &big_ratBite, DoBloodSplat);
                LaraItem->hitPoints -= BIG_RAT_CHARGE_DAMAGE;
                LaraItem->hitStatus = true;
                item->requiredAnimState = BIG_RAT_RUN;
            }
            break;

        case BIG_RAT_POSE:
            if (big_rat->mood != BORED_MOOD || GetRandomControl() < BIG_RAT_POSE_CHANCE)
                item->goalAnimState = BIG_RAT_STOP;
            break;

        case BIG_RAT_SWIM:
            big_rat->maximumTurn = BIG_RAT_SWIM_TURN;

            if (!RatIsInWater(item, big_rat))
            {
                item->requiredAnimState = BIG_RAT_RUN;
                item->goalAnimState = BIG_RAT_RUN;
                break;
            }

            if (info.ahead && (item->touchBits & BIG_RAT_TOUCH))
                item->goalAnimState = BIG_RAT_SWIM_ATTACK;
            break;

        case BIG_RAT_SWIM_ATTACK:
            if (!item->requiredAnimState && info.ahead && (item->touchBits & BIG_RAT_TOUCH))
            {
                CreatureEffect(item, &big_ratBite, DoBloodSplat);
                LaraItem->hitPoints -= BIG_RAT_BITE_DAMAGE;
                LaraItem->hitStatus = true;
            }

            item->goalAnimState = BIG_RAT_SWIM;
            break;
        }

    }

    CreatureJoint(item, 0, head);
    CreatureAnimation(itemNumber, angle, 0);

    if (RatIsInWater(item, big_rat))
    {
        CreatureUnderwater(item, CLICK(0));
        item->pos.yPos = WaterHeight;
    }
    else
    {
        item->pos.yPos = item->floor;
    }
}