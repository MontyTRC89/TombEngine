#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::Switches
{
	void UnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
	void CeilingUnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
	void WallUnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}