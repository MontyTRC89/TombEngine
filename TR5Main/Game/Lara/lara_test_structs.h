#pragma once

struct MoveTestData
{
	short angle;
	int lowerBound;
	int upperBound;
	bool checkSlopeDown = true;
	bool checkSlopeUp = true;
	bool checkDeath = true;
};

struct MonkeyMoveTestData
{
	short Angle;
	int LowerBound;
	int UpperBound;
};

struct VaultTestData
{
	int lowerBound;
	int upperBound;
	int clampMin;
	int clampMax;
	int gapMin;
	bool checkSwampDepth = true;
};

struct VaultTestResultData
{
	bool success;
	int height;
};

struct CrawlVaultTestData
{
	int lowerBound;
	int upperBound;
	int clampMin;
	int gapMin;
	int crossDist;
	int destDist;
	int probeDeltaMax;
	bool checkSlope = true;
	bool checkDeath = true;
};

struct JumpTestData
{
	short Angle;
	int Dist = CLICK(0.85f);
	bool CheckWadeStatus = true;
};
