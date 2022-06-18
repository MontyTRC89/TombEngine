#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Vehicles
{
	void InitialiseQuadBike(short itemNumber);
	void QuadBikeCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool QuadBikeControl(ItemInfo* laraItem, CollisionInfo* coll);
}
