#pragma once

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::Switches
{
	void JumpSwitchCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
