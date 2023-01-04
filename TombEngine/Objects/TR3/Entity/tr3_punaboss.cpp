#include "framework.h"
#include "tr3_punaboss.h"
#include "Specific/level.h"
#include "Game/misc.h"
#include "Game/control/box.h"
#include "Game/control/los.h"
#include "Game/effects/lightning.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara_helpers.h"
#include "Specific/setup.h"
#include "Objects/TR3/Object/tr3_boss_object.h"

using namespace TEN::Entities::Object::TR3;
using namespace TEN::Entities::Object::TR3::Boss;
using namespace TEN::Effects::Lightning;
using namespace TEN::Effects::Items;

namespace TEN::Entities::Creatures::TR3
{
    const auto PunaBossEffectColor = Vector4(0.0f, 0.75f, 0.75f, 1.0f);
    const auto PunaBossHeadBite = BiteInfo(0, 0, 0, 8);
    const auto PunaBossHandBite = BiteInfo(0, 0, 0, 14);

    constexpr auto PUNABOSS_ITEM_ROTATE_RATE_MAX = ANGLE(7.0f);
    constexpr auto PUNABOSS_HEAD_X_ANGLE_MAX = ANGLE(20);
    constexpr auto PUMABOSS_TURN_RATE_MAX = ANGLE(3.0f);
    constexpr auto PUMABOSS_ADJUST_LASER_XANGLE = ANGLE(3);
    constexpr auto PUNABOSS_EXPLOSION_COUNT_MAX = 120;
    constexpr auto PUNABOSS_LASER_DAMAGE = 350;
    constexpr auto PUNABOSS_MAX_HEAD_ATTACK = 4;
    constexpr auto PUNABOSS_SHOOTING_DISTANCE = BLOCK(20);
    constexpr auto PUNABOSS_AWAIT_LARA_DISTANCE = BLOCK(2.5f);

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
        PUNA_AwaitLara,
        PUNA_DeathLaser,
        PUNA_SummonLaser,
        PUNA_Wait // when the summon is fighting !
    };

    std::vector<short> FoundAllLizards(ItemInfo* item)
    {
        std::vector<short> itemList;
        for (short itemNumber = 0; itemNumber < g_Level.NumItems; itemNumber++)
        {
            auto& result = g_Level.Items[itemNumber];
            if (result.ObjectNumber == ID_LIZARD && result.RoomNumber == item->RoomNumber && 
                result.HitPoints > 0 && !(result.Flags & IFLAG_KILLED) && result.Status == ITEM_INVISIBLE)
                itemList.push_back(itemNumber);
        }
        return itemList;
    }

    bool FoundAnyLizardInRooms(ItemInfo* item, bool duringInitialize = false)
    {
        int lizardCount = 0;
        for (auto& result : g_Level.Items)
        {
            if (duringInitialize)
            {
                if (result.ObjectNumber == ID_LIZARD && result.RoomNumber == item->RoomNumber)
                    lizardCount++;
            }
            else // in-game..
            {
                if (result.ObjectNumber == ID_LIZARD && result.RoomNumber == item->RoomNumber &&
                    result.HitPoints > 0 && !(result.Flags & IFLAG_KILLED) && result.Status == ITEM_INVISIBLE)
                    lizardCount++;
            }
        }
        return lizardCount > 0;
    }

    void TriggerSummonSmoke(int x, int y, int z)
    {
        auto* sptr = GetFreeParticle();
        sptr->sR = 16;
        sptr->sG = 64;
        sptr->sB = 0;
        sptr->dR = 8;
        sptr->dG = 32;
        sptr->dB = 0;
        sptr->colFadeSpeed = 16 + (GetRandomControl() & 7);
        sptr->fadeToBlack = 64;
        sptr->sLife = sptr->life = (GetRandomControl() & 15) + 96;

        sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
        sptr->extras = 0;
        sptr->dynamic = -1;

        sptr->x = x + ((GetRandomControl() & 127) - 64);
        sptr->y = y - (GetRandomControl() & 31);
        sptr->z = z + ((GetRandomControl() & 127) - 64);
        sptr->xVel = ((GetRandomControl() & 255) - 128);
        sptr->yVel = -(GetRandomControl() & 15) - 16;
        sptr->zVel = ((GetRandomControl() & 255) - 128);
        sptr->friction = 0;

        if (GetRandomControl() & 1)
        {
            sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_WIND;
            sptr->rotAng = GetRandomControl() & 4095;
            if (GetRandomControl() & 1)
                sptr->rotAdd = -(GetRandomControl() & 7) - 4;
            else
                sptr->rotAdd = (GetRandomControl() & 7) + 4;
        }
        else
            sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_WIND;

        sptr->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
        sptr->scalar = 3;
        sptr->gravity = -(GetRandomControl() & 7) - 8;
        sptr->maxYvel = -(GetRandomControl() & 7) - 4;
        int size = (GetRandomControl() & 128) + 256;
        sptr->size = sptr->sSize = size >> 1;
        sptr->dSize = size;
        sptr->on = true;
    }

    void InitialisePuna(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];
        InitialiseCreature(itemNumber);
        SetAnimation(item, PUNA_IDLE);
        CheckForRequiredObjects(*item);

        // save the angle of puna, it will be used to restore this angle when he is waiting (after summoning the lizard).
        // puna is rotated to not face lara, so add 180° to face her.
        item->SetFlag(BOSSFlag_Rotation, item->Pose.Orientation.y - ANGLE(180.0f));
        item->SetFlag(BOSSFlag_AttackType, PUNA_AwaitLara); // normal behaviour at start !
        item->SetFlag(BOSSFlag_ShieldIsEnabled, 1); // activated at start.
        item->SetFlag(BOSSFlag_AttackCount, 0);
        item->SetFlag(BOSSFlag_DeathCount, 0);
        item->SetFlag(BOSSFlag_ItemNumber, NO_ITEM);
        item->SetFlag(BOSSFlag_ExplodeCount, 0);

        if (!FoundAnyLizardInRooms(item, true)) // if there is no lizard in the current room then remove the flag !
            item->RemoveFlag(BOSSFlag_Object, BOSS_Lizard);
    }

    Vector3i GetLizardTargetPosition(ItemInfo* item, CreatureInfo* creature) // creature is in case the BOSSFlag_ItemNumber is NO_ITEM, to set the target to lara instead.
    {
        if (item->TestFlagDiff(BOSSFlag_ItemNumber, NO_ITEM))
        {
            auto& target = g_Level.Items[item->GetFlag(BOSSFlag_ItemNumber)];
            return target.Pose.Position;
        }
        return creature->Target; // alternative for avoiding crash or something wrong.
    }

    void SpawnLizard(ItemInfo* item)
    {
        if (item->TestFlagDiff(BOSSFlag_ItemNumber, NO_ITEM))
        {
            auto itemNumber = item->GetFlag(BOSSFlag_ItemNumber);
            auto* item = &g_Level.Items[itemNumber];
            for (int lp = 0; lp < 20; lp++)
                TriggerSummonSmoke(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
            AddActiveItem(itemNumber);
            item->ItemFlags[0] = 1; // spawned lizard flag !
        }
    }

    void PunaLaser(ItemInfo* item, CreatureInfo* creature, const Vector3i& pos, const BiteInfo& bite, int intensity, bool isSummon)
    {
        GameVector src = GameVector(GetJointPosition(item, bite.meshNum, bite.Position), item->RoomNumber);
        GameVector target = GameVector(Geometry::GetPointAlongLine(src.ToVector3(), pos.ToVector3(), PUNABOSS_SHOOTING_DISTANCE), creature->Enemy->RoomNumber);
        
        if (isSummon)
        {
            TriggerLightning((Vector3i*)&src, (Vector3i*)&target, intensity, 0, 255, 0, 30, LI_SPLINE | LI_THINOUT, 50, 10);
            TriggerDynamicLight(src.x, src.y, src.z, 20, 0, 255, 0);
            item->SetFlag(BOSSFlag_AttackType, PUNA_Wait);
            SpawnLizard(item);
        }
        else
        {
            Vector3i hitPos = Vector3i::Zero;
            MESH_INFO* mesh = nullptr;
            TriggerLightning((Vector3i*)&src, (Vector3i*)&target, intensity, 0, 255, 255, 30, LI_SPLINE | LI_THINOUT, 50, 10);
            TriggerDynamicLight(src.x, src.y, src.z, 20, 0, 255, 255);
            if (ObjectOnLOS2(&src, &target, &hitPos, &mesh, ID_LARA) == GetLaraInfo(creature->Enemy)->ItemNumber)
            {
                if (creature->Enemy->HitPoints <= PUNABOSS_LASER_DAMAGE)
                {
                    ItemElectricBurn(creature->Enemy);
                    DoDamage(creature->Enemy, PUNABOSS_LASER_DAMAGE);
                }
                else
                    DoDamage(creature->Enemy, PUNABOSS_LASER_DAMAGE);
            }
        }
    }

    short GetPunaHeadAngleTarget(ItemInfo* item, Vector3i target)
    {
        if (!item->TestFlag(BOSSFlag_Object, BOSS_Lizard)) return NO_ITEM;
        auto headJointPos = GetJointPosition(item, PunaBossHeadBite.meshNum);
        auto headAngle = Math::Geometry::GetOrientToPoint(headJointPos.ToVector3(), target.ToVector3());
        return headAngle.y - item->Pose.Orientation.y;
    }

    short SelectLizardItemNumber(ItemInfo* item, CreatureInfo* creature)
    {
        if (!item->TestFlag(BOSSFlag_Object, BOSS_Lizard)) return NO_ITEM;
        std::vector<short> lizardList = FoundAllLizards(item);
        if (lizardList.size() <= 0) return NO_ITEM;
        return lizardList.size() == 1 ? lizardList[0] : lizardList[Random::GenerateInt(0, lizardList.size() - 1)];
    }

    void PunaControl(short itemNumber)
    {
        if (!CreatureActive(itemNumber))
            return;
        
        auto* item = &g_Level.Items[itemNumber];
        auto* creature = GetCreatureInfo(item);
        auto* object = &Objects[item->ObjectNumber];
        static auto targetPosition = Vector3i::Zero;

        auto head = EulerAngles::Zero;
        short angle = 0, oldrotation = 0;
        bool haveTurned = false;
        bool haveAnyLizardLeft = FoundAnyLizardInRooms(item);

        if (item->HitPoints <= 0)
        {
            if (item->Animation.ActiveState != PUNA_STATE_DEATH)
            {
                creature->MaxTurn = 0;
                SetAnimation(item, PUNA_ANIMATION_DEATH);
                SoundEffect(SFX_TR3_PUNA_BOSS_DEATH, &item->Pose);
                item->ItemFlags[BOSSFlag_DeathCount] = 1;
            }

            int frameMaxLess1 = g_Level.Anims[object->animIndex + PUNA_ANIMATION_DEATH].frameEnd;
            if (item->Animation.FrameNumber >= frameMaxLess1)
            {
                item->Animation.FrameNumber = frameMaxLess1; // avoid the object to stop working.
                item->MeshBits.ClearAll();

                if (item->GetFlag(BOSSFlag_ExplodeCount) < PUNABOSS_EXPLOSION_COUNT_MAX)
                    item->ItemFlags[BOSSFlag_ExplodeCount]++;
                if (item->GetFlag(BOSSFlag_ExplodeCount) < PUNABOSS_EXPLOSION_COUNT_MAX)
                    ExplodeBoss(itemNumber, *item, 61, PunaBossEffectColor); // Do explosion effect.

                return;
            }
            else
            {
                auto deathCount = item->GetFlag(BOSSFlag_DeathCount);
                item->Pose.Orientation.z = (Random::GenerateInt() % deathCount) - (item->ItemFlags[BOSSFlag_DeathCount] >> 1);
                if (deathCount < 2048)
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

            if (item->TestFlagEqual(BOSSFlag_AttackType, PUNA_AwaitLara) && creature->Enemy)
            {
                float distance = Vector3i::Distance(creature->Enemy->Pose.Position, item->Pose.Position);
                if (distance <= BLOCK(2.5f))
                    item->SetFlag(BOSSFlag_AttackType, PUNA_DeathLaser);
                creature->JointRotation[0] += PUNABOSS_ITEM_ROTATE_RATE_MAX; // Rotate the object on puna boss chair !
                return;
            }

            // Get target
            if (item->TestFlagEqual(BOSSFlag_AttackType, PUNA_DeathLaser))
            {
                creature->Target = creature->Enemy->Pose.Position;
            }
            else if (item->TestFlag(BOSSFlag_Object, BOSS_Lizard) &&
                     item->TestFlagEqual(BOSSFlag_AttackType, PUNA_SummonLaser) &&
                     item->TestFlagEqual(BOSSFlag_ItemNumber, NO_ITEM) &&
                     item->TestFlagDiff(BOSSFlag_AttackType, PUNA_Wait) && haveAnyLizardLeft)
            {
                // get a random lizard item number !
                item->SetFlag(BOSSFlag_ItemNumber, SelectLizardItemNumber(item, creature));
                creature->Target = GetLizardTargetPosition(item, creature);
            }
            else if (item->TestFlag(BOSSFlag_Object, BOSS_Lizard) &&
                     item->TestFlagEqual(BOSSFlag_AttackType, PUNA_Wait) &&
                     item->TestFlagDiff(BOSSFlag_ItemNumber, NO_ITEM))
            {
                // rotate to idle position (for waiting lara to die by lizard)
                auto targetOrient = EulerAngles(item->Pose.Orientation.x, item->GetFlag(BOSSFlag_Rotation), item->Pose.Orientation.z);
                item->Pose.Orientation.InterpolateConstant(targetOrient, ANGLE(3.0f));

                // check if target is dead !
                auto& summonItem = g_Level.Items[item->GetFlag(BOSSFlag_ItemNumber)];
                if (summonItem.HitPoints <= 0)
                {
                    // Reset the attack type, attack count and itemNumber and restart the sequence !
                    item->SetFlag(BOSSFlag_AttackType, PUNA_DeathLaser);
                    item->SetFlag(BOSSFlag_AttackCount, 0);
                    item->SetFlag(BOSSFlag_ItemNumber, NO_ITEM);
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
                item->SetFlag(BOSSFlag_ShieldIsEnabled, 1);

                if ((item->Animation.TargetState != PUNA_STATE_ATTACK_WITH_HAND && item->Animation.TargetState != PUNA_STATE_ATTACK_WITH_HEAD) &&
                    AI.angle > -ANGLE(1) && AI.angle < ANGLE(1) &&
                    creature->Enemy->HitPoints > 0 &&
                    item->GetFlag(BOSSFlag_AttackCount) < PUNABOSS_MAX_HEAD_ATTACK &&
                    item->TestFlagDiff(BOSSFlag_AttackType, PUNA_SummonLaser) && item->TestFlagDiff(BOSSFlag_AttackType, PUNA_Wait))
                {
                    creature->MaxTurn = 0;
                    targetPosition = creature->Target;
                    targetPosition.y -= CLICK(2);
                    item->SetFlag(BOSSFlag_AttackType, PUNA_DeathLaser);


                    if (Random::GenerateInt(0, 2) == 1) // alternate attack with head/hand, 1/3 chance
                        item->Animation.TargetState = PUNA_STATE_ATTACK_WITH_HEAD;
                    else
                        item->Animation.TargetState = PUNA_STATE_ATTACK_WITH_HAND;
                    if (item->TestFlag(BOSSFlag_Object, BOSS_Lizard) && haveAnyLizardLeft)
                        item->ItemFlags[BOSSFlag_AttackCount]++;
                }
                else if (item->ItemFlags[BOSSFlag_AttackCount] >= PUNABOSS_MAX_HEAD_ATTACK && creature->Enemy->HitPoints > 0 && item->ItemFlags[BOSSFlag_AttackType] != PUNA_Wait)
                {
                    item->SetFlag(BOSSFlag_AttackType, PUNA_SummonLaser);
                    if (item->TestFlagDiff(BOSSFlag_ItemNumber, NO_ITEM))
                    {
                        if (angleHead > -ANGLE(1) && angleHead < ANGLE(1))
                        {
                            targetPosition = creature->Target;
                            targetPosition.y -= CLICK(2);
                            item->Animation.TargetState = PUNA_STATE_ATTACK_WITH_HAND;
                        }
                    }
                }

                break;
            case PUNA_STATE_ATTACK_WITH_HEAD:
                creature->MaxTurn = 0;
                item->SetFlag(BOSSFlag_ShieldIsEnabled, 0);

                if (item->Animation.FrameNumber == GetFrameNumber(item, 14))
                {
                    PunaLaser(item, creature, targetPosition, PunaBossHeadBite, 10, false);
                }

                break;
            case PUNA_STATE_ATTACK_WITH_HAND:
                creature->MaxTurn = 0;
                item->SetFlag(BOSSFlag_ShieldIsEnabled, 0);

                if (item->Animation.FrameNumber == GetFrameNumber(item, 30))
                {
                    if (item->TestFlag(BOSSFlag_Object, BOSS_Lizard) &&
                        item->TestFlagEqual(BOSSFlag_AttackType, PUNA_SummonLaser) &&
                        item->TestFlagDiff(BOSSFlag_ItemNumber, NO_ITEM) && haveAnyLizardLeft)
                        PunaLaser(item, creature, targetPosition, PunaBossHandBite, 5, true);
                    else
                        PunaLaser(item, creature, targetPosition, PunaBossHandBite, 10, false);
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
            // Remove these sound, else they will loop when puna die !
            StopSoundEffect(SFX_TR3_PUNA_BOSS_CHAIR_2);
            StopSoundEffect(SFX_TR3_PUNA_BOSS_TURN_CHAIR);
        }
    }
}
