#pragma once

#include "..\Global\global.h"

// used by coll->badPos
#define NO_BAD_POS (-NO_HEIGHT)
// used by coll->badNeg
#define NO_BAD_NEG NO_HEIGHT

#define GenericSphereBoxCollision ((void (__cdecl*)(short, ITEM_INFO*, COLL_INFO*)) 0x00413A90)

int CollideStaticObjects(COLL_INFO* coll, int x, int y, int z, short roomNumber, int hite);
int GetCollidedObjects(ITEM_INFO* collidingItem, int radius, int flag1, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int flag2);
int TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll);
void TrapCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll);
void ShiftItem(ITEM_INFO* item, COLL_INFO* coll);
void UpdateLaraRoom(ITEM_INFO* item, int height);
short GetTiltType(FLOOR_INFO* floor, int x, int y, int z);
int FindGridShift(int x, int z);
int TestBoundsCollideStatic(short* bounds, PHD_3DPOS* pos, int radius);
int ItemPushLaraStatic(ITEM_INFO* item, short* bounds, PHD_3DPOS* pos, COLL_INFO* coll);
void AIPickupCollision(short itemNumber);
void ObjectCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l);
void TriggerLaraBlood();
int ItemPushLara(ITEM_INFO* item, ITEM_INFO* l, COLL_INFO* coll, int spazon, char bigpush);
int TestLaraPosition(short* bounds, ITEM_INFO* item, ITEM_INFO* l);
int Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angAdd);
int MoveLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* l);
int TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, int radius);
void CreatureCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void GetCollisionInfo(COLL_INFO* coll, int xPos, int yPos, int zPos, int roomNumber, int objectHeight);
void LaraBaddieCollision(ITEM_INFO* item, COLL_INFO* coll);

void Inject_Collide();