#pragma once
#include "collide.h"

typedef struct SubsuitInfo
{
	short XRot;
	short dXRot;
	short XRotVel;
	short Vel[2];
	short YVel;
};

// Auxillary Functions
void LaraWaterCurrent(COLL_INFO* coll);
int GetWaterDepth(int x, int y, int z, short roomNumber);
void UpdateSubsuitAngles();
void DoLaraSubsuitSwimTurn(ITEM_INFO* item);
void DoLaraSwimTurn(ITEM_INFO* item);
void LaraSwimCollision(ITEM_INFO* item, COLL_INFO* coll);
void TestLaraWaterDepth(ITEM_INFO* item, COLL_INFO* coll);

// State & Collision Functions
void lara_as_underwater_stop(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_underwater_stop(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_underwater_swim(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_underwater_swim(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_glide(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_glide(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_dive(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_dive(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_underwater_death(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_underwater_death(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_underwater_roll_180(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_underwater_roll_180(ITEM_INFO* item, COLL_INFO* coll);
