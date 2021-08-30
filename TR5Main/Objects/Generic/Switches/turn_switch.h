#pragma once

#include "items.h"
#include <collide.h>

namespace TEN::Entities::Switches
{
	void TurnSwitchControl(short itemNumber);
	void TurnSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}