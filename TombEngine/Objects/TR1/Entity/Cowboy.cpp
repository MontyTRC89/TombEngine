#include "framework.h"
#include "Objects/TR1/Entity/Cowboy.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Renderer11.h"
#include "Specific/level.h"

using namespace TEN::Renderer;

namespace TEN::Entities::Creatures::TR1
{
    enum CowboyState
    {
        // No state 0,
        COWBOY_STATE_STOP = 1,
        COWBOY_STATE_WALK,
        COWBOY_STATE_RUN,
        COWBOY_STATE_AIM,
        COWBOY_STATE_DEATH,
        COWBOY_STATE_SHOOT
    };

    enum CowboyAnim
    {
        COWBOY_ANIM_RUN,
        COWBOY_ANIM_RUN_TO_IDLE,
        COWBOY_ANIM_RUN_TO_IDLE_END,
        COWBOY_ANIM_IDLE_TO_AIM,
        COWBOY_ANIM_SHOOT,
        COWBOY_ANIM_AIM,
        COWBOY_ANIM_AIM_TO_IDLE,
        COWBOY_ANIM_DEATH,
        COWBOY_ANIM_WALK,
        COWBOY_ANIM_WALK_TO_IDLE_START,
        COWBOY_ANIM_WALK_TO_IDLE_END,
        COWBOY_ANIM_IDLE_TO_WALK_START,
        COWBOY_ANIM_IDLE_TO_WALK_END,
        COWBOY_ANIM_IDLE_TO_RUN,
        COWBOY_ANIM_IDLE
    };

    const auto CowboyGunLeft = BiteInfo(Vector3(1.0f, 200.0f, 40.0f), 5);
    const auto CowboyGunRight = BiteInfo(Vector3(-1.0f, 200.0f, 40.0f), 8);

    constexpr auto COWBOY_WALK_TURN_RATE = ANGLE(3);
    constexpr auto COWBOY_RUN_TURN_RATE = ANGLE(6);
    constexpr auto COWBOY_WALK_RANGE = SQUARE(BLOCK(3));
    constexpr auto COWBOY_SHOOT_DAMAGE = 70;

    void InitialiseCowboy(short itemNumber)
    {
        auto& item = g_Level.Items[itemNumber];
        InitialiseCreature(itemNumber);
        SetAnimation(item, COWBOY_ANIM_IDLE);
    }

    void CowboyControl(short itemNumber)
    {
        if (!CreatureActive(itemNumber))
            return;

        auto& item = g_Level.Items[itemNumber];
        auto& creature = *GetCreatureInfo(&item);
        auto head = EulerAngles::Zero, torso = EulerAngles::Zero;
        short angle = 0, tilt = 0;

        if (item.HitPoints <= 0)
        {
            if (item.Animation.ActiveState != COWBOY_STATE_DEATH)
                SetAnimation(item, COWBOY_ANIM_DEATH);
        }
        else
        {
            AI_INFO ai;
            CreatureAIInfo(&item, &ai);
            GetCreatureMood(&item, &ai, false);
            CreatureMood(&item, &ai, false);

            if (ai.ahead)
            {
                head.y = ai.angle;
                head.x = ai.xAngle;
            }

            angle = CreatureTurn(&item, creature.MaxTurn);

            switch (item.Animation.ActiveState)
            {
            case COWBOY_STATE_STOP:
                creature.MaxTurn = 0;

                if (item.Animation.RequiredState != NO_STATE)
                    item.Animation.TargetState = item.Animation.RequiredState;
                else if (Targetable(&item, &ai))
                    item.Animation.TargetState = COWBOY_STATE_AIM;
                else if (creature.Mood == MoodType::Bored)
                    item.Animation.TargetState = COWBOY_STATE_WALK;
                else
                    item.Animation.TargetState = COWBOY_STATE_RUN;

                break;
            case COWBOY_STATE_WALK:
                creature.MaxTurn = COWBOY_WALK_TURN_RATE;

                if (creature.Mood == MoodType::Escape || !ai.ahead || ai.distance > COWBOY_WALK_RANGE)
                {
                    item.Animation.RequiredState = COWBOY_STATE_RUN;
                    item.Animation.TargetState = COWBOY_STATE_STOP;
                }
                else if (Targetable(&item, &ai))
                {
                    item.Animation.RequiredState = COWBOY_STATE_AIM;
                    item.Animation.TargetState = COWBOY_STATE_STOP;
                }

                break;
            case COWBOY_STATE_RUN:
                creature.MaxTurn = COWBOY_RUN_TURN_RATE;
                tilt = angle / 2;

                if (creature.Mood == MoodType::Escape && !ai.ahead)
                    break;

                if (Targetable(&item, &ai))
                {
                    item.Animation.RequiredState = COWBOY_STATE_AIM;
                    item.Animation.TargetState = COWBOY_STATE_STOP;
                }
                else if (ai.ahead && ai.distance < COWBOY_WALK_RANGE)
                {
                    item.Animation.RequiredState = COWBOY_STATE_WALK;
                    item.Animation.TargetState = COWBOY_STATE_STOP;
                }

                break;
            case COWBOY_STATE_AIM:
                creature.Flags = 0;
                creature.MaxTurn = 0;

                if (ai.ahead)
                {
                    torso.x = ai.xAngle / 2;
                    torso.y = ai.angle / 2;
                }

                if (item.Animation.RequiredState != NO_STATE)
                    item.Animation.TargetState = COWBOY_STATE_STOP;
                else if (Targetable(&item, &ai))
                    item.Animation.TargetState = COWBOY_STATE_SHOOT;
                else
                    item.Animation.TargetState = COWBOY_STATE_STOP;

                break;
            case COWBOY_STATE_SHOOT:
                creature.MaxTurn = 0;

                if (ai.ahead)
                {
                    torso.x = ai.xAngle / 2;
                    torso.y = ai.angle / 2;
                }

                if (Targetable(&item, &ai))
                {
                    // TODO: add gunflash for the cowboy after PR #1069 is on develop !

                    if (!(creature.Flags & 1) && item.Animation.FrameNumber == GetFrameIndex(&item, 2))
                    {
                        ShotLara(&item, &ai, CowboyGunLeft, head.y, COWBOY_SHOOT_DAMAGE);
                        creature.Flags |= 1;
                    }

                    if (!(creature.Flags & 2) && item.Animation.FrameNumber == GetFrameIndex(&item, 10))
                    {
                        ShotLara(&item, &ai, CowboyGunRight, head.y, COWBOY_SHOOT_DAMAGE);
                        creature.Flags |= 2;
                    }
                }

                if (creature.Mood == MoodType::Escape)
                    item.Animation.RequiredState = COWBOY_STATE_RUN;

                break;
            }
        }

        CreatureTilt(&item, tilt);
        CreatureJoint(&item, 0, head.y);
        CreatureJoint(&item, 1, head.x);
        CreatureJoint(&item, 2, torso.y);
        CreatureJoint(&item, 3, torso.x);
        CreatureAnimation(itemNumber, angle, tilt);
    }
}
