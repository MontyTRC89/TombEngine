#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::Doors
{
	void PushPullKickDoorControl(short itemNumber);
	void PushPullKickDoorCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
}
