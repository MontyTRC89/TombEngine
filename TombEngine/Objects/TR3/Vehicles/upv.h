#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

namespace TEN::Entities::Vehicles
{
	void UPVInitialise(short itemNumber);
	void UPVCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void UPVEffects(short itemNumber);
	bool UPVControl(ItemInfo* laraItem, CollisionInfo* coll);
}
