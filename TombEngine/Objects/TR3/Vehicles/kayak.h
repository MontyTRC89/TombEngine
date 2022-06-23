#pragma once

struct ItemInfo;
struct CollisionInfo;
struct Vector3Int;

namespace TEN::Entities::Vehicles
{
	void InitialiseKayak(short itemNumber);
	void KayakDraw(ItemInfo* kayakItem);

	void KayakDoWake(ItemInfo* kayakItem, int xOffset, int zOffset, short rotate);
	void KayakDoRipple(ItemInfo* kayakItem, int xOffset, int zOffset);
	void KayakUpdateWakeFX();

	int KayakGetCollisionAnim(ItemInfo* kayakItem, int xDiff, int zDiff);
	int KayakDoDynamics(int height, int verticalVelocity, int* y);
	void KayakDoCurrent(ItemInfo* laraItem, ItemInfo* kayakItem);
	int KayakTestHeight(ItemInfo* kayakItem, int x, int z, Vector3Int* pos);
	bool KayakCanGetOut(ItemInfo* kayakItem, int dir);
	int KayakDoShift(ItemInfo* kayakItem, Vector3Int* pos, Vector3Int* old);
	void KayakToBackground(ItemInfo* laraItem, ItemInfo* kayakItem);
	void KayakUserInput(ItemInfo* laraItem, ItemInfo* kayakItem);
	void KayakToItemCollision(ItemInfo* laraItem, ItemInfo* kayakItem);
	void KayakLaraRapidsDrown(ItemInfo* laraItem);

	void KayakCollision(short itemNum, ItemInfo* laraItem, CollisionInfo* coll);
	bool KayakControl(ItemInfo* laraItem);
}
