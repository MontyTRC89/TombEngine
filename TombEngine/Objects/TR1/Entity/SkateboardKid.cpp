#include "framework.h"
#include "Objects/TR1/Entity/SkateboardKid.h"
#include "Game/control/box.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Game/control/lot.h"
#include "Game/Setup.h"

namespace TEN::Entities::Creatures::TR1
{
    constexpr auto SKATEKID_TURN_RATE = ANGLE(4.0f);
    constexpr auto SKATEKID_TOOCLOSE_RANGE = SQUARE(BLOCK(1));
    constexpr auto SKATEKID_DONTSTOP_RANGE = SQUARE(BLOCK(2.5f));
    constexpr auto SKATEKID_STOP_RANGE = SQUARE(BLOCK(4));
    constexpr auto SKATEKID_PUSH_CHANCE = 512;
    constexpr auto SKATEKID_SKATE_CHANCE = 1024;
    constexpr auto SKATEKID_STOP_SHOT_DAMAGE = 50;
    constexpr auto SKATEKID_SKATE_SHOT_DAMAGE = 40;

    const auto kidGunRight = CreatureBiteInfo(Vector3i(0, 170, 34), 7);
    const auto kidGunLeft = CreatureBiteInfo(Vector3i(0, 170, 37), 4);

    enum SkateKidState
    {
        KID_STATE_STOP = 0,
        KID_STATE_SHOOT,
        KID_STATE_SKATE,
        KID_STATE_PUSH,
        KID_STATE_SHOOT2,
        KID_STATE_DEATH
    };

    enum SkateKidAnim
    {
        KID_ANIM_START_MOVING_1,
        KID_ANIM_START_MOVING_2,
        KID_ANIM_SKATE_MORESPEED_END,
        KID_ANIM_SKATE_MORESPEED,
        KID_ANIM_STOP_SKATING_1,
        KID_ANIM_STOP_SKATING_2,
        KID_ANIM_STOP_SKATING_FINISH,
        KID_ANIM_IDLE,
        KID_ANIM_SKATE_MORESPEED_START,
        KID_ANIM_IDLE_START_SKATE,
        KID_ANIM_IDLE_SHOOT,
        KID_ANIM_SKATE_SHOOT,
        KID_ANIM_SKATE_AIM,
        KID_ANIM_DEATH
    };

    static void CreateSkateboard(ItemInfo* item)
    {
        short skateboardNumber = CreateItem();
        if (skateboardNumber != NO_ITEM)
        {
            auto* skate = &g_Level.Items[skateboardNumber];
            skate->ObjectNumber = ID_SKATEBOARD;
            skate->Pose.Position = item->Pose.Position;
            skate->Pose.Orientation = item->Pose.Orientation;
            skate->StartPose = item->StartPose;
            skate->Model.Color = item->Model.Color;
            skate->Collidable = true;
            InitializeItem(skateboardNumber);
            AddActiveItem(skateboardNumber);
            skate->Active = false;
            skate->Status |= ITEM_INVISIBLE;
            item->ItemFlags[0] = skateboardNumber;
        }
    }

