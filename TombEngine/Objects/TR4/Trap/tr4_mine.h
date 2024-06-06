#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Traps
{
	void InitializeMine(short itemNumber);
	void MineControl(short itemNumber);
	void MineCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
