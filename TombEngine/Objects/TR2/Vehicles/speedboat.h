#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"

namespace TEN::Entities::Vehicles
{
	enum class SpeedboatMountType
	{
		None,
		StartPosition,
		WaterLeft,
		WaterRight,
		Jump
	};

	void InitialiseSpeedboat(short itemNumber);
	SpeedboatMountType GetSpeedboatMountType(ItemInfo* laraItem, ItemInfo* sBoatItem, CollisionInfo* coll);
	bool TestSpeedboatDismount(ItemInfo* sBoatItem, int direction);
	void DoSpeedboatDismount(ItemInfo* laraItem, ItemInfo* sBoatItem);

	void SpeedboatDoBoatShift(ItemInfo* sBoatItem, int itemNumber);
	short SpeedboatDoShift(ItemInfo* sBoatItem, Vector3Int* pos, Vector3Int* old);

	int GetSpeedboatHitAnim(ItemInfo* sBoatItem, Vector3Int* moved);
	int DoSpeedboatDynamics(int height, int verticalVelocity, int* y);
	int SpeedboatDynamics(ItemInfo* laraItem, short itemNumber);
	bool SpeedboatUserControl(ItemInfo* laraItem, ItemInfo* sBoatItem);
	void SpeedboatAnimation(ItemInfo* laraItem, ItemInfo* sBoatItem, int collide);
	void SpeedboatSplash(ItemInfo* item, long verticalVelocity, long water);
	void SpeedboatCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void SpeedboatControl(short itemNumber);
}
