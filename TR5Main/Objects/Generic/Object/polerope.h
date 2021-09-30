#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::Objects
{
	void PoleCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
}