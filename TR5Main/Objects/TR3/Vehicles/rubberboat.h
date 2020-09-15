#pragma once
#include  "items.h"
#include "collide.h"

typedef struct {
	int boatTurn;
	int leftFallspeed;
	int rightFallspeed;
	short tiltAngle;
	short extraRotation;
	int water;
	int pitch;
	short propRot;
}RUBBER_BOAT_INFO;

void InitialiseRubberBoat(short itemNum);
void RubberBoatCollision(short itemNum, ITEM_INFO *lara, COLL_INFO *coll);
void RubberBoatControl(short itemNum);
void DrawRubberBoat(ITEM_INFO *item);
