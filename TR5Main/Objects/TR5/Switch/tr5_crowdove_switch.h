#pragma once

#include "Game/items.h"
#include "Game/collision/collide_room.h"

namespace TEN::Entities::TR5
{
	void InitialiseCrowDoveSwitch(short itemNumber);
	void CrowDoveSwitchControl(short itemNumber);
	void CrowDoveSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}