#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::Switches
{
	void InitialisePulleySwitch(short itemNumber);
	void PulleySwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
}
