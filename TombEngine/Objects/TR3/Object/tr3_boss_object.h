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
		BOSSFlag_ExplodeCount = 7
	};

	enum BossFlagValue
	{
		BOSS_ShockwaveRing      = (1 << 0),
		BOSS_ShockwaveExplosion = (1 << 1),
		BOSS_Shield             = (1 << 2),
		BOSS_Lizard             = (1 << 3)
	};

	extern void BOSS_EffectShieldControl(int itemNumber);
	extern void BOSS_EffectShockwaveRingControl(int itemNumber);
	extern void BOSS_EffectShockwaveExplosionControl(int itemNumber);

	// Can really die after deathCount: 60
	extern void BOSS_ExplodeBoss(int itemNumber, ItemInfo& item, int deathCountToDie, const Vector4& color);
	extern void BOSS_CheckForRequiredObjects(ItemInfo& item);

	extern void BOSS_SpawnShield(const ItemInfo& item, const Vector4& color);
	extern void BOSS_SpawnShockwaveExplosion(const ItemInfo& item, const Vector4& color);
	extern void BOSS_SpawnShockwaveRing(const ItemInfo& item, const Vector3& pos, const Vector4& color);

	// Used by FireWeapon() of Lara. (only used if ID_BOSS_SHIELD is loaded)
	void BOSS_SpawnShieldAndRichochetSparksAtPosition(const ItemInfo& item, const Vector3& pos, const Vector4& color);
}
