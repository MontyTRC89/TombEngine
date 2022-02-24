#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::TR4
{
	void InitialiseElementPuzzle(short itemNumber);
	void ElementPuzzleControl(short itemNumber);
	void ElementPuzzleDoCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
	void ElementPuzzleCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
}
