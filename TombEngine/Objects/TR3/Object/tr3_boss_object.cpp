#include "framework.h"
#include "tr3_boss_object.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/effects/effects.h"
#include "Game/effects/spark.h"
#include "Game/collision/collide_room.h"
#include "Objects/TR3/Entity/tr3_punaboss.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Spark;
using namespace TEN::Entities::Creatures::TR3;

namespace TEN::Entities::Object::TR3
{
    void BOSS_SpawnShield(ItemInfo* item, const Vector4& shieldColor)
    {
        if (item->TestFlag(BOSSFlag_Object, BOSS_Shield)) // need ID_BOSS_SHIELD target be loaded !
        {
            short itemNumber = CreateItem();
            if (itemNumber != NO_ITEM)
            {
                auto* spawned = &g_Level.Items[itemNumber];
                spawned->Pose.Position.x = item->Pose.Position.x;
                spawned->Pose.Position.y = item->Pose.Position.y - CLICK(3); // A bit upward.
                spawned->Pose.Position.z = item->Pose.Position.z;
                spawned->Pose.Orientation = item->Pose.Orientation;
                spawned->ObjectNumber = ID_BOSS_SHIELD;
                spawned->RoomNumber = item->RoomNumber;
                spawned->Flags |= IFLAG_ACTIVATION_MASK;
                InitialiseItem(itemNumber);
                spawned->Model.Color = shieldColor;
                spawned->Collidable = true;
                spawned->ItemFlags[0] = 2;
                spawned->Model.Mutator[0].Scale = Vector3(2.0f); // change the model size target be *2.
                spawned->Status = ITEM_ACTIVE;
                AddActiveItem(itemNumber);
            }
        }
    }

    void BOSS_SpawnShockwaveExplosion(ItemInfo* item, const Vector4& explosionColor)
    {
        if (item->TestFlag(BOSSFlag_Object, BOSS_ShockwaveExplosion)) // need ID_BOSS_EXPLOSION_SHOCKWAVE target be loaded !
        {
            short itemNumber = CreateItem();
            if (itemNumber != NO_ITEM)
            {
                auto* spawned = &g_Level.Items[itemNumber];
                spawned->Pose.Position.x = item->Pose.Position.x;
                spawned->Pose.Position.y = item->Pose.Position.y - CLICK(2); // A bit upward.
                spawned->Pose.Position.z = item->Pose.Position.z;
                // random angle for shockwave ring !
                spawned->Pose.Orientation.x = Random::GenerateAngle(-ANGLE(20.0f), ANGLE(20.0f));
                spawned->Pose.Orientation.y = Random::GenerateAngle(-ANGLE(20.0f), ANGLE(20.0f));
                spawned->Pose.Orientation.z = Random::GenerateAngle(-ANGLE(20.0f), ANGLE(20.0f));
                spawned->ObjectNumber = ID_BOSS_EXPLOSION_SHOCKWAVE;
                spawned->RoomNumber = item->RoomNumber;
                spawned->Flags |= IFLAG_ACTIVATION_MASK;
                InitialiseItem(itemNumber);
                auto result = explosionColor;
                result.w = 1;
                spawned->Model.Color = result;
                spawned->Collidable = false; // no collision for this item !
                spawned->ItemFlags[0] = 70; // timer before clear, will fadeout when finished then destroyed !
                spawned->Model.Mutator[0].Scale = Vector3(0.0f); // start without size !
                spawned->Status = ITEM_ACTIVE;
                AddActiveItem(itemNumber);
                SoundEffect(SFX_TR3_BLAST_CIRCLE, &spawned->Pose);
            }
        }
    }

