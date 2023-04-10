#include "framework.h"
#include "tr3_claw_mutant.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Effects/enemy_missile.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/setup.h"

using namespace TEN::Entities::Effects;

namespace TEN::Entities::Creatures::TR3
{
    enum ClawMutantState
    {
        CLAWMUTANT_STATE_STOP,
        CLAWMUTANT_STATE_WALK,
        CLAWMUTANT_STATE_RUN,
        CLAWMUTANT_STATE_RUN_ATTACK,
        CLAWMUTANT_STATE_WALK_ATTACK_LEFT,
        CLAWMUTANT_STATE_WALK_ATTACK_RIGHT,
        CLAWMUTANT_STATE_SLASH_LEFT,
        CLAWMUTANT_STATE_SLASH_RIGHT,
        CLAWMUTANT_STATE_DEATH,
        CLAWMUTANT_STATE_CLAW_ATTACK,
        CLAWMUTANT_STATE_PLASMA_ATTACK
    };

    enum ClawMutantAnim
    {
        CLAWMUTANT_ANIM_IDLE,
        CLAWMUTANT_ANIM_START_WALK_LEFT,
        CLAWMUTANT_ANIM_START_RUN_LEFT,
        CLAWMUTANT_ANIM_STOP_WALK_RIGHT,
        CLAWMUTANT_ANIM_STOP_WALK_LEFT,
        CLAWMUTANT_ANIM_STOP_RUN_RIGHT,
        CLAWMUTANT_ANIM_STOP_RUN_LEFT,
        CLAWMUTANT_ANIM_WALK,
        CLAWMUTANT_ANIM_UNKNOWN,
        CLAWMUTANT_ANIM_RUN,
        CLAWMUTANT_ANIM_RUN_TO_WALK_LEFT,
        CLAWMUTANT_ANIM_RUN_TO_WALK_RIGHT,
        CLAWMUTANT_ANIM_RUN_ATTACK,
        CLAWMUTANT_ANIM_RUN_ATTACK_CANCEL,
        CLAWMUTANT_ANIM_IDLE_ATTACK_LEFT,
        CLAWMUTANT_ANIM_IDLE_ATTACK_RIGHT,
        CLAWMUTANT_ANIM_IDLE_DOUBLE_ATTACK,
        CLAWMUTANT_ANIM_WALK_ATTACK_LEFT,
        CLAWMUTANT_ANIM_WALK_ATTACK_RIGHT,
        CLAWMUTANT_ANIM_SHOOT,
        CLAWMUTANT_ANIM_DEATH
    };

    constexpr auto CLAWMUTANT_WALK_TURN_RATE_MAX = ANGLE(3.0f);
    constexpr auto CLAWMUTANT_RUN_TURN_RATE_MAX = ANGLE(4.0f);
    constexpr auto CLAWMUTANT_SLASH_RANGE = SQUARE(BLOCK(1));
    constexpr auto CLAWMUTANT_RUN_ATTACK_RANGE = SQUARE(BLOCK(2));
    constexpr auto CLAWMUTANT_CLAW_RANGE = SQUARE(BLOCK(1.25f));
    constexpr auto CLAWMUTANT_SHOOT_RANGE = SQUARE(BLOCK(4));
    constexpr auto CLAWMUTANT_DAMAGE = 100;
    constexpr auto CLAWMUTANT_PLASMA_DAMAGE = 200;
    constexpr auto CLAWMUTANT_PLASMA_SPEED = 250;
    constexpr auto CLAWMUTANT_ROAR_CHANCE = 0x60;
    constexpr auto CLAWMUTANT_WALK_CHANCE = CLAWMUTANT_ROAR_CHANCE + 0x400;

    const auto ClawMutantLeftBite = BiteInfo(Vector3(19.0f, -13.0f, 3.0f), 4);
    const auto ClawMutantRightBite = BiteInfo(Vector3(19.0f, -13.0f, 3.0f), 7);

