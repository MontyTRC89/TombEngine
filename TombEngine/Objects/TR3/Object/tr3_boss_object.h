#pragma once
#include "Game/items.h"
#include "Game/collision/collide_item.h"

namespace TEN::Entities::Object::TR3
{
    enum BossItemFlags
    {
        BOSSFlag_Object = 0, // BossFlagValue enum
        BOSSFlag_Rotation = 1, // Store rotation for use, like puna when summoning.
        BOSSFlag_ShieldIsEnabled = 2,
        BOSSFlag_AttackType = 3, // Which attack was performed.
        BOSSFlag_AttackCount = 4, // Will change behaviour after some attack.
        BOSSFlag_DeathCount = 5,
        BOSSFlag_ItemNumber = 6, // To check if summon is dead.
        BOSSFlag_ExplodeCount = 7,
    };

    enum BossFlagValue
    {
        BOSS_ShockwaveRing = 0x1,
        BOSS_ShockwaveExplosion = 0x2,
        BOSS_Shield = 0x4,
        BOSS_Lizard = 0x8
    };

    void BOSS_EffectShieldControl(short itemNumber);
    void BOSS_EffectShockwaveRingControl(short itemNumber);
    void BOSS_EffectShockwaveExplosionControl(short itemNumber);
    void BOSS_EffectShockwaveDummyCollision(short itemNumber, ItemInfo* laraitem, CollisionInfo* coll);

    void BOSS_ExplodeBoss(ItemInfo* item, const Vector4& explosionColor);
    void BOSS_CheckForRequiredObjects(ItemInfo* item);

    void BOSS_SpawnShield(ItemInfo* item, const Vector4& shieldColor);
    void BOSS_SpawnShockwaveExplosion(ItemInfo* item, const Vector4& explosionColor);
    void BOSS_SpawnShockwaveRing(ItemInfo* item, const Vector4& ringColor);

    /// <summary>
    /// Used by FireWeapon() of Lara. (only used if ID_BOSS_SHIELD is loaded)
    /// </summary>
    void BOSS_SpawnShieldAndRichochetSparksAtPosition(int x, int y, int z, ItemInfo* item, const Vector4& shieldColor);
}
