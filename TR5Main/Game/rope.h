#pragma once

#include "..\Global\global.h"

struct ROPE_STRUCT
{
	PHD_VECTOR segment[24]; // size=288, offset=0
	PHD_VECTOR velocity[24]; // size=288, offset=288
	PHD_VECTOR normalisedSegment[24]; // size=288, offset=576
	PHD_VECTOR meshSegment[24]; // size=288, offset=864
	PHD_VECTOR position; // size=12, offset=1152
	PHD_VECTOR Unknown[24];
	int segmentLength; // size=0, offset=1164
	short active; // size=0, offset=1168
	short coiled; // size=0, offset=1170
};

struct PENDULUM
{
	PHD_VECTOR Position; // size=12, offset=0
	PHD_VECTOR Velocity; // size=12, offset=12
	int node; // size=0, offset=24
	ROPE_STRUCT* Rope; // size=1172, offset=28
};

//#define InitialiseRope ((void (__cdecl*)(short)) 0x0046F060)
#define RopeControl ((void (__cdecl*)(short)) 0x0046DD40)
#define RopeCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x0046DAE0)
#define CalculateRopePoints ((void (__cdecl*)(ROPE_STRUCT*)) 0x0046EC70)

void InitialiseRope(short itemNumber);
void PrepareRope(ROPE_STRUCT* rope, PHD_VECTOR* pos1, PHD_VECTOR* pos2, int length, ITEM_INFO* item);
void NormaliseRopeVector(PHD_VECTOR* vec);