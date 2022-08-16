#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Doors
{
	void InitialiseSteelDoor(short itemNumber);
	void SteelDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
