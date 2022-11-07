#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	enum class LadderMountType;

	void LadderCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	LadderMountType GetLadderMountType(ItemInfo& ladderItem, ItemInfo& laraItem);
	bool			TestLadderMount(const ItemInfo& ladderItem, ItemInfo& laraItem);
	void			DoLadderMount(int itemNumber, ItemInfo& ladderItem, ItemInfo& laraItem, LadderMountType mountType);

	void DisplayLadderDebug(ItemInfo& ladderItem);
}
