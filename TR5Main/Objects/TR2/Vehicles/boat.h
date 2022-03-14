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

void InitialiseSpeedBoat(short itemNumber);
void DoBoatWakeEffect(ITEM_INFO* sBoatItem);
BoatMountType GetSpeedBoatMountType(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem, CollisionInfo* coll);
bool TestSpeedBoatDismount(ITEM_INFO* sBoatItem, int direction);
void DoSpeedBoatDismount(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem);
int SpeedBoatTestWaterHeight(ITEM_INFO* sBoatItem, int zOffset, int xOffset, PHD_VECTOR* pos);

void SpeedBoatDoBoatShift(ITEM_INFO* sBoatItem, int itemNumber);
short SpeedBoatDoShift(ITEM_INFO* sBoatItem, PHD_VECTOR* pos, PHD_VECTOR* old);

int GetSpeedBoatHitAnim(ITEM_INFO* sBoatItem, PHD_VECTOR* moved);
int DoSpeedBoatDynamics(int height, int verticalVelocity, int* y);
int SpeedBoatDynamics(ITEM_INFO* laraItem, short itemNumber);
bool SpeedBoatUserControl(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem);
void SpeedBoatAnimation(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem, int collide);
void SpeedBoatSplash(ITEM_INFO* item, long verticalVelocity, long water);
void SpeedBoatCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void SpeedBoatControl(short itemNumber);
