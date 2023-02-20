#include "framework.h"
#include "Objects/TR3/Entity/tr3_wasp.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/spark.h"
#include "Game/misc.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Spark;

namespace TEN::Entities::Creatures::TR3
{
    constexpr auto WASP_TURN_RATE_FLOOR_MAX = ANGLE(1.0f);
    constexpr auto WASP_TURN_RATE_FLY_MAX = ANGLE(3.0f);

    constexpr auto WASP_START_TO_FLY_RANGE = SQUARE(BLOCK(3));
    constexpr auto WASP_ATTACK_RANGE = SQUARE(CLICK(2.0f));
    constexpr auto WASP_IDLE_TO_FLY_SPEED = BLOCK(1) / 20;
    constexpr auto WASP_MOVE_CHANCE = 384;
    constexpr auto WASP_LAND_CHANCE = 128;
    constexpr auto WASP_DAMAGE = 50;

    const auto WaspVenomSackLightColor = Vector4(0, 0.35f, 0, 1.0f);
    const auto WaspVenomSackBite = BiteInfo(Vector3::Zero, 10);
    const auto WaspBite = BiteInfo(Vector3::Zero, 12);

    // NOTE: Same value for Animation.
    enum WaspState
    {
        WASP_STATE_FLY_IDLE,
        WASP_STATE_FLY_IDLE_TO_IDLE, // Floor
        WASP_STATE_IDLE,  // Floor
        WASP_STATE_IDLE_TO_FLY_IDLE,
        WASP_STATE_ATTACK, // Flying
        WASP_STATE_FALL,
        WASP_STATE_DEATH,
        WASP_STATE_FLY_FORWARD
    };

    enum WaspAnims
    {
        WASP_ANIM_FLY_IDLE,
        WASP_ANIM_FLY_IDLE_TO_IDLE,
        WASP_ANIM_IDLE,
        WASP_ANIM_IDLE_TO_FLY_IDLE,
        WASP_ANIM_ATTACK,
        WASP_ANIM_FALL,
        WASP_ANIM_DEATH,
        WASP_ANIM_FLY_FORWARD
    };

    static void TriggerWaspParticles(short itemNumber)
    {
        auto* sptr = GetFreeParticle();

        sptr->on = 1;
        sptr->sG = (GetRandomControl() & 63) + 32;
        sptr->sB = sptr->sG >> 1;
        sptr->sR = sptr->sG >> 2;

        sptr->dG = (GetRandomControl() & 31) + 224;
        sptr->dB = sptr->dG >> 1;
        sptr->dR = sptr->dG >> 2;

        sptr->colFadeSpeed = 4;
        sptr->fadeToBlack = 2;
        sptr->sLife = sptr->life = 8;

        sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;

        sptr->extras = 0;
        sptr->dynamic = -1;

        sptr->x = (GetRandomControl() & 15) - 8;
        sptr->y = (GetRandomControl() & 15) - 8;
        sptr->z = (GetRandomControl() & 127) - 64;

        sptr->xVel = (GetRandomControl() & 31) - 16;
        sptr->yVel = (GetRandomControl() & 31) - 16;
        sptr->zVel = (GetRandomControl() & 31) - 16;
        sptr->friction = 2 | (2 << 4);

        sptr->flags = SP_SCALE | SP_ITEM | SP_NODEATTACH | SP_DEF;
        sptr->gravity = sptr->maxYvel = 0;

        sptr->fxObj = itemNumber;
        sptr->nodeNumber = ParticleNodeOffsetIDs::NodeWasp;

        sptr->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
        sptr->scalar = 3;
        auto size = (GetRandomControl() & 3) + 3;
        sptr->size = sptr->sSize = size;
        sptr->dSize = size >> 1;
    }

    static void WaspEffects(short itemNumber, ItemInfo& item)
    {
        // Spawn a light
        auto lightpos = GetJointPosition(&item, WaspVenomSackBite.meshNum, WaspVenomSackBite.Position);
        TriggerDynamicLight(lightpos.x, lightpos.y, lightpos.z, 10, 
            WaspVenomSackLightColor.x * 255,
            WaspVenomSackLightColor.y * 255,
            WaspVenomSackLightColor.z * 255);

        // Then spawn the wasp effect (yes it's called 2 times).
        for (int i = 0; i < 2; i++)
            TriggerWaspParticles(itemNumber);
    }

    void InitialiseWaspMutant(short itemNumber)
    {
        auto& item = g_Level.Items[itemNumber];
        InitialiseCreature(itemNumber);
        SetAnimation(&item, WASP_STATE_IDLE); // Start as flying idle.
    }

