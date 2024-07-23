#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Player
{
	// Stand, run, walk inquirers

	bool CanChangeElevation(const ItemInfo& item, const CollisionInfo& coll);
	bool CanStepUp(const ItemInfo& item, const CollisionInfo& coll);
	bool CanStepDown(const ItemInfo& item, const CollisionInfo& coll);
	bool CanStrikeAfkPose(const ItemInfo& item, const CollisionInfo& coll);
	bool CanTurn180(const ItemInfo& item, const CollisionInfo& coll);
	bool CanTurnFast(const ItemInfo& item, const CollisionInfo& coll, bool isGoingRight);
	bool CanRoll180Running(const ItemInfo& item);
	bool CanRunForward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanRunBackward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanWalkForward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanWalkBackward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanSidestepLeft(const ItemInfo& item, const CollisionInfo& coll);
	bool CanSidestepRight(const ItemInfo& item, const CollisionInfo& coll);
	bool CanWadeForward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanWadeBackward(const ItemInfo& item, const CollisionInfo& coll);

	// Crouch, crawl inquirers

	bool IsInLowSpace(const ItemInfo& item, const CollisionInfo& coll);
	bool CanCrouch(const ItemInfo& item, const CollisionInfo& coll);
	bool CanCrouchToCrawl(const ItemInfo& item, const CollisionInfo& coll);
	bool CanCrouchRoll(const ItemInfo& item, const CollisionInfo& coll);
	bool CanCrawlForward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanCrawlBackward(const ItemInfo& item, const CollisionInfo& coll);
}
