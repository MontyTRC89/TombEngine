#include "framework.h"
#include "tr3_boss_object.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/effects/effects.h"
#include "Game/collision/collide_room.h"
#include "Objects/TR3/Entity/tr3_punaboss.h"
#include "Specific/setup.h"

using namespace TEN::Entities::Creatures::TR3;

namespace TEN::Entities::Object::TR3
{
    void BOSS_SpawnShield(ItemInfo* item, const Vector4& shieldColor)
    {
        if (CHK_ANY(item->ItemFlags[BOSSFlag_Object], BOSS_Shield)) // need ID_BOSS_SHIELD to be loaded !
        {
            short itemNumber = CreateItem();
            if (itemNumber != NO_ITEM)
            {
                auto* spawned = &g_Level.Items[itemNumber];
                spawned->Pose.Position.x = item->Pose.Position.x;
                spawned->Pose.Position.y = item->Pose.Position.y - CLICK(2); // A bit upward.
                spawned->Pose.Position.z = item->Pose.Position.z;
                spawned->Pose.Orientation = item->Pose.Orientation;
                spawned->ObjectNumber = ID_BOSS_SHIELD;
                spawned->RoomNumber = item->RoomNumber;
                InitialiseItem(itemNumber);
                spawned->Color = shieldColor;
                spawned->Collidable = true;
                spawned->ItemFlags[0] = 2;
                spawned->Model.Mutator[0].Scale = Vector3(2.0f); // change the model size to be *2.
                spawned->Status = ITEM_ACTIVE;
                spawned->Flags |= IFLAG_ACTIVATION_MASK;
                AddActiveItem(itemNumber);
            }
        }
    }

