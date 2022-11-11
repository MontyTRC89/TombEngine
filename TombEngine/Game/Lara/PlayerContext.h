#pragma once
#include "Game/Lara/PlayerContextStructs.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player::Context
{
	// ------------------
	// CONTEXT ASSESSMENT
	// ------------------
	
	// Basic movement
	bool CanSlide(ItemInfo* item, CollisionInfo* coll);
	bool CanAFKPose(ItemInfo* item, CollisionInfo* coll);
	bool CanTurn180(ItemInfo* item, CollisionInfo* coll);
	bool CanTurnFast(ItemInfo* item, CollisionInfo* coll);
	bool CanRunForward(ItemInfo* item, CollisionInfo* coll);
	bool CanRunBackward(ItemInfo* item, CollisionInfo* coll);
	bool CanWalkForward(ItemInfo* item, CollisionInfo* coll);
	bool CanWalkBackward(ItemInfo* item, CollisionInfo* coll);
	bool CanSidestepLeft(ItemInfo* item, CollisionInfo* coll);
	bool CanSidestepRight(ItemInfo* item, CollisionInfo* coll);
	bool CanWadeForward(ItemInfo* item, CollisionInfo* coll);
	bool CanWadeBackward(ItemInfo* item, CollisionInfo* coll);

	// Crouch and crawl movement
	bool IsInNarrowSpace(ItemInfo* item, CollisionInfo* coll);
	bool CanCrouch(ItemInfo* item, CollisionInfo* coll);
	bool CanCrouchToCrawl(ItemInfo* item, CollisionInfo* coll);
	bool CanCrouchRoll(ItemInfo* item, CollisionInfo* coll);
	bool CanCrawlForward(ItemInfo* item, CollisionInfo* coll);
	bool CanCrawlBackward(ItemInfo* item, CollisionInfo* coll);

	// Monkey swing movement
	bool CanGrabMonkeySwing(ItemInfo* item, CollisionInfo* coll);
	bool CanFallFromMonkeySwing(ItemInfo* item, CollisionInfo* coll);
	bool CanMonkeySwingForward(ItemInfo* item, CollisionInfo* coll);
	bool CanMonkeySwingBackward(ItemInfo* item, CollisionInfo* coll);
	bool CanMonkeySwingShimmyLeft(ItemInfo* item, CollisionInfo* coll);
	bool CanMonkeySwingShimmyRight(ItemInfo* item, CollisionInfo* coll);

	// Jump movement
	bool CanLand(ItemInfo* item, CollisionInfo* coll);
	bool CanFall(ItemInfo* item, CollisionInfo* coll);
	bool CanPerformJump(ItemInfo* item, CollisionInfo* coll);
	bool CanJumpUp(ItemInfo* item, CollisionInfo* coll);
	bool CanJumpForward(ItemInfo* item, CollisionInfo* coll);
	bool CanJumpBackward(ItemInfo* item, CollisionInfo* coll);
	bool CanJumpLeft(ItemInfo* item, CollisionInfo* coll);
	bool CanJumpRight(ItemInfo* item, CollisionInfo* coll);
	bool CanRunJumpForward(ItemInfo* item, CollisionInfo* coll);
	bool CanSprintJumpForward(ItemInfo* item, CollisionInfo* coll);
	bool CanSlideJumpForward(ItemInfo* item, CollisionInfo* coll);
	bool CanCrawlspaceDive(ItemInfo* item, CollisionInfo* coll);

	// Vault movement
	Context::Vault GetVaultUp2Steps(ItemInfo* item, CollisionInfo* coll);

	// Crawl vault movement
	// Water tread climb out movement

	// ----------------------------------------------------------------------------------------------------------------------
	
	// Helper inquirers
	bool TestSidestep(ItemInfo* item, CollisionInfo* coll, bool isGoingRight);
	bool TestMonkeyShimmy(ItemInfo* item, CollisionInfo* coll, bool isGoingRight);
	bool TestDirectionalStandingJump(ItemInfo* item, CollisionInfo* coll, short relativeHeadingAngle);

	// Context setup inquirers
	bool TestGroundMovementSetup(ItemInfo* item, CollisionInfo* coll, const Context::GroundMovementSetup& contextSetup, bool useCrawlSetup = false);
	bool TestMonkeyMovementSetup(ItemInfo* item, CollisionInfo* coll, const Context::MonkeyMovementSetup& contextSetup);
	bool TestJumpMovementSetup(ItemInfo* item, CollisionInfo* coll, const Context::JumpSetup& contextSetup);

	// Context getters
	Context::Vault GetVault(ItemInfo* item, CollisionInfo* coll, const Context::VaultSetup& contextSetup);
}
