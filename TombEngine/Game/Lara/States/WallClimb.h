#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player
{
	void PlayerStateWallClimbIdle(ItemInfo* item, CollisionInfo* coll);
	void PlayerStateWallClimbUp(ItemInfo* item, CollisionInfo* coll);
	void PlayerStateWallClimbDown(ItemInfo* item, CollisionInfo* coll);
	void PlayerStateWallClimbLeft(ItemInfo* item, CollisionInfo* coll);
	void PlayerStateWallClimbRight(ItemInfo* item, CollisionInfo* coll);
	void PlayerStateWallClimbDismountLeft(ItemInfo* item, CollisionInfo* coll);
	void PlayerStateWallClimbDismountRight(ItemInfo* item, CollisionInfo* coll);
}
