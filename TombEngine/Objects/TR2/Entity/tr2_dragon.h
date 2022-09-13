#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"

namespace TEN::Entities::Creatures::TR2
{
	void DragonCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DragonControl(short backNumber);
	void InitialiseBartoli(short itemNumber);
	void BartoliControl(short itemNumber);
}
