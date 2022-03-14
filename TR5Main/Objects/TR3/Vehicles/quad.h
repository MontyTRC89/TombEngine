#pragma once

struct ITEM_INFO;
struct CollisionInfo;

void InitialiseQuadBike(short itemNumber);
//static void QuadbikeExplode(ITEM_INFO* laraItem, ITEM_INFO* quadItem);
//static int CanQuadbikeGetOff(int direction);
//static bool QuadCheckGetOff(ITEM_INFO* laraItem, ITEM_INFO* quadItem);
//static int GetOnQuadBike(ITEM_INFO* laraItem, ITEM_INFO* quadItem, COLL_INFO* coll);
//static void QuadBaddieCollision(ITEM_INFO* laraItem, ITEM_INFO* quadItem);
//static int GetQuadCollisionAnim(ITEM_INFO* quadItem, PHD_VECTOR* p);
//static int TestQuadHeight(ITEM_INFO* quadItem, int dz, int dx, PHD_VECTOR* pos);
//static int DoQuadShift(ITEM_INFO* quadItem, PHD_VECTOR* pos, PHD_VECTOR* old);
//static int DoQuadDynamics(int height, int fallspeed, int* y);
//static int QuadDynamics(ITEM_INFO* laraItem, ITEM_INFO* quadItem);
//static void AnimateQuadBike(ITEM_INFO* laraItem, ITEM_INFO* quadItem, int collide, bool dead);
//static int QuadUserControl(ITEM_INFO* quadItem, int height, int* pitch);
void QuadBikeCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
//static void TriggerQuadExhaustSmoke(int x, int y, int z, short angle, int speed, int moving);
bool QuadBikeControl(ITEM_INFO* laraItem, CollisionInfo* coll);
