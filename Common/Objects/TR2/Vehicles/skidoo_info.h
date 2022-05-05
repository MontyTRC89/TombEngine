#pragma once

struct SkidooInfo 
{
	float TurnRate;
	float MomentumAngle;
	float ExtraRotation;

	int LeftVerticalVelocity;
	int RightVerticalVelocity;

	int Pitch;
	int FlashTimer;
	short TrackMesh;
	bool Armed;
};
