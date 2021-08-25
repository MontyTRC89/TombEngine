#pragma once

#include "items.h"
#include <collide.h>

namespace ten::entities::switches
{
	void JumpSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}