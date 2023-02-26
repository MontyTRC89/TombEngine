#pragma once

struct CollisionInfo;
struct ItemInfo;
struct ObjectInfo;
struct BiteInfo;

namespace TEN::Entities::Traps::TR1
{
	void InitialiseTeethSpikeDoor(short itemNumber);
	void ControlTeethSpikeDoor(short itemNumber);

	void DoBiteEffect(ItemInfo* item, const BiteInfo& bite);
}