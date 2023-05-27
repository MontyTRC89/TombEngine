#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	void InitializeMotorbike(short itemNumber);

	void MotorbikePlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoMotorbikeMount(ItemInfo* motorbikeItem, ItemInfo* laraItem, enum class VehicleMountType mountType);

	bool MotorbikeControl(ItemInfo* laraItem, CollisionInfo* coll);
}
