#include "framework.h"
#include "tr4_crocodile.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/people.h"
#include "Game/items.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include "Game/misc.h"
#include "Game/itemdata/creature_info.h"
#include "Game/collision/collide_room.h"

enum CROCODILE_STATE
{
    CROC_EMPTY,
    CROC_IDLE,
    CROC_RUN,
    CROC_WALK,
    CROC_HIT,
    CROC_ATK,
    CROC_EMPTY1,
    CROC_DIE,
    WCROC_SWIM,
    WCROC_ATK,
    WCROC_DIE,
};

#define CROC_WALK_ANGLE ANGLE(3.0f)
#define CROC_SWIM_ANGLE ANGLE(3.0f)
#define CROC_RUN_ANGLE ANGLE(5.0f)

constexpr auto CROC_ANIM_IDLE = 0;
constexpr auto CROC_ANIM_DIE = 11;
constexpr auto CROC_ANIM_SWIM = 12;
constexpr auto CROC_ANIM_WDIE = 16;
constexpr auto CROC_ALERT_RANGE = SQUARE(SECTOR(1) + CLICK(2));
constexpr auto CROC_VISIBILITY_RANGE = SQUARE(SECTOR(5));
constexpr auto CROC_RUN_RANGE = SQUARE(SECTOR(1));
constexpr auto CROC_MAXRUN_RANGE = SQUARE(SECTOR(1) + CLICK(2));
constexpr auto CROC_ATTACK_RANGE = SQUARE(CLICK(3)); // NOTE: TR4 is CLICK(3), but the crocodile not go near lara to do damage in certain case !
constexpr auto CROC_SWIM_SPEED = 16;
constexpr auto CROC_TOUCHBITS = 768;
constexpr auto CROC_DAMAGE = 120;
static BITE_INFO crocBite = { 0, -100, 500, 9 };

// crocodile mode (land or swim) transition anim
constexpr auto CROC_ANIM_SWIM_MODE = 17;
constexpr auto CROC_ANIM_LAND_MODE = 18;

static bool CrocodileIsInWater(ITEM_INFO* item, CREATURE_INFO* crocodile)
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
        crocodile->LOT.step = SECTOR(20);
        crocodile->LOT.drop = -SECTOR(20);
        crocodile->LOT.fly = CROC_SWIM_SPEED;
        return true;
    }
    else
    {
        crocodile->LOT.step = CLICK(1);
        crocodile->LOT.drop = -CLICK(1);
        crocodile->LOT.fly = NO_FLYING;
        return false;
    }
}

void InitialiseCrocodile(short itemNumber)
{
    ITEM_INFO* item = &g_Level.Items[itemNumber];
    InitialiseCreature(itemNumber);

    // if the room is a "water room"
    if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
    {
        item->animNumber = Objects[item->objectNumber].animIndex + CROC_ANIM_SWIM;
        item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
        item->currentAnimState = WCROC_SWIM;
        item->goalAnimState = WCROC_SWIM;
    }
    // then it's a "ground room"
    else
    {
        item->animNumber = Objects[item->objectNumber].animIndex + CROC_ANIM_IDLE;
        item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
        item->currentAnimState = CROC_IDLE;
        item->goalAnimState = CROC_IDLE;
    }
}

