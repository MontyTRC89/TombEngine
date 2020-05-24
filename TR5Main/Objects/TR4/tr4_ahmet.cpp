#include "../newobjects.h"
#include "../../Game/control.h"
#include "../../Game/sphere.h"
#include "../../Game/effects.h"
#include "../../Game/effect2.h"
#include "../../Game/sound.h"
#include "../../Specific/setup.h"
#include "../../Game/Box.h"
#include "../../Specific/level.h"
#include "../../Game/misc.h"
#include "../../Game/lara.h"
#include "../../Game/people.h"

enum AHMET_STATE
{
    AHMET_EMPTY,
    AHMET_IDLE,
    AHMET_WALK,
    AHMET_RUN,
    AHMET_STAND_DUALATK,
    AHMET_JUMP_BITE,
    AHMET_JUMP_DUALATK,
    AHMET_DIE
};

#define AHMET_JUMP_ATK_ANIM 4
#define AHMET_START_JUMP_ANIM 7
#define AHMET_DIE_ANIM 10

#define AHMET_WALK_ANGLE ANGLE(5.0f)
#define AHMET_RUN_ANGLE ANGLE(8.0f)
#define AHMET_VIEW_ANGLE ANGLE(45.0f)
#define AHMET_ENEMY_ANGLE ANGLE(90.0f)
#define AHMET_AWARE_DISTANCE 0x100000
#define AHMET_IDLE_RANGE 0x190000
#define AHMET_RUN_RANGE 0x640000
#define AHMET_STAND_DUALATK_RANGE 0x718E4
#define AHMET_RIGHT_TOUCH 0xF00000
#define AHMET_LEFT_TOUCH 0x3C000
#define AHMET_HAND_DAMAGE 80
#define AHMET_JAW_DAMAGE 120

static BITE_INFO ahmetBiteLeft = { 0, 0, 0, 16 };
static BITE_INFO ahmetBiteRight = { 0, 0, 0, 22 };
static BITE_INFO ahmetBiteJaw = { 0, 0, 0, 11 };

static void AhmetHeavyTriggers(ITEM_INFO* item)
{
    FLOOR_INFO* floor;
    short room_number;

    room_number = item->roomNumber;
    floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room_number);
    GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
    TestTriggers(TriggerIndex, TRUE, FALSE);
}

static void TriggerAhmetDeathEffect(ITEM_INFO* item)
{
    if (!(Wibble & 7))
    {
        SPHERE* sphere;
        int meshCount;

        // cant be FALSE here because else it will be local space not world
        // because of that it cant be GetJointAbsPosition() !
        meshCount = GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
        sphere = &CreatureSpheres[(Wibble >> 3) & 1];

        for (int i = meshCount; i > 0; i--, sphere += 2)
            TriggerFireFlame(sphere->x, sphere->y, sphere->z, -1, 1);
    }

    // NOTE: fixed light below the ground with -STEP_L !
    TriggerDynamicLight(item->pos.xPos, (item->pos.yPos - STEP_SIZE), item->pos.zPos, 13, (GetRandomControl() & 0x3F) - 64, (GetRandomControl() & 0x1F) + 96, 0);
    SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->pos, NULL);
}

void InitialiseAhmet(short item_number)
{
    ITEM_INFO* item;
    item = &Items[item_number];

    InitialiseCreature(item_number);
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = Anims[item->animNumber].frameBase;
    item->goalAnimState = AHMET_IDLE;
    item->currentAnimState = AHMET_IDLE;
    item->itemFlags[0] = item->pos.xPos >> (WALL_SHIFT);
    item->itemFlags[1] = item->pos.yPos >> (WALL_SHIFT - 2);
    item->itemFlags[2] = item->pos.zPos >> (WALL_SHIFT);
}

