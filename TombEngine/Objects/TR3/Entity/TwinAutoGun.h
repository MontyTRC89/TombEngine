#pragma once

class GameVector;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR3
{
	void InitializeAutoGunMip(short itemNumber);
	void ControlAutoGunMip(short itemNumber);
	void HitAutoGunMip(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex);
}
