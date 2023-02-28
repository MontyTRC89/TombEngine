#pragma once

struct ItemInfo;
struct BiteInfo;

namespace TEN::Entities::Traps::TR1
{
	void InitialiseSlammingDoors(short itemNumber);
	void ControlSlammingDoors(short itemNumber);

	void DoBiteEffect(ItemInfo* item, const BiteInfo& bite);
}