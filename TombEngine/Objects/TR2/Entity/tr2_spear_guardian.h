#pragma once

struct ItemInfo;
class GameVector;
class Vector3i;

namespace TEN::Entities::Creatures::TR2
{
	void SpawnSpearGuardianSmoke(const Vector3i& pos, int roomNumber);
	void InitialiseSpearGuardian(short itemNumber);
	void SpearGuardianControl(short itemNumber);
	void SpearGuardianHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex);
}
