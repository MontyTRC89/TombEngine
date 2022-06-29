#pragma once
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	void InitialiseKayak(short itemNumber);

	void KayakPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoKayakMount(ItemInfo* kayakItem, ItemInfo* laraItem, VehicleMountType mountType);

	void KayakDraw(ItemInfo* kayakItem);

	void KayakDoWake(ItemInfo* kayakItem, int xOffset, int zOffset, short rotate);
	void KayakDoRipple(ItemInfo* kayakItem, int xOffset, int zOffset);
	void KayakUpdateWakeFX();

	int KayakGetCollisionAnim(ItemInfo* kayakItem, int xDiff, int zDiff);
	int KayakDoDynamics(int height, int verticalVelocity, int* y);
	void KayakDoCurrent(ItemInfo* kayakItem, ItemInfo* laraItem);
	bool KayakCanGetOut(ItemInfo* kayakItem, int dir);
	int KayakDoShift(ItemInfo* kayakItem, Vector3Int* pos, Vector3Int* old);
	void KayakToBackground(ItemInfo* kayakItem, ItemInfo* laraItem);
	void KayakUserInput(ItemInfo* kayakItem, ItemInfo* laraItem);
	void KayakToItemCollision(ItemInfo* kayakItem, ItemInfo* laraItem);
	void KayakLaraRapidsDrown(ItemInfo* laraItem);

	bool KayakControl(ItemInfo* laraItem);
}
