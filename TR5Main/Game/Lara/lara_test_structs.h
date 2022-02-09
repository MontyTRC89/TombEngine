#pragma once

struct MoveTestSetup
{
	short Angle;
	int	LowerFloorBound;
	int UpperFloorBound;
	bool CheckSlopeDown = true;
	bool CheckSlopeUp = true;
	bool CheckDeath = true;
};

struct MonkeyMoveTestSetup
{
	short Angle;
	int LowerCeilingBound;
	int UpperCeilingBound;
};

struct VaultTestSetup
{
	int LowerCeilingBound;
	int UpperCeilingBound;
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
	LARA_STATE TargetState;
};

struct CrawlVaultTestSetup
{
	int LowerFloorBound;
	int UpperFloorBound;
	int ClampMin;
	int GapMin;
	int CrossDist;
	int DestDist;
	int MaxProbeHeightDif;
	bool CheckSlope = true;
	bool CheckDeath = true;
};

struct CrawlVaultTestResult
{
	bool Success;
	LARA_STATE TargetState;
};

struct JumpTestSetup
{
	short Angle;
	int Dist = CLICK(0.85f);
	bool CheckWadeStatus = true;
};

struct CornerTestResult
{
	bool Success;
	PHD_3DPOS ProbeResult;
	PHD_3DPOS RealPositionResult;
};
