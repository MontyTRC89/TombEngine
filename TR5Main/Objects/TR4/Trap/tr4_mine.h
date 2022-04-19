#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::TR4
{
	void InitialiseMine(short itemNum);
	void MineControl(short itemNum);
	void MineCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}