    // NOTE: AI_MODIFY not allow the wasp to land,
    // and if he start at the land state (which is by default the case),
    // he will be forced to fly !
    void WaspMutantControl(short itemNumber)
    {
        if (!CreatureActive(itemNumber))
            return;

        auto& item = g_Level.Items[itemNumber];
        auto& creature = *GetCreatureInfo(&item);
        short angle = 0;

        if (item.HitPoints <= 0)
        {
            switch (item.Animation.ActiveState)
            {
            case WASP_STATE_FALL:
                if (item.Pose.Position.y >= item.Floor)
                {
                    item.Pose.Position.y = item.Floor;
                    item.Animation.IsAirborne = false;
                    item.Animation.Velocity.y = 0;
                    item.Animation.TargetState = WASP_STATE_DEATH;
                }
                break;
            case WASP_STATE_DEATH:
                item.Pose.Position.y = item.Floor;
                break;
            default:
                SetAnimation(&item, WASP_STATE_FALL);
                item.Animation.IsAirborne = true;
                item.Animation.Velocity = Vector3::Zero;
                break;
            }
            item.Pose.Orientation.x = 0;
        }
        else
        {
            if (item.AIBits)
                GetAITarget(&creature);

            AI_INFO ai;
            CreatureAIInfo(&item, &ai);

            GetCreatureMood(&item, &ai, true);
            CreatureMood(&item, &ai, true);

            angle = CreatureTurn(&item, creature.MaxTurn);

            switch (item.Animation.ActiveState)
            {
            case WASP_STATE_IDLE:
                creature.MaxTurn = WASP_TURN_RATE_FLOOR_MAX;
                item.Pose.Position.y = item.Floor;

                if (item.HitStatus ||
                    ai.distance < WASP_START_TO_FLY_RANGE ||
                    creature.HurtByLara ||
                    item.AIBits == MODIFY)
                    item.Animation.TargetState = WASP_STATE_IDLE_TO_FLY_IDLE;
                break;
            case WASP_STATE_FLY_IDLE_TO_IDLE:
                creature.MaxTurn = WASP_TURN_RATE_FLOOR_MAX;

                item.Pose.Position.y += WASP_IDLE_TO_FLY_SPEED;
                if (item.Pose.Position.y >= item.Floor)
                    item.Pose.Position.y = item.Floor;

                break;
            case WASP_STATE_FLY_IDLE:
                creature.MaxTurn = WASP_TURN_RATE_FLY_MAX;
                creature.Flags = 0;

                if (item.Animation.RequiredState)
                    item.Animation.TargetState = item.Animation.RequiredState;
                // NOTE: This cause the wasp to wait until the random value is valid or if lara has hit the wasp to move forward which is a bad conception !
                else if (item.HitStatus || GetRandomControl() < WASP_MOVE_CHANCE || item.AIBits == MODIFY)
                    item.Animation.TargetState = WASP_STATE_FLY_FORWARD;
                else if ((creature.Mood == MoodType::Bored || GetRandomControl() < WASP_LAND_CHANCE) && !creature.HurtByLara && item.AIBits != MODIFY)
                    item.Animation.TargetState = WASP_STATE_FLY_IDLE_TO_IDLE;
                else if (ai.ahead && ai.distance < WASP_ATTACK_RANGE)
                    item.Animation.TargetState = WASP_STATE_ATTACK;

                break;
            case WASP_STATE_FLY_FORWARD:
                creature.MaxTurn = WASP_TURN_RATE_FLY_MAX;
                creature.Flags = 0;

                if (item.Animation.RequiredState)
                    item.Animation.TargetState = item.Animation.RequiredState;
                else if ((creature.Mood == MoodType::Bored || GetRandomControl() < WASP_LAND_CHANCE) && !creature.HurtByLara && item.AIBits != MODIFY)
                    item.Animation.TargetState = WASP_STATE_FLY_IDLE;
                else if (ai.ahead && ai.distance < WASP_ATTACK_RANGE)
                    item.Animation.TargetState = WASP_STATE_ATTACK;

                break;
            case WASP_STATE_ATTACK:
                creature.MaxTurn = WASP_TURN_RATE_FLY_MAX;
                if (ai.ahead && ai.distance < WASP_ATTACK_RANGE)
                    item.Animation.TargetState = WASP_STATE_ATTACK;
                else if (ai.distance < WASP_ATTACK_RANGE)
                    item.Animation.TargetState = WASP_STATE_FLY_IDLE;
                else
                {
                    item.Animation.TargetState = WASP_STATE_FLY_IDLE;
                    item.Animation.RequiredState = WASP_STATE_FLY_FORWARD;
                }

                if (!creature.Flags && item.TouchBits.Test(WaspBite.meshNum))
                {
                    DoDamage(creature.Enemy, WASP_DAMAGE);
                    CreatureEffect(&item, WaspBite, DoBloodSplat);
                    creature.Flags = 1;
                }

                break;
            }

            // Avoid spawning dynamic light when dead.
            WaspEffects(itemNumber, item);
        }

        CreatureAnimation(itemNumber, angle, 0);
    }
}
