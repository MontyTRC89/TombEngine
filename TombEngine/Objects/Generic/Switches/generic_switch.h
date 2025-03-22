#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Switches
{
	enum SwitchStatus
	{
		SWITCH_OFF,
		SWITCH_ON,
		SWITCH_OFF_UNDERWATER,
		SWITCH_ON_UNDERWATER
	};
	
	void SwitchControl(short itemNumber);
	void SwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
