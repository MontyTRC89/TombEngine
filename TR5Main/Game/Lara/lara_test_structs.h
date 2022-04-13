#pragma once

struct MoveTestSetup
{
	float Angle;
	int	LowerFloorBound;
	int UpperFloorBound;
	bool CheckSlopeDown = true;
	bool CheckSlopeUp = true;
	bool CheckDeath = true;
};

struct MonkeyMoveTestSetup
{
	float Angle;
	int LowerCeilingBound;
	int UpperCeilingBound;
};

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

struct JumpTestSetup
{
	float Angle;
	int Distance = CLICK(0.85f);
	bool CheckWadeStatus = true;
};

struct CornerTestResult
{
	bool Success;
	PHD_3DPOS ProbeResult;
	PHD_3DPOS RealPositionResult;
};
