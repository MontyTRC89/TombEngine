#include "framework.h"
#include "tr3_punaboss.h"
#include "Game/misc.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/effects/spark.h"
#include "Objects/TR3/Object/tr3_boss_object.h"
#include "Specific/setup.h"
#include "Game/effects/lightning.h"
#include <Game/collision/collide_room.h>
#include <Game/control/los.h>
#include <Game/Lara/lara_helpers.h>

using namespace TEN::Entities::Object::TR3;
using namespace TEN::Effects::Lightning;

namespace TEN::Entities::Creatures::TR3
{
    const auto PunaBossEffectColor = Vector4(0.25f, 0.25f, 0.75f, 0.5f);
    const auto PunaBossHeadBite = BiteInfo(0, 0, -50, 8);
    const auto PunaBossHandBite = BiteInfo(0, 0, 0, 14);

    constexpr auto PUNABOSS_ITEM_ROTATE_RATE_MAX = ANGLE(7.0f);
    constexpr auto PUNABOSS_HEAD_X_ANGLE_MAX = ANGLE(20);
    constexpr auto PUMABOSS_TURN_RATE_MAX = ANGLE(3.0f);
    constexpr auto PUMABOSS_ADJUST_LASER_XANGLE = ANGLE(5);
    constexpr auto PUNABOSS_EXPLOSION_COUNT_MAX = 120;
    constexpr auto PUNABOSS_MAX_HEAD_ATTACK = 4;

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

    enum PunaAttackType
    {
        PUNA_Normal,
        PUNA_Summon
    };

    std::vector<GameVector> FoundAllLizardAISpawnPoint(ItemInfo* item, short objectNumber)
    {
        std::vector<GameVector> itemList;
        for (size_t i = 0; i < g_Level.AIObjects.size(); i++)
        {
            auto* itemTarget = &g_Level.AIObjects[i];
            if (itemTarget->objectNumber == objectNumber && itemTarget->roomNumber == item->RoomNumber)
                itemList.push_back(GameVector(itemTarget->pos.Position, itemTarget->roomNumber));
        }
        return itemList;
    }

    void InitialisePuna(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];
        InitialiseCreature(itemNumber);
        SetAnimation(item, PUNA_IDLE);
        BOSS_CheckForRequiredObjects(item);

