#pragma once
#include "Objects/TR4/Vehicles/motorbike_info.h"
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	MotorbikeInfo& GetMotorbikeInfo(ItemInfo* motorbikeItem);
	void InitialiseMotorbike(short itemNumber);

	int GetMotorbikeCollisionAnim(ItemInfo* motorbikeItem, Vector3i* pos);
	void SetLaraOnMotorbike(ItemInfo* motorbikeItem, ItemInfo* laraItem);

	void MotorbikePlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool MotorbikeControl(ItemInfo* laraItem, CollisionInfo* coll);
	int MotorbikeUserControl(ItemInfo* motorbikeItem, ItemInfo* laraItem, int height, int* pitch);
	void AnimateMotorbike(ItemInfo* motorbikeItem, ItemInfo* laraItem, int collide, bool isDead);

	void DoMotorbikeMount(ItemInfo* motorbikeItem, ItemInfo* laraItem, VehicleMountType mountType);
	bool TestMotorbikeDismount(ItemInfo* motorbikeItem, ItemInfo* laraItem);
	int DoMotorbikeDismount(ItemInfo* laraItem);

	int MotorbikeDynamics(ItemInfo* motorbikeItem, ItemInfo* laraItem);

	void DrawMotorbikeLight(ItemInfo* motorbikeItem);
	void TriggerMotorbikeSmokeEffect(ItemInfo* motorbikeItem, ItemInfo* laraItem);
	void TriggerMotorbikeExhaustSmokeEffect(int x, int y, int z, short angle, short speed, bool moving);

}
