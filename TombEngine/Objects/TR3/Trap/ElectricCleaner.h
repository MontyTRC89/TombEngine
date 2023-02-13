#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Traps
{
	void InitialiseElectricCleaner(short itemNumber);
	void ElectricCleanerControl(short itemNumber);
	void ElectricCleanerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	bool IsNextSectorValid(ItemInfo& item, const Vector3& Dir);
	Vector3 ElectricCleanerSearchDirections(ItemInfo& item, const Vector3& Dir1, const Vector3& Dir2, const Vector3& Dir3);
	void ElectricCleanerToItemCollision(ItemInfo& item);
	void SpawnElectricCleanerSparks(ItemInfo& item);

	//TODO method to detect pushables while Pushable_Object get refactored.
	void CollectLevelPushables (std::vector <ItemInfo*>& PushablesList);
	bool CheckPushableList (std::vector <ItemInfo* >& PushablesList, Vector3& refPoint);

}
