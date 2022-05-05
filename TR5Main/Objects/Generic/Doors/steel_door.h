#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Doors
{
	void InitialiseSteelDoor(short itemNumber);
	void SteelDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
