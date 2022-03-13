#pragma once

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::TR4
{
	void InitialiseMine(short itemNumber);
	void MineControl(short itemNumber);
	void MineCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
