#pragma once

struct MinecartInfo 
{
	short TurnRot;
	int TurnX;
	int TurnZ;
	short TurnLen;

	int Velocity;
	short VerticalVelocity;
	short Gradient;
	char StopDelay;

	int FloorHeightMiddle;
	int FloorHeightFront;

	char Flags;
};
