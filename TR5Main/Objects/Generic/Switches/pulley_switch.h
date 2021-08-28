#pragma once

#include "items.h"
#include <collide.h>

namespace TEN::Entities::Switches
{
	void InitialisePulleySwitch(short itemNumber);
	void PulleySwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}