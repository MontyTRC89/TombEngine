#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Objects/Utils/VehicleHelpers.h"

namespace TEN::Entities::Vehicles
{
	SkidooInfo* GetSkidooInfo(ItemInfo* skidooItem);
	void InitialiseSkidoo(short itemNumber);

	void SkidooPlayerCollision(short skidooItemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoSkidooMount(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleMountType mountType);
	bool TestSkidooDismountOK(ItemInfo* skidooItem, int direction);
	bool TestSkidooDismount(ItemInfo* skidooItem, ItemInfo* laraItem);

	bool SkidooControl(ItemInfo* laraItem, CollisionInfo* coll);
	bool SkidooUserControl(ItemInfo* skidooItem, ItemInfo* laraItem, int height, int* pitch);
	void SkidooAnimation(ItemInfo* skidooItem, ItemInfo* laraItem, int collide, bool dead);

	int GetSkidooCollisionAnim(ItemInfo* skidooItem, Vector3Int* moved);

	void SkidooGuns(ItemInfo* skidooItem, ItemInfo* laraItem);
	void DoSnowEffect(ItemInfo* skidooItem);

	int SkidooDynamics(ItemInfo* skidooItem, ItemInfo* laraItem);
	short DoSkidooShift(ItemInfo* skidooItem, Vector3Int* pos, Vector3Int* old);
	int DoSkidooDynamics(int height, int verticalVelocity, int* y);
}