    void BOSS_SpawnShockwaveExplosion(ItemInfo* item, const Vector4& explosionColor)
    {
        if (CHK_ANY(item->ItemFlags[BOSSFlag_Object], BOSS_ShockwaveExplosion)) // need ID_BOSS_EXPLOSION_SHOCKWAVE to be loaded !
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
                InitialiseItem(itemNumber);
                spawned->Color = explosionColor;
                spawned->Collidable = false; // no collision for this item !
                spawned->ItemFlags[0] = 70; // timer before clear, will fadeout when finished then destroyed !
                spawned->Model.Mutator[0].Scale = Vector3(0.0f); // start without size !
                spawned->Status = ITEM_ACTIVE;
                spawned->Flags |= IFLAG_ACTIVATION_MASK;
                AddActiveItem(itemNumber);
                SoundEffect(SFX_TR3_BLAST_CIRCLE, &item->Pose);
            }
        }
    }

    void BOSS_SpawnShockwaveRing(ItemInfo* item, const Vector4& ringColor)
    {
        if (CHK_ANY(item->ItemFlags[BOSSFlag_Object], BOSS_ShockwaveRing)) // need ID_BOSS_EXPLOSION_RING to be loaded !
        {
            short itemNumber = CreateItem();
            if (itemNumber != NO_ITEM)
            {
                auto* spawned = &g_Level.Items[itemNumber];
                spawned->Pose.Position.x = item->Pose.Position.x;
                spawned->Pose.Position.y = item->Pose.Position.y - CLICK(2); // A bit upward.
                spawned->Pose.Position.z = item->Pose.Position.z;
                // random angle for shockwave ring !
                spawned->Pose.Orientation.x = Random::GenerateAngle(-ANGLE(60.0f), ANGLE(60.0f));
                spawned->Pose.Orientation.y = Random::GenerateAngle();
                spawned->Pose.Orientation.z = Random::GenerateAngle(-ANGLE(60.0f), ANGLE(60.0f));
                spawned->ObjectNumber = ID_BOSS_EXPLOSION_RING;
                spawned->RoomNumber = item->RoomNumber;
                InitialiseItem(itemNumber);
                Vector4 resultColor = ringColor;
                resultColor.w = 1; // avoid the ring to have transparency (at last for the color).
                spawned->Color = resultColor;
                spawned->Collidable = false; // no collision for this item !
                spawned->ItemFlags[0] = 70; // timer before clear, will fadeout when finished then destroyed !
                spawned->Model.Mutator[0].Scale = Vector3(0.0f); // start without size !
                spawned->Status = ITEM_ACTIVE;
                spawned->Flags |= IFLAG_ACTIVATION_MASK;
                AddActiveItem(itemNumber);
                SoundEffect(SFX_TR3_BLAST_CIRCLE, &item->Pose);
            }
        }
    }

    void BOSS_EffectShieldControl(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];

        if (item->ItemFlags[0] > 0)
            item->ItemFlags[0]--;
        if (item->ItemFlags[0] <= 0)
        {
            RemoveActiveItem(itemNumber);
            KillItem(itemNumber);
        }

        item->Color = Vector4::Lerp(item->Color, Vector4(item->Color.x - 0.1f, item->Color.y - 0.1f, item->Color.z - 0.1f, item->Color.w), 0.35f);
        item->Pose.Orientation.y += Random::GenerateAngle();

        auto& result = GetCollision(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);
        if (result.RoomNumber != item->RoomNumber)
            ItemNewRoom(itemNumber, result.RoomNumber);
    }

    // Ring and Explosion wave are rotating and scaling up by default !

    void BOSS_EffectShockwaveRingControl(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];

        if (item->ItemFlags[0] > 0)
            item->ItemFlags[0]--;
        if (item->ItemFlags[0] <= 0)
        {
            item->Color = Vector4::Lerp(item->Color, Vector4(item->Color.x - 0.1f, item->Color.y - 0.1f, item->Color.z - 0.1f, item->Color.w), 0.35f);
            if (item->Color.x <= 0 &&
                item->Color.y <= 0 &&
                item->Color.z <= 0)
            {
                RemoveActiveItem(itemNumber);
                KillItem(itemNumber);
            }
        }

        item->Model.Mutator[0].Scale += Vector3(1.0f);

        auto& result = GetCollision(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);
        if (result.RoomNumber != item->RoomNumber)
            ItemNewRoom(itemNumber, result.RoomNumber);
    }

    void BOSS_EffectShockwaveExplosionControl(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];

        if (item->ItemFlags[0] > 0)
            item->ItemFlags[0]--;
        if (item->ItemFlags[0] <= 0)
        {
            item->Color = Vector4::Lerp(item->Color, Vector4(item->Color.x - 0.1f, item->Color.y - 0.1f, item->Color.z - 0.1f, item->Color.w), 0.35f);
            if (item->Color.x <= 0 &&
                item->Color.y <= 0 &&
                item->Color.z <= 0)
            {
                RemoveActiveItem(itemNumber);
                KillItem(itemNumber);
            }
        }

        item->Pose.Orientation.y += ANGLE(5);
        item->Model.Mutator[0].Scale += Vector3(0.5f);

        auto& result = GetCollision(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);
        if (result.RoomNumber != item->RoomNumber)
            ItemNewRoom(itemNumber, result.RoomNumber);
    }

    void BOSS_EffectShockwaveDummyCollision(short itemNumber, ItemInfo* laraitem, CollisionInfo* coll)
    {
        // need this for AddActiveItem() to activate these shockwave effect. (also we don't want collision to be enabled for them !)
        // AddActiveItem() check if the objectNumber have collision == nullptr, if true, then it wont activate the object !
    }

    void BOSS_ExplodeBoss(ItemInfo* item, const Vector4& explosionColor)
    { 
        // disable shield
        //item->ItemFlags[BOSSFlag_ShieldIsEnabled] = 0;
        item->HitPoints = NOT_TARGETABLE;

        // start doing the explosion (entity will do the count !)
        short counter = item->ItemFlags[BOSSFlag_ExplodeCount];
        if (counter == 1)
            BOSS_SpawnShockwaveExplosion(item, explosionColor);

        if (counter == 5 ||
            counter == 15 ||
            counter == 25 ||
            counter == 35 ||
            counter == 45 ||
            counter == 55)
        {
            BOSS_SpawnShockwaveRing(item, explosionColor);
        }
    }

    void BOSS_CheckForRequiredObjects(ItemInfo* item)
    {
        // Does the lizard are loaded ? if not then it will just fire at lara !
        if (Objects[ID_LIZARD].loaded)
            item->ItemFlags[BOSSFlag_Object] |= BOSS_Lizard;

        // These are only for rendering effects, these are not required but highly recommended for nice look in-game.
        if (Objects[ID_BOSS_EXPLOSION_RING].loaded)
            item->ItemFlags[BOSSFlag_Object] |= BOSS_ShockwaveRing;
        if (Objects[ID_BOSS_EXPLOSION_SHOCKWAVE].loaded)
            item->ItemFlags[BOSSFlag_Object] |= BOSS_ShockwaveExplosion;
        if (Objects[ID_BOSS_SHIELD].loaded)
            item->ItemFlags[BOSSFlag_Object] |= BOSS_Shield;
    }

    void BOSS_SpawnShieldAndRichochetSparksAtPosition(int x, int y, int z, ItemInfo* item, const Vector4& shieldColor)
    {
        BOSS_SpawnShield(item, shieldColor);
        GameVector to;
        to.x = x;
        to.y = y;
        to.z = z;
        to.RoomNumber = item->RoomNumber;
        float rotY = Math::Geometry::GetOrientToPoint(item->Pose.Position.ToVector3(), to.ToVector3()).y;
        TriggerRicochetSpark(to, rotY, 8, 0);
    }

}
