#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::Generic
{
	void PoleCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
}