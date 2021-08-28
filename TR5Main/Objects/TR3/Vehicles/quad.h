#pragma once

struct ITEM_INFO;
struct COLL_INFO;
struct QUAD_INFO
{
	int velocity;
	short frontRot;
	short rearRot;
	int revs;
	int engineRevs;
	short trackMesh;
	int skidooTurn;
	int leftFallspeed;
	int rightFallspeed;
	short momentumAngle;
	short extraRotation;
	int pitch;
	char flags;
};

void InitialiseQuadBike(short itemNumber);
void QuadBikeCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
int QuadBikeControl(void);