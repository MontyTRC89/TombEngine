#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Objects/Utils/VehicleHelpers.h"

namespace TEN::Entities::Vehicles
{
	void InitialiseRubberBoat(short itemNumber);

	void RubberBoatPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoRubberBoatMount(ItemInfo* rBoatItem, ItemInfo* laraItem, VehicleMountType mountType);

	void RubberBoatControl(short itemNumber);
	void DrawRubberBoat(ItemInfo* rBoatItem);
}
