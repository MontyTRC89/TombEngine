#include "framework.h"
#include "tr4_ahmet.h"
#include "Game/control/control.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/effects/weather.h"
#include "Sound/sound.h"
#include "Specific/setup.h"
#include "Game/control/box.h"
#include "Specific/level.h"
#include "Game/misc.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Game/items.h"
#include "Game/control/lot.h"
#include "Game/itemdata/creature_info.h"

using namespace TEN::Effects::Environment;

namespace TEN::Entities::TR4
{
    enum AhmetState
    {
        STATE_AHMET_EMPTY,
        STATE_AHMET_IDLE,
        STATE_AHMET_WALK,
        STATE_AHMET_RUN,
        STATE_AHMET_STAND_DUALATK,
        STATE_AHMET_JUMP_BITE,
        STATE_AHMET_JUMP_DUALATK,
        STATE_AHMET_DIE
    };

    constexpr auto AHMET_JUMP_ATK_ANIM = 4;
    constexpr auto AHMET_START_JUMP_ANIM = 7;
    constexpr auto AHMET_DIE_ANIM = 10;

    constexpr auto AHMET_WALK_ANGLE = 5 * ONE_DEGREE;
    constexpr auto AHMET_RUN_ANGLE = 8 * ONE_DEGREE;
    constexpr auto AHMET_VIEW_ANGLE = 45 * ONE_DEGREE;
    constexpr auto AHMET_ENEMY_ANGLE = 90 * ONE_DEGREE;
    constexpr auto AHMET_AWARE_DISTANCE = SQUARE(1024);
    constexpr auto AHMET_IDLE_RANGE = SQUARE(1280);
    constexpr auto AHMET_RUN_RANGE = SQUARE(2560);
    constexpr auto AHMET_STAND_DUALATK_RANGE = SQUARE(682);
    constexpr auto AHMET_RIGHT_TOUCH = 0xF00000;
    constexpr auto AHMET_LEFT_TOUCH = 0x3C000;
    constexpr auto AHMET_HAND_DAMAGE = 80;
    constexpr auto AHMET_JAW_DAMAGE = 120;

    BITE_INFO ahmetBiteLeft = { 0, 0, 0, 16 };
    BITE_INFO ahmetBiteRight = { 0, 0, 0, 22 };
    BITE_INFO ahmetBiteJaw = { 0, 0, 0, 11 };

    static void AhmetHeavyTriggers(ITEM_INFO* item)
    {
        TestTriggers(item, true);
    }

