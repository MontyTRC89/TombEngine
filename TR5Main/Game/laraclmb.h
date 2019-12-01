#pragma once
#include "..\Global\global.h"

#define lara_col_climbstnc ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00450E20)

__int16 __cdecl GetClimbTrigger(__int32 x, __int32 y, __int32 z, __int16 roomNumber);
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

__int32 __cdecl LaraTestClimbPos(ITEM_INFO* item, __int32 front, __int32 right, __int32 origin, __int32 height, __int32* shift);
void __cdecl LaraDoClimbLeftRight(ITEM_INFO* item, COLL_INFO* coll, __int32 result, __int32 shift);
__int32 __cdecl LaraClimbRightCornerTest(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl LaraClimbLeftCornerTest(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl LaraTestClimb(__int32 x, __int32 y, __int32 z, __int32 xFront, __int32 zFront, __int32 itemHeight, __int32 itemRoom, __int32* shift);
__int32 __cdecl LaraTestClimbUpPos(ITEM_INFO* item, int front, int right, int* shift, int* ledge);
__int32 __cdecl LaraCheckForLetGo(ITEM_INFO* item, COLL_INFO* coll);
