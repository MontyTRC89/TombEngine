#pragma once
struct MOTORBIKE_INFO {
	int wheelRight;  // (two wheel: front and back)
	int wheelLeft;   // (one wheel: left)
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