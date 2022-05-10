#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Switches
{
	void JumpSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
