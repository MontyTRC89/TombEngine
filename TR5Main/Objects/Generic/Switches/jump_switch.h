#pragma once

#include "items.h"
#include <collide.h>

namespace TEN::Entities::Switches
{
	void JumpSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}