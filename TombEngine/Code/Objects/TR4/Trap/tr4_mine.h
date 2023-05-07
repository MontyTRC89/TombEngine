#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::TR4
{
	void InitializeMine(short itemNumber);
	void MineControl(short itemNumber);
	void MineCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
