#pragma once

#include "items.h"
#include <collide.h>

namespace ten::entities::switches
{
	void UnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
	void CeilingUnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}