#pragma once
#include "..\Global\global.h"

#define lara_col_climbstnc ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00450E20)

short __cdecl GetClimbTrigger(int x, int y, int z, short roomNumber);
void __cdecl lara_col_climbend(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_climbend(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_climbdown(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_climbdown(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_climbing(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_climbing(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_climbright(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_climbright(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_climbleft(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_climbleft(ITEM_INFO* item, COLL_INFO* coll);
//void __cdecl lara_col_climbstnc(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_climbstnc(ITEM_INFO* item, COLL_INFO* coll);

int __cdecl LaraTestClimbPos(ITEM_INFO* item, int front, int right, int origin, int height, int* shift);
void __cdecl LaraDoClimbLeftRight(ITEM_INFO* item, COLL_INFO* coll, int result, int shift);
int __cdecl LaraClimbRightCornerTest(ITEM_INFO* item, COLL_INFO* coll);
int __cdecl LaraClimbLeftCornerTest(ITEM_INFO* item, COLL_INFO* coll);
int __cdecl LaraTestClimb(int x, int y, int z, int xFront, int zFront, int itemHeight, int itemRoom, int* shift);
int __cdecl LaraTestClimbUpPos(ITEM_INFO* item, int front, int right, int* shift, int* ledge);
int __cdecl LaraCheckForLetGo(ITEM_INFO* item, COLL_INFO* coll);
