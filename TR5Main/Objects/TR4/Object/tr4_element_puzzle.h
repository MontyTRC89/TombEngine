#pragma once

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::TR4
{
	void InitialiseElementPuzzle(short itemNumber);
	void ElementPuzzleControl(short itemNumber);
	void ElementPuzzleDoCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
	void ElementPuzzleCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
