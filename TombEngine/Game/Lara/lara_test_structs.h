#pragma once

struct VaultTestSetup
{
	int LowerFloorBound = 0.0f;
	int UpperFloorBound = 0.0f;
	int ClampMin		  = 0.0f;
	int ClampMax		  = 0.0f;
	int GapMin		  = 0.0f;

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
	int LowerFloorBound = 0.0f;
	int UpperFloorBound = 0.0f;
	int ClampMin		  = 0.0f;
	int GapMin		  = 0.0f;
	int CrossDist		  = 0.0f;
	int DestDist		  = 0.0f;
	int FloorBound	  = 0.0f;

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

struct CornerTestResult
{
	bool Success;
	Pose ProbeResult;
	Pose RealPositionResult;
};
