#pragma once

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::Doors
{
	void PushPullKickDoorControl(short itemNumber);
	void PushPullKickDoorCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
