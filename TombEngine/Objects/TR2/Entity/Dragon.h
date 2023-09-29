#pragma once

struct CollisionInfo;
struct CreatureBiteInfo;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR2
{
	void InitializeDragon(short itemNumber);
	void ControlDragon(short backNumber);
	void CollideDragon(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	void DragonFireBreath(ItemInfo* item, const CreatureBiteInfo& bite, int speed, ItemInfo* enemy);
	void DragonLightsManager(const ItemInfo& item, int type);

	void InstantiateDragonBones(short itemNumber);
}
