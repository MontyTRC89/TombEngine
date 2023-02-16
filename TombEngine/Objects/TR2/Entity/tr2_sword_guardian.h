#pragma once

struct ItemInfo;
class GameVector;

namespace TEN::Entities::Creatures::TR2
{
	void InitialiseSwordGuardian(short itemNumber);
	void SwordGuardianControl(short itemNumber);
	void SwordGuardianHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex);
}
