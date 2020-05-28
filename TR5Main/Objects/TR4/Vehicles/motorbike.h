#pragma once
#include "types.h"
#include "collide.h"

struct MOTORBIKE_INFO
{
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

void InitialiseMotorbike(short itemNumber);
void MotorbikeCollision(short itemNumber, ITEM_INFO* laraitem, COLL_INFO* coll);
int MotorbikeControl(void);
void DrawMotorbike(ITEM_INFO* item);
void DrawMotorbikeEffect(ITEM_INFO* item);