        // save the angle of puna, it will be used to restore this angle when he is waiting (after summoning the lizard).
        // puna is rotated to not face lara, so add 180° to face her.
        item->ItemFlags[BOSSFlag_Rotation] = item->Pose.Orientation.y + ANGLE(180);
        item->ItemFlags[BOSSFlag_AttackType] = PUNA_Normal; // normal behaviour at start !
        item->ItemFlags[BOSSFlag_ShieldIsEnabled] = 1; // activated at start.
        item->ItemFlags[BOSSFlag_AttackCount] = 0;
        item->ItemFlags[BOSSFlag_DeathCount] = 0;
        item->ItemFlags[BOSSFlag_ItemNumber] = NO_ITEM;
        item->ItemFlags[BOSSFlag_ExplodeCount] = 0;
    }

    void PunaLaser(ItemInfo* item, ItemInfo* enemy, GameVector targetPos, const BiteInfo& bite, short xHandAngle, int intensity, bool isSummon) // TODO: deal with LaraItem global !
    {
        GameVector src = GetJointPosition(item, bite.meshNum, bite.Position);
        src.RoomNumber = item->RoomNumber;
        GameVector target;
        Vector3i hitPos;
        MESH_INFO* mesh = nullptr;

        switch (item->ItemFlags[BOSSFlag_AttackType])
        {
        case PUNA_Normal:
            xHandAngle -= PUMABOSS_ADJUST_LASER_XANGLE;
            target.x = src.x + ((BLOCK(8) * phd_cos(xHandAngle)) * phd_sin(item->Pose.Orientation.y));
            target.y = src.y + BLOCK(8) * phd_sin(-xHandAngle);
            target.z = src.z + ((BLOCK(8) * phd_cos(xHandAngle)) * phd_cos(item->Pose.Orientation.y));
            TriggerLightning((Vector3i*)&src, (Vector3i*)&target, intensity, 0, 255, 255, 10, LI_SPLINE | LI_THINOUT, 50, 5);
            if (ObjectOnLOS2(&src, &target, &hitPos, &mesh, ID_LARA) == GetLaraInfo(enemy)->ItemNumber)
            {
                TENLog("Hit lara !", LogLevel::Info, LogConfig::All, true);
            }
            break;
        case PUNA_Summon:
            target.x = targetPos.x;
            target.y = targetPos.y;
            target.z = targetPos.z;
            target.RoomNumber = targetPos.RoomNumber;
            TriggerLightning((Vector3i*)&src, (Vector3i*)&target, intensity, 0, 255, 255, 10, LI_SPLINE | LI_THINOUT, 50, 5);
            // summon lizard at target !
            break;
        }
    }

    void PunaControl(short itemNumber)
    {
        if (!CreatureActive(itemNumber))
            return;
        
        auto* item = &g_Level.Items[itemNumber];
        auto* creature = GetCreatureInfo(item);
        auto* object = &Objects[item->ObjectNumber];
        auto head = EulerAngles::Zero;
        std::vector<GameVector> lizardList;
        int lizardID = 0;
        
        short angle = 0, oldrotation = 0;
        bool haveTurned = false;

        if (item->HitPoints <= 0)
        {
            if (item->Animation.ActiveState != PUNA_STATE_DEATH)
            {
                SetAnimation(item, PUNA_ANIMATION_DEATH);
                SoundEffect(SFX_TR3_PUNA_BOSS_DEATH, &item->Pose);
                item->ItemFlags[BOSSFlag_DeathCount] = 1;
            }

            int frameMaxLess1 = g_Level.Anims[object->animIndex + PUNA_ANIMATION_DEATH].frameEnd - 1;
            if (item->Animation.FrameNumber > frameMaxLess1)
            {
                if (item->ItemFlags[BOSSFlag_ExplodeCount] < PUNABOSS_EXPLOSION_COUNT_MAX)
                    item->Animation.FrameNumber = frameMaxLess1; // avoid the object to stop working.
                item->MeshBits.ClearAll();
                StopSoundEffect(SFX_TR3_PUNA_BOSS_CHAIR_2);
                StopSoundEffect(SFX_TR3_PUNA_BOSS_TURN_CHAIR);

                if (item->ItemFlags[BOSSFlag_ExplodeCount] < PUNABOSS_EXPLOSION_COUNT_MAX)
                    item->ItemFlags[BOSSFlag_ExplodeCount]++;

                if (item->ItemFlags[BOSSFlag_ExplodeCount] < PUNABOSS_EXPLOSION_COUNT_MAX)
                    BOSS_ExplodeBoss(item, PunaBossEffectColor); // Do explosion effect.
                return;
            }
            else
            {
                item->Pose.Orientation.z = (Random::GenerateInt() % item->ItemFlags[BOSSFlag_DeathCount]) - (item->ItemFlags[BOSSFlag_DeathCount] >> 1);
                if (item->ItemFlags[BOSSFlag_DeathCount] < 2048)
                    item->ItemFlags[BOSSFlag_DeathCount] += 32;
            }
        }
        else
        {
            oldrotation = item->Pose.Orientation.y;
            creature->Target = LaraItem->Pose.Position;

            AI_INFO AI;
            CreatureAIInfo(item, &AI);
            if (AI.ahead)
            {
                head.x = AI.xAngle;
                head.y = AI.angle;
            }

            // if puna take damage then he will say it
            if (item->HitStatus)
                SoundEffect(SFX_TR3_PUNA_BOSS_TAKE_HIT, &item->Pose);
            
            switch (item->ItemFlags[BOSSFlag_AttackType])
            {
            case PUNA_Normal:
                angle = CreatureTurn(item, creature->MaxTurn);

                switch (item->Animation.ActiveState)
                {
                case PUNA_STATE_STOP:
                    creature->MaxTurn = PUMABOSS_TURN_RATE_MAX;
                    item->ItemFlags[BOSSFlag_ShieldIsEnabled] = 1;

                    if ((item->Animation.TargetState != PUNA_STATE_ATTACK_WITH_HAND || item->Animation.TargetState != PUNA_STATE_ATTACK_WITH_HEAD) &&
                        AI.angle > -ANGLE(1) && AI.angle < ANGLE(1) &&
                        LaraItem->HitPoints > 0 &&
                        item->ItemFlags[BOSSFlag_AttackCount] < PUNABOSS_MAX_HEAD_ATTACK)
                    {
                        creature->MaxTurn = 0;
                        item->Animation.TargetState = PUNA_STATE_ATTACK_WITH_HAND;
                        item->ItemFlags[BOSSFlag_AttackCount]++;
                    }
                    else if (item->ItemFlags[BOSSFlag_AttackCount] >= PUNABOSS_MAX_HEAD_ATTACK && LaraItem->HitPoints > 0)
                    {
                        lizardList = FoundAllLizardAISpawnPoint(item, ID_AI_AMBUSH); // FINISH THIS !
                        lizardID = Random::GenerateInt(0, lizardList.size() - 1); // get a random lizard spawn point.
                        creature->Target = Vector3i(lizardList[lizardID].x, lizardList[lizardID].y, lizardList[lizardID].z);
                    }

                    break;
                case PUNA_STATE_ATTACK_WITH_HEAD:
                    creature->MaxTurn = 0;
                    item->ItemFlags[BOSSFlag_ShieldIsEnabled] = 0;
                    if (item->Animation.FrameNumber == GetFrameNumber(item, 14))
                    {
                        PunaLaser(item, creature->Enemy, GameVector::Zero, PunaBossHeadBite, head.x, 10, false);
                    }

                    break;
                case PUNA_STATE_ATTACK_WITH_HAND:
                    creature->MaxTurn = 0;
                    item->ItemFlags[BOSSFlag_ShieldIsEnabled] = 0;
                    if (item->Animation.FrameNumber == GetFrameNumber(item, 30))
                    {
                        PunaLaser(item, creature->Enemy, GameVector::Zero, PunaBossHandBite, head.x, 5, false);
                    }

                    break;
                }
                break;
            case PUNA_Summon:
                item->ItemFlags[BOSSFlag_ShieldIsEnabled] = 1;
                head.x = 0; // reset puna head rotation when doing summon.
                head.y = 0;

                break;
            }
        }


        creature->JointRotation[0] += PUNABOSS_ITEM_ROTATE_RATE_MAX; // Rotate the object on puna boss chair !
        CreatureJoint(item, 1, head.y);
        CreatureJoint(item, 2, head.x, PUNABOSS_HEAD_X_ANGLE_MAX);
        CreatureAnimation(itemNumber, angle, 0);

        // When puna rotate, the chair do a sound !
        if (oldrotation != item->Pose.Orientation.y && !haveTurned)
        {
            haveTurned = true;
            SoundEffect(SFX_TR3_PUNA_BOSS_TURN_CHAIR, &item->Pose);
        }
        else if (oldrotation == item->Pose.Orientation.y)
        {
            haveTurned = false;
        }
    }
}
