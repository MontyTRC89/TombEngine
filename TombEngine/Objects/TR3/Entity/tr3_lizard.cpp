#include "framework.h"
#include "tr3_lizard.h"
#include "Game/control/box.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/effects/tomb4fx.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/level.h"

namespace TEN::Entities::Creatures::TR3
{
    enum LizardState
    {
        // No state 0
        LIZARD_STATE_STOP = 1,
        LIZARD_STATE_WALK,
        LIZARD_STATE_PUNCH2,
        LIZARD_STATE_AIM2,
        LIZARD_STATE_WAIT,
        LIZARD_STATE_AIM1,
        LIZARD_STATE_AIM0,
        LIZARD_STATE_PUNCH1,
        LIZARD_STATE_PUNCH0,
        LIZARD_STATE_RUN,
        LIZARD_STATE_DEATH,
        LIZARD_STATE_CLIMB3,
        LIZARD_STATE_CLIMB1,
        LIZARD_STATE_CLIMB2,
        LIZARD_STATE_FALL4
    };

    enum LizardAnim
    {
        LIZARD_ANIMATION_SLIDE1 = 23,
        LIZARD_ANIMATION_DEATH = 26,
        LIZARD_ANIMATION_CLIMB4 = 27,
        LIZARD_ANIMATION_CLIMB2 = 28,
        LIZARD_ANIMATION_CLIMB3 = 29,
        LIZARD_ANIMATION_FALL4 = 30,
        LIZARD_ANIMATION_SLIDE2 = 31,
    };

    const std::vector<unsigned int> lizman_swipe_bits = { 5 };
    const std::vector<unsigned int> lizman_bite_bits = { 10 };

    const BiteInfo lizman_bite_hit = { 0, -120, 120, 10 };
    const BiteInfo lizman_swipe_hit = { 0, 0, 0, 5 };
    const BiteInfo lizman_gas = { 0, -64, 56, 9 };

    constexpr auto LIZARD_WALK_TURN_RATE_MAX = ANGLE(10);
    constexpr auto LIZARD_RUN_TURN_RATE_MAX = ANGLE(4);
    constexpr auto LIZARD_ATTACK0_RANGE = SQUARE(SECTOR(2.5f));
    constexpr auto LIZARD_ATTACK1_RANGE = SQUARE(SECTOR(0.75f));
    constexpr auto LIZARD_ATTACK2_RANGE = SQUARE(SECTOR(1.5f));
    constexpr auto LIZARD_WALK_RANGE = SQUARE(SECTOR(2));

    constexpr auto LIZARD_VAULT_SHIFT = 260;
    constexpr auto LIZARD_WALK_CHANCE = 256.0f;
    constexpr auto LIZARD_BORED_WALK_CHANCE = 512.0f;
    constexpr auto LIZARD_WAIT_CHANCE = 256.0f;

    bool LizardTargetIsBlocked(CreatureInfo* creature)
    {
        return creature->Enemy && creature->Enemy->BoxNumber != NO_BOX && g_Level.Boxes[creature->Enemy->BoxNumber].flags & BLOCKABLE;
    }

    void TriggerLizardGasThrower(short itemNumber, const BiteInfo& bite, int speed)
    {
        for (int i = 0; i < 2; i++)
            ThrowPoison(itemNumber, lizman_gas.meshNum, Vector3i(lizman_gas.Position.x, lizman_gas.Position.y, lizman_gas.Position.z), Vector3i(0, -100, speed << 2), Vector3(0, 1, 0));
        ThrowPoison(itemNumber, lizman_gas.meshNum, Vector3i(lizman_gas.Position.x, lizman_gas.Position.y, lizman_gas.Position.z), Vector3i(0, -100, speed << 1), Vector3(0, 1, 0));
    }

