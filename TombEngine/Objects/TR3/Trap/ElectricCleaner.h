#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Traps
{
	void InitialiseElectricCleaner(short itemNumber);
	void ElectricCleanerControl(short itemNumber);
	void ElectricCleanerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	bool NeedNewTarget(const ItemInfo& item);
	bool CheckObjectAhead(ItemInfo& item);
	void CheckCleanerHeading(ItemInfo& item, long x, long y, long z, short roomNumber, bool& heading);
	void CleanerToItemCollision(ItemInfo& item);
	void SpawnElectricCleanerSparks(ItemInfo& item);
}