    void InitialiseSkateboardKid(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];
        InitializeCreature(itemNumber);
        CreateSkateboard(item);
    }

    static void SkateboardKidShoot(ItemInfo& item, AI_INFO& ai, short head, CreatureInfo& creature, short damage)
    {
        if (creature.Flags == 0 && Targetable(&item, &ai))
        {
            ShotLara(&item, &ai, kidGunLeft, head, damage);
            creature.MuzzleFlash[0].Bite = kidGunLeft;
            creature.MuzzleFlash[0].Delay = 2;
            ShotLara(&item, &ai, kidGunRight, head, damage);
            creature.MuzzleFlash[1].Bite = kidGunRight;
            creature.MuzzleFlash[1].Delay = 2;
            creature.Flags = 1;
        }

        if (creature.Mood == MoodType::Escape || ai.distance < SKATEKID_TOOCLOSE_RANGE)
            item.Animation.RequiredState = KID_STATE_SKATE;
    }

    void SkateboardKidControl(short itemNumber)
    {
        if (!CreatureActive(itemNumber))
            return;

        auto& item = g_Level.Items[itemNumber];
        auto& creature = *GetCreatureInfo(&item);
        auto& skateboard = g_Level.Items[item.ItemFlags[0]];
        if (skateboard.Status & ITEM_INVISIBLE)
        {
            skateboard.Active = false;
            skateboard.Status &= ~(ITEM_INVISIBLE);
        }
        short angle = 0, headY = 0, torsoY = 0, torsoX = 0;

        if (creature.MuzzleFlash[0].Delay != 0)
            creature.MuzzleFlash[0].Delay--;
        if (creature.MuzzleFlash[1].Delay != 0)
            creature.MuzzleFlash[1].Delay--;

        if (item.HitPoints <= 0 && item.Animation.ActiveState != KID_STATE_DEATH)
        {
            creature.MaxTurn = 0;
            SetAnimation(item, KID_ANIM_DEATH);
        }
        else
        {
            AI_INFO ai;
            CreatureAIInfo(&item, &ai);
            if (ai.ahead)
            {
                headY = ai.angle / 2;
                torsoY = ai.angle / 2;
                torsoX = ai.xAngle / 2;
            }
            GetCreatureMood(&item, &ai, false);
            CreatureMood(&item, &ai, false);
            angle = CreatureTurn(&item, creature.MaxTurn);

            switch (item.Animation.ActiveState)
            {
            case KID_STATE_STOP:
                creature.Flags = 0;
                creature.MaxTurn = SKATEKID_TURN_RATE;
                if (item.Animation.RequiredState != -1)
                    item.Animation.TargetState = item.Animation.RequiredState;
                else if (Targetable(&item, &ai))
                    item.Animation.TargetState = KID_STATE_SHOOT;
                else
                    item.Animation.TargetState = KID_STATE_SKATE;
                break;
            case KID_STATE_SKATE:
                creature.Flags = 0;
                if (GetRandomControl() < SKATEKID_PUSH_CHANCE)
                {
                    item.Animation.TargetState = KID_STATE_PUSH;
                }
                else if (Targetable(&item, &ai))
                {
                    if (ai.distance > SKATEKID_DONTSTOP_RANGE && ai.distance < SKATEKID_STOP_RANGE && creature.Mood != MoodType::Escape)
                        item.Animation.TargetState = KID_STATE_STOP;
                    else
                        item.Animation.TargetState = KID_STATE_SHOOT2;
                }
                break;
            case KID_STATE_PUSH:
                if (GetRandomControl() < SKATEKID_SKATE_CHANCE)
                    item.Animation.TargetState = KID_STATE_SKATE;
                break;
            case KID_STATE_SHOOT:
                SkateboardKidShoot(item, ai, headY, creature, SKATEKID_STOP_SHOT_DAMAGE);
                break;
            case KID_STATE_SHOOT2:
                SkateboardKidShoot(item, ai, headY, creature, SKATEKID_SKATE_SHOT_DAMAGE);
                break;
            }
        }

        skateboard.Animation.AnimNumber = Objects[ID_SKATEBOARD].animIndex + (item.Animation.AnimNumber - Objects[ID_SKATEBOARD_KID].animIndex);
        skateboard.Animation.FrameNumber = g_Level.Anims[item.Animation.AnimNumber].frameBase + (item.Animation.FrameNumber - g_Level.Anims[item.Animation.AnimNumber].frameBase);
        skateboard.Pose.Position = item.Pose.Position;
        skateboard.Pose.Orientation = item.Pose.Orientation;
        AnimateItem(&skateboard);

        CreatureJoint(&item, 0, headY);
        CreatureJoint(&item, 1, torsoX);
        CreatureJoint(&item, 2, torsoY);
        CreatureAnimation(itemNumber, angle, 0);
    }
}
