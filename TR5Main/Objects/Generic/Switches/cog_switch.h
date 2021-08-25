#pragma once

#include "items.h"
#include <collide.h>

namespace ten::entities::switches
{
	constexpr auto COG_DOOR_TURN = 40;
	constexpr auto COG_DOOR_SPEED = 12;

	void CogSwitchControl(short itemNumber);
	void CogSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}