#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR2
{
	void InitializeBartoliOld(short itemNumber);
	void ControlBartoliOld(short itemNumber);
	void CollideDragonOld(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void ControlDragonOld(short backNumber);

	void ControlDragonTransformationSphereOld(short itemNumber);
}
