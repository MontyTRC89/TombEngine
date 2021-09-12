#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::TR4
{
	void StargateControl(short itemNum);
	void StargateCollision(short itemNum, ITEM_INFO* l, COLL_INFO* c);
}