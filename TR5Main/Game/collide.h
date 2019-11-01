#pragma once

#include "..\Global\global.h"

#define ObjectCollision ((void (__cdecl*)(__int16, ITEM_INFO*, COLL_INFO*)) 0x004126E0)
#define GetCollisionInfo ((void (__cdecl*)(COLL_INFO*, __int32, __int32, __int32, __int16, __int32)) 0x00411100)
#define GenericSphereBoxCollision ((void (__cdecl*)(__int16, ITEM_INFO*, COLL_INFO*)) 0x00413A90)
#define LaraBaddieCollision ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00412170)
#define TestBoundsCollide ((__int32 (__cdecl*)(ITEM_INFO*, ITEM_INFO*, __int32)) 0x00412CC0)
//#define GetCollidedObjects ((__int32 (__cdecl*)(ITEM_INFO*, __int32, __int32, ITEM_INFO**, MESH_INFO**, __int32)) 0x00413CF0)
#define ItemPushLara ((__int32 (__cdecl*)(ITEM_INFO*, ITEM_INFO*, COLL_INFO*, __int32, __int32)) 0x00412860)
#define TestCollision ((__int32 (__cdecl*)(ITEM_INFO*, ITEM_INFO*)) 0x00479170)
#define TestLaraPosition ((__int32 (__cdecl*)(__int16*, ITEM_INFO*, ITEM_INFO*)) 0x00413210)
#define MoveLaraPosition ((__int32 (__cdecl*)(PHD_VECTOR*, ITEM_INFO*, ITEM_INFO*)) 0x00413840)
#define AlignLaraPosition ((__int32 (__cdecl*)(PHD_VECTOR*, ITEM_INFO*, ITEM_INFO*)) 0x004133C0)

__int32 __cdecl CollideStaticObjects(COLL_INFO* coll, __int32 x, __int32 y, __int32 z, __int16 roomNumber, __int32 hite);
__int32 __cdecl GetCollidedObjects(ITEM_INFO* collidingItem, __int32 radius, __int32 flag1, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, __int32 flag2);
__int32 __cdecl TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll);
void __cdecl TrapCollision(__int16 itemNumber, ITEM_INFO* l, COLL_INFO* c);

void Inject_Collide();