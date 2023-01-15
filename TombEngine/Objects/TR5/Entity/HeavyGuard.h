#pragma once

class EulerAngles;
class GameVector;
struct ItemInfo;

//temp
class Pose;
class Vector3i;

namespace TEN::Entities::Creatures::TR5
{
	void InitialiseHeavyGuard(short itemNumber);
	void HeavyGuardControl(short itemNumber);

	void HeavyGuardHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex);
	void FireHeavyGuardRaygun(ItemInfo& item, bool fireRight, bool spawnLaser);
	void SpawnRaygunLaser(const Vector3i& posr, const Pose& pos, float life);
}
