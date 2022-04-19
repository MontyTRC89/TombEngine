#pragma once

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::Doors
{
	void InitialiseSteelDoor(short itemNumber);
	void SteelDoorCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
