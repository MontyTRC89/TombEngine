#pragma once
#include "Game/animation.h"

namespace TEN::Entities::Player::Context
{
	struct LandMovementSetupData
	{
		short HeadingAngle	  = 0;
		int	  LowerFloorBound = 0;
		int	  UpperFloorBound = 0;

		bool TestSlipperySlopeBelow = true;
		bool TestSlipperySlopeAbove = true;
		bool TestDeathFloor			= true;
	};

	struct MonkeySwingSetupData
	{
		short HeadingAngle		= 0;
		float LowerCeilingBound = 0.0f;
		float UpperCeilingBound = 0.0f;
	};

	struct LedgeClimbSetupData
	{
		short HeadingAngle = 0;
		float SpaceMin	   = 0.0f;
		float SpaceMax	   = 0.0f;
		float GapMin	   = 0.0f;

		bool CheckSlope = false;
	};

	struct JumpSetupData
	{
		short HeadingAngle = 0;
		float Distance	   = 0.0f;

		bool TestWadeStatus = true;
	};

	struct VaultSetupData
	{
		float LowerFloorBound = 0.0f;
		float UpperFloorBound = 0.0f;
		float SpaceMin		  = 0.0f;
		float SpaceMax		  = 0.0f;
		float GapMin		  = 0.0f;

		bool TestSwampDepth = true;
	};

	struct VaultData
	{
		bool Success	 = false;
		int	 Height		 = 0;
		int	 TargetState = NO_STATE;

		bool SetBusyHands	 = false;
		bool DoLedgeSnap	 = false;
		bool SetJumpVelocity = false;
	};

	struct CrawlVaultSetupData
	{
		float LowerFloorBound = 0.0f;
		float UpperFloorBound = 0.0f;
		float SpaceMin		  = 0.0f;
		float GapMin		  = 0.0f;
		float CrossDist		  = 0.0f;
		float DestDist		  = 0.0f;
		float FloorBound	  = 0.0f;

		bool TestSlope = true;
		bool TestDeath = true;
	};

	struct CrawlVaultData
	{
		bool Success	 = false;
		int	 TargetState = NO_STATE;
	};

	struct WaterClimbOutSetupData
	{
		float LowerFloorBound = 0.0f;
		float UpperFloorBound = 0.0f;
		float ClampMin		  = 0.0f;
		float ClampMax		  = 0.0f;
		float GapMin		  = 0.0f;
	};

	struct WaterClimbOutData
	{
		bool Success	 = false;
		int	 Height		 = 0;
		int	 TargetState = NO_STATE;
	};

	struct LedgeHangData
	{
		bool Success = false;
		int	 Height	 = 0;
	};

	struct CornerShimmyData
	{
		bool Success;
		Pose ProbeResult;
		Pose RealPositionResult;
	};
}
