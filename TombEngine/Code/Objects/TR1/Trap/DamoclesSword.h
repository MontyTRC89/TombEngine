#pragma once

struct CollisionInfo;
struct ItemInfo;
struct ObjectInfo;

namespace TEN::Entities::Traps::TR1
{
	void InitializeDamoclesSword(short itemNumber);

	void ControlDamoclesSword(short itemNumber);
	void CollideDamoclesSword(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
