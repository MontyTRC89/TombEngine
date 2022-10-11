#pragma once
#include "Game/Lara/PlayerContextStructs.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player
{
	class PlayerContext
	{
	private:
		// Private player pointers
		// TODO: Bad idea or okay?
		ItemInfo*	   PlayerItemPtr = nullptr;
		CollisionInfo* PlayerCollPtr = nullptr;

	public:
		// Constructors
		PlayerContext();
		PlayerContext(ItemInfo* item, CollisionInfo* coll);

		// Basic ground movement
		bool CanAFKPose();
		bool CanTurn180();
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
		bool IsInNarrowSpace();
		bool CanCrouch();
		bool CanCrouchToCrawl();
		bool CanCrouchRoll();
		bool CanCrawlForward();
		bool CanCrawlBackward();

		// Monkey swing movement
		bool CanGrabMonkeySwing();
		bool CanMonkeyForward();
		bool CanMonkeyBackward();
		bool CanMonkeyShimmyLeft();
		bool CanMonkeyShimmyRight();

		// Jump movement
		bool CanPerformJump();
		bool CanJumpUp();
		bool CanJumpForward();
		bool CanJumpBackward();
		bool CanJumpLeft();
		bool CanJumpRight();
		bool CanRunJumpForward();
		bool CanSprintJumpForward();
		bool CanSlideJumpForward();
		bool CanCrawlspaceDive();

		// Vault movement
		// Crawl vault movement
		// Water tread climb out movement

	private:
		// Helper inquirers
		bool TestSidestep(bool isGoingRight);
		bool TestMonkeyShimmy(bool isGoingRight);
		bool TestDirectionalStandingJump(short relativeHeadingAngle);

		// Setup inquirers
		bool TestGroundMovementSetup(const Context::SetupGroundMovement& contextSetup, bool useCrawlSetup = false);
		bool TestMonkeyMovementSetup(const Context::SetupMonkeyMovement& contextSetup);
		bool TestJumpMovementSetup(const Context::SetupJump& testSetup);
	};
}
