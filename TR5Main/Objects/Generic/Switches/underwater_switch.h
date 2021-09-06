#pragma once

#include "items.h"
#include <collide.h>

namespace TEN::Entities::Switches
{
	void UnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
	void CeilingUnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
	void WallUnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}