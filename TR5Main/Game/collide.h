#pragma once

#include "..\Global\global.h"

// used by coll->badPos
#define NO_BAD_POS (-NO_HEIGHT)
// used by coll->badNeg
#define NO_BAD_NEG NO_HEIGHT

#define ObjectCollision ((void (__cdecl*)(short, ITEM_INFO*, COLL_INFO*)) 0x004126E0)
#define GetCollisionInfo ((void (__cdecl*)(COLL_INFO*, int, int, int, short, int)) 0x00411100)
#define GenericSphereBoxCollision ((void (__cdecl*)(short, ITEM_INFO*, COLL_INFO*)) 0x00413A90)
#define LaraBaddieCollision ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00412170)
#define TestBoundsCollide ((int (__cdecl*)(ITEM_INFO*, ITEM_INFO*, int)) 0x00412CC0)
//#define GetCollidedObjects ((int (__cdecl*)(ITEM_INFO*, int, int, ITEM_INFO**, MESH_INFO**, int)) 0x00413CF0)
#define ItemPushLara ((int (__cdecl*)(ITEM_INFO*, ITEM_INFO*, COLL_INFO*, int, int)) 0x00412860)
#define TestCollision ((int (__cdecl*)(ITEM_INFO*, ITEM_INFO*)) 0x00479170)
#define TestLaraPosition ((int (__cdecl*)(short*, ITEM_INFO*, ITEM_INFO*)) 0x00413210)
#define MoveLaraPosition ((int (__cdecl*)(PHD_VECTOR*, ITEM_INFO*, ITEM_INFO*)) 0x00413840)
#define AlignLaraPosition ((int (__cdecl*)(PHD_VECTOR*, ITEM_INFO*, ITEM_INFO*)) 0x004133C0)

int CollideStaticObjects(COLL_INFO* coll, int x, int y, int z, short roomNumber, int hite);
int GetCollidedObjects(ITEM_INFO* collidingItem, int radius, int flag1, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int flag2);
int TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll);
void TrapCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll);
void ShiftItem(ITEM_INFO* item, COLL_INFO* coll);

void Inject_Collide();