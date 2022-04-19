#pragma once

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::Switches
{
	void TurnSwitchControl(short itemNumber);
	void TurnSwitchCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
