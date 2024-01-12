#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player
{
	void lara_as_edge_hang_idle(ItemInfo* item, CollisionInfo* coll);
	void lara_col_edge_hang_idle(ItemInfo* item, CollisionInfo* coll);
	void lara_as_edge_hang_shimmy_left(ItemInfo* item, CollisionInfo* coll);
	void lara_col_edge_hang_shimmy_left(ItemInfo* item, CollisionInfo* coll);
	void lara_as_edge_hang_shimmy_right(ItemInfo* item, CollisionInfo* coll);
	void lara_col_edge_hang_shimmy_right(ItemInfo* item, CollisionInfo* coll);
	void lara_as_edge_hang_shimmy_corner(ItemInfo* item, CollisionInfo* coll);
	void lara_as_edge_hang_handstand(ItemInfo* item, CollisionInfo* coll);
}
