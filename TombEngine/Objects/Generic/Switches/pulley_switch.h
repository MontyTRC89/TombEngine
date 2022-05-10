#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Switches
{
	void InitialisePulleySwitch(short itemNumber);
	void PulleySwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