    void LizardControl(short itemNumber)
    {
        if (!CreatureActive(itemNumber))
            return;

        auto* item = &g_Level.Items[itemNumber];
        auto* creature = GetCreatureInfo(item);
        short angle = 0, tilt = 0;
        auto neck = EulerAngles::Zero;
        
        if (item->HitPoints <= 0)
        {
            if (item->Animation.ActiveState != LIZARD_STATE_DEATH)
                SetAnimation(item, LIZARD_ANIMATION_DEATH);
        }
        else
        {
            AI_INFO AI;
            CreatureAIInfo(item, &AI);
            GetCreatureMood(item, &AI, true);
            CreatureMood(item, &AI, true);

            if (LizardTargetIsBlocked(creature))
                creature->Mood = MoodType::Attack;

            angle = CreatureTurn(item, creature->MaxTurn);
            if (AI.ahead && item->Animation.ActiveState < LIZARD_STATE_DEATH) // NOTE: Avoid turning the head when he is climbing or falling.
            {
                neck.x = AI.xAngle;
                neck.y = AI.angle;
            }

            bool isLaraPoisonedOrTargetBlocked = (creature->Enemy != nullptr && GetLaraInfo(creature->Enemy)->PoisonPotency < 256) || LizardTargetIsBlocked(creature);

            switch (item->Animation.ActiveState)
            {
            case LIZARD_STATE_STOP:
                creature->MaxTurn = 0;
                creature->Flags = 0;

                if (creature->Mood == MoodType::Escape)
                    item->Animation.TargetState = LIZARD_STATE_RUN;
                else if (creature->Mood == MoodType::Bored)
                {
                    if (item->Animation.RequiredState)
                        item->Animation.TargetState = item->Animation.RequiredState;
                    else if (Random::TestProbability(1.0f / LIZARD_BORED_WALK_CHANCE))
                        item->Animation.TargetState = LIZARD_STATE_WALK;
                    else
                        item->Animation.TargetState = LIZARD_STATE_WAIT;
                }
                else if (AI.bite && AI.distance < LIZARD_ATTACK1_RANGE)
                    item->Animation.TargetState = LIZARD_STATE_AIM1;
                else if (Targetable(item, &AI) && AI.bite && AI.distance < LIZARD_ATTACK0_RANGE && isLaraPoisonedOrTargetBlocked)
                    item->Animation.TargetState = LIZARD_STATE_AIM0;
                else
                    item->Animation.TargetState = LIZARD_STATE_RUN;
                break;
            case LIZARD_STATE_WAIT:
                creature->MaxTurn = 0;

                if (creature->Mood != MoodType::Bored)
                    item->Animation.TargetState = LIZARD_STATE_STOP;
                else if (Random::TestProbability(1.0f / LIZARD_WALK_CHANCE))
                {
                    item->Animation.RequiredState = LIZARD_STATE_WALK;
                    item->Animation.TargetState = LIZARD_STATE_STOP;
                }
                break;
            case LIZARD_STATE_WALK:
                if (TestCurrentAnimation(item, LIZARD_ANIMATION_SLIDE1) || TestCurrentAnimation(item, LIZARD_ANIMATION_SLIDE2))
                    creature->MaxTurn = 0;
                else
                    creature->MaxTurn = LIZARD_WALK_TURN_RATE_MAX;

                if (creature->Mood == MoodType::Escape)
                    item->Animation.TargetState = LIZARD_STATE_RUN;
                else if (creature->Mood == MoodType::Bored)
                {
                    if (Random::TestProbability(1.0f / LIZARD_WAIT_CHANCE))
                    {
                        item->Animation.RequiredState = LIZARD_STATE_WAIT;
                        item->Animation.TargetState = LIZARD_STATE_STOP;
                    }
                }
                else if (AI.bite && AI.distance < LIZARD_ATTACK1_RANGE)
                    item->Animation.TargetState = LIZARD_STATE_STOP;
                else if (AI.bite && AI.distance < LIZARD_ATTACK2_RANGE)
                    item->Animation.TargetState = LIZARD_STATE_AIM2;
                else if (Targetable(item, &AI) && AI.distance < LIZARD_ATTACK0_RANGE && isLaraPoisonedOrTargetBlocked)
                    item->Animation.TargetState = LIZARD_STATE_STOP;
                else if (AI.distance > LIZARD_WALK_RANGE)
                    item->Animation.TargetState = LIZARD_STATE_RUN;
                break;
            case LIZARD_STATE_RUN:
                creature->MaxTurn = LIZARD_RUN_TURN_RATE_MAX;
                tilt = angle / 2;

                if (creature->Mood == MoodType::Escape)
                    break;
                else if (creature->Mood == MoodType::Bored)
                    item->Animation.TargetState = LIZARD_STATE_WALK;
                else if (AI.bite && AI.distance < LIZARD_ATTACK1_RANGE)
                    item->Animation.TargetState = LIZARD_STATE_STOP;
                else if (Targetable(item, &AI) && AI.distance < LIZARD_ATTACK0_RANGE && isLaraPoisonedOrTargetBlocked)
                    item->Animation.TargetState = LIZARD_STATE_STOP;
                else if (AI.ahead && AI.distance < LIZARD_WALK_RANGE)
                    item->Animation.TargetState = LIZARD_STATE_WALK;
                break;
            case LIZARD_STATE_AIM0:
                creature->MaxTurn = 0;
                creature->Flags = 0;

                if (abs(AI.angle) < LIZARD_RUN_TURN_RATE_MAX)
                    item->Pose.Orientation.y += AI.angle;
                else if (AI.angle < 0)
                    item->Pose.Orientation.y -= LIZARD_RUN_TURN_RATE_MAX;
                else
                    item->Pose.Orientation.y += LIZARD_RUN_TURN_RATE_MAX;

                if (AI.bite && AI.distance < LIZARD_ATTACK0_RANGE && isLaraPoisonedOrTargetBlocked) // TS - maybe we should add targetable as well?
                    item->Animation.TargetState = LIZARD_STATE_PUNCH0;
                else
                    item->Animation.TargetState = LIZARD_STATE_STOP;
                break;
            case LIZARD_STATE_AIM1:
                creature->MaxTurn = LIZARD_WALK_TURN_RATE_MAX;
                creature->Flags = 0;

                if (AI.ahead && AI.distance < LIZARD_ATTACK1_RANGE)
                    item->Animation.TargetState = LIZARD_STATE_PUNCH1;
                else
                    item->Animation.TargetState = LIZARD_STATE_STOP;
                break;
            case LIZARD_STATE_AIM2:
                creature->MaxTurn = LIZARD_WALK_TURN_RATE_MAX;
                creature->Flags = 0;

                if (AI.ahead && AI.distance < LIZARD_ATTACK2_RANGE)
                    item->Animation.TargetState = LIZARD_STATE_PUNCH2;
                else
                    item->Animation.TargetState = LIZARD_STATE_STOP;
                break;
            case LIZARD_STATE_PUNCH0:
                creature->MaxTurn = 0;

                if (abs(AI.angle) < LIZARD_RUN_TURN_RATE_MAX)
                    item->Pose.Orientation.y += AI.angle;
                else if (AI.angle < 0)
                    item->Pose.Orientation.y -= LIZARD_RUN_TURN_RATE_MAX;
                else
                    item->Pose.Orientation.y += LIZARD_RUN_TURN_RATE_MAX;

                if (TestFrameBetween(item, 7, 28))
                {
                    if (creature->Flags < 24)
                        creature->Flags += 2;
                    if (creature->Flags < 24)
                        TriggerLizardGasThrower(itemNumber, lizman_gas, creature->Flags);
                    else
                        TriggerLizardGasThrower(itemNumber, lizman_gas, (GetRandomControl() & 15) + 8);
                }

                if (TestFrameSingle(item, 28))
                    creature->Flags = 0;
                break;
            case LIZARD_STATE_PUNCH1:
                creature->MaxTurn = 0;

                if (!creature->Flags && item->TouchBits.Test(lizman_swipe_bits))
                {
                    DoDamage(creature->Enemy, 120);
                    CreatureEffect(item, lizman_swipe_hit, DoBloodSplat);
                    SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
                    creature->Flags = 1;
                }

                if (AI.distance < LIZARD_ATTACK2_RANGE)
                    item->Animation.TargetState = LIZARD_STATE_PUNCH2;
                break;
            case LIZARD_STATE_PUNCH2:
                creature->MaxTurn = 0;

                if (creature->Flags != 2 && item->TouchBits.Test(lizman_bite_bits))
                {
                    DoDamage(creature->Enemy, 100);
                    CreatureEffect(item, lizman_swipe_hit, DoBloodSplat);
                    SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
                    creature->Flags = 2;
                }
                break;
            }
        }

        CreatureTilt(item, tilt);
        CreatureJoint(item, 0, neck.x);
        CreatureJoint(item, 1, neck.y);

        if (item->Animation.ActiveState < LIZARD_STATE_DEATH) //  CLIMB3 marks the start of the CLIMB states
        {
            switch (CreatureVault(itemNumber, angle, 2, LIZARD_VAULT_SHIFT))
            {
            case 2:
                creature->MaxTurn = 0;
                SetAnimation(item, LIZARD_ANIMATION_CLIMB2);
                break;

            case 3:
                creature->MaxTurn = 0;
                SetAnimation(item, LIZARD_ANIMATION_CLIMB3);
                break;

            case 4:
                creature->MaxTurn = 0;
                SetAnimation(item, LIZARD_ANIMATION_CLIMB4);
                break;
            case -4:
                creature->MaxTurn = 0;
                SetAnimation(item, LIZARD_ANIMATION_FALL4);
                break;
            }
        }
        else
        {
            CreatureAnimation(itemNumber, angle, 0);
        }
    }
}
