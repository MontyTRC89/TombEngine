#pragma once

struct MinecartInfo 
{
	float TurnRot;
	float TurnX;
	float TurnZ;
	int TurnLen;

	int Velocity;
	int VerticalVelocity;
	int Gradient;
	unsigned int StopDelay;

	int FloorHeightMiddle;
	int FloorHeightFront;

	char Flags;
};
