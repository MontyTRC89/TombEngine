#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player
{
	void HandlePlayerStateControl(ItemInfo& playerItem, CollisionInfo& coll);
	void HandlePlayerStateCollision(ItemInfo& playerItem, CollisionInfo& coll);
}
