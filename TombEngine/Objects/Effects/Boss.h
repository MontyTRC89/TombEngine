#pragma once
#include "Game/items.h"

namespace TEN::Effects::Boss
{
	enum class BossItemFlags
	{
		Object = 0,		  // BossFlagValue enum.
		Rotation = 1,		  // Store rotation for use (e.g. Puna when summoning).
		ShieldIsEnabled = 2,
		AttackType = 3,
		AttackCount = 4,	  // Change behaviour after some attack.
		DeathCount = 5,
		ItemNumber = 6,	  // Check if summon is dead.
		ExplodeCount = 7
	};

	enum class BossFlagValue
	{
		ShockwaveRing      = (1 << 0),
		ShockwaveExplosion = (1 << 1),
		Shield             = (1 << 2),
		Lizard             = (1 << 3)
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
