#pragma once

class Vector3i;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR5
{
	struct LaserHeadInfo;

	LaserHeadInfo& GetGuardianInfo(ItemInfo& item);

	void InitialiseLaserHead(short itemNumber);
	void LaserHeadControl(short itemNumber);

	void DoGuardianLaserAttack(ItemInfo* item);
	void DoGuardianDeath(int itemNumber, ItemInfo& item);
	void SpawnGuardianSparks(const Vector3& pos, const Vector3& color, unsigned int count, int unk = 0);
}
