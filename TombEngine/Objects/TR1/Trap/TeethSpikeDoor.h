#pragma once

struct ItemInfo;
struct BiteInfo;

namespace TEN::Entities::Traps::TR1
{
	void InitialiseTeethSpikeDoor(short itemNumber);
	void ControlTeethSpikeDoor(short itemNumber);

	void DoBiteEffect(ItemInfo* item, const BiteInfo& bite);
}