    static void TriggerAhmetDeathEffect(ITEM_INFO* item)
    {
		// HACK: Using CreatureSpheres here in release mode results in total mess-up
		// of LaraSpheres, which looks in game as ghost Lara fire silhouette.
		// Later both CreatureSpheres and LaraSpheres globals should be eradicated.

		static SPHERE spheres[MAX_SPHERES] = {};

        if (!(Wibble & 7))
        {
			int meshCount = GetSpheres(item, spheres, SPHERES_SPACE_WORLD, Matrix::Identity);
            auto sphere = &spheres[(Wibble / 8) & 1];

            for (int i = meshCount; i > 0; i--, sphere += 2)
                TriggerFireFlame(sphere->x, sphere->y, sphere->z, -1, 1);
        }

        // NOTE: fixed light below the ground with -STEP_L !
        TriggerDynamicLight(item->Position.xPos, (item->Position.yPos - STEP_SIZE), item->Position.zPos, 13, (GetRandomControl() & 0x3F) - 64, (GetRandomControl() & 0x1F) + 96, 0);
        SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Position, NULL);
    }

    void InitialiseAhmet(short itemNumber)
    {
        ITEM_INFO* item;
        item = &g_Level.Items[itemNumber];

        InitialiseCreature(itemNumber);
        item->AnimNumber = Objects[item->ObjectNumber].animIndex;
        item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
        item->TargetState = STATE_AHMET_IDLE;
        item->ActiveState = STATE_AHMET_IDLE;
        item->ItemFlags[0] = item->Position.xPos / SECTOR(1);
        item->ItemFlags[1] = item->Position.yPos * 4 / SECTOR(1);
        item->ItemFlags[2] = item->Position.zPos / SECTOR(1);
    }

    void AhmetControl(short itemNumber)
    {
        if (!CreatureActive(itemNumber))
            return;

        ITEM_INFO* item;
        CREATURE_INFO* ahmet;
        AI_INFO lara_info, info;
        short angle, head_y;

        item = &g_Level.Items[itemNumber];
        if (item->TriggerFlags == 1)
        {
            item->TriggerFlags = 0;
            return;
        }

        ahmet = GetCreatureInfo(item);
        angle = 0;
        head_y = 0;

        if (item->HitPoints <= 0)
        {
            if (item->ActiveState == STATE_AHMET_DIE)
            {
                // dont clear it !
                if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
                {
                    item->Collidable = false; // NOTE: not exist in the original game, avoid wreid collision with lara...
                    item->FrameNumber = (g_Level.Anims[item->AnimNumber].frameEnd - 1);
                }
            }
            else
            {
                item->AnimNumber = Objects[item->ObjectNumber].animIndex + AHMET_DIE_ANIM;
                item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
                item->ActiveState = STATE_AHMET_DIE;
                item->TargetState = STATE_AHMET_DIE;
                Lara.interactedItem = itemNumber;
            }

            TriggerAhmetDeathEffect(item);
        }
        else
        {
            if (item->AIBits & ALL_AIOBJ)
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
                dx = LaraItem->Position.xPos - item->Position.xPos;
                dz = LaraItem->Position.zPos - item->Position.zPos;
                ang = phd_atan(dx, dz);
                lara_info.angle = ang - item->Position.yRot;
                lara_info.distance = SQUARE(dx) + SQUARE(dz);
            }

            GetCreatureMood(item, &info, TRUE);
            CreatureMood(item, &info, TRUE);

            angle = CreatureTurn(item, ahmet->maximumTurn);
            ahmet->enemy = LaraItem;

            if (lara_info.distance < AHMET_AWARE_DISTANCE
                || item->HitStatus
                || TargetVisible(item, &lara_info))
                AlertAllGuards(itemNumber);

            if (info.ahead)
                head_y = info.angle;

            switch (item->ActiveState)
            {
            case STATE_AHMET_IDLE:
                ahmet->maximumTurn = 0;
                ahmet->flags = 0;

                if (item->AIBits & GUARD)
                {
                    head_y = AIGuard(ahmet);
                    item->TargetState = STATE_AHMET_IDLE;
                }
                else if (item->AIBits & PATROL1)
                {
                    item->TargetState = STATE_AHMET_WALK;
                    head_y = 0;
                }
                else if (ahmet->mood == ATTACK_MOOD && ahmet->mood != ESCAPE_MOOD)
                {
                    if (info.bite && info.distance < AHMET_STAND_DUALATK_RANGE)
                    {
                        item->TargetState = STATE_AHMET_STAND_DUALATK;
                    }
                    else if ((info.angle >= AHMET_VIEW_ANGLE || info.angle <= -AHMET_VIEW_ANGLE) || info.distance >= AHMET_IDLE_RANGE)
                    {
                        if (item->RequiredState)
                        {
                            item->TargetState = item->RequiredState;
                        }
                        else
                        {
                            if (!info.ahead || info.distance >= AHMET_RUN_RANGE)
                                item->TargetState = STATE_AHMET_RUN;
                            else
                                item->TargetState = STATE_AHMET_WALK;
                        }
                    }
                    else if (GetRandomControl() & 1)
                    {
                        item->TargetState = STATE_AHMET_JUMP_BITE;
                    }
                    else
                    {
                        item->TargetState = STATE_AHMET_JUMP_DUALATK;
                    }
                }
                else
                {
                    if (Lara.target == item || !info.ahead)
                        item->TargetState = STATE_AHMET_RUN;
                    else
                        item->TargetState = STATE_AHMET_IDLE;
                }
                break;

            case STATE_AHMET_WALK:
                ahmet->maximumTurn = AHMET_WALK_ANGLE;

                if (item->AIBits & PATROL1)
                {
                    item->TargetState = STATE_AHMET_WALK;
                    head_y = 0;
                }
                else if (info.bite && info.distance < AHMET_IDLE_RANGE)
                {
                    item->TargetState = STATE_AHMET_IDLE;
                }
                else if (ahmet->mood == ESCAPE_MOOD || info.distance > AHMET_RUN_RANGE || !info.ahead || (info.enemyFacing > -AHMET_ENEMY_ANGLE || info.enemyFacing < AHMET_ENEMY_ANGLE))
                {
                    item->TargetState = STATE_AHMET_RUN;
                }
                break;

            case STATE_AHMET_RUN:
                ahmet->maximumTurn = AHMET_RUN_ANGLE;
                ahmet->flags = 0;

                if (item->AIBits & GUARD || (ahmet->mood == BORED_MOOD || ahmet->mood == ESCAPE_MOOD) && (Lara.target == item && info.ahead) || (info.bite && info.distance < AHMET_IDLE_RANGE))
                    item->TargetState = STATE_AHMET_IDLE;
                else if (info.ahead && info.distance < AHMET_RUN_RANGE && (info.enemyFacing < -AHMET_ENEMY_ANGLE || info.enemyFacing > AHMET_ENEMY_ANGLE))
                    item->TargetState = STATE_AHMET_WALK;
                break;

            case STATE_AHMET_STAND_DUALATK:
                ahmet->maximumTurn = 0;

                if (abs(info.angle) >= 910)
                {
                    if (info.angle >= 0)
                        item->Position.yRot += 910;
                    else
                        item->Position.yRot -= 910;
                }
                else
                {
                    item->Position.yRot += info.angle;
                }

                if (!(ahmet->flags & 1) && item->FrameNumber > (g_Level.Anims[item->AnimNumber].frameBase + 7) && (item->TouchBits & AHMET_LEFT_TOUCH))
                {
                    LaraItem->HitStatus = true;
                    LaraItem->HitPoints -= AHMET_HAND_DAMAGE;
                    CreatureEffect2(item, &ahmetBiteLeft, 10, -1, DoBloodSplat);
                    ahmet->flags |= 1;
                }
                else if (!(ahmet->flags & 2) && item->FrameNumber > (g_Level.Anims[item->AnimNumber].frameBase + 32) && (item->TouchBits & AHMET_RIGHT_TOUCH))
                {
                    LaraItem->HitStatus = true;
                    LaraItem->HitPoints -= AHMET_HAND_DAMAGE;
                    CreatureEffect2(item, &ahmetBiteRight, 10, -1, DoBloodSplat);
                    ahmet->flags |= 2;
                }
                break;

            case STATE_AHMET_JUMP_BITE:
                ahmet->maximumTurn = 0;

                if (item->AnimNumber == Objects[item->ObjectNumber].animIndex + AHMET_START_JUMP_ANIM)
                {
                    if (abs(info.angle) >= 910)
                    {
                        if (info.angle >= 0)
                            item->Position.yRot += 910;
                        else
                            item->Position.yRot -= 910;
                    }
                    else
                    {
                        item->Position.yRot += info.angle;
                    }
                }
                else
                {
                    if (!(ahmet->flags & 1) && item->AnimNumber == Objects[item->ObjectNumber].animIndex + AHMET_JUMP_ATK_ANIM)
                    {
                        if (item->FrameNumber > (g_Level.Anims[item->AnimNumber].frameBase + 11) && (item->TouchBits & AHMET_LEFT_TOUCH))
                        {
                            LaraItem->HitStatus = true;
                            LaraItem->HitPoints -= AHMET_JAW_DAMAGE;
                            CreatureEffect2(item, &ahmetBiteJaw, 10, -1, DoBloodSplat);
                            ahmet->flags |= 1;
                        }
                    }
                }
                break;

            case STATE_AHMET_JUMP_DUALATK:
                ahmet->maximumTurn = 0;

                if (item->AnimNumber == Objects[item->ObjectNumber].animIndex + AHMET_START_JUMP_ANIM)
                {
                    if (abs(info.angle) >= 910)
                    {
                        if (info.angle >= 0)
                            item->Position.yRot += 910;
                        else
                            item->Position.yRot -= 910;
                    }
                    else
                    {
                        item->Position.yRot += info.angle;
                    }
                }
                else
                {
                    if (!(ahmet->flags & 1) && item->FrameNumber > (g_Level.Anims[item->AnimNumber].frameBase + 14) && (item->TouchBits & AHMET_LEFT_TOUCH))
                    {
                        LaraItem->HitStatus = true;
                        LaraItem->HitPoints -= AHMET_HAND_DAMAGE;
                        CreatureEffect2(item, &ahmetBiteLeft, 10, -1, DoBloodSplat);
                        ahmet->flags |= 1;
                    }
                    else if (!(ahmet->flags & 2) && item->FrameNumber > (g_Level.Anims[item->AnimNumber].frameBase + 22) && (item->TouchBits & AHMET_RIGHT_TOUCH))
                    {
                        LaraItem->HitStatus = true;
                        LaraItem->HitPoints -= AHMET_HAND_DAMAGE;
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
        CreatureAnimation(itemNumber, angle, 0);
    }

    bool RespawnAhmet(short itemNumber)
    {
        ITEM_INFO* item = &g_Level.Items[itemNumber];

        if (item->ActiveState != 7 || item->FrameNumber != g_Level.Anims[item->AnimNumber].frameEnd)
            return false;

		Weather.Flash(255, 64, 0, 0.03f);

        item->Position.xPos = (item->ItemFlags[0] * 1024) + 512;
        item->Position.yPos = (item->ItemFlags[1] * 256);
        item->Position.zPos = (item->ItemFlags[2] * 1024) + 512;

        auto outsideRoom = IsRoomOutside(item->Position.xPos, item->Position.yPos, item->Position.zPos);
        if (item->RoomNumber != outsideRoom)
            ItemNewRoom(itemNumber, outsideRoom);

        item->AnimNumber = Objects[item->ObjectNumber].animIndex;
        item->TargetState = 1;
        item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
        item->ActiveState = 1;
        item->HitPoints = Objects[item->ObjectNumber].HitPoints;

        AddActiveItem(itemNumber);

        item->Flags &= 0xFE;
        item->AfterDeath = 0;
        item->Status = ITEM_ACTIVE;
        item->Collidable = true;

        EnableBaddieAI(itemNumber, 1);

        item->TriggerFlags = 1;
        return true;
    }
}