    void BOSS_SpawnShockwaveRing(ItemInfo* item, const Vector3i& pos, const Vector4& ringColor)
    {
        if (item->TestFlag(BOSSFlag_Object, BOSS_ShockwaveExplosion)) // need ID_BOSS_EXPLOSION_RING target be loaded !
        {
            short itemNumber = CreateItem();
            if (itemNumber != NO_ITEM)
            {
                auto* spawned = &g_Level.Items[itemNumber];
                spawned->Pose.Position.x = pos.x;
                spawned->Pose.Position.y = pos.y - CLICK(2); // A bit upward.
                spawned->Pose.Position.z = pos.z;
                // random angle for shockwave ring !
                spawned->Pose.Orientation.x = Random::GenerateAngle(-ANGLE(60.0f), ANGLE(60.0f));
                spawned->Pose.Orientation.y = Random::GenerateAngle();
                spawned->Pose.Orientation.z = Random::GenerateAngle(-ANGLE(60.0f), ANGLE(60.0f));
                spawned->ObjectNumber = ID_BOSS_EXPLOSION_RING;
                spawned->RoomNumber = item->RoomNumber;
                spawned->Flags |= IFLAG_ACTIVATION_MASK;
                InitialiseItem(itemNumber);
                auto result = ringColor;
                result.w = 1;
                spawned->Model.Color = result;
                spawned->Collidable = false; // no collision for this item !
                spawned->ItemFlags[0] = 70; // timer before clear, will fadeout when finished then destroyed !
                spawned->Model.Mutator[0].Scale = Vector3(0.0f); // start without size !
                spawned->Status = ITEM_ACTIVE;
                AddActiveItem(itemNumber);
                SoundEffect(SFX_TR3_BLAST_CIRCLE, &spawned->Pose);
            }
        }
    }

    void BOSS_EffectShieldControl(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];

        if (item->ItemFlags[0] > 0) // item life.
            item->ItemFlags[0]--;
        if (item->ItemFlags[0] <= 0)
        {
            RemoveActiveItem(itemNumber);
            KillItem(itemNumber);
        }

