#include "framework.h"
#include "tr3_willard.h"
#include <Game/control/box.h>
#include <Game/misc.h>

namespace TEN::Entities::Creatures::TR3
{
    // Willard need 4 items:
    // - Infada stone
    // - Eye of isis
    // - Element 115
    // - Ora dagger

    enum WillardState
    {
        WILLARD_STATE_STOP,
        WILLARD_STATE_WALK,
        WILLARD_STATE_LUNGE,
        WILLARD_STATE_KILL_LARA,
        WILLARD_STATE_STUNNED,
        WILLARD_STATE_KNOCKOUT,
        WILLARD_STATE_GETUP,
        WILLARD_STATE_WALKATTACK1,
        WILLARD_STATE_WALKATTACK2,
        WILLARD_STATE_180,
        WILLARD_STATE_SHOOT
    };



    std::vector<Pose> GetListOfRequiredAIObjects(ItemInfo* item, GAME_OBJECT_ID aiObjectNumber)
    {
        std::vector<Pose> aiList;
        for (auto& aiObj : g_Level.AIObjects)
        {
            if (aiObj.roomNumber == item->RoomNumber && aiObj.objectNumber == aiObjectNumber)
                aiList.push_back(aiObj.pos);
        }
        return aiList;
    }



    void InitialiseWillard(short itemNumber)
    {
        InitialiseCreature(itemNumber);
    }

    void WillardControl(short itemNumber)
    {
        if (!CreatureActive(itemNumber))
            return;

        auto* item = &g_Level.Items[itemNumber];
        auto* creature = GetCreatureInfo(item);
        auto aiX1List = GetListOfRequiredAIObjects(item, ID_AI_X1);
        auto aiX2List = GetListOfRequiredAIObjects(item, ID_AI_X2);
        short angle = 0;

        //TENLog("AI_X1 count: " + std::to_string(aiX1List.size()) + ", AI_X2 count: " + std::to_string(aiX2List.size()));

        if (item->HitPoints <= 0)
        {
            
        }
        else
        {
            AI_INFO AI;

            angle = CreatureTurn(item, creature->MaxTurn);
            CreatureAIInfo(item, &AI);

            switch (item->Animation.ActiveState)
            {
            case WILLARD_STATE_STOP:
                creature->MaxTurn = 0;
                creature->Flags = 0;




                break;
            case WILLARD_STATE_WALK:

                break;
            }
        }

        CreatureAnimation(itemNumber, angle, 0);
    }
}
