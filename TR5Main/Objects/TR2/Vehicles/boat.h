#pragma once
#include "items.h"
#include "collide.h"

enum class BoatGetOn 
{
	None = 0,
	WaterRight = 1,
	WaterLeft = 2,
	Jump = 3,
	StartPosition = 4
};

void InitialiseSpeedBoat(short itemNum);
void SpeedBoatCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll);
void SpeedBoatControl(short itemNumber);
void DoBoatWakeEffect(ITEM_INFO* boat);
void SpeedBoatGetOff(ITEM_INFO* boat);
bool SpeedBoatCanGetOff(int direction);
BoatGetOn SpeedBoatCheckGeton(short itemNum, COLL_INFO* coll);
int SpeedBoatTestWaterHeight(ITEM_INFO* item, int zOff, int xOff, PHD_VECTOR* pos);
void SpeedBoatDoBoatShift(int itemNum);
int SpeedBoatDoBoatDynamics(int height, int fallspeed, int* y);
int SpeedBoatDynamics(short itemNum);
bool SpeedBoatUserControl(ITEM_INFO* boat);
void SpeedBoatAnimation(ITEM_INFO* boat, int collide);
void SpeedBoatSplash(ITEM_INFO* item, long fallspeed, long water);
short SpeedBoatDoShif(ITEM_INFO* skidoo, PHD_VECTOR* pos, PHD_VECTOR* old);
int SpeedBoatGetCollisionAnim(ITEM_INFO* skidoo, PHD_VECTOR* moved);