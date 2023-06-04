#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR2
{
	void InitializeBartoli(short itemNumber);
	void ControlBartoli(short itemNumber);
	void CollideDragon(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void ControlDragon(short backNumber);

	void ControlDragonTransformationSphere(short itemNumber);
}
