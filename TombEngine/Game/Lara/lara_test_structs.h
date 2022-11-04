#pragma once

struct VaultTestSetup
{
	int LowerFloorBound;
	int UpperFloorBound;
	int ClampMin;
	int ClampMax;
	int GapMin;
	bool CheckSwampDepth = true;
};

struct VaultTestResult
{
	bool Success;
	int Height;
	bool SetBusyHands;
	bool SnapToLedge;
	bool SetJumpVelocity;
	LaraState TargetState;
};

struct CrawlVaultTestSetup
{
	int LowerFloorBound;
	int UpperFloorBound;
	int ClampMin;
	int GapMin;
	int CrossDist;
	int DestDist;
	int FloorBound;
	bool CheckSlope = true;
	bool CheckDeath = true;
};

struct CrawlVaultTestResult
{
	bool Success;
	LaraState TargetState;
};

struct WaterClimbOutTestSetup
{
	int LowerFloorBound;
	int UpperFloorBound;
	int ClampMin;
	int ClampMax;
	int GapMin;
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
	int ClampMin;
	int ClampMax;
	int GapMin;
	bool CheckSlope;
};

struct CornerTestResult
{
	bool Success;
	Pose ProbeResult;
	Pose RealPositionResult;
};