    static void TriggerMutantPlasma(short itemNumber)
    {
        auto* sptr = GetFreeParticle();

        sptr->on = 1;
        sptr->sB = 255;
        sptr->sG = 48 + (GetRandomControl() & 31);
        sptr->sR = 48;

        sptr->dB = 192 + (GetRandomControl() & 63);
        sptr->dG = 128 + (GetRandomControl() & 63);
        sptr->dR = 32;

        sptr->colFadeSpeed = 12 + (GetRandomControl() & 3);
        sptr->fadeToBlack = 8;
        sptr->sLife = sptr->life = (GetRandomControl() & 7) + 24;

        sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;

        sptr->extras = 0;
        sptr->dynamic = -1;

        sptr->x = ((GetRandomControl() & 15) - 8);
        sptr->y = 0;
        sptr->z = ((GetRandomControl() & 15) - 8);

        sptr->xVel = ((GetRandomControl() & 31) - 16);
        sptr->yVel = (GetRandomControl() & 15) + 16;
        sptr->zVel = ((GetRandomControl() & 31) - 16);
        sptr->friction = 3;

        if (GetRandomControl() & 1)
        {
            sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_ITEM | SP_NODEATTACH;
            sptr->rotAng = GetRandomControl() & 4095;
            if (GetRandomControl() & 1)
                sptr->rotAdd = -(GetRandomControl() & 15) - 16;
            else
                sptr->rotAdd = (GetRandomControl() & 15) + 16;
        }
        else
            sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTACH;

        sptr->gravity = (GetRandomControl() & 31) + 16;
        sptr->maxYvel = (GetRandomControl() & 7) + 16;

        sptr->fxObj = itemNumber;
        sptr->nodeNumber = ParticleNodeOffsetIDs::NodeClawMutantPlasma;

        sptr->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
        sptr->scalar = 1;
        int size = (GetRandomControl() & 31) + 64;
        sptr->size = sptr->sSize = size;
        sptr->dSize = size >> 2;
    }

    void TriggerPlasmaBallFlame(short fxNumber, int xv, int yv, int zv, Vector3 offset, int life)
    {
        auto* sptr = GetFreeParticle();

        sptr->on = 1;
        sptr->sB = 255;
        sptr->sG = 48 + (GetRandomControl() & 31);
        sptr->sR = 48;

        sptr->dB = 192 + (GetRandomControl() & 63);
        sptr->dG = 128 + (GetRandomControl() & 63);
        sptr->dR = 32;

        sptr->colFadeSpeed = 12 + (GetRandomControl() & 3);
        sptr->fadeToBlack = 8;
        sptr->sLife = sptr->life = (GetRandomControl() & 7) + life;

        sptr->blendMode = BLENDMODE_ADDITIVE;

        sptr->extras = 0;
        sptr->dynamic = -1;

        sptr->x = offset.x + ((GetRandomControl() & 15) - 8);
        sptr->y = 0;
        sptr->z = offset.z + ((GetRandomControl() & 15) - 8);

        sptr->xVel = xv;
        sptr->yVel = yv;
        sptr->zVel = zv;
        sptr->friction = 5;

        if (GetRandomControl() & 1)
        {
            sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX;
            sptr->rotAng = GetRandomControl() & 4095;
            if (GetRandomControl() & 1)
                sptr->rotAdd = -(GetRandomControl() & 15) - 16;
            else
                sptr->rotAdd = (GetRandomControl() & 15) + 16;
        }
        else
        {
            sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX;
        }

        sptr->fxObj = fxNumber;
        sptr->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
        sptr->scalar = 1;
        sptr->gravity = sptr->maxYvel = 0;

        int size = (GetRandomControl() & 31) + 64;
        sptr->size = sptr->sSize = size;
        sptr->dSize = size >> 4;

        sptr->yVel = (GetRandomControl() & 511) - 256;
        sptr->xVel <<= 1;
        sptr->zVel <<= 1;
        sptr->scalar = 2;
        sptr->friction = 85;
        sptr->gravity = 22;
    }

    static void TriggerMutantPlasmaBall(ItemInfo* item, CreatureInfo* creature, short yangle, short xangle)
    {
        short fx_number = CreateNewEffect(item->RoomNumber);
        if (fx_number != NO_ITEM)
        {
            auto jointPos = GetJointPosition(item, 13, Vector3i(-32, -16, -119));
            auto enemyPos = creature->Enemy->Pose.Position;
            if (creature->Enemy->IsLara() && GetLaraInfo(creature->Enemy)->Control.IsLow)
                enemyPos.y += CLICK(1);
            auto angles = Math::Geometry::GetOrientToPoint(enemyPos.ToVector3(), jointPos.ToVector3());
            
            auto* fx = &EffectList[fx_number];
            fx->pos.Position = jointPos;
            fx->pos.Orientation.y = item->Pose.Orientation.y + yangle;
            fx->pos.Orientation.x = (-angles.x) + ANGLE(4.0f);
            fx->objectNumber = ID_ENERGY_BUBBLES;
            fx->color = Vector4::Zero;
            fx->speed = CLAWMUTANT_PLASMA_SPEED;
            fx->flag2 = CLAWMUTANT_PLASMA_DAMAGE;
            fx->flag1 = (int)MissileType::ClawMutantPlasma;
            fx->fallspeed = 0;
        }
    }

