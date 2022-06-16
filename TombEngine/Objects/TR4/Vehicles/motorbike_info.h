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
	int bikeTurn;
	int pitch;
	short flags;
	short lightPower;
};