#pragma once

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::Switches
{
	void UnderwaterSwitchCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
	void CeilingUnderwaterSwitchCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
	void WallUnderwaterSwitchCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
