#pragma once
struct MotorbikeInfo {
	int wheelRight;  // (two wheel: front and back)
	int wheelLeft;   // (one wheel: left)
	int velocity;
	int revs;
	int engineRevs;
	float momentumAngle;
	float extraRotation;
	float wallShiftRotation;
	float bikeTurn;
	int pitch;
	short flags;
	short lightPower;
};