#pragma once

struct ItemInfo;
class GameVector;

namespace TEN::Entities::Creatures::TR2
{
	void InitialiseSpearGuardian(short itemNumber);
	void SpearGuardianControl(short itemNumber);
	void SpearGuardianHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex);
}
