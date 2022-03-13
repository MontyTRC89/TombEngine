#pragma once

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::TR4
{
	void StargateControl(short itemNumber);
	void StargateCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
