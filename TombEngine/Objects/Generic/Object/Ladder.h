#pragma once

enum class LadderMountType;
struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	void LadderCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	bool			TestLadderMount(const ItemInfo& ladderItem, ItemInfo& laraItem);
	LadderMountType GetLadderMountType(ItemInfo& ladderItem, ItemInfo& laraItem);
	void			DoLadderMount(ItemInfo& ladderItem, ItemInfo& laraItem, LadderMountType mountType);

	void DisplayLadderDebug(ItemInfo& ladderItem);
}
