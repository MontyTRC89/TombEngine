#pragma once

#include "items.h"
#include "collide.h"

namespace TEN::Entities::Switches
{
	enum SwitchStatus
	{
		SWITCH_OFF,
		SWITCH_ON
	};
	
	void SwitchControl(short itemNumber);
	void SwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}