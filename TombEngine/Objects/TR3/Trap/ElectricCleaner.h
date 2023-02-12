#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Traps
{
	void InitialiseElectricCleaner(short itemNumber);
	void ElectricCleanerControl(short itemNumber);
	void ElectricCleanerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	bool IsNextSectorValid(ItemInfo& item, const Vector3& Dir);
	Vector3 SearchDirections(ItemInfo& item, const Vector3& Dir1, const Vector3& Dir2, const Vector3& Dir3);

	bool CheckObjectAhead(ItemInfo& item);
	void CheckCleanerHeading(ItemInfo& item, int x, int y, int z, short roomNumber, bool& heading);
	void CleanerToItemCollision(ItemInfo& item);
	void SpawnElectricCleanerSparks(ItemInfo& item);
}
