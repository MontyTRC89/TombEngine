#pragma once

#include "items.h"
#include <collide.h>

namespace ten::entities::switches
{
	enum SwitchStatus
	{
		SWITCH_OFF,
		SWITCH_ON
	};

	void InitialiseSwitch(short itemNumber);
	void SwitchControl(short itemNumber);
	void SwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}