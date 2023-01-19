#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Switches
{

	void LockSwitchControl(short itemNumber);
	void LockSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
