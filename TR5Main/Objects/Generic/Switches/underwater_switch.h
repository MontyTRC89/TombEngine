#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::Switches
{
	void UnderwaterSwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
	void CeilingUnderwaterSwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
	void WallUnderwaterSwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
}
