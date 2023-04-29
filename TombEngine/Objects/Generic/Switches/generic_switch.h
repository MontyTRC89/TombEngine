#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Switches
{
	enum SwitchStatus
	{
		SWITCH_ON,
		SWITCH_OFF
	};
	
	void SwitchControl(short itemNumber);
	void SwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
