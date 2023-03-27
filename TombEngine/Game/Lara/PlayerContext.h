#pragma once
#include "Game/Lara/PlayerContextStructs.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player::Context
{
	// -----------------------------
	// CONTEXT ASSESSMENT FUNCTIONS
	// For State Control & Collision
	// -----------------------------

	// Basic contexts
	bool CanPerformStep(ItemInfo* item, CollisionInfo* coll);
	bool CanStepUp(ItemInfo* item, CollisionInfo* coll);
	bool CanStepDown(ItemInfo* item, CollisionInfo* coll);
	bool CanStrikeAFKPose(ItemInfo* item, CollisionInfo* coll);
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

	// Slide contexts
	bool CanSlide(ItemInfo* item, CollisionInfo* coll);
	bool CanSteerOnSlide(ItemInfo* item, CollisionInfo* coll);

	// Crouch and crawl contexts
	bool IsInNarrowSpace(ItemInfo* item, CollisionInfo* coll);
	bool CanCrouch(ItemInfo* item, CollisionInfo* coll);
	bool CanCrouchToCrawl(ItemInfo* item, CollisionInfo* coll);
	bool CanCrouchRoll(ItemInfo* item, CollisionInfo* coll);
	bool CanCrawlForward(ItemInfo* item, CollisionInfo* coll);
	bool CanCrawlBackward(ItemInfo* item, CollisionInfo* coll);

	// Monkey swing contexts
	bool CanPerformMonkeyStep(ItemInfo* item, CollisionInfo* coll);
	bool CanFallFromMonkeySwing(ItemInfo* item, CollisionInfo* coll);
	bool CanGrabMonkeySwing(ItemInfo* item, CollisionInfo* coll);
	bool CanMonkeyForward(ItemInfo* item, CollisionInfo* coll);
	bool CanMonkeyBackward(ItemInfo* item, CollisionInfo* coll);
	bool CanMonkeyShimmyLeft(ItemInfo* item, CollisionInfo* coll);
	bool CanMonkeyShimmyRight(ItemInfo* item, CollisionInfo* coll);

	// Ledge contexts
	bool CanSwingOnLedge(ItemInfo* item, CollisionInfo* coll);
	bool CanPerformLedgeJump(ItemInfo* item, CollisionInfo* coll);
	bool CanClimbLedgeToCrouch(ItemInfo* item, CollisionInfo* coll);
	bool CanClimbLedgeToStand(ItemInfo* item, CollisionInfo* coll);
	bool CanLedgeShimmyLeft(ItemInfo* item, CollisionInfo* coll);
	bool CanLedgeShimmyRight(ItemInfo* item, CollisionInfo* coll);
	bool CanWallShimmyUp(ItemInfo* item, CollisionInfo* coll);
	bool CanWallShimmyDown(ItemInfo* item, CollisionInfo* coll);

	// Jump contexts
	bool CanFall(ItemInfo* item, CollisionInfo* coll);
	bool CanLand(ItemInfo* item, CollisionInfo* coll);
	bool CanPerformJump(ItemInfo* item, CollisionInfo* coll);
	bool CanJumpUp(ItemInfo* item, CollisionInfo* coll);
	bool CanJumpForward(ItemInfo* item, CollisionInfo* coll);
	bool CanJumpBackward(ItemInfo* item, CollisionInfo* coll);
	bool CanJumpLeft(ItemInfo* item, CollisionInfo* coll);
	bool CanJumpRight(ItemInfo* item, CollisionInfo* coll);
	bool CanRunJumpForward(ItemInfo* item, CollisionInfo* coll);
	bool CanSprintJumpForward(ItemInfo* item, CollisionInfo* coll);
	bool CanPerformSlideJump(ItemInfo* item, CollisionInfo* coll);
	bool CanCrawlspaceDive(ItemInfo* item, CollisionInfo* coll);

	// Object interaction contexts
	bool CanDismountTightrope(ItemInfo* item, CollisionInfo* coll);

	// Vault contexts
	Context::VaultData GetVaultUp2Steps(ItemInfo* item, CollisionInfo* coll);

	// Crawl vault contexts

	// Water climb out contexts
	Context::WaterClimbOutData GetWaterClimbOutDownStep(ItemInfo* item, CollisionInfo* coll);
	Context::WaterClimbOutData GetWaterClimbOutDownStepToCrouch(ItemInfo* item, CollisionInfo* coll);
	Context::WaterClimbOutData GetWaterClimbOutFlatStep(ItemInfo* item, CollisionInfo* coll);
	Context::WaterClimbOutData GetWaterClimbOutFlatStepToCrouch(ItemInfo* item, CollisionInfo* coll);
	Context::WaterClimbOutData GetWaterClimbOutUpStep(ItemInfo* item, CollisionInfo* coll);
	Context::WaterClimbOutData GetWaterClimbOutUpStepToCrouch(ItemInfo* item, CollisionInfo* coll);
	Context::WaterClimbOutData GetWaterClimbOut(ItemInfo* item, CollisionInfo* coll);

	// -------------------------------------------------------------------------------------------------
	
	// Context assessment helpers
	bool TestSidestep(ItemInfo* item, CollisionInfo* coll, bool isGoingRight);
	bool TestMonkeyShimmy(ItemInfo* item, CollisionInfo* coll, bool isGoingRight);
	bool TestDirectionalStandingJump(ItemInfo* item, CollisionInfo* coll, short relHeadingAngle);

	// Context setup inquirers
	bool TestLandMovementSetup(ItemInfo* item, CollisionInfo* coll, const Context::LandMovementSetupData& setupData, bool isCrawling = false);
	bool TestMonkeySwingSetup(ItemInfo* item, CollisionInfo* coll, const Context::MonkeySwingSetupData& setupData);
	bool TestLedgeClimbSetup(ItemInfo* item, CollisionInfo* coll, const Context::LedgeClimbSetupData& setupData);
	bool TestJumpSetup(ItemInfo* item, CollisionInfo* coll, const Context::JumpSetupData& setupData);

	// Context getters
	// NOTE: These are used by higher-level functions to partially fill specialized context structs.
	// TODO: Make these into classes?
	Context::VaultData		   GetVaultContextData(ItemInfo* item, CollisionInfo* coll, const Context::VaultSetupData& setupData);
	Context::WaterClimbOutData GetWaterClimbOutData(ItemInfo* item, CollisionInfo* coll, const Context::WaterClimbOutSetupData& setupData);
}
