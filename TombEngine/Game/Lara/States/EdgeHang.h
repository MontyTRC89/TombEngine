#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Player
{
	void PlayerStateEdgeHangIdle(ItemInfo* item, CollisionInfo* coll);
	void PlayerStateEdgeHangShimmyUp(ItemInfo* item, CollisionInfo* coll);
	void PlayerStateEdgeHangShimmyDown(ItemInfo* item, CollisionInfo* coll);
	void PlayerStateEdgeHangShimmyLeft(ItemInfo* item, CollisionInfo* coll);
	void PlayerStateEdgeHangShimmyRight(ItemInfo* item, CollisionInfo* coll);
	void PlayerStateEdgeHangShimmyCorner(ItemInfo* item, CollisionInfo* coll);
	void PlayerStateEdgeHangHanstand(ItemInfo* item, CollisionInfo* coll);
}
