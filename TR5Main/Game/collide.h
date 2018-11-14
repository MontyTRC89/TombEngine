#pragma once

#include "..\Global\global.h"

#define ObjectCollision ((void (__cdecl*)(__int16, ITEM_INFO*, COLL_INFO*)) 0x004126E0)
#define GetCollisionInfo ((void (__cdecl*)(COLL_INFO*, __int32, __int32, __int32, __int16, __int32)) 0x00411100)
#define GenericSphereBoxCollision ((void (__cdecl*)(__int16, ITEM_INFO*, COLL_INFO*)) 0x00413A90)
#define LaraBaddieCollision ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00412170)
#define TestBoundsCollide ((__int32 (__cdecl*)(ITEM_INFO*, ITEM_INFO*, __int32)) 0x00412CC0)
#define GetCollidedObjects ((__int32 (__cdecl*)(ITEM_INFO*, __int32, __int32, ITEM_INFO**, MESH_INFO**, __int32)) 0x00413CF0)

void __cdecl j_GetCollisionInfo(COLL_INFO* coll, __int32 x, __int32 y, __int32 z, __int16 roomNumber, __int32 objHeight);
__int32 __cdecl CollideStaticObjects(COLL_INFO* coll, __int32 x, __int32 y, __int32 z, __int16 roomNumber, __int32 hite);

void Inject_Collide();