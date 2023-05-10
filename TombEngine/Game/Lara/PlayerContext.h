#pragma once
#include "Game/Lara/PlayerContextData.h"
#include "Math/Math.h"

struct CollisionInfo;
struct ItemInfo;
struct LaraInfo;

using namespace TEN::Math;

namespace TEN::Player
{
	class PlayerContext
	{
	private:
		// Parent pointers
		const ItemInfo*		 ItemPtr = nullptr;
		const LaraInfo*		 PlayerPtr = nullptr;
		const CollisionInfo* CollPtr = nullptr;

	public:
		// Members
		int			ProjectedFloorHeight = 0;
		float		CalcJumpVelocity	 = 0;
		Pose		NextCornerPos		 = Pose::Zero;
		EulerAngles TargetOrientation	 = EulerAngles::Zero;

		int		 WaterSurfaceDist	= 0;
		short	 WaterCurrentActive = 0; // Sink number? Often used as bool.
		Vector3i WaterCurrentPull	= Vector3i::Zero;

		int InteractedItem = 0; // Item number.
		int Vehicle		   = 0; // Item number.

		PlayerContext() {};
		PlayerContext(const ItemInfo& item, const CollisionInfo& coll);

		// TODO: Move all functions here.
	};

	// Ground movement contexts
	bool CanPerformStep(const ItemInfo& item, const CollisionInfo& coll);
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

	// Slide contexts
	bool CanSlide(const ItemInfo& item, const CollisionInfo& coll);
	bool CanSteerOnSlide(const ItemInfo& item, const CollisionInfo& coll);

	// Crouch and crawl contexts
	bool IsInLowSpace(const ItemInfo& item, const CollisionInfo& coll);
	bool CanCrouch(const ItemInfo& item, const CollisionInfo& coll);
	bool CanCrouchToCrawl(const ItemInfo& item, const CollisionInfo& coll);
	bool CanCrouchRoll(const ItemInfo& item, const CollisionInfo& coll);
	bool CanCrawlForward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanCrawlBackward(const ItemInfo& item, const CollisionInfo& coll);

	// Monkey swing contexts
	bool CanPerformMonkeyStep(const ItemInfo& item, const CollisionInfo& coll);
	bool CanFallFromMonkeySwing(const ItemInfo& item, const CollisionInfo& coll);
	bool CanGrabMonkeySwing(const ItemInfo& item, const CollisionInfo& coll);
	bool CanMonkeyForward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanMonkeyBackward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanMonkeyShimmyLeft(const ItemInfo& item, const CollisionInfo& coll);
	bool CanMonkeyShimmyRight(const ItemInfo& item, const CollisionInfo& coll);

	// Jump contexts
	bool CanFall(const ItemInfo& item, const CollisionInfo& coll);
	bool CanLand(const ItemInfo& item, const CollisionInfo& coll);
	bool CanPerformJump(const ItemInfo& item, const CollisionInfo& coll);
	bool CanJumpUp(const ItemInfo& item, const CollisionInfo& coll);
	bool CanJumpForward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanJumpBackward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanJumpLeft(const ItemInfo& item, const CollisionInfo& coll);
	bool CanJumpRight(const ItemInfo& item, const CollisionInfo& coll);
	bool CanQueueRunningJump(const ItemInfo& item, const CollisionInfo& coll);
	bool CanRunJumpForward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanSprintJumpForward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanPerformSlideJump(const ItemInfo& item, const CollisionInfo& coll);
	bool CanCrawlspaceDive(const ItemInfo& item, const CollisionInfo& coll);

	// Ledge contexts
	bool CanPerformLedgeJump(const ItemInfo& item, const CollisionInfo& coll);

	// Object interaction contexts
	bool CanDismountTightrope(const ItemInfo& item, const CollisionInfo& coll);
}
