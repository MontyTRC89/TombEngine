#pragma once

struct CollisionInfo;
struct CreatureBiteInfo;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR2
{
	void InitializeDragon(short itemNumber);
	void ControlDragon(short itemNumber);
	void CollideDragon(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	void DragonFireBreath(ItemInfo* item, const CreatureBiteInfo& bite, int speed, ItemInfo* enemy);
	void UpdateDragonBack(short frontItemNumber, short backItemNumber);
	void DragonLightsManager(const ItemInfo& item, int type);

	void InstantiateDragonBack(short itemNumber);
	void InstantiateDragonBones(short itemNumber);
}
