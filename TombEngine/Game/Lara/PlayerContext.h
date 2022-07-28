#pragma once
#include "Game/Lara/PlayerContextStructs.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player
{
	class PlayerContext
	{
		// TODO: As a way of making the methods non-static and not require arguments, would this be weird?
	/*private:
		ItemInfo*	   PlayerItem;
		CollisionInfo* PlayerColl;*/

	public:
		PlayerContext();

		// Basic movement
		static bool CanTurnFast(ItemInfo* item);
		static bool CanRunForward(ItemInfo* item, CollisionInfo* coll);
		static bool CanRunBack(ItemInfo* item, CollisionInfo* coll);
		static bool CanWalkForward(ItemInfo* item, CollisionInfo* coll);
		static bool CanWalkBack(ItemInfo* item, CollisionInfo* coll);
		static bool CanWalkBackSwamp(ItemInfo* item, CollisionInfo* coll);
		static bool CanSidestepLeft(ItemInfo* item, CollisionInfo* coll);
		static bool CanSidestepLeftWade(ItemInfo* item, CollisionInfo* coll); // TODO
		static bool CanSidestepLeftSwamp(ItemInfo* item, CollisionInfo* coll);
		static bool CanSidestepRight(ItemInfo* item, CollisionInfo* coll);
		static bool CanSidestepRightWade(ItemInfo* item, CollisionInfo* coll); // TODO
		static bool CanSidestepRightSwamp(ItemInfo* item, CollisionInfo* coll);
		static bool CanWadeForward(ItemInfo* item, CollisionInfo* coll); // TODO
		static bool CanWadeForwardSwamp(ItemInfo* item, CollisionInfo* coll);

		// Crouch and crawl movement
		static bool CanCrouch(ItemInfo* item);
		static bool CanCrouchToCrawl(ItemInfo* item);
		static bool CanCrouchRoll(ItemInfo* item, CollisionInfo* coll);
		static bool CanCrawlForward(ItemInfo* item, CollisionInfo* coll);
		static bool CanCrawlBack(ItemInfo* item, CollisionInfo* coll);

		// Monkey swing movement
		static bool CanMonkeyForward(ItemInfo* item, CollisionInfo* coll);
		static bool CanMonkeyBack(ItemInfo* item, CollisionInfo* coll);
		static bool CanMonkeyShimmyLeft(ItemInfo* item, CollisionInfo* coll);
		static bool CanMonkeyShimmyRight(ItemInfo* item, CollisionInfo* coll);

		// Vault movement
		// Crawl vault movement
		// Water tread climb out movement
		// Miscellaneous movement

	private:
		static bool TestGroundMovementSetup(ItemInfo* item, CollisionInfo* coll, ContextSetupGroundMovement contextSetup, bool useCrawlSetup = false);
		static bool TestMonkeyMovementSetup(ItemInfo* item, CollisionInfo* coll, ContextSetupMonkeyMovement contextSetup);
	};
}
