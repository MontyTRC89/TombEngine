#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::TR4
{
	void InitialiseStatuePlinth(short itemNumber);
	void StatuePlinthCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
