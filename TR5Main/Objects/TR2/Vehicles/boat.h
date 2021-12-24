#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"

enum class BoatMountType 
{
	None = 0,
	WaterRight = 1,
	WaterLeft = 2,
	Jump = 3,
	StartPosition = 4
};

void InitialiseSpeedBoat(short itemNum);
void DoBoatWakeEffect(ITEM_INFO* sBoatItem);
BoatMountType GetSpeedBoatMountType(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem, COLL_INFO* coll);
bool TestSpeedBoatDismount(ITEM_INFO* sBoatItem, int direction);
void DoSpeedBoatDismount(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem);
int SpeedBoatTestWaterHeight(ITEM_INFO* sBoatItem, int zOff, int xOff, PHD_VECTOR* pos);

void SpeedBoatDoBoatShift(ITEM_INFO* sBoatItem, int itemNum);
short SpeedBoatDoShift(ITEM_INFO* skidoo, PHD_VECTOR* pos, PHD_VECTOR* old);

int GetSpeedBoatHitAnim(ITEM_INFO* sBoatItem, PHD_VECTOR* moved);
int SpeedBoatDoBoatDynamics(int height, int fallspeed, int* y);
int SpeedBoatDynamics(ITEM_INFO* laraItem, short itemNum);
bool SpeedBoatUserControl(ITEM_INFO* sBoatItem);
void SpeedBoatAnimation(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem, int collide);
void SpeedBoatSplash(ITEM_INFO* item, long fallspeed, long water);
void SpeedBoatCollision(short itemNum, ITEM_INFO* laraItem, COLL_INFO* coll);
void SpeedBoatControl(short itemNum);
