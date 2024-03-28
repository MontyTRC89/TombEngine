#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Switches
{
	void CollideUnderwaterWallSwitch(short itemNumber, ItemInfo* collided, CollisionInfo* coll);
	void CollideUnderwaterCeilingSwitch(short itemNumber, ItemInfo* collided, CollisionInfo* coll);
}
