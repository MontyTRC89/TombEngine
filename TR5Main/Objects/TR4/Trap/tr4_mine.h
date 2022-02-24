#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::TR4
{
	void InitialiseMine(short itemNumber);
	void MineControl(short itemNumber);
	void MineCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
}
