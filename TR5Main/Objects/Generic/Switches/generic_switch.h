#pragma once

#include "items.h"
#include <collide.h>

namespace TEN::Entities::Switches
{
	enum SwitchStatus
	{
		SWITCH_OFF,
		SWITCH_ON
	};
	
	void SwitchControl(short itemNumber);
	void SwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
	int GetKeyTrigger(ITEM_INFO* item);
	int GetSwitchTrigger(ITEM_INFO* item, short* itemNos, int AttatchedToSwitch);
	int SwitchTrigger(short itemNum, short timer);
}