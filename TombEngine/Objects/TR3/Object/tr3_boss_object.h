#pragma once
#include "Game/items.h"

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

    extern void BOSS_EffectShieldControl(short itemNumber);
    extern void BOSS_EffectShockwaveRingControl(short itemNumber);
    extern void BOSS_EffectShockwaveExplosionControl(short itemNumber);

    /// <summary>
    /// Can really die after deathCount: 60
    /// </summary>
    extern void BOSS_ExplodeBoss(short itemNumber, ItemInfo* item, int deathCountToDie, const Vector4& explosionColor);
    extern void BOSS_CheckForRequiredObjects(ItemInfo* item);

    extern void BOSS_SpawnShield(ItemInfo* item, const Vector4& shieldColor);
    extern void BOSS_SpawnShockwaveExplosion(ItemInfo* item, const Vector4& explosionColor);
    extern void BOSS_SpawnShockwaveRing(ItemInfo* item, const Vector3i& pos, const Vector4& ringColor);

    /// <summary>
    /// Used by FireWeapon() of Lara. (only used if ID_BOSS_SHIELD is loaded)
    /// </summary>
    extern void BOSS_SpawnShieldAndRichochetSparksAtPosition(int x, int y, int z, ItemInfo* item, const Vector4& shieldColor);
}
