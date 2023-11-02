#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Switches
{
	void UnderwaterSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	void CollideUnderwaterWallSwitch(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void CollideUnderwaterCeilingSwitch(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
