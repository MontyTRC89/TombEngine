#pragma once

struct CollisionInfo;
struct ItemInfo;
struct ObjectInfo;

namespace TEN::Entities::Traps::TR1
{
	void SetupDamoclesSword(ObjectInfo* object);
	void InitialiseDamoclesSword(short itemNumber);

	void ControlDamoclesSword(short itemNumber);
	void CollisionDamoclesSword(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
