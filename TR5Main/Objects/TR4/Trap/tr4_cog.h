#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::TR4
{
	void CogControl(short itemNum);
	void CogCollision(__int16 itemNumber, ITEM_INFO* l, COLL_INFO* coll);
}