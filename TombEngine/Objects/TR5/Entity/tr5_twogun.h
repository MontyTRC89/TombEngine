#pragma once

class EulerAngles;
struct ItemInfo;

//temp
class Pose;
class Vector3i;

namespace TEN::Entities::Creatures::TR5
{
	void InitialiseHeavyGuard(short itemNumber);
	void HeavyGuardControl(short itemNumber);

	//temp
	void FireTwogunWeapon(short itemNumber, short LeftRight, short plasma);
	void TriggerTwogunPlasma(const Vector3i& posr, const Pose& pos, float life);

	//void FireHeavyGuardRaygun(ItemInfo& item, bool fireRight, bool spawnLaser);
	//void SpawnRaygunLaser(const Vector3& pos, const EulerAngles& orient, float life);
}
