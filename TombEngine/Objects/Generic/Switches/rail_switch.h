#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Switches
{
	void RailSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
