#pragma once
#include "Game/items.h"

namespace TEN::Effects::Boss
{
	enum BossItemFlags
	{
		BOSSFlag_Object = 0,		  // BossFlagValue enum.
		BOSSFlag_Rotation = 1,		  // Store rotation for use (e.g. Puna when summoning).
		BOSSFlag_ShieldIsEnabled = 2,
		BOSSFlag_AttackType = 3,
		BOSSFlag_AttackCount = 4,	  // Change behaviour after some attack.
		BOSSFlag_DeathCount = 5,
		BOSSFlag_ItemNumber = 6,	  // Check if summon is dead.
		BOSSFlag_ExplodeCount = 7
	};

	enum BossFlagValue
	{
		BOSS_ShockwaveRing      = (1 << 0),
		BOSS_ShockwaveExplosion = (1 << 1),
		BOSS_Shield             = (1 << 2),
		BOSS_Lizard             = (1 << 3)
	};

	void ShieldControl(int itemNumber);
	void ShockwaveRingControl(int itemNumber);
	void ShockwaveExplosionControl(int itemNumber);

	// Can really die after deathCount 60.
	void ExplodeBoss(int itemNumber, ItemInfo& item, int deathCountToDie, const Vector4& color);
	void CheckForRequiredObjects(ItemInfo& item);

	void SpawnShield(const ItemInfo& item, const Vector4& color);
	void SpawnShockwaveExplosion(const ItemInfo& item, const Vector4& color);
	void SpawnShockwaveRing(const ItemInfo& item, const Vector3& pos, const Vector4& color);

	// Used by player's FireWeapon(), only if ID_BOSS_SHIELD is loaded.
	void SpawnShieldAndRichochetSparks(const ItemInfo& item, const Vector3& pos, const Vector4& color);
}
