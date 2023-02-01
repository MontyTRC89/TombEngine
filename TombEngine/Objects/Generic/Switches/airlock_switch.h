#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Switches
{
	void AirLockSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}

