#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::Switches
{
	enum SwitchStatus
	{
		SWITCH_OFF,
		SWITCH_ON
	};
	
	void SwitchControl(short itemNumber);
	void SwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
}
