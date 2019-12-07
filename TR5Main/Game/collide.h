#pragma once

#include "..\Global\global.h"

// used by coll->badPos
#define NO_BAD_POS (-NO_HEIGHT)
// used by coll->badNeg
#define NO_BAD_NEG NO_HEIGHT

//#define ObjectCollision ((void (__cdecl*)(short, ITEM_INFO*, COLL_INFO*)) 0x004126E0)
#define GetCollisionInfo ((void (__cdecl*)(COLL_INFO*, int, int, int, short, int)) 0x00411100)
#define GenericSphereBoxCollision ((void (__cdecl*)(short, ITEM_INFO*, COLL_INFO*)) 0x00413A90)
#define LaraBaddieCollision ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00412170)
//#define TestBoundsCollide ((int (__cdecl*)(ITEM_INFO*, ITEM_INFO*, int)) 0x00412CC0)
//#define GetCollidedObjects ((int (__cdecl*)(ITEM_INFO*, int, int, ITEM_INFO**, MESH_INFO**, int)) 0x00413CF0)
//#define ItemPushLara ((int (__cdecl*)(ITEM_INFO*, ITEM_INFO*, COLL_INFO*, int, int)) 0x00412860)
#define TestCollision ((int (__cdecl*)(ITEM_INFO*, ITEM_INFO*)) 0x00479170)
//#define TestLaraPosition ((int (__cdecl*)(short*, ITEM_INFO*, ITEM_INFO*)) 0x00413210)
//#define MoveLaraPosition ((int (__cdecl*)(PHD_VECTOR*, ITEM_INFO*, ITEM_INFO*)) 0x00413840)
//#define AlignLaraPosition ((int (__cdecl*)(PHD_VECTOR*, ITEM_INFO*, ITEM_INFO*)) 0x004133C0)

int CollideStaticObjects(COLL_INFO* coll, int x, int y, int z, short roomNumber, int hite);
int GetCollidedObjects(ITEM_INFO* collidingItem, int radius, int flag1, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int flag2);
int TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll);
void TrapCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll);
void ShiftItem(ITEM_INFO* item, COLL_INFO* coll);
void UpdateLaraRoom(ITEM_INFO* item, int height);
int GetTiltType(FLOOR_INFO* floor, int x, int y, int z);
int FindGridShift(int x, int z);
int TestBoundsCollideStatic(short* bounds, PHD_3DPOS* pos, int radius);
int ItemPushLaraStatic(ITEM_INFO* item, short* bounds, PHD_3DPOS* pos, COLL_INFO* coll);
void AIPickupCollision(short itemNumber);
void ObjectCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l);
void TriggerLaraBlood();
int ItemPushLara(ITEM_INFO* item, ITEM_INFO* l, COLL_INFO* coll, int spazon, char bigpush);
int TestLaraPosition(__int16* bounds, ITEM_INFO* item, ITEM_INFO* l);
int Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angAdd);
int MoveLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* l);
int TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, int radius);
void CreatureCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);

void Inject_Collide();