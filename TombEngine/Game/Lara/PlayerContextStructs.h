#pragma once
#include "Game/animation.h"

namespace TEN::Entities::Player::Context
{
	struct GroundMovementSetup
	{
		short HeadingAngle	  = 0;
		float LowerFloorBound = 0.0f;
		float UpperFloorBound = 0.0f;

		bool TestSlopeDown  = true;
		bool TestSlopeUp	= true;
		bool TestDeathFloor = true;
	};

	struct MonkeyMovementSetup
	{
		short HeadingAngle		= 0;
		float LowerCeilingBound = 0.0f;
		float UpperCeilingBound = 0.0f;
	};

	struct JumpSetup
	{
		short HeadingAngle = 0;
		float Distance	   = 0.0f;

		bool TestWadeStatus = true;
	};

	struct VaultSetup
	{
		float LowerFloorBound = 0.0f;
		float UpperFloorBound = 0.0f;
		float ClampMin		  = 0.0f;
		float ClampMax		  = 0.0f;
		float GapMin		  = 0.0f;

		bool TestSwampDepth = true;
	};

	struct Vault
	{
		bool Success	 = false;
		int	 Height		 = 0;
		int	 TargetState = NO_STATE;

		bool SetBusyHands	 = false;
		bool DoLedgeSnap	 = false;
		bool SetJumpVelocity = false;
	};

	struct CrawlVaultSetup
	{
		float LowerFloorBound = 0.0f;
		float UpperFloorBound = 0.0f;
		float ClampMin		  = 0.0f;
		float GapMin		  = 0.0f;
		float CrossDist		  = 0.0f;
		float DestDist		  = 0.0f;
		float FloorBound	  = 0.0f;

		bool TestSlope = true;
		bool TestDeath = true;
	};

	struct CrawlVault
	{
		bool Success	 = false;
		int	 TargetState = NO_STATE;
	};

	struct WaterClimbOutSetup
	{
		float LowerFloorBound = 0.0f;
		float UpperFloorBound = 0.0f;
		float ClampMin		  = 0.0f;
		float ClampMax		  = 0.0f;
		float GapMin		  = 0.0f;
	};

	struct WaterClimbOut
	{
		bool Success	 = false;
		int	 Height		 = 0;
		int	 TargetState = NO_STATE;
	};

	struct LedgeHang
	{
		bool Success = false;
		int	 Height	 = 0;
	};

	struct HangClimbSetup
	{
		float ClampMin;
		float ClampMax;
		float GapMin;
		bool CheckSlope;
	};

	struct CornerShimmy
	{
		bool Success;
		Pose ProbeResult;
		Pose RealPositionResult;
	};
}
