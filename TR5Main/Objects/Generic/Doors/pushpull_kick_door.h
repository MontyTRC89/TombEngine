#pragma once
#include "items.h"
#include "collide.h"
#include "room.h"

namespace TEN::Entities::Doors
{
	void PushPullKickDoorControl(short itemNumber);
	void PushPullKickDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}