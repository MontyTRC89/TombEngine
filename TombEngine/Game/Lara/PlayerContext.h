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

		// Basic ground movement
		bool CanTurnFast();
		bool CanRunForward();
		bool CanRunBackward();
		bool CanWalkForward();
		bool CanWalkBackward();
		bool CanSidestepLeft();
		bool CanSidestepRight();
		bool CanWadeForward();
		bool CanWadeBackward();

		// Crouch and crawl movement
		bool CanCrouch();
		bool CanCrouchToCrawl();
		bool CanCrouchRoll();
		bool CanCrawlForward();
		bool CanCrawlBack();

		// Monkey swing movement
		bool CanMonkeyForward();
		bool CanMonkeyBackward();
		bool CanMonkeyShimmyLeft();
		bool CanMonkeyShimmyRight();

		// Vault movement
		// Crawl vault movement
		// Water tread climb out movement
		// Miscellaneous movement

	private:
		bool TestGroundMovementSetup(Context::SetupGroundMovement contextSetup, bool useCrawlSetup = false);
		bool TestMonkeyMovementSetup(Context::SetupMonkeyMovement contextSetup);
	};
}
