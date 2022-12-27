#include "framework.h"
#include "tr3_punaboss.h"
#include "Game/misc.h"
#include "Game/Lara/lara.h"
#include "Specific/setup.h"

namespace TEN::Entities::Creatures::TR3
{
    constexpr auto PUMABOSS_TURN_RATE_MAX = ANGLE(3.0f);

    enum PunaState
    {
        PUNA_STATE_STOP = 0,
        PUNA_STATE_ATTACK_WITH_HEAD = 1,
        PUNA_STATE_ATTACK_WITH_HAND = 2,
        PUNA_STATE_DEATH = 3,
    };

    enum PunaAnim
    {
        PUNA_IDLE = 0,
        PUNA_ANIMATION_ATTACK_WITH_HEAD = 1,
        PUNA_ANIMATION_ATTACK_WITH_HAND = 2,
        PUNA_ANIMATION_DEATH = 3,
    };

    void InitialisePuna(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];
        InitialiseCreature(itemNumber);
        SetAnimation(item, PUNA_IDLE);
        item->ItemFlags[0] = Objects[ID_LIZARD].loaded; // does the lizard are loaded ? if not then it will just fire at lara !
        // save the angle of puna, it will be used to restore this angle when he is waiting (after summoning the lizard).
        // puna is rotated to not face lara, so add 180° to face her.
        item->ItemFlags[1] = item->Pose.Orientation.y + ANGLE(180);
    }

    void PunaControl(short itemNumber)
    {
        if (CreatureActive(itemNumber))
            return;

        auto* item = &g_Level.Items[itemNumber];
        auto* creature = GetCreatureInfo(item);
        creature->Target = LaraItem->Pose.Position;

        //auto* punaboss = &BossData[BOSS_Puna];
        short angle = 0, headY = 0, oldrotation = 0;
        bool haveTurned = false;

        if (item->HitPoints <= 0)
        {
            if (item->Animation.ActiveState != PUNA_STATE_DEATH)
                SetAnimation(item, PUNA_ANIMATION_DEATH);
        }
        else
        {
            oldrotation = item->Pose.Orientation.y;

            AI_INFO AI;
            CreatureAIInfo(item, &AI);

            // if puna take damage then he will say it :)
           
            if (item->HitStatus)
                SoundEffect(SFX_TR3_PUNA_BOSS_TAKE_HIT, &item->Pose);
            
            angle = CreatureTurn(item, creature->MaxTurn);

            switch (item->Animation.ActiveState)
            {
            case PUNA_STATE_STOP:
                creature->MaxTurn = PUMABOSS_TURN_RATE_MAX;

                break;
            case PUNA_STATE_ATTACK_WITH_HEAD:
                creature->MaxTurn = 0;

                break;
            case PUNA_STATE_ATTACK_WITH_HAND:
                creature->MaxTurn = 0;

                break;
            }

        }

        CreatureAnimation(itemNumber, angle, 0);

        if (oldrotation != item->Pose.Orientation.y && !haveTurned)
        {
            haveTurned = true;
            SoundEffect(SFX_TR3_PUNA_BOSS_TURN_CHAIR, &item->Pose, SoundEnvironment::Land, 1.2f);
        }
        else if (oldrotation == item->Pose.Orientation.y)
            haveTurned = false;
    }
}
