#pragma once
#include "Game/Lara/PlayerContextStructs.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player
{
	class PlayerContext
	{
	private:
		ItemInfo*	   PlayerItemPtr = nullptr;
		CollisionInfo* PlayerCollPtr = nullptr;

	public:
		PlayerContext();
		PlayerContext(ItemInfo* item, CollisionInfo* coll);

		// Basic movement
		bool CanTurnFast();
		bool CanRunForward();
		bool CanRunBack();
		bool CanWalkForward();
		bool CanWalkBack();
		bool CanWalkBackSwamp();
		bool CanSidestepLeft();
		bool CanSidestepLeftWade(); // TODO
		bool CanSidestepLeftSwamp();
		bool CanSidestepRight();
		bool CanSidestepRightWade(); // TODO
		bool CanSidestepRightSwamp();
		bool CanWadeForward(); // TODO
		bool CanWadeForwardSwamp();

		// Crouch and crawl movement
		bool CanCrouch();
		bool CanCrouchToCrawl();
		bool CanCrouchRoll();
		bool CanCrawlForward();
		bool CanCrawlBack();

		// Monkey swing movement
		bool CanMonkeyForward();
		bool CanMonkeyBack();
		bool CanMonkeyShimmyLeft();
		bool CanMonkeyShimmyRight();

		// Vault movement
		// Crawl vault movement
		// Water tread climb out movement
		// Miscellaneous movement

	private:
		bool TestGroundMovementSetup(ContextSetupGroundMovement contextSetup, bool useCrawlSetup = false);
		bool TestMonkeyMovementSetup(ContextSetupMonkeyMovement contextSetup);
	};
}
