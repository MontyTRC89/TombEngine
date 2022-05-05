#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Switches
{
	enum SwitchStatus
	{
		SWITCH_OFF,
		SWITCH_ON
	};
	
	void SwitchControl(short itemNumber);
	void SwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
