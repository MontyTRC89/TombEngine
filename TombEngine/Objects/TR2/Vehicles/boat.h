#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"

namespace TEN::Entities::Vehicles
{
	enum class BoatMountType
	{
		None = 0,
		WaterRight = 1,
		WaterLeft = 2,
		Jump = 3,
		StartPosition = 4
	};

	void InitialiseSpeedBoat(short itemNumber);
	BoatMountType GetSpeedBoatMountType(ItemInfo* laraItem, ItemInfo* sBoatItem, CollisionInfo* coll);
	bool TestSpeedBoatDismount(ItemInfo* sBoatItem, int direction);
	void DoSpeedBoatDismount(ItemInfo* laraItem, ItemInfo* sBoatItem);
	int SpeedBoatTestWaterHeight(ItemInfo* sBoatItem, int zOffset, int xOffset, Vector3Int* pos);

	void SpeedBoatDoBoatShift(ItemInfo* sBoatItem, int itemNumber);
	short SpeedBoatDoShift(ItemInfo* sBoatItem, Vector3Int* pos, Vector3Int* old);

	int GetSpeedBoatHitAnim(ItemInfo* sBoatItem, Vector3Int* moved);
	int DoSpeedBoatDynamics(int height, int verticalVelocity, int* y);
	int SpeedBoatDynamics(ItemInfo* laraItem, short itemNumber);
	bool SpeedBoatUserControl(ItemInfo* laraItem, ItemInfo* sBoatItem);
	void SpeedBoatAnimation(ItemInfo* laraItem, ItemInfo* sBoatItem, int collide);
	void SpeedBoatSplash(ItemInfo* item, long verticalVelocity, long water);
	void SpeedBoatCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void SpeedBoatControl(short itemNumber);
}
