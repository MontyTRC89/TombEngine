#pragma once

class GameVector;
struct BiteInfo;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR3
{
	void InitialiseGunTurret(short itemNumber);
	void GunTurretControl(short itemNumber);
	void GunTurretHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex);
}
