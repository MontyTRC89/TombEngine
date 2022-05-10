#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::TR4
{
	void StargateControl(short itemNumber);
	void StargateCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
