#pragma once

struct CollisionInfo;
struct ItemInfo;
struct ObjectInfo;

namespace TEN::Entities::Creatures::TR4
{
	void InitialiseSas(short itemNumber);
	void SasControl(short itemNumber);
	void SasFireGrenade(ItemInfo& item, short angle1, short angle2);
	void SasDragBlokeCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
