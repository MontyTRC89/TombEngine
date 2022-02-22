#pragma once
#include "Specific/phd_global.h"

struct KayakInfo 
{
	int TurnRate;

	int Velocity;
	int FrontVerticalVelocity;
	int LeftVerticalVelocity;
	int RightVerticalVelocity;

	unsigned int LeftRightCount;
	int WaterHeight;
	PHD_3DPOS OldPos;
	bool Turn;
	bool Forward;
	bool TrueWater;

	char Flags;
};
