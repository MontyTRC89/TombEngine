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
    info.x = item->Position.xPos;
    info.y = item->Position.yPos;
    info.z = item->Position.zPos;
    info.roomNumber = item->RoomNumber;
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
    if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
    {
        item->AnimNumber = Objects[item->ObjectNumber].animIndex + CROC_ANIM_SWIM;
        item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
        item->ActiveState = WCROC_SWIM;
        item->TargetState = WCROC_SWIM;
    }
    // then it's a "ground room"
    else
    {
        item->AnimNumber = Objects[item->ObjectNumber].animIndex + CROC_ANIM_IDLE;
        item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
        item->ActiveState = CROC_IDLE;
        item->TargetState = CROC_IDLE;
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
    obj = &Objects[item->ObjectNumber];
    crocodile = GetCreatureInfo(item);
    angle = 0;
    boneAngle = 0;

    if (item->HitPoints <= 0)
    {
        angle = 0;
        boneAngle = 0;

        if (item->ActiveState != CROC_DIE && item->ActiveState != WCROC_DIE)
        {
            // water
            if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
            {
                item->AnimNumber = obj->animIndex + CROC_ANIM_WDIE;
                item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
                item->ActiveState = WCROC_DIE;
                item->TargetState = WCROC_DIE;
            }
            // land
            else
            {
                item->AnimNumber = obj->animIndex + CROC_ANIM_DIE;
                item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
                item->ActiveState = CROC_DIE;
                item->TargetState = CROC_DIE;
            }
        }

        // creature in water are floating after death.
        if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
            CreatureFloat(itemNumber);
    }
    else
    {
        if (item->AIBits & ALL_AIOBJ)
            GetAITarget(crocodile);
        else if (crocodile->hurtByLara)
            crocodile->enemy = LaraItem;

        CreatureAIInfo(item, &info);
        GetCreatureMood(item, &info, VIOLENT);
        CreatureMood(item, &info, VIOLENT);
        angle = CreatureTurn(item, crocodile->maximumTurn);

        if ((item->HitStatus || info.distance < CROC_ALERT_RANGE) || (TargetVisible(item, &info) && info.distance < CROC_VISIBILITY_RANGE))
        {
            if (!crocodile->alerted)
                crocodile->alerted = true;
            AlertAllGuards(itemNumber);
        }

        boneAngle = angle * 4;
        switch (item->ActiveState)
        {
        case CROC_IDLE:
            crocodile->maximumTurn = 0;

            if (item->AIBits & GUARD)
            {
                boneAngle = item->ItemFlags[0];
                item->TargetState = CROC_IDLE;
                item->ItemFlags[0] = item->ItemFlags[1] + boneAngle;

                if (!(GetRandomControl() & 0x1F))
                {
                    if (GetRandomControl() & 1)
                        item->ItemFlags[1] = 0;
                    else
                        item->ItemFlags[1] = (GetRandomControl() & 1) != 0 ? 12 : -12;
                }

                if (item->ItemFlags[0] < -1024)
                    item->ItemFlags[0] = -1024;
                else if (item->ItemFlags[0] > 1024)
                    item->ItemFlags[0] = 1024;
            }
            else if (info.bite && info.distance < CROC_ATTACK_RANGE)
            {
                item->TargetState = CROC_ATK;
            }
            else
            {
                if (info.ahead && info.distance < CROC_RUN_RANGE)
                    item->TargetState = CROC_WALK;
                else
                    item->TargetState = CROC_RUN;
            }
            break;
        case CROC_WALK:
            crocodile->maximumTurn = CROC_WALK_ANGLE;

            // land to water transition:
            if (CrocodileIsInWater(item, crocodile))
            {
                item->RequiredState = WCROC_SWIM;
                item->TargetState = WCROC_SWIM;
                break;
            }

            if (item->RequiredState)
                item->TargetState = item->RequiredState;
            else if (info.bite && info.distance < CROC_ATTACK_RANGE)
                item->TargetState = CROC_IDLE;
            else if (!info.ahead || info.distance > CROC_MAXRUN_RANGE)
                item->TargetState = CROC_RUN;
            break;
        case CROC_RUN:
            crocodile->maximumTurn = CROC_RUN_ANGLE;

            // land to water transition:
            if (CrocodileIsInWater(item, crocodile))
            {
                item->RequiredState = CROC_WALK;
                item->TargetState = CROC_WALK;
                break;
            }

            if (item->RequiredState)
                item->TargetState = item->RequiredState;
            else if (info.bite && info.distance < CROC_ATTACK_RANGE)
                item->TargetState = CROC_IDLE;
            else if (info.ahead && info.distance < CROC_RUN_RANGE)
                item->TargetState = CROC_WALK;
            break;
        case CROC_ATK:
            if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
                item->RequiredState = 0;

            if (info.bite && (item->TouchBits & CROC_TOUCHBITS))
            {
                if (!item->RequiredState)
                {
                    CreatureEffect2(item, &crocBite, 10, -1, DoBloodSplat);
                    LaraItem->HitPoints -= CROC_DAMAGE;
                    LaraItem->HitStatus = true;
                    item->RequiredState = CROC_IDLE;
                }
            }
            else
            {
                item->TargetState = CROC_IDLE;
            }
            break;
        case WCROC_SWIM:
            crocodile->maximumTurn = CROC_SWIM_ANGLE;

            // water to land transition:
            if (!CrocodileIsInWater(item, crocodile))
            {
                item->AnimNumber = obj->animIndex + CROC_ANIM_LAND_MODE;
                item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
                item->RequiredState = CROC_WALK;
                item->ActiveState = CROC_WALK;
                item->TargetState = CROC_WALK;
                break;
            }

            if (item->RequiredState)
            {
                item->TargetState = item->RequiredState;
            }
            else if (info.bite)
            {
                if (item->TouchBits & 768)
                    item->TargetState = WCROC_ATK;
            }
            break;
        case WCROC_ATK:
            if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
                item->RequiredState = CROC_EMPTY;

            if (info.bite && (item->TouchBits & CROC_TOUCHBITS))
            {
                if (!item->RequiredState)
                {
                    CreatureEffect2(item, &crocBite, 10, -1, DoBloodSplat);
                    LaraItem->HitPoints -= CROC_DAMAGE;
                    LaraItem->HitStatus = true;
                    item->RequiredState = WCROC_SWIM;
                }
            }
            else
            {
                item->TargetState = WCROC_SWIM;
            }
            break;
        }
    }

    if (item->ActiveState == CROC_IDLE || item->ActiveState == CROC_ATK || item->ActiveState == WCROC_ATK)
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

    if (item->ActiveState < WCROC_SWIM)
        CalcItemToFloorRotation(item, 2);

    CreatureAnimation(itemNumber, angle, 0);

    if (item->ActiveState >= WCROC_SWIM && item->ActiveState <= WCROC_DIE)
        CreatureUnderwater(item, CLICK(1));
    else
        CreatureUnderwater(item, CLICK(0));
}