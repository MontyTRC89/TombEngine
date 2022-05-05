#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Switches
{
	void UnderwaterSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void CeilingUnderwaterSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void WallUnderwaterSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