    static void TriggerMutantPlasmaLight(ItemInfo* item)
    {
        int bright = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
        if (bright > 16)
        {
            bright = g_Level.Anims[item->Animation.AnimNumber].frameBase + 28 + 16 - item->Animation.FrameNumber;
            if (bright > 16)
                bright = 16;
        }

        if (bright > 0)
        {
            auto pos = GetJointPosition(item, 13, Vector3i(-32, -16, -192));
            int rnd = GetRandomControl();
            byte r, g, b;

            b = 31 - ((rnd >> 4) & 3);
            g = 24 - ((rnd >> 6) & 3);
            r = rnd & 7;

            r = (r * bright) >> 4;
            g = (g * bright) >> 4;
            b = (b * bright) >> 4;
            TriggerDynamicLight(pos.x, pos.y, pos.z, bright, r, g, b);
        }
    }

    void ClawMutantControl(short itemNumber)
    {
        if (!CreatureActive(itemNumber))
            return;

        auto* item = &g_Level.Items[itemNumber];
        auto* object = &Objects[item->ObjectNumber];
        auto* creature = GetCreatureInfo(item);
        short head = 0;
        short torso_x = 0, torso_y = 0;
        short angle = 0;

        if (item->HitPoints <= 0)
        {
            if (item->Animation.ActiveState != CLAWMUTANT_STATE_DEATH)
                SetAnimation(item, CLAWMUTANT_ANIM_DEATH);

            int frameEnd = g_Level.Anims[object->animIndex + CLAWMUTANT_ANIM_DEATH].frameEnd;
            if (item->Animation.FrameNumber >= frameEnd)
            {
                CreatureDie(itemNumber, true);
            }
        }
        else
        {
            if (item->AIBits)
                GetAITarget(creature);

            AI_INFO ai;
            CreatureAIInfo(item, &ai);
            GetCreatureMood(item, &ai, ai.zoneNumber == ai.enemyZone ? true : false);
            CreatureMood(item, &ai, ai.zoneNumber == ai.enemyZone ? true : false);

            angle = CreatureTurn(item, creature->MaxTurn);
            bool canShoot = Targetable(item, &ai) && ((ai.distance > CLAWMUTANT_SHOOT_RANGE && !item->ItemFlags[0]) || ai.zoneNumber != ai.enemyZone);

            switch (item->Animation.ActiveState)
            {
            case CLAWMUTANT_STATE_STOP:
                creature->Flags = 0;
                creature->MaxTurn = 0;

                if (item->AIBits & GUARD)
                {
                    head = AIGuard(creature);
                    item->Animation.TargetState = CLAWMUTANT_STATE_STOP;
                    break;
                }
                else if (item->AIBits & PATROL1)
                {
                    head = 0;
                    item->Animation.TargetState = CLAWMUTANT_STATE_WALK;
                }
                else if (creature->Mood == MoodType::Escape)
                {
                    item->Animation.TargetState = CLAWMUTANT_STATE_RUN;
                }
                else if (ai.bite && ai.distance < CLAWMUTANT_SLASH_RANGE)
                {
                    torso_y = ai.angle;
                    torso_x = ai.xAngle;
                    if (ai.angle < 0)
                        item->Animation.TargetState = CLAWMUTANT_STATE_SLASH_LEFT;
                    else
                        item->Animation.TargetState = CLAWMUTANT_STATE_SLASH_RIGHT;
                }
                else if (ai.bite && ai.distance < CLAWMUTANT_CLAW_RANGE)
                {
                    torso_y = ai.angle;
                    torso_x = ai.xAngle;
                    item->Animation.TargetState = CLAWMUTANT_STATE_CLAW_ATTACK;
                }
                else if (canShoot)
                {
                    item->Animation.TargetState = CLAWMUTANT_STATE_PLASMA_ATTACK;
                }
                else if (creature->Mood == MoodType::Bored)
                {
                    if (GetRandomControl() < CLAWMUTANT_WALK_CHANCE)
                        item->Animation.TargetState = CLAWMUTANT_STATE_WALK;
                }
                else if (item->Animation.RequiredState != NO_STATE)
                {
                    item->Animation.TargetState = item->Animation.RequiredState;
                }
                else
                {
                    item->Animation.TargetState = CLAWMUTANT_STATE_RUN;
                }
                break;
            case CLAWMUTANT_STATE_WALK:
                creature->Flags = 0;
                creature->MaxTurn = CLAWMUTANT_WALK_TURN_RATE_MAX;

                if (ai.ahead)
                    head = ai.angle;

                if (item->AIBits & PATROL1)
                {
                    item->Animation.TargetState = CLAWMUTANT_STATE_WALK;
                    head = 0;
                }
                else if (ai.bite && ai.distance < CLAWMUTANT_CLAW_RANGE)
                {
                    if (ai.angle < 0)
                        item->Animation.TargetState = CLAWMUTANT_STATE_WALK_ATTACK_LEFT;
                    else
                        item->Animation.TargetState = CLAWMUTANT_STATE_WALK_ATTACK_RIGHT;
                }
                else if (canShoot)
                {
                    item->Animation.TargetState = CLAWMUTANT_STATE_STOP;
                }
                else if (creature->Mood == MoodType::Escape || creature->Mood == MoodType::Attack)
                {
                    item->Animation.TargetState = CLAWMUTANT_STATE_RUN;
                }
                break;
            case CLAWMUTANT_STATE_RUN:
                creature->Flags = 0;
                creature->MaxTurn = CLAWMUTANT_RUN_TURN_RATE_MAX;

                if (ai.ahead)
                    head = ai.angle;

                if (item->AIBits & GUARD)
                    item->Animation.TargetState = CLAWMUTANT_STATE_STOP;
                else if (creature->Mood == MoodType::Bored)
                    item->Animation.TargetState = CLAWMUTANT_STATE_STOP;
                else if (creature->Flags != 0 && ai.ahead)
                    item->Animation.TargetState = CLAWMUTANT_STATE_STOP;
                else if (ai.bite && ai.distance < CLAWMUTANT_RUN_ATTACK_RANGE)
                {
                    if (creature->Enemy != nullptr && creature->Enemy->Animation.Velocity.z == 0)
                        item->Animation.TargetState = CLAWMUTANT_STATE_STOP;
                    else
                        item->Animation.TargetState = CLAWMUTANT_STATE_RUN_ATTACK;
                }
                else if (canShoot)
                {
                    item->Animation.TargetState = CLAWMUTANT_STATE_STOP;
                }
                break;
            case CLAWMUTANT_STATE_WALK_ATTACK_LEFT:
            case CLAWMUTANT_STATE_WALK_ATTACK_RIGHT:
            case CLAWMUTANT_STATE_RUN_ATTACK:
            case CLAWMUTANT_STATE_CLAW_ATTACK:
            case CLAWMUTANT_STATE_SLASH_LEFT:
            case CLAWMUTANT_STATE_SLASH_RIGHT:
                if (ai.ahead)
                {
                    torso_y = ai.angle;
                    torso_x = ai.xAngle;
                }

                if (!(creature->Flags & 0x1) && item->TouchBits.Test(ClawMutantLeftBite.meshNum))
                {
                    DoDamage(creature->Enemy, CLAWMUTANT_DAMAGE);
                    CreatureEffect(item, ClawMutantLeftBite, DoBloodSplat);
                    creature->Flags |= 0x1;
                }

                if (!(creature->Flags & 0x2) && item->TouchBits.Test(ClawMutantRightBite.meshNum))
                {
                    DoDamage(creature->Enemy, CLAWMUTANT_DAMAGE);
                    CreatureEffect(item, ClawMutantRightBite, DoBloodSplat);
                    creature->Flags |= 0x2;
                }

                break;
            case CLAWMUTANT_STATE_PLASMA_ATTACK:
                if (ai.ahead)
                {
                    torso_y = ai.angle;
                    torso_x = ai.xAngle;
                }

                if (item->Animation.FrameNumber == GetFrameNumber(item, 0) && Random::GenerateInt(0, 3) == 0)
                    item->ItemFlags[0] = 1;

                if (item->Animation.FrameNumber < GetFrameNumber(item, 28))
                    TriggerMutantPlasma(itemNumber);
                else if (item->Animation.FrameNumber == GetFrameNumber(item, 28))
                    TriggerMutantPlasmaBall(item, creature, torso_y, torso_x);

                TriggerMutantPlasmaLight(item);
                break;
            }
        }

        CreatureJoint(item, 0, torso_x);
        CreatureJoint(item, 1, torso_y);
        CreatureJoint(item, 2, head);
        CreatureAnimation(itemNumber, angle, 0);
    }
}
