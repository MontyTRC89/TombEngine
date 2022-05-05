#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Switches
{
	void TurnSwitchControl(short itemNumber);
	void TurnSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
