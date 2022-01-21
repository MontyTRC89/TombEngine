#pragma once

struct MoveTestData
{
	short Angle;
	int LowerBound;
	int UpperBound;
	bool CheckSlopeDown = true;
	bool CheckSlopeUp = true;
	bool CheckDeath = true;
};

struct VaultTestData
{
	int LowerBound;
	int UpperBound;
	int ClampMin;
	int ClampMax;
	int GapMin;
	bool CheckSwampDepth = true;
};

struct VaultTestResultData
{
	bool Success;
	int Height;
};

struct CrawlVaultTestData
{
	int LowerBound;
	int UpperBound;
	int ClampMin;
	int GapMin;
	int CrossDist;
	int DestDist;
	int ProbeHeightDifMax;
	bool CheckSlope = true;
	bool CheckDeath = true;
};
