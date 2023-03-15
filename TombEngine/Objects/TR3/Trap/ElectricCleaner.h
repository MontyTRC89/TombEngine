#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Traps
{
	void InitialiseElectricCleaner(short itemNumber);
	void ElectricCleanerControl(short itemNumber);
	void ElectricCleanerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	bool IsNextSectorValid(ItemInfo& item, const Vector3& direction);
	Vector3 ElectricCleanerSearchDirections(ItemInfo& item, const Vector3& dir1, const Vector3& dir2, const Vector3& dir3);
	void ElectricCleanerToItemCollision(ItemInfo& item);
	void SpawnElectricCleanerSparks(ItemInfo& item);

}
