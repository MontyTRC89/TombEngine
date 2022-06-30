#pragma once
#include "Objects/TR3/Vehicles/minecart_info.h"
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	MinecartInfo* GetMinecartInfo(ItemInfo* minecartItem);
	void InitialiseMinecart(short itemNumber);

	void MinecartPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool MinecartControl(ItemInfo* laraItem);
	void MinecartUserControl(ItemInfo* minecartItem, ItemInfo* laraItem);
	void AnimateMinecart(ItemInfo* minecartItem, ItemInfo* laraItem);

	void DoMinecartMount(ItemInfo* minecartItem, ItemInfo* laraItem, VehicleMountType mountType);
	bool TestMinecartDismount(ItemInfo* laraItem, int direction);

	short GetMinecartCollision(ItemInfo* minecartItem, short angle, int distance);
	void MinecartToEntityCollision(ItemInfo* minecartItem, ItemInfo* laraItem);
	void TriggerWheelSparkEffect(ItemInfo* minecartItem, bool left);
}
