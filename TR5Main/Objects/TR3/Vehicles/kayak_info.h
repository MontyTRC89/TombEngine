#pragma once
#include "Specific/phd_global.h"

struct KayakInfo 
{
	float TurnRate;

	int Velocity;
	int FrontVerticalVelocity;
	int LeftVerticalVelocity;
	int RightVerticalVelocity;

	unsigned int LeftRightCount;
	int WaterHeight;
	PoseData OldPos;
	bool Turn;
	bool Forward;
	bool TrueWater;

	int CurrentStartWake;
	int WakeShade;

	char Flags;
};
