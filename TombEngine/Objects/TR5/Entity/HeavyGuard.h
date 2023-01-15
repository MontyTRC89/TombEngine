#pragma once

class EulerAngles;
class GameVector;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR5
{
	void InitialiseHeavyGuard(short itemNumber);
	void HeavyGuardControl(short itemNumber);

	void HeavyGuardHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex);
	void FireHeavyGuardRaygun(ItemInfo& item, bool isRight, bool spawnLaser);
	void SpawnRaygunSmoke(const Vector3& pos, const EulerAngles& orient, float life);
}
