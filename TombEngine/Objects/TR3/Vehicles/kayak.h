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

	void KayakDoRipple(ItemInfo* kayakItem, int xOffset, int zOffset);

	int KayakGetCollisionAnim(ItemInfo* kayakItem, int xDiff, int zDiff);
	int KayakDoDynamics(int height, int verticalVelocity, int* y);
	void KayakDoCurrent(ItemInfo* kayakItem, ItemInfo* laraItem);
	bool KayakCanGetOut(ItemInfo* kayakItem, int dir);
	int KayakDoShift(ItemInfo* kayakItem, Vector3i* pos, Vector3i* old);
	void KayakToBackground(ItemInfo* kayakItem, ItemInfo* laraItem);
	void KayakUserInput(ItemInfo* kayakItem, ItemInfo* laraItem);
	void KayakToItemCollision(ItemInfo* kayakItem, ItemInfo* laraItem);
	void KayakLaraRapidsDrown(ItemInfo* laraItem);
	void PreDrawWakeFx(ItemInfo* kayakItem);
	bool KayakControl(ItemInfo* laraItem);
}
