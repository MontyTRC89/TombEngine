#pragma once
#include "Game/Lara/PlayerContextStructs.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player
{
	class PlayerContext
	{
	private:
		// Private pointer variables
		// TODO: Bad idea or okay?
		ItemInfo*	   PlayerItemPtr = nullptr;
		CollisionInfo* PlayerCollPtr = nullptr;

	public:
		// Constructors
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
		bool CanCrawlBackward();

		// Monkey swing movement
		bool CanMonkeyForward();
		bool CanMonkeyBackward();
		bool CanMonkeyShimmyLeft();
		bool CanMonkeyShimmyRight();

		// Jump movement
		bool CanJumpUp();
		bool CanJumpForward();
		bool CanJumpBackward();
		bool CanJumpLeft();
		bool CanJumpRight();
		bool CanRunJumpForward();
		bool CanSlideJumpForward();
		bool CanCrawlspaceDive();

		// Vault movement
		// Crawl vault movement
		// Water tread climb out movement

	private:
		// Helper inquirers
		bool TestSidestep(bool goingRight);
		bool TestMonkeyShimmy(bool goingRight);
		bool TestDirectionalStandingJump(short angle);

		// Setup inquirers
		bool TestGroundMovementSetup(Context::SetupGroundMovement contextSetup, bool useCrawlSetup = false);
		bool TestMonkeyMovementSetup(Context::SetupMonkeyMovement contextSetup);
		bool TestJumpMovementSetup(Context::SetupJump testSetup);
	};
}
