#include "framework.h"
#include "tr3_punaboss.h"
#include "Game/misc.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/control/lot.h"
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
        PUNA_DeathLaser,
        PUNA_SummonLaser,
        PUNA_Wait // when the summon is fighting !
    };

    std::vector<AI_OBJECT> FoundAllLizardAISpawnPoint(ItemInfo* item, short objectNumber)
    {
        std::vector<AI_OBJECT> itemList;
        for (auto& aiObj : g_Level.AIObjects)
        {
            if (aiObj.objectNumber == objectNumber && aiObj.roomNumber == item->RoomNumber)
                itemList.push_back(aiObj);
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
        item->ItemFlags[BOSSFlag_Rotation] = item->Pose.Orientation.y - ANGLE(180.0f);
        item->ItemFlags[BOSSFlag_AttackType] = PUNA_DeathLaser; // normal behaviour at start !
        item->ItemFlags[BOSSFlag_ShieldIsEnabled] = 1; // activated at start.
        item->ItemFlags[BOSSFlag_AttackCount] = 0;
        item->ItemFlags[BOSSFlag_DeathCount] = 0;
        item->ItemFlags[BOSSFlag_ItemNumber] = NO_ITEM;
        item->ItemFlags[BOSSFlag_ExplodeCount] = 0;
    }

    void SpawnLizard(ItemInfo* item, AI_OBJECT targetPos)
    {
        if (CHK_ANY(item->ItemFlags[BOSSFlag_Object], BOSS_Lizard))
        {
            short itemNumber = CreateItem();
            if (itemNumber != NO_ITEM)
            {
                auto* spawned = &g_Level.Items[itemNumber];
                spawned->Pose.Position = targetPos.pos.Position;
                spawned->Pose.Orientation = targetPos.pos.Orientation;
                spawned->RoomNumber = targetPos.roomNumber;
                spawned->ObjectNumber = ID_LIZARD;
                spawned->Model.Color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // set the color the same as puna, to avoid black lizard.
                InitialiseItem(itemNumber);
                AddActiveItem(itemNumber);
                spawned->ItemFlags[0] = 1; // specify that this lizard is summoned !
                spawned->Active = true;
                spawned->Flags |= IFLAG_ACTIVATION_MASK;
                spawned->Status = ITEM_ACTIVE;
                EnableEntityAI(itemNumber, 1);
                UpdateItemRoom(itemNumber);
                item->ItemFlags[BOSSFlag_ItemNumber] = itemNumber;
            }
        }
    }

    void PunaLaserSummon(ItemInfo* item, AI_OBJECT spawnPoint, const BiteInfo& bite, short xHandAngle, int intensity)
    {
        xHandAngle -= PUMABOSS_ADJUST_LASER_XANGLE;

        GameVector src = GameVector(GetJointPosition(item, bite.meshNum, bite.Position), item->RoomNumber);
        GameVector target = GameVector::Zero;
        target.x = src.x + ((BLOCK(8) * phd_cos(xHandAngle)) * phd_sin(item->Pose.Orientation.y));
        target.y = src.y + BLOCK(8) * phd_sin(-xHandAngle);
        target.z = src.z + ((BLOCK(8) * phd_cos(xHandAngle)) * phd_cos(item->Pose.Orientation.y));
        target.RoomNumber = item->RoomNumber;
        TriggerLightning((Vector3i*)&src, (Vector3i*)&target, intensity, 0, 255, 255, 10, LI_SPLINE | LI_THINOUT, 50, 5);

        item->ItemFlags[BOSSFlag_AttackType] = PUNA_Wait;
        SpawnLizard(item, spawnPoint);
    }

    void PunaLaser(ItemInfo* item, ItemInfo* enemy, const BiteInfo& bite, short xHandAngle, int intensity) // TODO: deal with LaraItem global !
    {
        xHandAngle -= PUMABOSS_ADJUST_LASER_XANGLE;

        GameVector src = GameVector(GetJointPosition(item, bite.meshNum, bite.Position), item->RoomNumber);
        GameVector target = GameVector::Zero;
        Vector3i hitPos = Vector3i::Zero;
        MESH_INFO* mesh = nullptr;
        target.x = src.x + ((BLOCK(8) * phd_cos(xHandAngle)) * phd_sin(item->Pose.Orientation.y));
        target.y = src.y + BLOCK(8) * phd_sin(-xHandAngle);
        target.z = src.z + ((BLOCK(8) * phd_cos(xHandAngle)) * phd_cos(item->Pose.Orientation.y));
        target.RoomNumber = enemy->RoomNumber;
        TriggerLightning((Vector3i*)&src, (Vector3i*)&target, intensity, 0, 255, 255, 10, LI_SPLINE | LI_THINOUT, 50, 5);
        if (ObjectOnLOS2(&src, &target, &hitPos, &mesh, ID_LARA) == GetLaraInfo(enemy)->ItemNumber)
        {
            TENLog("Hit lara !", LogLevel::Info, LogConfig::All, true);
        }
    }

    short GetPunaHeadAngleTarget(ItemInfo* item, Vector3i target)
    {
        auto headJointPos = GetJointPosition(item, PunaBossHeadBite.meshNum);
        auto headAngle = Math::Geometry::GetOrientToPoint(headJointPos.ToVector3(), target.ToVector3());
        return headAngle.y - item->Pose.Orientation.y;
    }

    void PunaControl(short itemNumber)
    {
        if (!CreatureActive(itemNumber))
            return;
        
        auto* item = &g_Level.Items[itemNumber];
        auto* creature = GetCreatureInfo(item);
        auto* object = &Objects[item->ObjectNumber];
        std::vector<AI_OBJECT> lizardList;
        auto head = EulerAngles::Zero;
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

            AI_INFO AI;
            CreatureAIInfo(item, &AI);
            if (AI.ahead)
            {
                head.x = AI.xAngle;
                head.y = AI.angle;
            }

            // Get target
            if (item->ItemFlags[BOSSFlag_AttackType] == PUNA_DeathLaser)
            {
                creature->Target = LaraItem->Pose.Position;
            }
            else if (item->ItemFlags[BOSSFlag_AttackType] == PUNA_SummonLaser &&
                     item->ItemFlags[BOSSFlag_ItemNumber] == NO_ITEM &&
                     item->ItemFlags[BOSSFlag_AttackType] != PUNA_Wait)
            {
                // get a random spawn point !
                lizardList = FoundAllLizardAISpawnPoint(item, ID_AI_AMBUSH);
                item->ItemFlags[BOSSFlag_ItemNumber] = Random::GenerateInt(0, lizardList.size() - 1);
                creature->Target = lizardList[item->ItemFlags[BOSSFlag_ItemNumber]].pos.Position;
                TENLog("Done setting the lizard spawn point !");
            }
            else if (item->ItemFlags[BOSSFlag_AttackType] == PUNA_Wait && item->ItemFlags[BOSSFlag_ItemNumber] != NO_ITEM)
            {
                // rotate to idle position (for waiting lara to die by lizard)
                auto targetOrient = EulerAngles(item->Pose.Orientation.x, item->ItemFlags[BOSSFlag_Rotation], item->Pose.Orientation.z);
                item->Pose.Orientation.InterpolateConstant(targetOrient, ANGLE(3.0f));

                // check if target is dead !
                auto& summonItem = g_Level.Items[item->ItemFlags[BOSSFlag_ItemNumber]];
                if (summonItem.HitPoints <= 0)
                {
                    // Reset the attack type, attack count and itemNumber and restart the sequence !
                    item->ItemFlags[BOSSFlag_AttackType] = PUNA_DeathLaser;
                    item->ItemFlags[BOSSFlag_AttackCount] = 0;
                    item->ItemFlags[BOSSFlag_ItemNumber] = NO_ITEM;
                }
            }

            // if puna take damage then he will say it
            if (item->HitStatus)
                SoundEffect(SFX_TR3_PUNA_BOSS_TAKE_HIT, &item->Pose);

            short angleHead = GetPunaHeadAngleTarget(item, creature->Target);
            angle = CreatureTurn(item, creature->MaxTurn);

            switch (item->Animation.ActiveState)
            {
            case PUNA_STATE_STOP:
                creature->MaxTurn = PUMABOSS_TURN_RATE_MAX;
                item->ItemFlags[BOSSFlag_ShieldIsEnabled] = 1;

                if ((item->Animation.TargetState != PUNA_STATE_ATTACK_WITH_HAND && item->Animation.TargetState != PUNA_STATE_ATTACK_WITH_HEAD) &&
                    AI.angle > -ANGLE(1) && AI.angle < ANGLE(1) &&
                    LaraItem->HitPoints > 0 &&
                    item->ItemFlags[BOSSFlag_AttackCount] < PUNABOSS_MAX_HEAD_ATTACK &&
                    item->ItemFlags[BOSSFlag_AttackType] != PUNA_SummonLaser && item->ItemFlags[BOSSFlag_AttackType] != PUNA_Wait)
                {
                    creature->MaxTurn = 0;
                    item->ItemFlags[BOSSFlag_AttackType] = PUNA_DeathLaser;
                    if (Random::GenerateInt(0, 2) == 1) // alternate attack with head/hand, 1/3 chance
                        item->Animation.TargetState = PUNA_STATE_ATTACK_WITH_HEAD;
                    else
                        item->Animation.TargetState = PUNA_STATE_ATTACK_WITH_HAND;
                    item->ItemFlags[BOSSFlag_AttackCount]++;
                }
                else if (item->ItemFlags[BOSSFlag_AttackCount] >= PUNABOSS_MAX_HEAD_ATTACK && LaraItem->HitPoints > 0 &&
                    item->ItemFlags[BOSSFlag_AttackType] != PUNA_Wait)
                {
                    item->ItemFlags[BOSSFlag_AttackType] = PUNA_SummonLaser;
                    if (item->ItemFlags[BOSSFlag_ItemNumber] != NO_ITEM)
                    {
                        if (angleHead > -ANGLE(1) && angleHead < ANGLE(1))
                            item->Animation.TargetState = PUNA_STATE_ATTACK_WITH_HAND;
                    }
                }

                break;
            case PUNA_STATE_ATTACK_WITH_HEAD:
                creature->MaxTurn = 0;
                item->ItemFlags[BOSSFlag_ShieldIsEnabled] = 0;
                if (item->Animation.FrameNumber == GetFrameNumber(item, 14))
                {
                    PunaLaser(item, creature->Enemy, PunaBossHeadBite, head.x, 10);
                }

                break;
            case PUNA_STATE_ATTACK_WITH_HAND:
                creature->MaxTurn = 0;
                item->ItemFlags[BOSSFlag_ShieldIsEnabled] = 0;
                if (item->Animation.FrameNumber == GetFrameNumber(item, 30))
                {
                    if (item->ItemFlags[BOSSFlag_AttackType] == PUNA_SummonLaser && item->ItemFlags[BOSSFlag_ItemNumber] != NO_ITEM)
                    {
                        lizardList = FoundAllLizardAISpawnPoint(item, ID_AI_AMBUSH);
                        PunaLaserSummon(item, lizardList[item->ItemFlags[BOSSFlag_ItemNumber]], PunaBossHandBite, head.x, 5);
                    }
                    else
                    {
                        PunaLaser(item, creature->Enemy, PunaBossHandBite, head.x, 10);
                    }
                }

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
