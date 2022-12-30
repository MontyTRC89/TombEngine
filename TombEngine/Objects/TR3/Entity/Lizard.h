#pragma once

struct BiteInfo;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR3
{
	void LizardControl(short itemNumber);

	bool IsLizardTargetBlocked(ItemInfo& item);
	void SpawnLizardGas(int itemNumber, const BiteInfo& bite, int speed);
}
