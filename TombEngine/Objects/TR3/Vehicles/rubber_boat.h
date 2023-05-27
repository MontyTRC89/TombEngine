#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	void InitializeRubberBoat(short itemNumber);

	void RubberBoatPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoRubberBoatMount(ItemInfo* rBoatItem, ItemInfo* laraItem, enum class VehicleMountType mountType);

	void RubberBoatControl(short itemNumber);
	void DrawRubberBoat(ItemInfo* rBoatItem);
}
