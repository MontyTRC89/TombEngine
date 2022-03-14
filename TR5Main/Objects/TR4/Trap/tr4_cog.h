#pragma once

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::TR4
{
	void CogControl(short itemNumber);
	void CogCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
