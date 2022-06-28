#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Objects/Utils/VehicleHelpers.h"

namespace TEN::Entities::Vehicles
{
	void InitialiseSpeedboat(short itemNumber);

	void SpeedboatPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoSpeedboatMount(ItemInfo* speedboatItem, ItemInfo* laraItem, VehicleMountType mountType);
	bool TestSpeedboatDismount(ItemInfo* speedboatItem, int direction);
	void DoSpeedboatDismount(ItemInfo* speedboatItem, ItemInfo* laraItem);

	void SpeedboatDoBoatShift(ItemInfo* speedboatItem, int itemNumber);
	short SpeedboatDoShift(ItemInfo* speedboatItem, Vector3Int* pos, Vector3Int* old);

	int GetSpeedboatHitAnim(ItemInfo* speedboatItem, Vector3Int* moved);
	int DoSpeedboatDynamics(int height, int verticalVelocity, int* y);
	int SpeedboatDynamics(short itemNumber, ItemInfo* laraItem);
	bool SpeedboatUserControl(ItemInfo* speedboatItem, ItemInfo* laraItem);
	void SpeedboatAnimation(ItemInfo* speedboatItem, ItemInfo* laraItem, int collide);
	void SpeedboatSplash(ItemInfo* speedboatItem, long verticalVelocity, long water);
	void SpeedboatControl(short itemNumber);
}
