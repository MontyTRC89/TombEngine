#pragma once
struct MotorbikeInfo {
	short wheelRight;  // (two wheel: front and back)
	short wheelLeft;   // (one wheel: left)
	int velocity;
	int revs;
	int engineRevs;
	short momentumAngle;
	short extraRotation;
	short wallShiftRotation;
	short bikeTurn;
	int pitch;
	short flags;
	short lightPower;
};