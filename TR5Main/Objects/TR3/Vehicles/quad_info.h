#pragma once

struct QuadInfo 
{
	short TurnRate;
	short FrontRot;
	short RearRot;
	short ExtraRotation;
	short MomentumAngle;

	int Velocity;
	int LeftVerticalVelocity;
	int RightVerticalVelocity;

	int Revs;
	int EngineRevs;
	int Pitch;

	char Flags;
	int SmokeStart;
	bool CanStartDrift;
	bool DriftStarting;
	bool NoDismount;
};
