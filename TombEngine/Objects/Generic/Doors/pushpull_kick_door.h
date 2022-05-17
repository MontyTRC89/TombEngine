#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Doors
{
	void PushPullKickDoorControl(short itemNumber);
	void PushPullKickDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
