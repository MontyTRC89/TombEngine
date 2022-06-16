#pragma once

struct MotorbikeInfo
{
	short TurnRate;
	short MomentumAngle;
	short ExtraRotation;
	short WallShiftRotation;

	int Velocity;

	int Revs;
	int EngineRevs;
	int Pitch;

	short RightWheelsRotation;
	short LeftWheelRotation;

	short LightPower;
	short Flags;
};
