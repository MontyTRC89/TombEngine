#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player
{
	void lara_as_wall_climb_idle(ItemInfo* item, CollisionInfo* coll);
	void lara_as_wall_climb_up(ItemInfo* item, CollisionInfo* coll);
	void lara_as_wall_climb_down(ItemInfo* item, CollisionInfo* coll);
	void lara_as_wall_climb_left(ItemInfo* item, CollisionInfo* coll);
	void lara_as_wall_climb_right(ItemInfo* item, CollisionInfo* coll);
	void lara_as_wall_climb_dismount_left(ItemInfo* item, CollisionInfo* coll);
	void lara_as_wall_climb_dismount_right(ItemInfo* item, CollisionInfo* coll);

	// TODO: May not be necessary.
	void lara_as_wall_climb_end(ItemInfo* item, CollisionInfo* coll);
}
