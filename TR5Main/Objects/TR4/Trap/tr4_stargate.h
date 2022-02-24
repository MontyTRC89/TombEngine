#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::TR4
{
	void StargateControl(short itemNumber);
	void StargateCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
}
