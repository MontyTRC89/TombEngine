#pragma once

struct VaultTestSetup
{
	float LowerFloorBound = 0.0f;
	float UpperFloorBound = 0.0f;
	float ClampMin		  = 0.0f;
	float ClampMax		  = 0.0f;
	float GapMin		  = 0.0f;

	bool TestSwampDepth = true;
};

struct VaultTestResult
{
	bool Success	 = false;
	int	 Height		 = 0;

	bool SetBusyHands	 = false;
	bool SnapToLedge	 = false;
	bool SetJumpVelocity = false;

	int	 TargetState = NO_STATE;
};

struct CrawlVaultTestSetup
{
	float LowerFloorBound = 0.0f;
	float UpperFloorBound = 0.0f;
	float ClampMin		  = 0.0f;
	float GapMin		  = 0.0f;
	float CrossDist		  = 0.0f;
	float DestDist		  = 0.0f;
	float FloorBound	  = 0.0f;

	bool CheckSlope = true;
	bool CheckDeath = true;
};

struct CrawlVaultTestResult
{
	bool Success	 = false;
	int	 TargetState = NO_STATE;
};

struct WaterClimbOutTestSetup
{
	float LowerFloorBound = 0.0f;
	float UpperFloorBound = 0.0f;
	float ClampMin = 0.0f;
	float ClampMax = 0.0f;
	float GapMin = 0.0f;
};

struct WaterClimbOutTestResult
{
	bool Success;
	int Height;
	LaraState TargetState;
};

struct LedgeHangTestResult
{
	bool Success;
	int Height;
};

struct HangClimbTestSetup
{
	float ClampMin;
	float ClampMax;
	float GapMin;
	bool CheckSlope;
};

struct CornerTestResult
{
	bool Success;
	Pose ProbeResult;
	Pose RealPositionResult;
};
