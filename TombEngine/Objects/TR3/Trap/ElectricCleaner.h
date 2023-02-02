#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Traps
{
	void InitialiseElectricCleaner(short itemNumber);
	void ElectricCleanerControl(short itemNumber);
	void ElectricCleanerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	bool NeedBlockAlignment(const ItemInfo& item);
	bool CheckObjectAhead(ItemInfo& item);
	void CheckCleanerHeading(ItemInfo& item, int x, int y, int z, short roomNumber, bool& heading);
	void CleanerToItemCollision(ItemInfo& item);
	void SpawnElectricCleanerSparks(ItemInfo& item);
}
