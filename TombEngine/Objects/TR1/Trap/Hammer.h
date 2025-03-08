#pragma once

struct CollisionInfo;
struct ItemInfo;
struct ObjectInfo;

namespace TEN::Entities::Traps
{
	void InitializeHammer(short itemNumber);
	void ControlHammer(short itemNumber);
	void CollideHammer(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll);
	void CollideHammerHandle(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll);
}