        item->Model.Color = Vector4::Lerp(item->Model.Color, Vector4(item->Model.Color.x - 0.1f, item->Model.Color.y - 0.1f, item->Model.Color.z - 0.1f, item->Model.Color.w), 0.35f);
        item->Pose.Orientation.y += Random::GenerateAngle();
        UpdateItemRoom(itemNumber);
    }

    // Ring and Explosion wave are rotating and scaling up by default !

    void BOSS_EffectShockwaveRingControl(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];

        if (item->ItemFlags[0] > 0) // item life.
            item->ItemFlags[0]--;
        if (item->ItemFlags[0] <= 0)
        {
            item->Model.Color = Vector4::Lerp(item->Model.Color, Vector4(item->Model.Color.x - 0.1f, item->Model.Color.y - 0.1f, item->Model.Color.z - 0.1f, item->Model.Color.w), 0.35f);
            if (item->Model.Color.x <= 0 &&
                item->Model.Color.y <= 0 &&
                item->Model.Color.z <= 0)
            {
                RemoveActiveItem(itemNumber);
                KillItem(itemNumber);
            }
        }

        item->Model.Mutator[0].Scale += Vector3(1.0f);
        UpdateItemRoom(itemNumber);
    }

    void BOSS_EffectShockwaveExplosionControl(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];

        if (item->ItemFlags[0] > 0) // item life.
            item->ItemFlags[0]--;
        if (item->ItemFlags[0] <= 0)
        {
            item->Model.Color = Vector4::Lerp(item->Model.Color, Vector4(item->Model.Color.x - 0.1f, item->Model.Color.y - 0.1f, item->Model.Color.z - 0.1f, item->Model.Color.w), 0.35f);
            if (item->Model.Color.x <= 0 &&
                item->Model.Color.y <= 0 &&
                item->Model.Color.z <= 0)
            {
                RemoveActiveItem(itemNumber);
                KillItem(itemNumber);
            }
        }

        item->Pose.Orientation.y += ANGLE(5);
        item->Model.Mutator[0].Scale += Vector3(0.5f);
        UpdateItemRoom(itemNumber);
    }

    void BOSS_TriggerExplosionSmoke(const Vector3i& pos)
    {
        auto* spark = GetFreeParticle();
        spark->on = true;
        spark->sR = 75;
        spark->sG = 75;
        spark->sB = 175;
        spark->dR = 25;
        spark->dG = 25;
        spark->dB = 100;
        spark->colFadeSpeed = 8;
        spark->fadeToBlack = 64;
        spark->life = spark->sLife = (GetRandomControl() & 0x1F) + 96;
        spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
        spark->x = (GetRandomControl() & 0x1F) + pos.x - 16;
        spark->y = (GetRandomControl() & 0x1F) + pos.y - 16;
        spark->z = (GetRandomControl() & 0x1F) + pos.z - 16;
        spark->xVel = ((GetRandomControl() & 0xFFF) - 2048) >> 2;
        spark->yVel = GetRandomControl() - 128;
        spark->zVel = ((GetRandomControl() & 0xFFF) - 2048) >> 2;
        spark->friction = 6;
        spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
        spark->rotAng = GetRandomControl() & 0xFFF;

        if (GetRandomControl() & 1)
            spark->rotAdd = -16 - (GetRandomControl() & 0xF);
        else
            spark->rotAdd = (GetRandomControl() & 0xF) + 16;

        spark->scalar = 3;
        spark->gravity = -3 - (GetRandomControl() & 3);
        spark->maxYvel = -8 - (GetRandomControl() & 3);
        spark->dSize = (GetRandomControl() & 0x1F) + 256;
        spark->sSize = spark->dSize / 2;
        spark->size = spark->dSize / 2;
    }

    void BOSS_ExplodeBoss(short itemNumber, ItemInfo* item, int deathCountToDie, const Vector4& explosionColor)
    {
        Vector3i random;

        // disable shield
        item->SetFlag(BOSSFlag_ShieldIsEnabled, 0);
        item->HitPoints = NOT_TARGETABLE;

        // start doing the explosion (entity will do the count !)
        short counter = item->ItemFlags[BOSSFlag_ExplodeCount];
        if (counter == 1)
        {
            BOSS_SpawnShockwaveExplosion(item, explosionColor);
            for (int i = 0; i < 3; i++)
            {
                random.x = item->Pose.Position.x + Random::GenerateInt(-512, 512);
                random.y = (item->Pose.Position.y - CLICK(3)) + Random::GenerateInt(-512, 512);
                random.z = item->Pose.Position.z + Random::GenerateInt(-512, 512);
                BOSS_TriggerExplosionSmoke(random);
            }
        }

        if (counter == 10 ||
            counter == 20 ||
            counter == 30 ||
            counter == 40 ||
            counter == 50 ||
            counter == 60)
        {
            
            for (int i = 0; i < 3; i++)
            {
                random.x = item->Pose.Position.x + Random::GenerateInt(-512, 512);
                random.y = (item->Pose.Position.y - CLICK(3)) + Random::GenerateInt(-512, 512);
                random.z = item->Pose.Position.z + Random::GenerateInt(-512, 512);
                BOSS_TriggerExplosionSmoke(random);
            }
            random.x = item->Pose.Position.x + Random::GenerateInt(-64, 64);
            random.y = (item->Pose.Position.y - CLICK(2)) + Random::GenerateInt(-64, 64);
            random.z = item->Pose.Position.z + Random::GenerateInt(-64, 64);
            BOSS_SpawnShockwaveRing(item, random, explosionColor);
        }

        TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y - CLICK(2), item->Pose.Position.z, counter / 2, explosionColor.x * 255, explosionColor.y * 255, explosionColor.z * 255);
        if (counter == deathCountToDie)
            CreatureDie(itemNumber, true);
    }

    void BOSS_CheckForRequiredObjects(ItemInfo* item)
    {
        short flagResult = 0;
        if (item->ObjectNumber == ID_PUNA_BOSS && Objects[ID_LIZARD].loaded)
            flagResult |= BOSS_Lizard;
        if (Objects[ID_BOSS_EXPLOSION_RING].loaded) // These are only for rendering effects, these are not required but highly recommended for nice look in-game.
            flagResult |= BOSS_ShockwaveRing;
        if (Objects[ID_BOSS_EXPLOSION_SHOCKWAVE].loaded)
            flagResult |= BOSS_ShockwaveExplosion;
        if (Objects[ID_BOSS_SHIELD].loaded)
            flagResult |= BOSS_Shield;
        item->SetFlag(BOSSFlag_Object, flagResult);
    }

    void BOSS_SpawnShieldAndRichochetSparksAtPosition(int x, int y, int z, ItemInfo* item, const Vector4& shieldColor)
    {
        BOSS_SpawnShield(item, shieldColor);
        auto target = GameVector(x, y, z, item->RoomNumber);
        auto rotY = Geometry::GetOrientToPoint(item->Pose.Position.ToVector3(), target.ToVector3()).y;
        auto sparkColor = shieldColor;
        sparkColor.w = 1;
        TriggerRicochetSpark(target, rotY, 13, sparkColor);
    }

}
