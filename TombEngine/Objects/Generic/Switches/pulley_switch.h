#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Switches
{
	void InitializePulleySwitch(short itemNumber);
	void PulleySwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