void CrocodileControl(short itemNumber)
{
    if (!CreatureActive(itemNumber))
        return;

    ITEM_INFO* item;
    OBJECT_INFO* obj;
    CREATURE_INFO* crocodile;
    AI_INFO info;
    OBJECT_BONES boneRot;
    short angle;
    short boneAngle;

    item = &g_Level.Items[itemNumber];
    obj = &Objects[item->objectNumber];
    crocodile = GetCreatureInfo(item);
    angle = 0;
    boneAngle = 0;

    if (item->hitPoints <= 0)
    {
        angle = 0;
        boneAngle = 0;

        if (item->currentAnimState != CROC_DIE && item->currentAnimState != WCROC_DIE)
        {
            // water
            if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
            {
                item->animNumber = obj->animIndex + CROC_ANIM_WDIE;
                item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
                item->currentAnimState = WCROC_DIE;
                item->goalAnimState = WCROC_DIE;
            }
            // land
            else
            {
                item->animNumber = obj->animIndex + CROC_ANIM_DIE;
                item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
                item->currentAnimState = CROC_DIE;
                item->goalAnimState = CROC_DIE;
            }
        }

        // creature in water are floating after death.
        if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
            CreatureFloat(itemNumber);
    }
    else
    {
        if (item->aiBits & ALL_AIOBJ)
            GetAITarget(crocodile);
        else if (crocodile->hurtByLara)
            crocodile->enemy = LaraItem;

        CreatureAIInfo(item, &info);
        GetCreatureMood(item, &info, VIOLENT);
        CreatureMood(item, &info, VIOLENT);
        angle = CreatureTurn(item, crocodile->maximumTurn);

        if ((item->hitStatus || info.distance < CROC_ALERT_RANGE) || (TargetVisible(item, &info) && info.distance < CROC_VISIBILITY_RANGE))
        {
            if (!crocodile->alerted)
                crocodile->alerted = true;
            AlertAllGuards(itemNumber);
        }

        boneAngle = angle * 4;
        switch (item->currentAnimState)
        {
        case CROC_IDLE:
            crocodile->maximumTurn = 0;

            if (item->aiBits & GUARD)
            {
                boneAngle = item->itemFlags[0];
                item->goalAnimState = CROC_IDLE;
                item->itemFlags[0] = item->itemFlags[1] + boneAngle;

                if (!(GetRandomControl() & 0x1F))
                {
                    if (GetRandomControl() & 1)
                        item->itemFlags[1] = 0;
                    else
                        item->itemFlags[1] = (GetRandomControl() & 1) != 0 ? 12 : -12;
                }

                if (item->itemFlags[0] < -1024)
                    item->itemFlags[0] = -1024;
                else if (item->itemFlags[0] > 1024)
                    item->itemFlags[0] = 1024;
            }
            else if (info.bite && info.distance < CROC_ATTACK_RANGE)
            {
                item->goalAnimState = CROC_ATK;
            }
            else
            {
                if (info.ahead && info.distance < CROC_RUN_RANGE)
                    item->goalAnimState = CROC_WALK;
                else
                    item->goalAnimState = CROC_RUN;
            }
            break;
        case CROC_WALK:
            crocodile->maximumTurn = CROC_WALK_ANGLE;

            // land to water transition:
            if (CrocodileIsInWater(item, crocodile))
            {
                item->requiredAnimState = WCROC_SWIM;
                item->goalAnimState = WCROC_SWIM;
                break;
            }

            if (item->requiredAnimState)
                item->goalAnimState = item->requiredAnimState;
            else if (info.bite && info.distance < CROC_ATTACK_RANGE)
                item->goalAnimState = CROC_IDLE;
            else if (!info.ahead || info.distance > CROC_MAXRUN_RANGE)
                item->goalAnimState = CROC_RUN;
            break;
        case CROC_RUN:
            crocodile->maximumTurn = CROC_RUN_ANGLE;

            // land to water transition:
            if (CrocodileIsInWater(item, crocodile))
            {
                item->requiredAnimState = CROC_WALK;
                item->goalAnimState = CROC_WALK;
                break;
            }

            if (item->requiredAnimState)
                item->goalAnimState = item->requiredAnimState;
            else if (info.bite && info.distance < CROC_ATTACK_RANGE)
                item->goalAnimState = CROC_IDLE;
            else if (info.ahead && info.distance < CROC_RUN_RANGE)
                item->goalAnimState = CROC_WALK;
            break;
        case CROC_ATK:
            if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
                item->requiredAnimState = 0;

            if (info.bite && (item->touchBits & CROC_TOUCHBITS))
            {
                if (!item->requiredAnimState)
                {
                    CreatureEffect2(item, &crocBite, 10, -1, DoBloodSplat);
                    LaraItem->hitPoints -= CROC_DAMAGE;
                    LaraItem->hitStatus = true;
                    item->requiredAnimState = CROC_IDLE;
                }
            }
            else
            {
                item->goalAnimState = CROC_IDLE;
            }
            break;
        case WCROC_SWIM:
            crocodile->maximumTurn = CROC_SWIM_ANGLE;

            // water to land transition:
            if (!CrocodileIsInWater(item, crocodile))
            {
                item->animNumber = obj->animIndex + CROC_ANIM_LAND_MODE;
                item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
                item->requiredAnimState = CROC_WALK;
                item->currentAnimState = CROC_WALK;
                item->goalAnimState = CROC_WALK;
                break;
            }

            if (item->requiredAnimState)
            {
                item->goalAnimState = item->requiredAnimState;
            }
            else if (info.bite)
            {
                if (item->touchBits & 768)
                    item->goalAnimState = WCROC_ATK;
            }
            break;
        case WCROC_ATK:
            if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
                item->requiredAnimState = CROC_EMPTY;

            if (info.bite && (item->touchBits & CROC_TOUCHBITS))
            {
                if (!item->requiredAnimState)
                {
                    CreatureEffect2(item, &crocBite, 10, -1, DoBloodSplat);
                    LaraItem->hitPoints -= CROC_DAMAGE;
                    LaraItem->hitStatus = true;
                    item->requiredAnimState = WCROC_SWIM;
                }
            }
            else
            {
                item->goalAnimState = WCROC_SWIM;
            }
            break;
        }
    }

    if (item->currentAnimState == CROC_IDLE || item->currentAnimState == CROC_ATK || item->currentAnimState == WCROC_ATK)
    {
        boneRot.bone0 = info.angle;
        boneRot.bone1 = info.angle;
        boneRot.bone2 = 0;
        boneRot.bone3 = 0;
    }
    else
    {
        boneRot.bone0 = boneAngle;
        boneRot.bone1 = boneAngle;
        boneRot.bone2 = -boneAngle;
        boneRot.bone3 = -boneAngle;
    }

    CreatureTilt(item, 0);
    CreatureJoint(item, 0, boneRot.bone0);
    CreatureJoint(item, 1, boneRot.bone1);
    CreatureJoint(item, 2, boneRot.bone2);
    CreatureJoint(item, 3, boneRot.bone3);

    if (item->currentAnimState < WCROC_SWIM)
        CalcItemToFloorRotation(item, 2);

    CreatureAnimation(itemNumber, angle, 0);

    if (item->currentAnimState >= WCROC_SWIM && item->currentAnimState <= WCROC_DIE)
        CreatureUnderwater(item, CLICK(1));
    else
        CreatureUnderwater(item, CLICK(0));
}