void AhmetControl(short item_number)
{
    ITEM_INFO* item;
    CREATURE_INFO* ahmet;
    AI_INFO lara_info, info;
    short angle, head_y;

    if (!CreatureActive(item_number))
        return;

    item = &Items[item_number];
    if (item->triggerFlags == 1)
    {
        item->triggerFlags = 0;
        return;
    }

    ahmet = GetCreatureInfo(item);
    angle = 0;
    head_y = 0;

    if (item->hitPoints <= 0)
    {
        TriggerAhmetDeathEffect(item);

        if (item->currentAnimState == AHMET_DIE)
        {
            // dont clear it !
            if (item->frameNumber == Anims[item->animNumber].frameEnd)
            {
                item->collidable = FALSE; // NOTE: not exist in the original game, avoid wreid collision with lara...
                item->frameNumber = (Anims[item->animNumber].frameEnd - 1);
            }
        }
        else
        {
            item->animNumber = Objects[item->objectNumber].animIndex + AHMET_DIE_ANIM;
            item->frameNumber = Anims[item->animNumber].frameBase;
            item->currentAnimState = AHMET_DIE;
            item->goalAnimState = AHMET_DIE;
            Lara.generalPtr = (void*)item_number;
        }
    }
    else
    {
        if (item->aiBits & ALL_AIOBJ)
            GetAITarget(ahmet);

        CreatureAIInfo(item, &info);

        if (ahmet->enemy == LaraItem)
        {
            lara_info.angle = info.angle;
            lara_info.distance = info.distance;
        }
        else
        {
            int dx, dz;
            int ang;
            dx = LaraItem->pos.xPos - item->pos.xPos;
            dz = LaraItem->pos.zPos - item->pos.zPos;
            ang = phd_atan(dx, dz);
            lara_info.angle = ang - item->pos.yRot;
            lara_info.distance = SQUARE(dx) + SQUARE(dz);
        }

        GetCreatureMood(item, &info, TRUE);
        CreatureMood(item, &info, TRUE);

        angle = CreatureTurn(item, ahmet->maximumTurn);
        ahmet->enemy = LaraItem;

        if (lara_info.distance < AHMET_AWARE_DISTANCE || item->hitStatus || TargetVisible(item, &lara_info))
            AlertAllGuards(item_number);

        if (info.ahead)
            head_y = info.angle;

        switch (item->currentAnimState)
        {
        case AHMET_IDLE:
            ahmet->maximumTurn = 0;
            ahmet->flags = 0;

            if (item->aiBits & GUARD)
            {
                head_y = AIGuard(ahmet);
                item->goalAnimState = AHMET_IDLE;
            }
            else if (item->aiBits & PATROL1)
            {
                item->goalAnimState = AHMET_WALK;
                head_y = 0;
            }
            else if (ahmet->mood == ATTACK_MOOD && ahmet->mood != ESCAPE_MOOD)
            {
                if (info.bite && info.distance < AHMET_STAND_DUALATK_RANGE)
                {
                    item->goalAnimState = AHMET_STAND_DUALATK;
                }
                else if ((info.angle >= AHMET_VIEW_ANGLE || info.angle <= -AHMET_VIEW_ANGLE) || info.distance >= AHMET_IDLE_RANGE)
                {
                    if (item->requiredAnimState)
                    {
                        item->goalAnimState = item->requiredAnimState;
                    }
                    else
                    {
                        if (!info.ahead || info.distance >= AHMET_RUN_RANGE)
                            item->goalAnimState = AHMET_RUN;
                        else
                            item->goalAnimState = AHMET_WALK;
                    }
                }
                else if (GetRandomControl() & 1)
                {
                    item->goalAnimState = AHMET_JUMP_BITE;
                }
                else
                {
                    item->goalAnimState = AHMET_JUMP_DUALATK;
                }
            }
            else
            {
                if (Lara.target == item || !info.ahead)
                    item->goalAnimState = AHMET_RUN;
                else
                    item->goalAnimState = AHMET_IDLE;
            }
            break;
        case AHMET_WALK:
            ahmet->maximumTurn = AHMET_WALK_ANGLE;

            if (item->aiBits & PATROL1)
            {
                item->goalAnimState = AHMET_WALK;
                head_y = 0;
            }
            else if (info.bite && info.distance < AHMET_IDLE_RANGE)
            {
                item->goalAnimState = AHMET_IDLE;
            }
            else if (ahmet->mood == ESCAPE_MOOD || info.distance > AHMET_RUN_RANGE || !info.ahead || (info.enemyFacing > -AHMET_ENEMY_ANGLE || info.enemyFacing < AHMET_ENEMY_ANGLE))
            {
                item->goalAnimState = AHMET_RUN;
            }
            break;
        case AHMET_RUN:
            ahmet->maximumTurn = AHMET_RUN_ANGLE;
            ahmet->flags = 0;

            if (item->aiBits & GUARD || (ahmet->mood == BORED_MOOD || ahmet->mood == ESCAPE_MOOD) && (Lara.target == item && info.ahead) || (info.bite && info.distance < AHMET_IDLE_RANGE))
                item->goalAnimState = AHMET_IDLE;
            else if (info.ahead && info.distance < AHMET_RUN_RANGE && (info.enemyFacing < -AHMET_ENEMY_ANGLE || info.enemyFacing > AHMET_ENEMY_ANGLE))
                item->goalAnimState = AHMET_WALK;
            break;
        case AHMET_STAND_DUALATK:
            ahmet->maximumTurn = 0;

            if (abs(info.angle) >= 910)
            {
                if (info.angle >= 0)
                    item->pos.yRot += 910;
                else
                    item->pos.yRot -= 910;
            }
            else
            {
                item->pos.yRot += info.angle;
            }

            if (!(ahmet->flags & 1) && item->frameNumber > (Anims[item->animNumber].frameBase + 7) && (item->touchBits & AHMET_LEFT_TOUCH))
            {
                LaraItem->hitStatus = TRUE;
                LaraItem->hitPoints -= AHMET_HAND_DAMAGE;
                CreatureEffect2(item, &ahmetBiteLeft, 10, -1, DoBloodSplat);
                ahmet->flags |= 1;
            }
            else if (!(ahmet->flags & 2) && item->frameNumber > (Anims[item->animNumber].frameBase + 32) && (item->touchBits & AHMET_RIGHT_TOUCH))
            {
                LaraItem->hitStatus = TRUE;
                LaraItem->hitPoints -= AHMET_HAND_DAMAGE;
                CreatureEffect2(item, &ahmetBiteRight, 10, -1, DoBloodSplat);
                ahmet->flags |= 2;
            }
            break;
        case AHMET_JUMP_BITE:
            ahmet->maximumTurn = 0;

            if (item->animNumber == Objects[item->objectNumber].animIndex + AHMET_START_JUMP_ANIM)
            {
                if (abs(info.angle) >= 910)
                {
                    if (info.angle >= 0)
                        item->pos.yRot += 910;
                    else
                        item->pos.yRot -= 910;
                }
                else
                {
                    item->pos.yRot += info.angle;
                }
            }
            else
            {
                if (!(ahmet->flags & 1) && item->animNumber == Objects[item->objectNumber].animIndex + AHMET_JUMP_ATK_ANIM)
                {
                    if (item->frameNumber > (Anims[item->animNumber].frameBase + 11) && (item->touchBits & AHMET_LEFT_TOUCH))
                    {
                        LaraItem->hitStatus = TRUE;
                        LaraItem->hitPoints -= AHMET_JAW_DAMAGE;
                        CreatureEffect2(item, &ahmetBiteJaw, 10, -1, DoBloodSplat);
                        ahmet->flags |= 1;
                    }
                }
            }
            break;
        case AHMET_JUMP_DUALATK:
            ahmet->maximumTurn = 0;

            if (item->animNumber == Objects[item->objectNumber].animIndex + AHMET_START_JUMP_ANIM)
            {
                if (abs(info.angle) >= 910)
                {
                    if (info.angle >= 0)
                        item->pos.yRot += 910;
                    else
                        item->pos.yRot -= 910;
                }
                else
                {
                    item->pos.yRot += info.angle;
                }
            }
            else
            {
                if (!(ahmet->flags & 1) && item->frameNumber > (Anims[item->animNumber].frameBase + 14) && (item->touchBits & AHMET_LEFT_TOUCH))
                {
                    LaraItem->hitStatus = TRUE;
                    LaraItem->hitPoints -= AHMET_HAND_DAMAGE;
                    CreatureEffect2(item, &ahmetBiteLeft, 10, -1, DoBloodSplat);
                    ahmet->flags |= 1;
                }
                else if (!(ahmet->flags & 2) && item->frameNumber > (Anims[item->animNumber].frameBase + 22) && (item->touchBits & AHMET_RIGHT_TOUCH))
                {
                    LaraItem->hitStatus = TRUE;
                    LaraItem->hitPoints -= AHMET_HAND_DAMAGE;
                    CreatureEffect2(item, &ahmetBiteRight, 10, -1, DoBloodSplat);
                    ahmet->flags |= 2;
                }
            }
            break;
        }
    }

    CreatureTilt(item, 0);
    CreatureJoint(item, 0, head_y);
    AhmetHeavyTriggers(item);
    CreatureAnimation(item_number, angle, 0);
}
